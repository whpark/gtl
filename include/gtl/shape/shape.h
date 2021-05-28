//////////////////////////////////////////////////////////////////////
//
// shape.h:
//
// 2017.07.20
// PWH
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*

	1. CShapeObject 에서 변경 내용 :

		class name :

			CShapeObject	-> CQShape
			CShapeLine		-> CQSLine
			CShapePolyLine	-> CQSPolyline
			...

		Container :

			TList -> boost::ptr_deque

		Point / Line : x, y, z 3차원으로 변경

			typedef CSize3d					QSSize;
			typedef CPoint3d				QSPoint;
			typedef CRect3d					QSRect;
			typedef std::vector<QSLine>		QSLines;

			typedef CCoordTrans3d			CQSCoordTrans;



*/
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "shape_default.h"
#include "canvas.h"

//export module shape;

namespace gtl::shape {

	struct s_layer : public s_shape {
		std::wstring name;

		boost::ptr_deque<s_shape> shapes;

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
			ar & (s_shape&)*this;

			ar & name;
			ar & shapes;
		}

	};

	struct s_dot : public s_shape {
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
			ar & (s_shape&)*this;

			ar & pt;
		}

	};

	struct s_line : public s_shape {
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
			ar & (s_shape&)*this;
			ar & pt0 & pt1;
		}
	};

	struct s_polyline : public s_shape {
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
			ar & (s_shape&)*this;
			ar & pts;
		}
	};

	struct s_circleXY : public s_shape {
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
			ar & (s_shape&)*this;

			ar & ptCenter & radius & (double&)angle_length;
		}
	};

	struct s_arcXY : public s_circleXY {
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
			ar & (s_circleXY&)*this;

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

	struct s_ellipseXY : public s_arcXY {
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
			ar & (s_arcXY&)*this;
			ar & radiusH & (double&)angle_first_axis;
		}
	};

	struct s_spline : public s_shape {
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
		};
		virtual void Draw(ICanvas& canvas) const override {

		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_spline);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_shape&)*this;

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

	struct s_text : public s_shape {
		enum class eVERT_ALIGN : int { base_line = 0, bottom, mid, top };
		enum class eHORZ_ALIGN : int { left = 0, center, right, aligned, middle, fit };

		point_t pt0, pt1;
		std::wstring text;
		double height{};
		deg_t angle{};
		double widthScale{};
		deg_t oblique{};
		std::u8string textStyle;
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
			ar & (s_shape&)*this;

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

	struct s_mtext : public s_text {
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

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_mtext);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_text&)*this;

			ar & (int&)eAttch;
			ar & interlin;
		}
	};

	struct s_hatch : public s_shape {
		std::u8string name;
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
		}

		GTL__DYNAMIC_VIRTUAL_DERIVED(s_hatch);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_shape&)*this;

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


	void s_polyline::Draw(ICanvas& canvas) const {
		for (int iPt = 0; iPt < pts.size(); iPt++) {
			polypoint_t pt0{pts[iPt]};
			polypoint_t pt1;
			bool bLast = false;
			if (bLoop) {
				if (iPt == pts.size()-1) {
					pt1 = pts[0];
					bLast = true;
				} else {
					pt1 = pts[iPt+1];
				}
			} else {
				if (iPt == pts.size()-1)
					break;
				bLast = iPt == pts.size()-2;
				pt1 = pts[iPt+1];
			}

			double dBulge = pt0.Bulge();

			if (dBulge == 0.0) {
				canvas.LineTo(pt1);
			} else {
				canvas.LineTo(pt0);
				s_arcXY arc = s_arcXY::GetFromBulge(dBulge, pt0, pt1);

				arc.Draw(canvas);

				canvas.LineTo(pt1);
			}
		}
	}

#if 0
	/// @brief Shape class
	class CShape : public s_shape {
	public:
		eSHAPE const eShape_;

		// Constructor
	protected:
		CShape(eSHAPE eShape, color_t cr = CR_DEFAULT) : eShape_(eShape), s_shape{cr} {
		}
	public:
		CShape(const CShape& B) = delete;
		virtual ~CShape(void) {
		}

	public:
		GTL__DYNAMIC_VIRTUAL_INTERFACE;
		GTL_DYN__BASE(eSHAPE);

	public:
		// default assign operator
		CShape& operator = (const CShape& B) { return CopyFrom(B); }
		virtual CShape& CopyFrom(const CShape& B) {
			if (this == &B)
				return *this;
			if (eShape_ != B.eShape_)
				throw std::invalid_argument(fmt::format("Cannot copy from {} to {}", B.eShape_, eShape_));

			return *this;
		}
		virtual CShape& CopyAttrFrom(const CShape& B) {
			if (this == &B)
				return *this;
			m_cr = B.m_cr;
			m_eHatching = B.m_eHatching;
			m_dHatching = B.m_dHatching;
			return *this;
		}

		virtual CArchive& Serialize(CArchive& ar) {
			if (ar.IsStoring()) {
				ar << m_cr;
				ar << m_eHatching;
				ar << m_dHatching;
				//ar << m_strCookie;
			} else {
				ar >> m_cr;
				ar >> m_eHatching;
				ar >> m_dHatching;
				//ar >> m_strCookie;
			}
			return ar;
		}

	public:
		BOOL operator == (const CShape& B) const { return Compare(B); }

		// Attributes
		static LPCTSTR GetShapeName(eSHAPE eShape);
		virtual LPCTSTR GetShapeName() const;
		eSHAPE GetShape() const { return m_eShape; }
		COLORREF GetColor() const { return m_cr; }
		virtual COLORREF SetColor(COLORREF cr);

		virtual DWORD GetHatching() const { return m_eHatching; }
		virtual double GetHatchingDensity() const { return m_dHatching; }
		virtual void SetHatching(DWORD eHatching = SH_NONE, double dHatching = 0.0) { m_eHatching = eHatching; m_dHatching = dHatching; }

		virtual CString Print() const = NULL;

	public:
		// Operations
		virtual CPoint2d GetCenterPoint() const { TRectD rect; GetBoundingRect(rect); return rect.CenterPoint(); };
	private:
		void GetPointsConst(TList<TLineD>& lines) const { GetPoints(lines); }	// Helper
	public:
		virtual void GetPoints(TList<TLineD>& lines) const = NULL;						// Bounding Rect계산용. Add Points incrementally. DO NOT RESET pts (DO NOT CALL pts.DeleteAll())
		virtual void GetPoints(TList<TLineD>& lines) { GetPointsConst(lines) ; }			// Bounding Rect계산용. Add Points incrementally. DO NOT RESET pts (DO NOT CALL pts.DeleteAll())
		virtual BOOL SetFromPoints(const TLineD& line) { return FALSE; }
		virtual void GetBoundingRect(TRectD& rect, BOOL bResetRect = TRUE) const;
		virtual BOOL GetStartEndPoint(CPoint2d& pt0, CPoint2d& pt1) const = NULL;
		virtual BOOL Compare(const CShape& B) const;

		virtual BOOL AddLaserOffset(double dThickness) = NULL;

		// Transform Operations
		virtual void Revert() = NULL;														// 가공 순서 Reverse
		virtual void AddOffset(const CPoint2d& ptOffset) = NULL;								// Object Center 이동.
		virtual void FlipX(const CPoint2d& ptCenter) = NULL;									// Object Center 중심으로 Y축 반전. (X값 변경)
		virtual void FlipY(const CPoint2d& ptCenter) = NULL;									// Object Center 중심으로 X축 반전. (Y값 변경)
		virtual void Rotate(rad_t dTheta, const CPoint2d& ptCenter) = NULL;					// Object Center 중심으로 회전
		virtual void Resize(double dScale, const CPoint2d& ptCenter) = NULL;					// Object Center 중심으로 크기 변환
		virtual void Resize(double dScaleX, double dScaleY, const CPoint2d& ptCenter) = NULL;// Object Center 중심으로 크기 변환
		virtual BOOL Deflate(double dSizeX, double dSizeY) = NULL;							// 줄어드는 양. (원 Size가 10일경우, 2을 넣으면 8가 됨. 양쪽으로 2 씩 줄일 경우, *2 해서, 4를 넣어야 함)
		virtual BOOL ClipByRect(const CRect2d& rect, TRefList<CShape>& objects) = NULL;	// by 이범호
		enum eSORT_DIRECTION {
			SD_AS_IS, SD_ANY,
			SD_RIGHT, SD_LEFT, SD_UP, SD_DOWN,
			SD_RIGHT_UP, SD_RIGHT_DOWN, SD_LEFT_UP, SD_LEFT_DOWN,
			SD_UP_RIGHT, SD_UP_LEFT, SD_DOWN_RIGHT, SD_DOWN_LEFT,
			SD_CCW, SD_CW,
			nSD
		};
		virtual void ChangeDirection(eSORT_DIRECTION eDirection, const CPoint2d& ptCenter = CPoint2d()) = NULL;
		static LPCTSTR GetDirectionName(eSORT_DIRECTION eDirection);

	public:
		// Drawing
		virtual BOOL Draw(ISCanvas& canvas) const = NULL;

	public:
		// Hepler
		static BOOL GetBoundingRect(TRectD& rect, const CPoint2d& pt, BOOL bResetRect = TRUE);
		static BOOL GetBoundingRect(TRectD& rect, const TLineD& pts, BOOL bResetRect = TRUE);
		static BOOL GetBoundingRect(TRectD& rect, const TList<TLineD>& lines, BOOL bResetRect = TRUE);
		static BOOL GetBoundingRect(TRectD& rect, const TRectD& rectObject, BOOL bResetRect = TRUE);

		static BOOL MergeLines(TRefList<CShape>& objects, double dMaxSizeX = 0.0, double dMaxSizeY = 0.0);	// 시작/끝점이 겹치는 line object 를 하나로 합침
	};

	typedef CShape* PShapeObject;

	//-----------------------------------------------------------------------------

	AFX_EXT_API_SHAPE int CompareObjectTolerance_PXPY(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectTolerance_PXNY(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectTolerance_NXPY(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectTolerance_NXNY(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectTolerance_PYPX(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectTolerance_PYNX(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectTolerance_NYPX(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectTolerance_NYNX(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);

	AFX_EXT_API_SHAPE int CompareObjectX(const CShape& ob1, const CShape& ob2);
	AFX_EXT_API_SHAPE int CompareObjectY(const CShape& ob1, const CShape& ob2);
	AFX_EXT_API_SHAPE int CompareObjectXD(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareObjectYD(const CShape& ob1, const CShape& ob2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareArcX(const CShape& arc1, const CShape& arc2);
	AFX_EXT_API_SHAPE int CompareArcY(const CShape& arc1, const CShape& arc2);
	AFX_EXT_API_SHAPE int CompareLineX(const CShape& line1, const CShape& line2);
	AFX_EXT_API_SHAPE int CompareLineY(const CShape& line1, const CShape& line2);
	AFX_EXT_API_SHAPE int CompareLineXD(const CShape& line1, const CShape& line2, void* pdToleranceLineDistance);
	AFX_EXT_API_SHAPE int CompareLineYD(const CShape& line1, const CShape& line2, void* pdToleranceLineDistance);

	//-----------------------------------------------------------------------------
	BOOL AFX_EXT_API_SHAPE AddLaserOffsetToLine(const TLineD& pts, TLineD& ptsResult, double dThickness, BOOL bLoop);

#endif

}
