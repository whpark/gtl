//////////////////////////////////////////////////////////////////////
//
// shape_others.h:
//
// PWH
// 2017.07.20
// 2021.05.27
// 2021.08.11 Compare
// 2023-06-09 분리
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../shape_primitives.h"
#include "../canvas.h"

//export module shape;

namespace gtl::shape {
#pragma pack(push, 8)

	class GTL__SHAPE_CLASS xPolyline : public xShape {
	public:
		using base_t = xShape;
		using this_t = xPolyline;

	public:
		bool m_bLoop{};
		std::vector<polypoint_t> m_pts;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_bLoop == B.m_bLoop) and (m_pts == B.m_pts);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::polyline; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			if (m_pts.empty())
				return {};
			return std::pair{ m_pts.front(), m_bLoop ? m_pts.front() : m_pts.back() };
		}
		virtual void FlipX() override { for (auto& pt : m_pts) { pt.x = -pt.x;  pt.Bulge() = -pt.Bulge(); } }
		virtual void FlipY() override { for (auto& pt : m_pts) { pt.y = -pt.y;  pt.Bulge() = -pt.Bulge(); } }
		virtual void FlipZ() override { for (auto& pt : m_pts) { pt.z = -pt.z;  pt.Bulge() = -pt.Bulge(); } }
		virtual void Reverse() override {
			if (m_bLoop) {
				if (m_pts.size()) {
					auto pt = m_pts.front();
					m_pts.erase(m_pts.begin());
					m_pts.push_back(pt);
				}
			}

			std::ranges::reverse(m_pts);

			if (m_pts.size() > 1) {
				auto b0 = m_pts.front().Bulge();
				for (size_t i{1}; i < m_pts.size(); i++) {
					m_pts[i-1].Bulge() = - m_pts[i].Bulge();
				}
				m_pts.back().Bulge() = - b0;

			}
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& pt : m_pts) {
				(point_t&)pt = ct((point_t&)pt);
			}
			if (!bRightHanded) {
				for (auto& pt : m_pts) { pt.x = -pt.x;  pt.Bulge() = -pt.Bulge(); } 
			}
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bModified{};

			auto nPt = m_pts.size();
			if (!m_bLoop)
				nPt--;
			for (int iPt = 0; iPt < nPt; iPt++) {
				auto pt0 = m_pts[iPt];
				bModified |= rectBoundary.UpdateBoundary(pt0);
				if (pt0.Bulge() != 0.0) {
					auto iPt2 = (iPt+1) % m_pts.size();
					auto pt1 = m_pts[iPt2];
					xArc arc = xArc::GetFromBulge(pt0.Bulge(), pt0, pt1);
					bModified |= arc.UpdateBoundary(rectBoundary);
				}
			}

			return bModified;
		};
		virtual void Draw(ICanvas& canvas) const override;
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\t{}", m_bLoop ? L"loop ":L"");
			for (auto const& pt : m_pts)
				fmt::print(os, L"({},{},{},{}), ", pt.x, pt.y, pt.z, pt.w);
			fmt::print(os, L"\n");
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xPolyline);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xPolyline, xShape);
		//GTL__REFLECTION_MEMBERS(m_pts);
		auto operator <=> (xPolyline const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xPolyline& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xPolyline& var) {
			ar & boost::serialization::base_object<xShape>(var);
			ar & var.m_pts;
			ar & var.m_bLoop;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			auto jpts = j["vertlist"sv];
			auto apts = jpts.json().as_array();
			for (auto iter = apts.begin(); iter != apts.end(); iter++) {
				m_pts.push_back(polypoint_t{});
				m_pts.back() = PolyPointFromVertex(*iter);
			}

			m_bLoop = (j["flags"].value_or(0) & 1) != 0;
			return true;
		}

		boost::ptr_deque<xShape> Split() const;

		template < typename t_iterator >
		static std::unique_ptr<xPolyline> MergeShapeAsPolyline(t_iterator begin, t_iterator end, double dThreshold) {
			if (begin == end)
				return {};
			auto rPolyline = std::make_unique<xPolyline>();
			rPolyline->m_color = begin->m_color;
			rPolyline->m_bLoop = false;
			auto& pts = rPolyline->m_pts;
			if (auto ptSE = begin->GetStartEndPoint()) {
				pts.push_back(polypoint_t(ptSE->first));
			} else {
				return {};
			}

			for (auto pos = begin; pos != end; pos++) {
				xShape const& shape = *pos;
				auto const& ptBack = pts.back();
				if (shape.GetShapeType() == eSHAPE::line) {
					auto const& line = (xLine const&)shape;
					if (ptBack.Distance(line.m_pt0) > dThreshold) {
						continue;
					}
					pts.push_back(polypoint_t(line.m_pt1));
				} else if (shape.GetShapeType() == eSHAPE::arc_xy) {
					auto const& arc = (xArc const&)shape;
					auto ptC = arc.GetStartEndPoint();
					if (!ptC)
						continue;
					auto pt0 = ptC->first;
					auto pt1 = ptC->second;
					if (ptBack.Distance(pt0) > dThreshold) {
						continue;
					}
					pts.back().Bulge() = gtl::rad_t(arc.m_angle_length)/4.;
					pts.push_back(polypoint_t(pt1));
				}
				else {
					return {};	// if the source object includes not an line nor arc, cannot be reduced to polyline.
				}
			}
			if ( (pts.size() > 1) and xPoint3d(pts.front()) == xPoint3d(pts.back())) {
				pts.pop_back();
				rPolyline->m_bLoop = true;
			}
			return rPolyline;
		}
	};

	class GTL__SHAPE_CLASS xPolylineLW : public xPolyline {
	public:
		using base_t = xPolyline;
		using this_t = xPolylineLW;

	public:
		//protected:
		//	int dummy{};
		//public:

		//virtual bool Compare(xShape const& B_) const override {
		//	if (!base_t::Compare(B_))
		//		return false;
		//	return true;
		//}
		virtual eSHAPE GetShapeType() const { return eSHAPE::lwpolyline; }

		GTL__DYNAMIC_VIRTUAL_DERIVED(xPolylineLW);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xPolylineLW, xPolyline);
		//GTL__REFLECTION_MEMBERS(dummy);
		auto operator <=> (xPolylineLW const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xPolylineLW& var, unsigned int const file_version) {
			boost::serialization::base_object<xPolyline>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xPolylineLW& var) {
			ar & boost::serialization::base_object<xPolyline>(var);
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			auto jpts = j["vertlist"sv];
			auto apts = jpts.json().as_array();
			for (auto iter = apts.begin(); iter != apts.end(); iter++) {
				m_pts.push_back(polypoint_t{});
				m_pts.back() = PolyPointFrom(*iter);
			}

			m_bLoop = (j["flags"].value_or(0) & 1) != 0;
			return true;
		}
	};



#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolyline, "polyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolylineLW, "lwpolyline")
