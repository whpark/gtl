module;

#include <cstdint>
#include <optional>
#include <vector>
#include <deque>

#include "gtl/gtl.h"

//#include "gtl/unit.h"
//#include "gtl/coord.h"

export module shape;

import gtl;

using namespace std::literals;
using namespace gtl::literals;

export namespace gtl::shape {

	using point_t = CPoint3d;
	struct line_t { point_t beg, end; };
	struct polypoint_t : public point_t {
		double bulge{};
	};

	using color_t = color_rgba_t;

	enum class eSHAPE : uint8_t { none, layer, dot, line, polyline, spline, circle, arc, ellipse, text, mtext, nSHAPE };
	constexpr color_t const CR_DEFAULT = RGBA(255, 255, 255);

	struct hatching_t {
		uint32_t eFlag{};
		double dInterval{};
	};

	struct s_shape {
		color_t color;
		std::optional<hatching_t> hatch;
	};

	struct s_dot : public s_shape {
		point_t pt;
	};

	struct s_line : public s_shape {
		point_t pt0, pt1;
	};

	struct s_polyline : public s_shape {
		std::vector<polypoint_t> pts;
	};

	struct s_circle : public s_shape {
		point_t center;
		double radius{};
		deg_t angle_length{360_deg};	// 회전 방향.
	};

	struct s_arc : public s_circle {
		deg_t angle_start{};
	};

	struct s_ellipse : public s_arc {
		double radius2{};
		deg_t angle_first_axis{};
	};

	struct s_spline : public s_shape {
	};

	struct s_text : public s_shape {
	};

	struct s_mtext : public s_shape {
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
