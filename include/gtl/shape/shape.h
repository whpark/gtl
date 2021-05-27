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

#include <cstdint>
#include <optional>
#include <vector>
#include <deque>
#include "gtl/unit.h"
#include "gtl/coord.h"
#include "gtl/dynamic.h"

#include "boost/ptr_container/ptr_container.hpp"
#include "boost/serialization/string.hpp"
#include "boost/serialization/serialization.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"

//export module shape;

namespace gtl {

	template < typename archive >
	void serialize(archive& ar, gtl::mm_t& len, unsigned int const file_version) {
		ar & len.dValue;
	}

	template < typename archive >
	void serialize(archive& ar, gtl::deg_t& len, unsigned int const file_version) {
		ar & len.dValue;
	}

	template < typename archive >
	void serialize(archive& ar, gtl::rad_t& len, unsigned int const file_version) {
		ar & len.dValue;
	}
};

namespace gtl::shape {

	using namespace gtl::literals;

	using point_t = CPoint3d;
	struct line_t { point_t beg, end; };
	struct polypoint_t : public TPointT<double, 4> {
		double& Bulge() { return w; }
		double Bulge() const { return w; }
	};

	using color_t = color_rgba_t;

	enum class eSHAPE : uint8_t { none, layer, dot, line, polyline, spline, circle, arc, ellipse, text, mtext, nSHAPE };
	constexpr color_t const CR_DEFAULT = RGBA(255, 255, 255);

	//struct hatching_t {
	//	uint32_t eFlag{};
	//	double dInterval{};
	//};
	struct cookie_t {
		void* ptr{};
		std::vector<uint8_t> buffer;
		std::u8string str;
		std::chrono::nanoseconds duration;

		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (ptrdiff_t&)ptr;
			ar & buffer;
			ar & str;
			ar & duration;
		};
	};

	struct s_shape {
		color_t color;
		//std::optional<hatching_t> hatch;
		std::optional<cookie_t> cookie;

		virtual ~s_shape() {}

		GTL__VIRTUAL_DYNAMIC_INTERFACE(s_shape);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & color;
			//ar & hatch;
			ar & cookie;
		}

		virtual point_t PointAt(double t) const = 0;
		virtual bool Transform(CCoordTrans3d const&) = 0;
		virtual bool GetBoundingRect(CRect2d&) const = 0;


	};

	struct s_layer : public s_shape {
		std::wstring name;

		boost::ptr_deque<s_shape> shapes;

		virtual point_t PointAt(double t) const override { throw std::exception{"not here."}; return point_t {}; }	// no PointAt();
		virtual bool Transform(CCoordTrans3d const& ct) override {
			bool r{true};
			for (auto& shape : shapes)
				r &= shape.Transform(ct);
			return r;
		}
		virtual bool GetBoundingRect(CRect2d& rect) const override {
			bool r{};
			for (auto& shape : shapes)
				r |= shape.GetBoundingRect(rect);
			return r;
		}

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_layer);
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

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_dot);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_shape&)*this;

			ar & pt;
		}

	};

	struct s_line : public s_shape {
		point_t pt0, pt1;

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_line);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_shape&)*this;
			ar & pt0 & pt1;
		}
	};

	struct s_polyline : public s_shape {
		std::vector<polypoint_t> pts;

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_polyline);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_shape&)*this;
			ar & pts;
		}
	};

	struct s_circle : public s_shape {
		point_t center;
		double radius{};
		deg_t angle_length{360_deg};	// 회전 방향.

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_circle);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_shape&)*this;

			ar & center & radius & (double&)angle_length;
		}
	};

	struct s_arc : public s_circle {
		deg_t angle_start{};

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_arc);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_circle&)*this;

			ar & angle_start;
		}
	};

	struct s_ellipse : public s_arc {
		double radius2{};
		deg_t angle_first_axis{};

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_ellipse);
		friend class boost::serialization::access;
		template < typename archive >
		void serialize(archive& ar, unsigned int const file_version) {
			ar & (s_arc&)*this;
			ar & radius2 & (double&)angle_first_axis;
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

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_spline);
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
		deg_t oblique;
		std::u8string textStyle;
		int textgen{};
		eHORZ_ALIGN horzAlign{eHORZ_ALIGN::left};
		eVERT_ALIGN vertAlign{eVERT_ALIGN::base_line};
	
		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_text);
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

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_mtext);
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

		virtual point_t PointAt(double t) const override {};
		virtual bool Transform(CCoordTrans3d const&) override { return true; };
		virtual bool GetBoundingRect(CRect2d&) const override { return true; };

		GTL__VIRTUAL_DYNAMIC_DERIVED(s_hatch);
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
		GTL__VIRTUAL_DYNAMIC_INTERFACE;
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
