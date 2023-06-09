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

	class GTL__SHAPE_CLASS xSpline : public xShape {
	public:
		using base_t = xShape;
		using this_t = xSpline;

	public:
		enum class eFLAG { closed = 1, periodic = 2, rational = 4, planar = 8, linear = planar|16 };
		int m_flags{};
		point_t m_ptNormal;
		point_t m_vStart, m_vEnd;
		int m_degree{};

		std::vector<double> m_knots;
		std::vector<point_t> m_ptsControl;
		std::vector<point_t> m_ptsFit;

		double m_toleranceKnot{0.0000001};
		double m_toleranceControlPoint{0.0000001};
		double m_toleranceFitPoint{0.0000001};

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_flags					== B.m_flags )
				and ( m_ptNormal				== B.m_ptNormal )
				and ( m_vStart					== B.m_vStart )
				and ( m_vEnd					== B.m_vEnd )
				and ( m_degree					== B.m_degree )

				and ( m_knots					== B.m_knots )
				and ( m_ptsControl				== B.m_ptsControl )
				and ( m_ptsFit					== B.m_ptsFit )

				and ( m_toleranceKnot			== B.m_toleranceKnot )
				and ( m_toleranceControlPoint	== B.m_toleranceControlPoint )
				and ( m_toleranceFitPoint		== B.m_toleranceFitPoint )
			;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::spline; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			if (m_ptsControl.empty())
				return {};
			return std::pair{ m_ptsControl.front(), m_ptsControl.back() };
		}
		virtual void FlipX() override { for (auto& pt : m_ptsControl) pt.x = -pt.x; for (auto& pt : m_ptsFit) pt.x = -pt.x; m_ptNormal.x = -m_ptNormal.x; m_vStart.x = -m_vStart.x; m_vEnd.x = -m_vEnd.x; }
		virtual void FlipY() override { for (auto& pt : m_ptsControl) pt.y = -pt.y; for (auto& pt : m_ptsFit) pt.y = -pt.y; m_ptNormal.y = -m_ptNormal.y; m_vStart.y = -m_vStart.y; m_vEnd.y = -m_vEnd.y; }
		virtual void FlipZ() override { for (auto& pt : m_ptsControl) pt.z = -pt.z; for (auto& pt : m_ptsFit) pt.z = -pt.z; m_ptNormal.z = -m_ptNormal.z; m_vStart.z = -m_vStart.z; m_vEnd.z = -m_vEnd.z; }
		virtual void Reverse() override {
			std::swap(m_vStart, m_vEnd);
			std::ranges::reverse(m_knots);
			std::ranges::reverse(m_ptsControl);
			std::ranges::reverse(m_ptsFit);
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_ptNormal = ct(m_ptNormal);
			m_vStart = ct(m_vStart);
			m_vEnd = ct(m_vEnd);
			for (auto& pt : m_ptsControl)
				pt = ct(pt);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool b{};
			for (auto& pt : m_ptsControl)
				b = rect.UpdateBoundary(pt);
			return b;
		};
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Spline(m_degree, m_ptsControl, m_knots, false);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tflags:{}, degree:{}\n", m_flags, m_degree);
			fmt::print(os, L"\tknot ");
			for (auto knot : m_knots)
				fmt::print(os, L"{}, ", knot);
			fmt::print(os, L"\n\tcontrol points ");
			for (auto pt : m_ptsControl)
				fmt::print(os, L"({},{},{}), ", pt.x, pt.y, pt.z);
			fmt::print(os, L"\n");
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xSpline);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xSpline, xShape);
		//GTL__REFLECTION_MEMBERS(flags, ptNormal, vStart, vEnd, degree, knots, ptsControl, toleranceKnot, toleranceControlPoint, toleranceFitPoint);
		auto operator <=> (xSpline const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xSpline& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xSpline& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_flags;
			ar & var.m_ptNormal;
			ar & var.m_vStart & var.m_vEnd;
			ar & var.m_degree;

			ar & var.m_knots;
			ar & var.m_ptsControl;

			ar & var.m_toleranceKnot;
			ar & var.m_toleranceControlPoint;
			ar & var.m_toleranceFitPoint;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_ptNormal = PointFrom(j["normalVec"sv]);
			m_vStart = PointFrom(j["tgStart"sv]);
			m_vEnd = PointFrom(j["tgEnd"sv]);
			m_flags = j["flags"sv];
			m_degree = j["degree"sv];

			m_toleranceKnot = j["tolknot"sv];
			m_toleranceControlPoint = j["tolcontrol"sv];
			m_toleranceFitPoint = j["tolfit"sv];

			for (auto& v : j["knotslist"sv].json().as_array()) {
				m_knots.push_back(v.as_double());
			}
			for (auto& v : j["controllist"sv].json().as_array()) {
				m_ptsControl.push_back(PointFrom(v));
			}
			for (auto& v : j["fitlist"sv].json().as_array()) {
				m_ptsFit.push_back(PointFrom(v));
			}

			return true;
		}

	};

#pragma pack(pop)
}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xSpline, "spline")
