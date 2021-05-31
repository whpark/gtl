//////////////////////////////////////////////////////////////////////
//
// shape.h:
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

#include "shape_default.h"
#include "canvas.h"

//export module shape;

namespace gtl::shape {

	struct GTL_SHAPE_CLASS s_layer : public s_shape {
		string_t name;
		bool bUse{true};
		uint32_t flags{};
		string_t lineType;
		boost::ptr_deque<s_shape> shapes;

		s_layer() = default;
		s_layer(s_layer const&) = default;
		s_layer(s_layer&&) = default;
		s_layer(string_t const& name) : name(name) {}

		//virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual void FlipX() override { for (auto& shape : shapes) shape.FlipX(); }
		virtual void FlipY() override { for (auto& shape : shapes) shape.FlipY(); }
		virtual void FlipZ() override { for (auto& shape : shapes) shape.FlipZ(); }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& shape : shapes)
				shape.Transform(ct, bRightHanded);
		}
		virtual bool GetBoundingRect(CRect2d& rect) const override {
			bool r{};
			for (auto& shape : shapes)
				r |= shape.GetBoundingRect(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			for (auto& shape : shapes) {
				canvas.PreDraw(shape);
				shape.Draw(canvas);
				canvas.PostDraw(shape);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_layer);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & name;
			ar & flags;
			ar & lineType;
			ar & shapes;
		}
		virtual bool FromJson(boost::json::value& _j, string_t& layer) override {
			s_shape::FromJson(_j, layer);
			using namespace std::literals;
			bjson j(_j);
			name = j["name"sv];
			flags = (int)j["flags"sv];
			bUse = (bool)j["plotF"sv];
			lineType = j["lineType"sv];
			return true;
		}

	};

	struct GTL_SHAPE_CLASS s_dot : public s_shape {
		point_t pt;

		//virtual point_t PointAt(double t) const override { return pt; };
		virtual void FlipX() override { pt.x = -pt.x; }
		virtual void FlipY() override { pt.y = -pt.y; }
		virtual void FlipZ() override { pt.z = -pt.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			pt = ct(pt);
		};
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			return rectBoundary.CheckBoundary(pt);
		};
		virtual void Draw(ICanvas& canvas) const override {
			canvas.MoveTo(pt);
			canvas.LineTo(pt);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_dot);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & pt;
		}

		virtual bool FromJson(boost::json::value& _j, string_t& layer) override {
			s_shape::FromJson(_j, layer);
			using namespace std::literals;
			bjson j(_j);
			FromJson_ShapeCoord(j["basePoint"sv], pt);
			return true;
		}

	};

	struct GTL_SHAPE_CLASS s_line : public s_shape {
		point_t pt0, pt1;

		//virtual point_t PointAt(double t) const override { return lerp(pt0, pt1, t); }
		virtual void FlipX() override { pt0.x = -pt0.x; pt1.x = -pt1.x; }
		virtual void FlipY() override { pt0.y = -pt0.y; pt1.y = -pt1.y; }
		virtual void FlipZ() override { pt0.z = -pt0.z; pt1.z = -pt1.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			pt0 = ct(pt0); pt1 = ct(pt1);
		};
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			bool bModified{};
			bModified |= rectBoundary.CheckBoundary(pt0);
			bModified |= rectBoundary.CheckBoundary(pt1);
			return bModified;
		};
		virtual void Draw(ICanvas& canvas) const override {
			canvas.Line(pt0, pt1);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_line);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & pt0 & pt1;
		}

		virtual bool FromJson(boost::json::value& _j, string_t& layer) override {
			s_shape::FromJson(_j, layer);
			using namespace std::literals;
			bjson j(_j);
			FromJson_ShapeCoord(j["basePoint"sv], pt0);
			FromJson_ShapeCoord(j["secPoint"sv], pt1);
			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_polyline : public s_shape {
		bool bLoop{};
		std::vector<polypoint_t> pts;

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
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			bool bModified{};
			for (auto const& pt : pts)
				bModified |= rectBoundary.CheckBoundary(pt);
			return bModified;
		};
		virtual void Draw(ICanvas& canvas) const override;

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_polyline);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & pts;
		}

		virtual bool FromJson(boost::json::value& _j, string_t& layer) override {
			s_shape::FromJson(_j, layer);
			using namespace std::literals;
			bjson j(_j);

			auto jpts = j["vertlist"sv];
			auto apts = jpts.json().as_array();
			for (auto iter = apts.begin(); iter != apts.end(); iter++) {
				pts.push_back(polypoint_t{});
				FromJson_ShapeCoord(*iter, pts.back());
			}

			return true;
		}
	};

	struct GTL_SHAPE_CLASS s_circleXY : public s_shape {
		point_t ptCenter;
		double radius{};
		deg_t angle_length{360_deg};	// 회전 방향.

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { ptCenter.x = -ptCenter.x; angle_length = -angle_length; }
		virtual void FlipY() override { ptCenter.y = -ptCenter.y; angle_length = -angle_length; }
		virtual void FlipZ() override { ptCenter.z = -ptCenter.z; angle_length = -angle_length; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			ptCenter = ct(ptCenter); radius = ct.Trans(radius);
			if (!bRightHanded)
				angle_length = -angle_length;
		}
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			bool bResult{};
			bResult |= rectBoundary.CheckBoundary(point_t(ptCenter.x-radius, ptCenter.y-radius, ptCenter.z));
			bResult |= rectBoundary.CheckBoundary(point_t(ptCenter.x+radius, ptCenter.y+radius, ptCenter.z));
			return bResult;
		};
		virtual void Draw(ICanvas& canvas) const override {
			canvas.Arc(ptCenter, radius, 0._deg, angle_length);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_circleXY);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & ptCenter & radius & (double&)angle_length;
		}
	};

	struct GTL_SHAPE_CLASS s_arcXY : public s_circleXY {
		deg_t angle_start{};

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
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			// todo : ... upgrade?
			return s_circleXY::GetBoundingRect(rectBoundary);
		}
		virtual void Draw(ICanvas& canvas) const override {
			canvas.Arc(ptCenter, radius, angle_start, angle_length);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_arcXY);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_circleXY>(*this);

			ar & angle_start;
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
			arc.angle_start = atan2(pt0.y - arc.ptCenter.y, pt0.x - arc.ptCenter.x);
			double dT1 = atan2(pt1.y - arc.ptCenter.y, pt1.x - arc.ptCenter.x);
			//arc.m_eDirection = (dBulge > 0) ? 1 : -1;
			//arc.m_dTLength = (dBulge > 0) ? fabs(dT1-arc.m_dT0) : -fabs(dT1-arc.m_dT0);
			if (bulge > 0) {
				while (dT1 < arc.angle_start)
					dT1 += std::numbers::pi*2;
				arc.angle_length = dT1 - arc.angle_start;
			} else {
				while (dT1 > arc.angle_start)
					dT1 -= std::numbers::pi*2;
				arc.angle_length = dT1 - arc.angle_start;
			}

			return arc;
		}

	};

	struct GTL_SHAPE_CLASS s_ellipseXY : public s_arcXY {
		double radiusH{};
		deg_t angle_first_axis{};

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
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			// todo : ... upgrade
			bool bResult{};
			bResult |= rectBoundary.CheckBoundary(point_t(ptCenter.x-radius, ptCenter.y-radiusH, ptCenter.z));
			bResult |= rectBoundary.CheckBoundary(point_t(ptCenter.x+radius, ptCenter.y+radiusH, ptCenter.z));
			return bResult;
		}
		virtual void Draw(ICanvas& canvas) const override {
			canvas.Ellipse(ptCenter, radius, radiusH, angle_first_axis, angle_start, angle_length);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_ellipseXY);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_arcXY>(*this);

			ar & radiusH & (double&)angle_first_axis;
		}
	};

	struct GTL_SHAPE_CLASS s_spline : public s_shape {
		point_t ptNormal;
		point_t ptStartTangent, ptEndTangent;
		int degree{};
		int nFit{};

		std::vector<double> knots;
		std::vector<point_t> controlPoints;

		double knotTolerance{0.0000001};
		double controlPointTolerance{0.0000001};
		double fitPointTolerance{0.0000001};

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { for (auto& pt : controlPoints) pt.x = -pt.x; ptNormal.x = -ptNormal.x; ptStartTangent.x = -ptStartTangent.x; ptEndTangent.x = -ptEndTangent.x; }
		virtual void FlipY() override { for (auto& pt : controlPoints) pt.y = -pt.y; ptNormal.y = -ptNormal.y; ptStartTangent.y = -ptStartTangent.y; ptEndTangent.y = -ptEndTangent.y; }
		virtual void FlipZ() override { for (auto& pt : controlPoints) pt.z = -pt.z; ptNormal.z = -ptNormal.z; ptStartTangent.z = -ptStartTangent.z; ptEndTangent.z = -ptEndTangent.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			ptNormal = ct(ptNormal);
			ptStartTangent = ct(ptStartTangent);
			ptEndTangent = ct(ptEndTangent);
			for (auto& pt : controlPoints)
				pt = ct(pt);
		}
		virtual bool GetBoundingRect(CRect2d& rect) const override {
			bool b{};
			for (auto& pt : controlPoints)
				b = rect.CheckBoundary(pt);
			return b;
		};
		virtual void Draw(ICanvas& canvas) const override {
			canvas.Spline(degree, controlPoints, knots, false);
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_spline);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & ptNormal;
			ar & ptStartTangent & ptEndTangent;
			ar & degree;
			ar & nFit;

			ar & knots;
			ar & controlPoints;

			ar & knotTolerance;
			ar & controlPointTolerance;
			ar & fitPointTolerance;
		}
	};

	struct GTL_SHAPE_CLASS s_text : public s_shape {
		enum class eVERT_ALIGN : int { base_line = 0, bottom, mid, top };
		enum class eHORZ_ALIGN : int { left = 0, center, right, aligned, middle, fit };

		point_t pt0, pt1;
		string_t text;
		double height{};
		deg_t angle{};
		double widthScale{};
		deg_t oblique{};
		string_t textStyle;
		int textgen{};
		eHORZ_ALIGN horzAlign{eHORZ_ALIGN::left};
		eVERT_ALIGN vertAlign{eVERT_ALIGN::base_line};
	
		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { pt0.x = -pt0.x; pt1.x = -pt1.x; }
		virtual void FlipY() override { pt0.y = -pt0.y; pt1.y = -pt1.y; }
		virtual void FlipZ() override { pt0.z = -pt0.z; pt1.z = -pt1.z; }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			pt0 = ct(pt0);
			pt1 = ct(pt1);
		}
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			// todo : upgrade.
			bool b{};
			b |= rectBoundary.CheckBoundary(pt0);
			b |= rectBoundary.CheckBoundary(pt1);
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			// todo :
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_text);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & pt0 & pt1;
			ar & text;
			ar & height;
			ar & angle;
			ar & widthScale;
			ar & oblique;
			ar & textStyle;
			ar & textgen;
			ar & (int&)horzAlign;
			ar & (int&)vertAlign;
		}
	};

	struct GTL_SHAPE_CLASS s_mtext : public s_text {
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

		eATTACH eAttch{eATTACH::topLeft};
		int interlin{};

		//virtual point_t PointAt(double t) const override {};
		//virtual void FlipX() override {}
		//virtual void FlipY() override {}
		//virtual void FlipY() override {}
		//virtual void Transform(CCoordTrans3d const&, bool bRightHanded) override { return true; };
		//virtual bool GetBoundingRect(CRect2d& rectBoundary) const override { return true; };
		virtual void Draw(ICanvas& canvas) const override {
			// todo : draw mtext
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_mtext);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_text>(*this);

			ar & (int&)eAttch;
			ar & interlin;
		}
	};

	struct GTL_SHAPE_CLASS s_hatch : public s_shape {
		string_t name;
		bool bSolid{};
		int associative{};
		int hstyle{};
		int hpattern{};
		int doubleflag{};
		deg_t angle{};
		double scale{};
		int deflines{};

		std::vector<s_polyline> boundaries;

		//virtual point_t PointAt(double t) const override {};
		virtual void FlipX() override { for (auto& b : boundaries) b.FlipX(); }
		virtual void FlipY() override { for (auto& b : boundaries) b.FlipY(); }
		virtual void FlipZ() override { for (auto& b : boundaries) b.FlipZ(); }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
		};
		virtual bool GetBoundingRect(CRect2d& rectBoundary) const override {
			bool b{};
			for (auto const& bound : boundaries) {
				b |= bound.GetBoundingRect(rectBoundary);
			}
			return b;
		}
		virtual void Draw(ICanvas& canvas) const override {
			// todo : draw hatch
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_hatch);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & name;
			ar & bSolid;
			ar & associative;
			ar & hstyle;
			ar & hpattern;
			ar & doubleflag;
			ar & angle;
			ar & scale;
			ar & deflines;
		}
	};


	struct GTL_SHAPE_CLASS s_drawing : public s_shape {

		boost::ptr_deque<s_layer> layers;

		s_drawing() {
			layers.push_back(std::make_unique<s_layer>(L"0"));
		}

		//virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual void FlipX() override { for (auto& layer : layers) layer.FlipX(); }
		virtual void FlipY() override { for (auto& layer : layers) layer.FlipY(); }
		virtual void FlipZ() override { for (auto& layer : layers) layer.FlipZ(); }
		virtual void Transform(CCoordTrans3d const& ct, bool bRightHanded) override {
			for (auto& layer : layers)
				layer.Transform(ct, bRightHanded);
		}
		virtual bool GetBoundingRect(CRect2d& rect) const override {
			bool r{};
			for (auto& layer : layers)
				r |= layer.GetBoundingRect(rect);
			return r;
		}
		virtual void Draw(ICanvas& canvas) const override {
			for (auto& layer : layers) {
				canvas.PreDraw(layer);
				layer.Draw(canvas);
				canvas.PostDraw(layer);
			}
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_drawing);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & boost::serialization::base_object<s_shape>(*this);

			ar & layers;
		}

	public:
		virtual bool FromJson(boost::json::value& _j, string_t& layer) override {
			//s_shape::FromJson(_j, layer);

			return true;
		}

	};


}
