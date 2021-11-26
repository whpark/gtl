//////////////////////////////////////////////////////////////////////
//
// shape_others.h:
//
// PWH
// 2017.07.20
// 2021.05.27
// 2021.08.11 Compare
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*

	1. CShapeObject 에서 변경 내용 :

		class name :

			CShapeObject	-> xShape
			CShapeLine		-> xLine
			CShapePolyLine	-> xPolyline
			...

		Container :

			TList -> boost::ptr_deque

		Point / Line : x, y, z 3차원으로 변경

			typedef xSize3d					size2d_t;
			typedef xPoint3d				point_t;
			typedef xRect3d					rect_t;
			typedef std::vector<xLine>		s_lines;

*/
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/iconv_wrapper.h"

#include "shape_primitives.h"
#include "canvas.h"

//export module shape;

namespace gtl::shape {
#pragma pack(push, 8)


	class GTL__SHAPE_CLASS xLayer : public xShape {
	public:
		using base_t = xShape;
		using this_t = xLayer;

	public:
		string_t m_name;
		bool m_bUse{true};
		int m_flags{};
		boost::ptr_deque<xShape> m_shapes;

		xLayer() = default;
		xLayer(xLayer const&) = default;
		xLayer(xLayer&&) = default;
		xLayer(string_t const& name) : m_name(name) {}

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and (m_name == B.m_name)
				and (m_bUse == B.m_bUse)
				and (m_flags == B.m_flags)
				and (m_shapes == B.m_shapes);
		}

		virtual eSHAPE GetShapeType() const { return eSHAPE::layer; }
		//virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			if (m_shapes.empty())
				return{};
			auto r0 = m_shapes.front().GetStartEndPoint();
			if (!r0)
				return {};
			auto r1 = m_shapes.back().GetStartEndPoint();
			if (!r1)
				return {};
			return std::pair{r0->first, r1->second};
		}
		virtual void FlipX() override { for (auto& shape : m_shapes) shape.FlipX(); }
		virtual void FlipY() override { for (auto& shape : m_shapes) shape.FlipY(); }
		virtual void FlipZ() override { for (auto& shape : m_shapes) shape.FlipZ(); }
		virtual void Reverse() override {
			std::ranges::reverse(m_shapes.base());
			for (auto& shape : m_shapes) {
				shape.Reverse();
			}
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& shape : m_shapes)
				shape.Transform(ct, bRightHanded);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool r{};
			for (auto& shape : m_shapes)
				r |= shape.UpdateBoundary(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			for (auto& shape : m_shapes) {
				shape.Draw(canvas);
			}
		}
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const override {
			bool result{};
			for (auto& shape : m_shapes) {
				result |= shape.DrawROI(canvas, rectROI);
			}
			return result;
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			for (auto& shape : m_shapes) {
				shape.PrintOut(os);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xLayer);
		//GTL__REFLECTION_DERIVED(xLayer, xShape);
		//GTL__REFLECTION_MEMBERS(name, flags, m_strLineType, m_lineWeight, shapes);
		auto operator <=> (xLayer const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xLayer& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xLayer& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_name & var.m_bUse &var.m_flags;
			ar & var.m_strLineType & var.m_lineWeight;	// duplicated.... hot to delete ?
			ar & var.m_shapes;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			m_name = j["name"];	// not a "layer"
			m_flags = j["flags"];
			m_bUse = j["plotF"];

			return true;
		}

		void clear() {
			m_shapes.clear();
			m_strLineType.clear();
			m_flags = {};
			m_bUse = true;
			m_name.clear();
		}

		virtual void Sort_Loop();
		bool IsLoop(double dMinGap = 1.e-3) const;

		inline static bool RemoveConnectedComponents(std::deque<xShape const*>& shapes, xShape const* pSeed, double const dMinDistance = 0.000'1) {
			if (!pSeed)
				return false;

			std::deque<xShape const*> lst_found;
			lst_found.push_back(pSeed);

			while (lst_found.size()) {

				auto* p0 = lst_found.front();
				lst_found.pop_front();

				auto r = p0->GetStartEndPoint();
				if (!r)
					continue;
				auto [pt0, pt1] = *r;

				for (size_t i{}; i < shapes.size(); i++) {
					auto const* pNext = shapes[i];
					auto pts = pNext->GetStartEndPoint();
					if (!pts)
						continue;
					if ((pt0.Distance(pts->first) <= dMinDistance) or (pt0.Distance(pts->second) <= dMinDistance) or
						(pt1.Distance(pts->first) <= dMinDistance) or (pt1.Distance(pts->second) <= dMinDistance)) {
						lst_found.push_back(pNext);
						shapes.erase(shapes.begin()+i--);
					}
				}

				if (shapes.empty())
					break;
			}

			return true;
		}

	protected:
		friend class xDrawing;
		line_type_t* pLineType{};

	};

	// internally
	class GTL__SHAPE_CLASS xBlock : public xLayer {
	public:
		using base_t = xLayer;
		using this_t = xBlock;

	public:
		string_t m_layer;
		point_t m_pt;

	public:
		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and (m_layer == B.m_layer)
				and (m_pt == B.m_pt)
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::block; }

		GTL__DYNAMIC_VIRTUAL_DERIVED(xBlock);
		auto operator <=> (xBlock const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xBlock& var, unsigned int const file_version) {
			boost::serialization::base_object<xLayer>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xBlock& var) {
			ar & boost::serialization::base_object<xLayer>(var);

			ar & var.m_layer & var.m_pt;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			this->m_layer = m_layer;
			using namespace std::literals;
			gtl::bjson j(_j);

			m_name = j["name"];
			m_flags = j["flags"];
			m_pt = PointFrom(j["basePoint"]);

			return true;
		}
	};

	class GTL__SHAPE_CLASS xDot : public xShape {
	public:
		using base_t = xShape;
		using this_t = xDot;

	public:
		point_t m_pt;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_pt == B.m_pt);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::dot; }

		//virtual point_t PointAt(double t) const override { return pt; };
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			return std::pair{ m_pt, m_pt };
		}
		virtual void FlipX() override { m_pt.x = -m_pt.x; }
		virtual void FlipY() override { m_pt.y = -m_pt.y; }
		virtual void FlipZ() override { m_pt.z = -m_pt.z; }
		virtual void Reverse() override {
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_pt = ct(m_pt);
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			return rectBoundary.UpdateBoundary(m_pt);
		};
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.MoveTo(m_pt);
			canvas.LineTo(m_pt);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tpt({},{},{})\n", m_pt.x, m_pt.y, m_pt.z);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xDot);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xDot, xShape);
		//GTL__REFLECTION_MEMBERS(pt);
		auto operator <=> (xDot const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xDot& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xDot& var) {
			ar & boost::serialization::base_object<xShape>(var);
			ar & var.m_pt;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			m_pt = PointFrom(j["basePoint"sv]);
			return true;
		}

	};

	class GTL__SHAPE_CLASS xLine : public xShape {
	public:
		using base_t = xShape;
		using this_t = xLine;

	public:
		point_t m_pt0, m_pt1;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_pt0 == B.m_pt0) and (m_pt1 == B.m_pt1);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::line; }

		//virtual point_t PointAt(double t) const override { return lerp(m_pt0, m_pt1, t); }
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			return std::pair{ m_pt0, m_pt1 };
		}
		virtual void FlipX() override { m_pt0.x = -m_pt0.x; m_pt1.x = -m_pt1.x; }
		virtual void FlipY() override { m_pt0.y = -m_pt0.y; m_pt1.y = -m_pt1.y; }
		virtual void FlipZ() override { m_pt0.z = -m_pt0.z; m_pt1.z = -m_pt1.z; }
		virtual void Reverse() override {
			std::swap(m_pt0, m_pt1);
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_pt0 = ct(m_pt0); m_pt1 = ct(m_pt1);
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bModified{};
			bModified |= rectBoundary.UpdateBoundary(m_pt0);
			bModified |= rectBoundary.UpdateBoundary(m_pt1);
			return bModified;
		};
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Line(m_pt0, m_pt1);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tpt0({},{},{}) - pt1({},{},{})\n", m_pt0.x, m_pt0.y, m_pt0.z, m_pt1.x, m_pt1.y, m_pt1.z);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xLine);
		auto operator <=> (xLine const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xLine& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xLine& var) {
			ar & boost::serialization::base_object<xShape>(var);
			return ar & var.m_pt0 & var.m_pt1;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			m_pt0 = PointFrom(j["basePoint"sv]);
			m_pt1 = PointFrom(j["secPoint"sv]);
			return true;
		}
	};

	class GTL__SHAPE_CLASS xCircle : public xShape {
	public:
		using base_t = xShape;
		using this_t = xCircle;

	public:
		point_t m_ptCenter;
		double m_radius{};
		deg_t m_angle_length{360_deg};	// 회전 방향.

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_ptCenter == B.m_ptCenter) and (m_radius == B.m_radius) and (m_angle_length == B.m_angle_length);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::circle_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			auto pt = m_ptCenter+point_t{m_radius,0,0};
			return std::pair{ pt, pt };
		}
		virtual void FlipX() override { m_ptCenter.x = -m_ptCenter.x; m_angle_length = -m_angle_length; }
		virtual void FlipY() override { m_ptCenter.y = -m_ptCenter.y; m_angle_length = -m_angle_length; }
		virtual void FlipZ() override { m_ptCenter.z = -m_ptCenter.z; m_angle_length = -m_angle_length; }
		virtual void Reverse() override {
			m_angle_length = -m_angle_length;
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_ptCenter = ct(m_ptCenter); m_radius = ct.Trans(m_radius);
			if (!bRightHanded)
				m_angle_length = -m_angle_length;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bResult{};
			bResult |= rectBoundary.UpdateBoundary(point_t(m_ptCenter.x-m_radius, m_ptCenter.y-m_radius, m_ptCenter.z));
			bResult |= rectBoundary.UpdateBoundary(point_t(m_ptCenter.x+m_radius, m_ptCenter.y+m_radius, m_ptCenter.z));
			return bResult;
		};
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Arc(m_ptCenter, m_radius, 0._deg, m_angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tcenter({},{},{}), r {}\n", m_ptCenter.x, m_ptCenter.y, m_ptCenter.z, m_radius);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xCircle);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xCircle, xShape);
		//GTL__REFLECTION_MEMBERS(m_ptCenter, m_radius, m_angle_length);
		auto operator <=> (xCircle const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xCircle& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xCircle& var) {
			ar & boost::serialization::base_object<xShape>(var);
			return ar & var.m_ptCenter & var.m_radius & (double&)var.m_angle_length;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_ptCenter = PointFrom(j["basePoint"sv]);
			m_radius = j["radious"sv];
			return true;
		}
	};

	class GTL__SHAPE_CLASS xArc : public xCircle {
	public:
		using base_t = xCircle;
		using this_t = xArc;

	public:
		deg_t m_angle_start{};

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return m_angle_start == B.m_angle_start;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::arc_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			auto c = gtl::cos(m_angle_start);
			auto s = gtl::sin(m_angle_start);
			auto pt0 = m_ptCenter + m_radius*point_t{c, s};
			c = gtl::cos(m_angle_start + m_angle_length);
			s = gtl::sin(m_angle_start + m_angle_length);
			auto pt1 = m_ptCenter + m_radius*point_t{c, s};
			return std::pair{ pt0, pt1 };
		}
		virtual void FlipX() override { xCircle::FlipX(); m_angle_start = AdjustAngle(180._deg - m_angle_start); }
		virtual void FlipY() override { xCircle::FlipY(); m_angle_start = AdjustAngle(-m_angle_start); }
		virtual void FlipZ() override { xCircle::FlipZ(); m_angle_start = AdjustAngle(180._deg - m_angle_start); }	// ????.....  성립 안되지만,
		virtual void Reverse() override {
			m_angle_start = m_angle_start+m_angle_length;
			m_angle_length = -m_angle_length;
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			// todo : ... upgrade?
			xCircle::Transform(ct, bRightHanded);
			if (!bRightHanded)
				m_angle_start = -m_angle_start;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			// todo : ... upgrade?
			return xCircle::UpdateBoundary(rectBoundary);
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Arc(m_ptCenter, m_radius, m_angle_start, m_angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xCircle::PrintOut(os);
			fmt::print(os, L"\tangle_start:{} deg, length:{} deg\n", (double)(deg_t)m_angle_start, (double)(deg_t)m_angle_length);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xArc);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xArc, xCircle);
		//GTL__REFLECTION_MEMBERS(m_angle_start);
		auto operator <=> (xArc const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xArc& var, unsigned int const file_version) {
			boost::serialization::base_object<xCircle>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xArc& var) {
			ar & boost::serialization::base_object<xCircle>(var);
			return ar & var.m_angle_start;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xCircle::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_angle_start = deg_t{(double)j["staangle"sv]};
			bool bCCW = j["isccw"sv].value_or(0) != 0;
			deg_t angle_end { (double)j["endangle"sv] };
			m_angle_length = angle_end - m_angle_start;
			if (bCCW) {
				if (m_angle_length < 0_deg)
					m_angle_length += 360_deg;
			} else {
				if (m_angle_length > 0_deg)
					m_angle_length -= 360_deg;
			}
			return true;
		}

		deg_t AdjustAngle(deg_t angle) {
			//if ( (angle < 0) || (angle > 360.) ) {
			//	int r = angle / 360.;
			//	angle = angle - 360. * r;
			//}
			//while (angle < 0)
			//	angle += 360.;
			angle = std::fmod(angle, 360.);
			if (angle < 0.)
				angle += 360.;
			return angle;
		}

		static xArc GetFromBulge(double bulge, point_t const& pt0, point_t const& pt1) {
			xArc arc;
			xPoint2d vecPerpendicular(-(pt0.y-pt1.y), (pt0.x-pt1.x));
			// Normalize
			{
				double d = vecPerpendicular.Distance(xPoint2d(0.0, 0.0));
				vecPerpendicular.x /= d;
				vecPerpendicular.y /= d;
			}
			xPoint2d ptCenterOfLine((pt0.x+pt1.x)/2., (pt0.y+pt1.y)/2.);
			double l = pt1.Distance(pt0)/2.;
			xPoint2d ptBulge;
			ptBulge.x = ptCenterOfLine.x + vecPerpendicular.x * (bulge * l);
			ptBulge.y = ptCenterOfLine.y + vecPerpendicular.y * (bulge * l);
			double h = ptBulge.Distance(ptCenterOfLine);
			arc.m_radius = (Square(l) + Square(h)) / (2 * h);

			arc.m_ptCenter.x = ptBulge.x + (arc.m_radius / h) * (ptCenterOfLine.x - ptBulge.x);
			arc.m_ptCenter.y = ptBulge.y + (arc.m_radius / h) * (ptCenterOfLine.y - ptBulge.y);
			arc.m_angle_start = rad_t::atan2(pt0.y - arc.m_ptCenter.y, pt0.x - arc.m_ptCenter.x);
			rad_t dT1 = rad_t::atan2(pt1.y - arc.m_ptCenter.y, pt1.x - arc.m_ptCenter.x);
			//arc.m_eDirection = (dBulge > 0) ? 1 : -1;
			//arc.m_dTLength = (dBulge > 0) ? fabs(dT1-arc.m_dT0) : -fabs(dT1-arc.m_dT0);
			if (bulge > 0) {
				while (dT1 < arc.m_angle_start)
					dT1 += rad_t(std::numbers::pi*2);
				arc.m_angle_length = dT1 - arc.m_angle_start;
			} else {
				while (dT1 > arc.m_angle_start)
					dT1 -= rad_t(std::numbers::pi*2);
				arc.m_angle_length = dT1 - arc.m_angle_start;
			}

			return arc;
		}

	};

	class GTL__SHAPE_CLASS xEllipse : public xArc {
	public:
		using base_t = xArc;
		using this_t = xEllipse;

	public:
		double m_radiusH{};
		deg_t m_angle_first_axis{};

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return (m_radiusH == B.m_radiusH) and (m_angle_first_axis == B.m_angle_first_axis);
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::ellipse_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			xCoordTrans2d ct;
			ct.Init(1.0, (rad_t)m_angle_first_axis, point_t{}, m_ptCenter);

			deg_t t0 = m_angle_start;
			deg_t t1 = m_angle_start + m_angle_length;
			auto radius1 = m_radius;
			auto radius2 = m_radiusH;
			point_t pt0 = ct(point_t{radius1*cos(t0), radius2*sin(t0)});
			point_t pt1 = ct(point_t{radius1*cos(t1), radius2*sin(t1)});

			return std::pair{ pt0, pt1 };
		}
		virtual void FlipX() override { xArc::FlipX(); m_angle_first_axis = 180._deg - m_angle_first_axis; }
		virtual void FlipY() override { xArc::FlipY(); m_angle_first_axis = - m_angle_first_axis; }
		virtual void FlipZ() override { xArc::FlipZ(); m_angle_first_axis = 180._deg - m_angle_first_axis; }
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			xArc::Transform(ct, bRightHanded);
			m_radiusH = ct.Trans(m_radiusH);
			if (!bRightHanded)
				m_angle_first_axis = -m_angle_first_axis;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			// todo : ... upgrade
			bool bResult{};
			bResult |= rectBoundary.UpdateBoundary(point_t(m_ptCenter.x-m_radius, m_ptCenter.y-m_radiusH, m_ptCenter.z));
			bResult |= rectBoundary.UpdateBoundary(point_t(m_ptCenter.x+m_radius, m_ptCenter.y+m_radiusH, m_ptCenter.z));
			return bResult;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Ellipse(m_ptCenter, m_radius, m_radiusH, m_angle_first_axis, m_angle_start, m_angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			xArc::PrintOut(os);
			fmt::print(os, L"\tradiusH:{}, angle_first_axis:{} deg\n", m_radiusH, (double)(deg_t)m_angle_first_axis);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xEllipse);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xEllipse, xArc);
		//GTL__REFLECTION_MEMBERS(m_radiusH, m_angle_first_axis);
		auto operator <=> (xEllipse const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xEllipse& var, unsigned int const file_version) {
			boost::serialization::base_object<xArc>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xEllipse& var) {
			ar & boost::serialization::base_object<xArc>(var);
			return ar & var.m_radiusH & (double&)var.m_angle_first_axis;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_ptCenter = PointFrom(j["basePoint"sv]);
			point_t firstAxis = PointFrom(j["secPoint"sv]);
			m_angle_first_axis = deg_t::atan2(firstAxis.y, firstAxis.x);
			double ratio = j["ratio"sv];
			m_radius = firstAxis.GetLength();
			m_radiusH = ratio * m_radius;

			rad_t t0 {(double)j["staparam"sv]};
			rad_t t1 {(double)j["endparam"sv]};
			bool bCCW = j["isccw"sv];

			m_angle_start = t0;
			m_angle_length = t1-t0;

			if (bCCW) {
				if (m_angle_length < 0_deg)
					m_angle_length += 360_deg;
			} else {
				if (m_angle_length > 0_deg)
					m_angle_length -= 360_deg;
			}

			return true;
		}

	};

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
			for (auto const& pt : m_pts)
				bModified |= rectBoundary.UpdateBoundary(pt);
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

			size_t count{};
			for (auto pos = begin; pos != end; pos++) {
				count++;
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
			fmt::print(os, L"\tflags:{}, degree:{}\n");
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

	class GTL__SHAPE_CLASS xText : public xShape {
	public:
		using base_t = xShape;
		using this_t = xText;

	public:
		enum class eALIGN_VERT : int { base_line = 0, bottom, mid, top };
		enum class eALIGN_HORZ : int { left = 0, center, right, aligned, middle, fit };

		point_t m_pt0, m_pt1;
		string_t m_text;
		double m_height{};
		deg_t m_angle{};
		double m_widthScale{};
		deg_t m_oblique{};
		string_t m_textStyle;
		int m_textgen{};
		eALIGN_HORZ m_alignHorz{eALIGN_HORZ::left};
		eALIGN_VERT m_alignVert{eALIGN_VERT::base_line};

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_pt0			== B.m_pt0 )
				and ( m_pt1			== B.m_pt1 )
				and ( m_text		== B.m_text )
				and ( m_height		== B.m_height )
				and ( m_angle		== B.m_angle )
				and ( m_widthScale	== B.m_widthScale )
				and ( m_oblique		== B.m_oblique )
				and ( m_textStyle	== B.m_textStyle )
				and ( m_textgen		== B.m_textgen )
				and ( m_alignHorz	== B.m_alignHorz )
				and ( m_alignVert	== B.m_alignVert )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::text; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			// todo : Start/End Point
			return std::pair{ m_pt0, m_pt1 };
		}
		virtual void FlipX() override { m_pt0.x = -m_pt0.x; m_pt1.x = -m_pt1.x; }
		virtual void FlipY() override { m_pt0.y = -m_pt0.y; m_pt1.y = -m_pt1.y; }
		virtual void FlipZ() override { m_pt0.z = -m_pt0.z; m_pt1.z = -m_pt1.z; }
		virtual void Reverse() override {
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			m_pt0 = ct(m_pt0);
			m_pt1 = ct(m_pt1);
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			// todo : upgrade.
			bool b{};
			b |= rectBoundary.UpdateBoundary(m_pt0);
			//b |= rectBoundary.UpdateBoundary(pt1);
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Text(*this);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xText);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xText, xShape);
		//GTL__REFLECTION_MEMBERS(pt0, pt1, text, height, angle, widthScale, oblique, textStyle, textgen, alignHorz, alignVert);
		auto operator <=> (xText const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xText& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xText& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_pt0 & var.m_pt1;
			ar & var.m_text;
			ar & var.m_height;
			ar & var.m_angle;
			ar & var.m_widthScale;
			ar & var.m_oblique;
			ar & var.m_textStyle;
			ar & var.m_textgen;
			ar & (int&)var.m_alignHorz;
			ar & (int&)var.m_alignVert;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_pt0 = PointFrom(j["basePoint"sv]);
			m_pt1 = PointFrom(j["secPoint"sv]);
			m_text = j["text"sv];
			m_height = j["height"sv];
			m_angle = deg_t{(double)j["angle"sv]};
			m_widthScale = j["widthscale"sv];
			m_oblique = deg_t{(double)j["oblique"sv]};
			m_textStyle = j["style"sv];
			m_textgen = j["textgen"sv];
			m_alignHorz = (eALIGN_HORZ)(int)j["alignH"sv];
			m_alignVert = (eALIGN_VERT)(int)j["alignV"sv];

			return true;
		}
	};

	class GTL__SHAPE_CLASS xMText : public xText {
	public:
		using base_t = xText;
		using this_t = xMText;

	protected:
		using xText::m_alignVert;
	public:
		double m_interlin{};
		enum class eATTACH {
			topLeft = 1,
			topCenter,
			topRight,
			middleLeft,
			middleCenter,
			middleRight,
			bottomLeft,
			bottomCenter,
			bottomRight
		};

		//eATTACH& eAttch{(eATTACH&)alignVert};
		eATTACH GetAttachPoint() const { return (eATTACH)m_alignVert; }
		void SetAttachPoint(eATTACH eAttach) { m_alignVert = (eALIGN_VERT)eAttach; }

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return m_interlin == B.m_interlin;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::mtext; }

		//virtual point_t PointAt(double t) const override {};
		//virtual void FlipX() override {}
		//virtual void FlipY() override {}
		//virtual void FlipY() override {}
		//virtual void Transform(xCoordTrans3d const&, bool bRightHanded) override { return true; };
		//virtual bool UpdateBoundary(rect_t& rectBoundary) const override { return true; };
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			canvas.Text(*this);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xMText);
		auto operator <=> (xMText const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xMText& var, unsigned int const file_version) {
			boost::serialization::base_object<xText>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xMText& var) {
			ar & boost::serialization::base_object<xText>(var);
			return ar & var.m_interlin;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xText::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_interlin = j["interlin"sv];

			return true;
		}
	};

	class GTL__SHAPE_CLASS xHatch : public xShape {
	public:
		using base_t = xShape;
		using this_t = xHatch;

	public:
		string_t m_name;
		bool m_bSolid{};
		bool m_bAssociative{};
		int m_nLoops{};
		int m_hstyle{};
		int m_hpattern{};
		bool m_bDouble{};
		deg_t m_angle{};
		double m_scale{};
		int m_deflines{};

		std::vector<xPolyline> m_boundaries;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_name			== B.m_name )
				and ( m_bSolid			== B.m_bSolid )
				and ( m_bAssociative	== B.m_bAssociative )
				and ( m_nLoops			== B.m_nLoops )
				and ( m_hstyle			== B.m_hstyle )
				and ( m_hpattern		== B.m_hpattern )
				and ( m_bDouble			== B.m_bDouble )
				and ( m_angle			== B.m_angle )
				and ( m_scale			== B.m_scale )
				and ( m_deflines		== B.m_deflines )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::hatch; }

		//virtual point_t PointAt(double t) const override {};
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			return {};
		}
		virtual void FlipX() override { for (auto& b : m_boundaries) b.FlipX(); }
		virtual void FlipY() override { for (auto& b : m_boundaries) b.FlipY(); }
		virtual void FlipZ() override { for (auto& b : m_boundaries) b.FlipZ(); }
		virtual void Reverse() override {
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool b{};
			for (auto const& bound : m_boundaries) {
				b |= bound.UpdateBoundary(rectBoundary);
			}
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			// todo : draw hatch
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xHatch);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xHatch, xShape);
		//GTL__REFLECTION_MEMBERS(name, bSolid, bAssociative, nLoops, hstyle, hpattern, bDouble, angle, scale, deflines);
		auto operator <=> (xHatch const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xHatch& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xHatch& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_name;
			ar & var.m_bSolid;
			ar & var.m_bAssociative;
			ar & var.m_nLoops;
			ar & var.m_hstyle;
			ar & var.m_hpattern;
			ar & var.m_angle;
			ar & var.m_scale;
			ar & var.m_deflines;
			ar & var.m_boundaries;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			xShape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			m_name			= j["name"sv];
			m_bSolid		= (bool)(int)j["solid"sv];
			m_bAssociative	= (bool)(int)j["associative"sv];
			m_hstyle		= j["hstyle"sv];
			m_hpattern		= j["hpattern"sv];
			m_bDouble		= (bool)(int)j["doubleflag"sv];
			m_nLoops		= j["loopsnum"sv];
			m_angle			= rad_t{(double)j["angle"sv]};
			m_scale			= j["scale"sv];
			m_deflines		= j["deflines"sv];

			return true;
		}

	};

	// temporary object
	class GTL__SHAPE_CLASS xInsert : public xShape {
	public:
		using base_t = xShape;
		using this_t = xInsert;

	public:
		string_t m_name;	// block name
		point_t m_pt;
		double m_xscale{1};
		double m_yscale{1};
		double m_zscale{1};
		rad_t m_angle{};
		int m_nCol{1};
		int m_nRow{1};
		double m_spacingCol{};
		double m_spacingRow{};

		GTL__DYNAMIC_VIRTUAL_DERIVED(xInsert);
		//GTL__REFLECTION_VIRTUAL_DERIVED(xInsert, xShape);
		//GTL__REFLECTION_MEMBERS(name, xscale, yscale, zscale, angle, nCol, nRow, spacingCol, spacingRow);
		auto operator <=> (xInsert const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xInsert& var, unsigned int const file_version) {
			boost::serialization::base_object<xShape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xInsert& var) {
			ar & var.m_name & var.m_xscale & var.m_yscale & var.m_zscale & (double&)m_angle;
			ar & var.m_nCol & var.m_nRow & var.m_spacingCol & var.m_spacingRow;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			using namespace std::literals;
			xShape::LoadFromCADJson(_j);
			gtl::bjson j(_j);

			m_pt = PointFrom(j["basePoint"]);
			m_name = j["name"];
			m_xscale = j["xscale"];
			m_yscale = j["yscale"];
			m_zscale = j["zscale"];
			m_angle = rad_t{(double)j["angle"]};
			m_nCol = j["colcount"];
			m_nRow = j["rowcount"];
			m_spacingCol = j["colspace"];
			m_spacingRow = j["rowspace"];

			return true;
		}

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_name		== B.m_name )
				and ( m_pt			== B.m_pt )
				and ( m_xscale		== B.m_xscale )
				and ( m_yscale		== B.m_yscale )
				and ( m_zscale		== B.m_zscale )
				and ( m_angle		== B.m_angle )
				and ( m_nCol		== B.m_nCol )
				and ( m_nRow		== B.m_nRow )
				and ( m_spacingCol	== B.m_spacingCol )
				and ( m_spacingRow	== B.m_spacingRow )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::insert; }

		//virtual point_t PointAt(double t) const = 0;
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override { return {}; }
		virtual void FlipX() override {}
		virtual void FlipY() override {}
		virtual void FlipZ() override {}
		virtual void Reverse() override {}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) override {};
		virtual bool UpdateBoundary(rect_t&) const override { return false; };
		virtual void Draw(ICanvas& canvas) const override {
			//xShape::Draw(canvas);
		};

		virtual void PrintOut(std::wostream& os) const override {
			xShape::PrintOut(os);
			fmt::print(os, L"\tname:{}, pt:({},{},{}), scale:({},{},{}), angle:{} deg, nCol:{}, nRow:{}, SpaceCol:{}, SpaceRow:{}\n",
					   m_name, m_pt.x, m_pt.y, m_pt.z, m_xscale, m_yscale, m_zscale, (double)(deg_t)m_angle, m_nCol, m_nRow, m_spacingCol, m_spacingRow);
		}
	};


	class GTL__SHAPE_CLASS xDrawing : public xShape {
	public:
		using base_t = xShape;
		using this_t = xDrawing;

	public:
		std::map<std::string, variable_t> m_vars;
		boost::ptr_deque<line_type_t> m_line_types;
		//boost::ptr_deque<xBlock> blocks;
		rect_t m_rectBoundary;
		boost::ptr_deque<xLayer> m_layers;

		//xDrawing() = default;
		//xDrawing(xDrawing const&) = default;
		//xDrawing(xDrawing &&) = default;
		//xDrawing& operator = (xDrawing const&) = default;
		//xDrawing& operator = (xDrawing &&) = default;

		virtual bool Compare(xShape const& B_) const override {
			if (!base_t::Compare(B_))
				return false;
			this_t const& B = (this_t const&)B_;
			return true
				and ( m_vars			== B.m_vars )
				and ( m_line_types		== B.m_line_types )
				and ( m_rectBoundary	== B.m_rectBoundary )
				and ( m_layers			== B.m_layers )
				;
		}
		virtual eSHAPE GetShapeType() const { return eSHAPE::drawing; }

		//virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual std::optional<std::pair<point_t, point_t>> GetStartEndPoint() const override {
			if (m_layers.empty())
				return {};
			auto r0 = m_layers.front().GetStartEndPoint();
			if (!r0)
				return {};
			auto r1 = m_layers.back().GetStartEndPoint();
			if (!r1)
				return {};
			return std::pair{ r0->first, r1->second };
		}
		virtual void FlipX() override { for (auto& layer : m_layers) layer.FlipX(); }
		virtual void FlipY() override { for (auto& layer : m_layers) layer.FlipY(); }
		virtual void FlipZ() override { for (auto& layer : m_layers) layer.FlipZ(); }
		virtual void Reverse() override {
			std::ranges::reverse(m_layers.base());
			for (auto& layer : m_layers) {
				layer.Reverse();
			}
		}
		virtual void Transform(xCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& layer : m_layers)
				layer.Transform(ct, bRightHanded);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool r{};
			for (auto& layer : m_layers)
				r |= layer.UpdateBoundary(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			xShape::Draw(canvas);
			for (auto& layer : m_layers) {
				layer.Draw(canvas);
			}
		}
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const override {
			bool result{};
			for (auto& layer : m_layers) {
				result |= layer.DrawROI(canvas, rectROI);
			}
			return result;
		}
		virtual void PrintOut(std::wostream& os) const override {
			for (auto& layer : m_layers) {
				layer.PrintOut(os);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(xDrawing);
		//GTL__REFLECTION_DERIVED(xDrawing, xShape);
		//GTL__REFLECTION_MEMBERS(vars, line_types, layers);
	#if 0
		bool operator == (xDrawing const& B) const {
			if ((base_t const&)*this != (base_t const&)B)
				return false;
			bool bEQ = (m_vars == B.m_vars);
			bEQ &= (m_line_types == B.m_line_types);
			bEQ &= (m_rectBoundary == B.m_rectBoundary);
			bEQ &= (m_layers == B.m_layers);
			if (m_layers.size() == B.m_layers.size()) {
				for (size_t i{}; i < m_layers.size(); i++) {
					auto const& layerA = m_layers[i];
					auto const& layerB = B.m_layers[i];
					if (layerA.m_shapes.size() != layerB.m_shapes.size())
						return false;
					for (size_t iShape{}; iShape < layerA.m_shapes.size(); iShape++) {
						auto const& shapeA = layerA.m_shapes[iShape];
						auto const& shapeB = layerB.m_shapes[iShape];
						if ((xShape const&)shapeA != (xShape const&)shapeB)
							return false;
						if (shapeA != shapeB)
							return false;
					}
					if (layerA != layerB)
						return false;
				}
			}
			if (!bEQ)
				return false;
			return true;
		}
	#endif
		auto operator <=> (xDrawing const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, xDrawing& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, xDrawing& var) {
			ar & boost::serialization::base_object<xShape>(var);

			ar & var.m_vars;
			ar & var.m_line_types;
			//ar & var.blocks;
			ar & var.m_rectBoundary;
			ar & var.m_layers;

			return ar;
		}

		xLayer& Layer(string_t const& name) {
			if (auto iter = std::find_if(m_layers.begin(), m_layers.end(), [&name](xLayer const& layer) -> bool { return name == layer.m_name; }); iter != m_layers.end())
				return *iter;
			return m_layers.front();
		}
		xLayer const& Layer(string_t const& name) const {
			if (auto iter = std::find_if(m_layers.begin(), m_layers.end(), [&name](xLayer const& layer) -> bool { return name == layer.m_name; }); iter != m_layers.end())
				return *iter;
			return m_layers.front();
		}

		bool AddEntity(std::unique_ptr<xShape> rShape, std::map<string_t, xLayer*> const& mapLayers, std::map<string_t, xBlock*> const& mapBlocks, rect_t& rectBoundary);

	public:
		virtual bool LoadFromCADJson(json_t& _j) override;

		void clear() {
			m_vars.clear();
			m_line_types.clear();
			m_layers.clear();
		}
	};


#pragma pack(pop)


}

BOOST_CLASS_EXPORT_GUID(gtl::shape::xShape, "shape")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xLayer, "layer")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDot, "dot")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xLine, "line")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolyline, "polyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xPolylineLW, "lwpolyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xCircle, "circleXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xArc, "arcXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xEllipse, "ellipseXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xSpline, "spline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xText, "text")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xMText, "mtext")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xHatch, "hatch")
BOOST_CLASS_EXPORT_GUID(gtl::shape::xDrawing, "drawing")


