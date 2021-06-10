//////////////////////////////////////////////////////////////////////
//
// shape_others.h:
//
// PWH
// 2017.07.20
// 2021.05.27
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*

	1. CShapeObject 에서 변경 내용 :

		class name :

			CShapeObject	-> s_shape
			CShapeLine		-> s_line
			CShapePolyLine	-> s_polyline
			...

		Container :

			TList -> boost::ptr_deque

		Point / Line : x, y, z 3차원으로 변경

			typedef CSize3d					size2d_t;
			typedef CPoint3d				point_t;
			typedef CRect3d					rect_t;
			typedef std::vector<s_line>		s_lines;

			typedef CCoordTrans3d			ct_t;



*/
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/iconv_wrapper.h"

#include "shape_primitives.h"
#include "canvas.h"

//export module shape;

namespace gtl::shape {
#pragma pack(push, 8)


	struct GTL_SHAPE_CLASS s_layer : public s_shape {
		string_t name;
		bool bUse{true};
		int flags{};
		boost::ptr_deque<s_shape> shapes;

		s_layer() = default;
		s_layer(s_layer const&) = default;
		s_layer(s_layer&&) = default;
		s_layer(string_t const& name) : name(name) {}

		virtual eSHAPE GetShapeType() const { return eSHAPE::layer; }
		//virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual void FlipX() override { for (auto& shape : shapes) shape.FlipX(); }
		virtual void FlipY() override { for (auto& shape : shapes) shape.FlipY(); }
		virtual void FlipZ() override { for (auto& shape : shapes) shape.FlipZ(); }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& shape : shapes)
				shape.Transform(ct, bRightHanded);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool r{};
			for (auto& shape : shapes)
				r |= shape.UpdateBoundary(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			for (auto& shape : shapes) {
				shape.Draw(canvas);
			}
		}
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const override {
			bool result{};
			for (auto& shape : shapes) {
				result |= shape.DrawROI(canvas, rectROI);
			}
			return result;
		}
		virtual void PrintOut(std::wostream& os) const override {
			s_shape::PrintOut(os);
			for (auto& shape : shapes) {
				shape.PrintOut(os);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_layer);
		//GTL__REFLECTION_DERIVED(s_layer, s_shape);
		//GTL__REFLECTION_MEMBERS(name, flags, strLineType, lineWeight, shapes);
		auto operator <=> (s_layer const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_layer& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_layer& var) {
			ar & boost::serialization::base_object<s_shape>(var);

			ar & var.name & var.bUse &var.flags & var.strLineType & var.lineWeight;
			ar & var.shapes;
			//size_t size = var.shapes.size();
			//ar & size;
			//for (size_t i{}; i < var.shapes.size(); i++) {
			//	auto& shape = var.shapes.at(i);
			//	auto* pShape = &shape;
			//	try {
			//		ar & pShape;
			//	} catch (std::exception& e) {
			//	#ifdef _DEBUG
			//	#define DEBUG_PRINT(...) fmt::print(__VA_ARGS__)
			//	#else
			//	#define DEBUG_PRINT(...) 
			//	#endif

			//		DEBUG_PRINT("{}\n", e.what());

			//	#undef DEBUG_PRINT
			//	}
			//}

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			name = j["name"];	// not a "layer"
			flags = j["flags"];
			bUse = j["plotF"];

			return true;
		}

		void clear() {
			shapes.clear();
			strLineType.clear();
			flags = {};
			bUse = true;
			name.clear();
		}

	protected:
		friend struct s_drawing;
		line_type_t* pLineType{};

	};

	// internally
	struct GTL_SHAPE_CLASS s_block : public s_layer {
		string_t layer;
		point_t pt;

		virtual eSHAPE GetShapeType() const { return eSHAPE::block; }

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_block);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_block, s_layer);
		//GTL__REFLECTION_MEMBERS(layer, pt);
		auto operator <=> (s_block const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_block& var, unsigned int const file_version) {
			boost::serialization::base_object<s_layer>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_block& var) {
			ar & boost::serialization::base_object<s_layer>(var);

			ar & var.layer & var.pt;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			this->layer = layer;
			using namespace std::literals;
			gtl::bjson j(_j);

			name = j["name"];
			flags = j["flags"];
			pt = PointFrom(j["basePoint"]);

			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_dot : public s_shape {
		point_t pt;

		virtual eSHAPE GetShapeType() const { return eSHAPE::dot; }

		//virtual point_t PointAt(double t) const override { return pt; };
		virtual void FlipX() override { pt.x = -pt.x; }
		virtual void FlipY() override { pt.y = -pt.y; }
		virtual void FlipZ() override { pt.z = -pt.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			pt = ct(pt);
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			return rectBoundary.UpdateBoundary(pt);
		};
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			canvas.MoveTo(pt);
			canvas.LineTo(pt);
		}
		virtual void PrintOut(std::wostream& os) const override {
			s_shape::PrintOut(os);
			fmt::print(os, L"\tpt({},{},{})\n", pt.x, pt.y, pt.z);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_dot);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_dot, s_shape);
		//GTL__REFLECTION_MEMBERS(pt);
		auto operator <=> (s_dot const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_dot& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_dot& var) {
			ar & boost::serialization::base_object<s_shape>(var);
			ar & var.pt;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			pt = PointFrom(j["basePoint"sv]);
			return true;
		}

	};

	struct GTL_SHAPE_CLASS s_line : public s_shape {
		point_t pt0, pt1;

		virtual eSHAPE GetShapeType() const { return eSHAPE::line; }

		//virtual point_t PointAt(double t) const override { return lerp(pt0, pt1, t); }
		virtual void FlipX() override { pt0.x = -pt0.x; pt1.x = -pt1.x; }
		virtual void FlipY() override { pt0.y = -pt0.y; pt1.y = -pt1.y; }
		virtual void FlipZ() override { pt0.z = -pt0.z; pt1.z = -pt1.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			pt0 = ct(pt0); pt1 = ct(pt1);
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bModified{};
			bModified |= rectBoundary.UpdateBoundary(pt0);
			bModified |= rectBoundary.UpdateBoundary(pt1);
			return bModified;
		};
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			canvas.Line(pt0, pt1);
		}
		virtual void PrintOut(std::wostream& os) const override {
			s_shape::PrintOut(os);
			fmt::print(os, L"\tpt0({},{},{}) - pt1({},{},{})\n", pt0.x, pt0.y, pt0.z, pt1.x, pt1.y, pt1.z);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_line);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_line, s_shape);
		//GTL__REFLECTION_MEMBERS(pt0, pt1);
		auto operator <=> (s_line const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_line& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_line& var) {
			ar & boost::serialization::base_object<s_shape>(var);
			return ar & var.pt0 & var.pt1;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);
			pt0 = PointFrom(j["basePoint"sv]);
			pt1 = PointFrom(j["secPoint"sv]);
			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_polyline : public s_shape {
		bool bLoop{};
		std::vector<polypoint_t> pts;

		virtual eSHAPE GetShapeType() const { return eSHAPE::polyline; }

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { for (auto& pt : pts) { pt.x = -pt.x;  pt.Bulge() = -pt.Bulge(); } }
		virtual void FlipY() override { for (auto& pt : pts) { pt.y = -pt.y;  pt.Bulge() = -pt.Bulge(); } }
		virtual void FlipZ() override { for (auto& pt : pts) { pt.z = -pt.z;  pt.Bulge() = -pt.Bulge(); } }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& pt : pts) {
				(point_t&)pt = ct((point_t&)pt);
			}
			if (!bRightHanded) {
				for (auto& pt : pts) { pt.x = -pt.x;  pt.Bulge() = -pt.Bulge(); } 
			}
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bModified{};
			for (auto const& pt : pts)
				bModified |= rectBoundary.UpdateBoundary(pt);
			return bModified;
		};
		virtual void Draw(ICanvas& canvas) const override;
		virtual void PrintOut(std::wostream& os) const override {
			s_shape::PrintOut(os);
			fmt::print(os, L"\t{}", bLoop ? L"loop ":L"");
			for (auto const& pt : pts)
				fmt::print(os, L"({},{},{},{}), ", pt.x, pt.y, pt.z, pt.w);
			fmt::print(os, L"\n");
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_polyline);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_polyline, s_shape);
		//GTL__REFLECTION_MEMBERS(pts);
		auto operator <=> (s_polyline const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_polyline& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_polyline& var) {
			ar & boost::serialization::base_object<s_shape>(var);
			ar & var.pts;
			ar & var.bLoop;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			auto jpts = j["vertlist"sv];
			auto apts = jpts.json().as_array();
			for (auto iter = apts.begin(); iter != apts.end(); iter++) {
				pts.push_back(polypoint_t{});
				pts.back() = PolyPointFromVertex(*iter);
			}

			bLoop = (j["flags"].value_or(0) & 1) != 0;
			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_lwpolyline : public s_polyline {
		int dummy{};

		virtual eSHAPE GetShapeType() const { return eSHAPE::lwpolyline; }

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_lwpolyline);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_lwpolyline, s_polyline);
		//GTL__REFLECTION_MEMBERS(dummy);
		auto operator <=> (s_lwpolyline const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_lwpolyline& var, unsigned int const file_version) {
			boost::serialization::base_object<s_polyline>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_lwpolyline& var) {
			ar & boost::serialization::base_object<s_polyline>(var);
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			auto jpts = j["vertlist"sv];
			auto apts = jpts.json().as_array();
			for (auto iter = apts.begin(); iter != apts.end(); iter++) {
				pts.push_back(polypoint_t{});
				pts.back() = PolyPointFrom(*iter);
			}

			bLoop = (j["flags"].value_or(0) & 1) != 0;
			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_circleXY : public s_shape {
		point_t ptCenter;
		double radius{};
		deg_t angle_length{360_deg};	// 회전 방향.

		virtual eSHAPE GetShapeType() const { return eSHAPE::circle_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { ptCenter.x = -ptCenter.x; angle_length = -angle_length; }
		virtual void FlipY() override { ptCenter.y = -ptCenter.y; angle_length = -angle_length; }
		virtual void FlipZ() override { ptCenter.z = -ptCenter.z; angle_length = -angle_length; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			ptCenter = ct(ptCenter); radius = ct.Trans(radius);
			if (!bRightHanded)
				angle_length = -angle_length;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool bResult{};
			bResult |= rectBoundary.UpdateBoundary(point_t(ptCenter.x-radius, ptCenter.y-radius, ptCenter.z));
			bResult |= rectBoundary.UpdateBoundary(point_t(ptCenter.x+radius, ptCenter.y+radius, ptCenter.z));
			return bResult;
		};
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			canvas.Arc(ptCenter, radius, 0._deg, angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			s_shape::PrintOut(os);
			fmt::print(os, L"\tcenter({},{},{}), r {}\n", ptCenter.x, ptCenter.y, ptCenter.z, radius);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_circleXY);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_circleXY, s_shape);
		//GTL__REFLECTION_MEMBERS(ptCenter, radius, angle_length);
		auto operator <=> (s_circleXY const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_circleXY& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_circleXY& var) {
			ar & boost::serialization::base_object<s_shape>(var);
			return ar & var.ptCenter & var.radius & (double&)var.angle_length;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			ptCenter = PointFrom(j["basePoint"sv]);
			radius = j["radious"sv];
			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_arcXY : public s_circleXY {
		deg_t angle_start{};

		virtual eSHAPE GetShapeType() const { return eSHAPE::arc_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { s_circleXY::FlipX(); angle_start = AdjustAngle(180._deg - angle_start); }
		virtual void FlipY() override { s_circleXY::FlipY(); angle_start = AdjustAngle(-angle_start); }
		virtual void FlipZ() override { s_circleXY::FlipZ(); angle_start = AdjustAngle(180._deg - angle_start); }	// ????.....  성립 안되지만,
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			// todo : ... upgrade?
			s_circleXY::Transform(ct, bRightHanded);
			if (!bRightHanded)
				angle_start = -angle_start;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			// todo : ... upgrade?
			return s_circleXY::UpdateBoundary(rectBoundary);
		}
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			canvas.Arc(ptCenter, radius, angle_start, angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			s_circleXY::PrintOut(os);
			fmt::print(os, L"\tangle_start:{} deg, length:{} deg\n", (double)(deg_t)angle_start, (double)(deg_t)angle_length);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_arcXY);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_arcXY, s_circleXY);
		//GTL__REFLECTION_MEMBERS(angle_start);
		auto operator <=> (s_arcXY const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_arcXY& var, unsigned int const file_version) {
			boost::serialization::base_object<s_circleXY>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_arcXY& var) {
			ar & boost::serialization::base_object<s_circleXY>(var);
			return ar & var.angle_start;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_circleXY::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			angle_start = deg_t{(double)j["staangle"sv]};
			bool bCCW = j["isccw"sv].value_or(0) != 0;
			deg_t angle_end { (double)j["endangle"sv] };
			angle_length = angle_end - angle_start;
			if (bCCW) {
				if (angle_length < 0_deg)
					angle_length += 360_deg;
			} else {
				if (angle_length > 0_deg)
					angle_length -= 360_deg;
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

		static s_arcXY GetFromBulge(double bulge, point_t const& pt0, point_t const& pt1) {
			s_arcXY arc;
			CPoint2d vecPerpendicular(-(pt0.y-pt1.y), (pt0.x-pt1.x));
			// Normalize
			{
				double d = vecPerpendicular.Distance(CPoint2d(0.0, 0.0));
				vecPerpendicular.x /= d;
				vecPerpendicular.y /= d;
			}
			CPoint2d ptCenterOfLine((pt0.x+pt1.x)/2., (pt0.y+pt1.y)/2.);
			double l = pt1.Distance(pt0)/2.;
			CPoint2d ptBulge;
			ptBulge.x = ptCenterOfLine.x + vecPerpendicular.x * (bulge * l);
			ptBulge.y = ptCenterOfLine.y + vecPerpendicular.y * (bulge * l);
			double h = ptBulge.Distance(ptCenterOfLine);
			arc.radius = (Square(l) + Square(h)) / (2 * h);

			arc.ptCenter.x = ptBulge.x + (arc.radius / h) * (ptCenterOfLine.x - ptBulge.x);
			arc.ptCenter.y = ptBulge.y + (arc.radius / h) * (ptCenterOfLine.y - ptBulge.y);
			arc.angle_start = rad_t::atan2(pt0.y - arc.ptCenter.y, pt0.x - arc.ptCenter.x);
			rad_t dT1 = rad_t::atan2(pt1.y - arc.ptCenter.y, pt1.x - arc.ptCenter.x);
			//arc.m_eDirection = (dBulge > 0) ? 1 : -1;
			//arc.m_dTLength = (dBulge > 0) ? fabs(dT1-arc.m_dT0) : -fabs(dT1-arc.m_dT0);
			if (bulge > 0) {
				while (dT1 < arc.angle_start)
					dT1 += rad_t(std::numbers::pi*2);
				arc.angle_length = dT1 - arc.angle_start;
			} else {
				while (dT1 > arc.angle_start)
					dT1 -= rad_t(std::numbers::pi*2);
				arc.angle_length = dT1 - arc.angle_start;
			}

			return arc;
		}

	};

	struct GTL_SHAPE_CLASS s_ellipseXY : public s_arcXY {
		double radiusH{};
		deg_t angle_first_axis{};

		virtual eSHAPE GetShapeType() const { return eSHAPE::ellipse_xy; }

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { s_arcXY::FlipX(); angle_first_axis = 180._deg - angle_first_axis; }
		virtual void FlipY() override { s_arcXY::FlipY(); angle_first_axis = - angle_first_axis; }
		virtual void FlipZ() override { s_arcXY::FlipZ(); angle_first_axis = 180._deg - angle_first_axis; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			s_arcXY::Transform(ct, bRightHanded);
			radiusH = ct.Trans(radiusH);
			if (!bRightHanded)
				angle_first_axis = -angle_first_axis;
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			// todo : ... upgrade
			bool bResult{};
			bResult |= rectBoundary.UpdateBoundary(point_t(ptCenter.x-radius, ptCenter.y-radiusH, ptCenter.z));
			bResult |= rectBoundary.UpdateBoundary(point_t(ptCenter.x+radius, ptCenter.y+radiusH, ptCenter.z));
			return bResult;
		}
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			canvas.Ellipse(ptCenter, radius, radiusH, angle_first_axis, angle_start, angle_length);
		}
		virtual void PrintOut(std::wostream& os) const override {
			s_arcXY::PrintOut(os);
			fmt::print(os, L"\tradiusH:{}, angle_first_axis:{} deg\n", radiusH, (double)(deg_t)angle_first_axis);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_ellipseXY);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_ellipseXY, s_arcXY);
		//GTL__REFLECTION_MEMBERS(radiusH, angle_first_axis);
		auto operator <=> (s_ellipseXY const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_ellipseXY& var, unsigned int const file_version) {
			boost::serialization::base_object<s_arcXY>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_ellipseXY& var) {
			ar & boost::serialization::base_object<s_arcXY>(var);
			return ar & var.radiusH & (double&)var.angle_first_axis;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			ptCenter = PointFrom(j["basePoint"sv]);
			point_t firstAxis = PointFrom(j["secPoint"sv]);
			angle_first_axis = deg_t::atan2(firstAxis.y, firstAxis.x);
			double ratio = j["ratio"sv];
			radius = firstAxis.GetLength();
			radiusH = ratio * radius;

			rad_t t0 {(double)j["staparam"sv]};
			rad_t t1 {(double)j["endparam"sv]};
			bool bCCW = j["isccw"sv];

			angle_start = t0;
			angle_length = t1-t0;

			if (bCCW) {
				if (angle_length < 0_deg)
					angle_length += 360_deg;
			} else {
				if (angle_length > 0_deg)
					angle_length -= 360_deg;
			}

			return true;
		}

	};

	struct GTL_SHAPE_CLASS s_spline : public s_shape {
		enum class eFLAG { closed = 1, periodic = 2, rational = 4, planar = 8, linear = planar|16 };
		int flags{};
		point_t ptNormal;
		point_t vStart, vEnd;
		int degree{};

		std::vector<double> knots;
		std::vector<point_t> ptsControl;
		std::vector<point_t> ptsFit;

		double toleranceKnot{0.0000001};
		double toleranceControlPoint{0.0000001};
		double toleranceFitPoint{0.0000001};

		virtual eSHAPE GetShapeType() const { return eSHAPE::spline; }

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { for (auto& pt : ptsControl) pt.x = -pt.x; for (auto& pt : ptsFit) pt.x = -pt.x; ptNormal.x = -ptNormal.x; vStart.x = -vStart.x; vEnd.x = -vEnd.x; }
		virtual void FlipY() override { for (auto& pt : ptsControl) pt.y = -pt.y; for (auto& pt : ptsFit) pt.y = -pt.y; ptNormal.y = -ptNormal.y; vStart.y = -vStart.y; vEnd.y = -vEnd.y; }
		virtual void FlipZ() override { for (auto& pt : ptsControl) pt.z = -pt.z; for (auto& pt : ptsFit) pt.z = -pt.z; ptNormal.z = -ptNormal.z; vStart.z = -vStart.z; vEnd.z = -vEnd.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			ptNormal = ct(ptNormal);
			vStart = ct(vStart);
			vEnd = ct(vEnd);
			for (auto& pt : ptsControl)
				pt = ct(pt);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool b{};
			for (auto& pt : ptsControl)
				b = rect.UpdateBoundary(pt);
			return b;
		};
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			canvas.Spline(degree, ptsControl, knots, false);
		}
		virtual void PrintOut(std::wostream& os) const override {
			s_shape::PrintOut(os);
			fmt::print(os, L"\tflags:{}, degree:{}\n");
			fmt::print(os, L"\tknot ");
			for (auto knot : knots)
				fmt::print(os, L"{}, ", knot);
			fmt::print(os, L"\n\tcontrol points ");
			for (auto pt : ptsControl)
				fmt::print(os, L"({},{},{}), ", pt.x, pt.y, pt.z);
			fmt::print(os, L"\n");
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_spline);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_spline, s_shape);
		//GTL__REFLECTION_MEMBERS(flags, ptNormal, vStart, vEnd, degree, knots, ptsControl, toleranceKnot, toleranceControlPoint, toleranceFitPoint);
		auto operator <=> (s_spline const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_spline& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_spline& var) {
			ar & boost::serialization::base_object<s_shape>(var);

			ar & var.flags;
			ar & var.ptNormal;
			ar & var.vStart & var.vEnd;
			ar & var.degree;

			ar & var.knots;
			ar & var.ptsControl;

			ar & var.toleranceKnot;
			ar & var.toleranceControlPoint;
			ar & var.toleranceFitPoint;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			ptNormal = PointFrom(j["normalVec"sv]);
			vStart = PointFrom(j["tgStart"sv]);
			vEnd = PointFrom(j["tgEnd"sv]);
			flags = j["flags"sv];
			degree = j["degree"sv];

			toleranceKnot = j["tolknot"sv];
			toleranceControlPoint = j["tolcontrol"sv];
			toleranceFitPoint = j["tolfit"sv];

			for (auto& v : j["knotslist"sv].json().as_array()) {
				knots.push_back(v.as_double());
			}
			for (auto& v : j["controllist"sv].json().as_array()) {
				ptsControl.push_back(PointFrom(v));
			}
			for (auto& v : j["fitlist"sv].json().as_array()) {
				ptsFit.push_back(PointFrom(v));
			}

			return true;
		}

	};

	struct GTL_SHAPE_CLASS s_text : public s_shape {
		enum class eALIGN_VERT : int { base_line = 0, bottom, mid, top };
		enum class eALIGN_HORZ : int { left = 0, center, right, aligned, middle, fit };

		point_t pt0, pt1;
		string_t text;
		double height{};
		deg_t angle{};
		double widthScale{};
		deg_t oblique{};
		string_t textStyle;
		int textgen{};
		eALIGN_HORZ alignHorz{eALIGN_HORZ::left};
		eALIGN_VERT alignVert{eALIGN_VERT::base_line};
	
		virtual eSHAPE GetShapeType() const { return eSHAPE::text; }

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { pt0.x = -pt0.x; pt1.x = -pt1.x; }
		virtual void FlipY() override { pt0.y = -pt0.y; pt1.y = -pt1.y; }
		virtual void FlipZ() override { pt0.z = -pt0.z; pt1.z = -pt1.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			pt0 = ct(pt0);
			pt1 = ct(pt1);
		}
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			// todo : upgrade.
			bool b{};
			b |= rectBoundary.UpdateBoundary(pt0);
			//b |= rectBoundary.UpdateBoundary(pt1);
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			// todo :
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_text);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_text, s_shape);
		//GTL__REFLECTION_MEMBERS(pt0, pt1, text, height, angle, widthScale, oblique, textStyle, textgen, alignHorz, alignVert);
		auto operator <=> (s_text const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_text& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_text& var) {
			ar & boost::serialization::base_object<s_shape>(var);

			ar & var.pt0 & var.pt1;
			ar & var.text;
			ar & var.height;
			ar & var.angle;
			ar & var.widthScale;
			ar & var.oblique;
			ar & var.textStyle;
			ar & var.textgen;
			ar & (int&)var.alignHorz;
			ar & (int&)var.alignVert;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			pt0 = PointFrom(j["basePoint"sv]);
			pt1 = PointFrom(j["secPoint"sv]);
			text = j["text"sv];
			height = j["height"sv];
			angle = deg_t{(double)j["angle"sv]};
			widthScale = j["widthscale"sv];
			oblique = deg_t{(double)j["oblique"sv]};
			textStyle = j["style"sv];
			textgen = j["textgen"sv];
			alignHorz = (eALIGN_HORZ)(int)j["alignH"sv];
			alignVert = (eALIGN_VERT)(int)j["alignV"sv];

			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_mtext : public s_text {
	protected:
		using s_text::alignVert;
	public:
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
		eATTACH GetAttachPoint() const { return (eATTACH)alignVert; }
		void SetAttachPoint(eATTACH eAttach) { alignVert = (eALIGN_VERT)eAttach; }
		double interlin{};

		virtual eSHAPE GetShapeType() const { return eSHAPE::mtext; }

		//virtual point_t PointAt(double t) const override {};
		//virtual void FlipX() override {}
		//virtual void FlipY() override {}
		//virtual void FlipY() override {}
		//virtual void Transform(CCoordTrans3d const&, bool bRightHanded) override { return true; };
		//virtual bool UpdateBoundary(rect_t& rectBoundary) const override { return true; };
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			// todo : draw mtext
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_mtext);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_mtext, s_text);
		//GTL__REFLECTION_MEMBERS(interlin);
		auto operator <=> (s_mtext const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_mtext& var, unsigned int const file_version) {
			boost::serialization::base_object<s_text>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_mtext& var) {
			ar & boost::serialization::base_object<s_text>(var);
			return ar & var.interlin;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_text::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			interlin = j["interlin"sv];

			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_hatch : public s_shape {
		string_t name;
		bool bSolid{};
		bool bAssociative{};
		int nLoops{};
		int hstyle{};
		int hpattern{};
		bool bDouble{};
		deg_t angle{};
		double scale{};
		int deflines{};

		std::vector<s_polyline> boundaries;

		virtual eSHAPE GetShapeType() const { return eSHAPE::hatch; }

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { for (auto& b : boundaries) b.FlipX(); }
		virtual void FlipY() override { for (auto& b : boundaries) b.FlipY(); }
		virtual void FlipZ() override { for (auto& b : boundaries) b.FlipZ(); }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
		};
		virtual bool UpdateBoundary(rect_t& rectBoundary) const override {
			bool b{};
			for (auto const& bound : boundaries) {
				b |= bound.UpdateBoundary(rectBoundary);
			}
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			// todo : draw hatch
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_hatch);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_hatch, s_shape);
		//GTL__REFLECTION_MEMBERS(name, bSolid, bAssociative, nLoops, hstyle, hpattern, bDouble, angle, scale, deflines);
		auto operator <=> (s_hatch const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_hatch& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_hatch& var) {
			ar & boost::serialization::base_object<s_shape>(var);

			ar & var.name;
			ar & var.bSolid;
			ar & var.bAssociative;
			ar & var.nLoops;
			ar & var.hstyle;
			ar & var.hpattern;
			ar & var.angle;
			ar & var.scale;
			ar & var.deflines;
			ar & var.boundaries;

			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			s_shape::LoadFromCADJson(_j);
			using namespace std::literals;
			gtl::bjson j(_j);

			name			= j["name"sv];
			bSolid			= (bool)(int)j["solid"sv];
			bAssociative	= (bool)(int)j["associative"sv];
			hstyle			= j["hstyle"sv];
			hpattern		= j["hpattern"sv];
			bDouble			= (bool)(int)j["doubleflag"sv];
			nLoops			= j["loopsnum"sv];
			angle			= rad_t{(double)j["angle"sv]};
			scale			= j["scale"sv];
			deflines		= j["deflines"sv];

			return true;
		}

	};

	// temporary object
	struct s_insert : public s_shape {
		string_t name;	// block name
		point_t pt;
		double xscale{1};
		double yscale{1};
		double zscale{1};
		rad_t angle{};
		int nCol{1};
		int nRow{1};
		double spacingCol{};
		double spacingRow{};

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_insert);
		//GTL__REFLECTION_VIRTUAL_DERIVED(s_insert, s_shape);
		//GTL__REFLECTION_MEMBERS(name, xscale, yscale, zscale, angle, nCol, nRow, spacingCol, spacingRow);
		auto operator <=> (s_insert const&) const = default;

		template < typename archive >
		friend void serialize(archive& ar, s_insert& var, unsigned int const file_version) {
			boost::serialization::base_object<s_shape>(var);
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_insert& var) {
			ar & var.name & var.xscale & var.yscale & var.zscale & (double&)angle;
			ar & var.nCol & var.nRow & var.spacingCol & var.spacingRow;
			return ar;
		}

		virtual bool LoadFromCADJson(json_t& _j) override {
			using namespace std::literals;
			s_shape::LoadFromCADJson(_j);
			gtl::bjson j(_j);

			pt = PointFrom(j["basePoint"]);
			name = j["name"];
			xscale = j["xscale"];
			yscale = j["yscale"];
			zscale = j["zscale"];
			angle = rad_t{(double)j["angle"]};
			nCol = j["colcount"];
			nRow = j["rowcount"];
			spacingCol = j["colspace"];
			spacingRow = j["rowspace"];

			return true;
		}

		virtual eSHAPE GetShapeType() const { return eSHAPE::insert; }

		//virtual point_t PointAt(double t) const = 0;
		virtual void FlipX() override {};
		virtual void FlipY() override {};
		virtual void FlipZ() override {};
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded /*= ct.IsRightHanded()*/) override {};
		virtual bool UpdateBoundary(rect_t&) const override { return false; };
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
		};

		virtual void PrintOut(std::wostream& os) const override {
			s_shape::PrintOut(os);
			fmt::print(os, L"\tname:{}, pt:({},{},{}), scale:({},{},{}), angle:{} deg, nCol:{}, nRow:{}, SpaceCol:{}, SpaceRow:{}\n",
					   name, pt.x, pt.y, pt.z, xscale, yscale, zscale, (double)(deg_t)angle, nCol, nRow, spacingCol, spacingRow);
		}
	};


	struct GTL_SHAPE_CLASS s_drawing : public s_shape {
		std::map<std::string, variable_t> vars;
		boost::ptr_deque<line_type_t> line_types;
		//boost::ptr_deque<s_block> blocks;
		rect_t rectBoundary;
		boost::ptr_deque<s_layer> layers;

		//s_drawing() = default;
		//s_drawing(s_drawing const&) = default;
		//s_drawing(s_drawing &&) = default;
		//s_drawing& operator = (s_drawing const&) = default;
		//s_drawing& operator = (s_drawing &&) = default;

		virtual eSHAPE GetShapeType() const { return eSHAPE::drawing; }

		//virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual void FlipX() override { for (auto& layer : layers) layer.FlipX(); }
		virtual void FlipY() override { for (auto& layer : layers) layer.FlipY(); }
		virtual void FlipZ() override { for (auto& layer : layers) layer.FlipZ(); }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& layer : layers)
				layer.Transform(ct, bRightHanded);
		}
		virtual bool UpdateBoundary(rect_t& rect) const override {
			bool r{};
			for (auto& layer : layers)
				r |= layer.UpdateBoundary(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			s_shape::Draw(canvas);
			for (auto& layer : layers) {
				layer.Draw(canvas);
			}
		}
		virtual bool DrawROI(ICanvas& canvas, rect_t const& rectROI) const override {
			bool result{};
			for (auto& layer : layers) {
				result |= layer.DrawROI(canvas, rectROI);
			}
			return result;
		}
		virtual void PrintOut(std::wostream& os) const override {
			for (auto& layer : layers) {
				layer.PrintOut(os);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_drawing);
		//GTL__REFLECTION_DERIVED(s_drawing, s_shape);
		//GTL__REFLECTION_MEMBERS(vars, line_types, layers);

		template < typename archive >
		friend void serialize(archive& ar, s_drawing& var, unsigned int const file_version) {
			ar & var;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, s_drawing& var) {
			ar & boost::serialization::base_object<s_shape>(var);

			ar & var.vars;
			ar & var.line_types;
			//ar & var.blocks;
			ar & var.rectBoundary;
			ar & var.layers;

			return ar;
		}

		s_layer& Layer(string_t const& name) {
			if (auto iter = std::find_if(layers.begin(), layers.end(), [&name](s_layer const& layer) -> bool { return name == layer.name; }); iter != layers.end())
				return *iter;
			return layers.front();
		}
		s_layer const& Layer(string_t const& name) const {
			if (auto iter = std::find_if(layers.begin(), layers.end(), [&name](s_layer const& layer) -> bool { return name == layer.name; }); iter != layers.end())
				return *iter;
			return layers.front();
		}

		bool AddEntity(std::unique_ptr<s_shape> rShape, std::map<string_t, s_layer*> const& mapLayers, std::map<string_t, s_block*> const& mapBlocks, rect_t& rectBoundary);

	public:
		virtual bool LoadFromCADJson(json_t& _j) override;

		void clear() {
			vars.clear();
			line_types.clear();
			layers.clear();
		}
	};


#pragma pack(pop)


}

BOOST_CLASS_EXPORT_GUID(gtl::shape::s_shape, "shape")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_layer, "layer")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_dot, "dot")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_line, "line")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_polyline, "polyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_lwpolyline, "lwpolyline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_circleXY, "circleXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_arcXY, "arcXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_ellipseXY, "ellipseXY")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_spline, "spline")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_text, "text")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_mtext, "mtext")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_hatch, "hatch")
BOOST_CLASS_EXPORT_GUID(gtl::shape::s_drawing, "drawing")


