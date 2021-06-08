//////////////////////////////////////////////////////////////////////
//
// canvas.h:
//
// PWH
// 2017.07.20
// 2021.05.28 from QCanvas -> canvas
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#include <afxwin.h>

#include "shape_primitives.h"

namespace gtl::shape {
#pragma pack(push, 8)


	//=============================================================================================================================
	// CoordSystem

	using ct_t = CCoordTrans3d;

	////-------------------------------------------------------------------------
	///// @brief 3점(or 2점) 얼라인.
	//class CCoordSystem : public CCoordTransChain {
	//public:
	//	//ct_t ctShape2Real_;	// DXF 좌표계에서 실 좌표계.
	//	//ct_t ctAlign_;			// Align 미세 보정. (3 점 보정)
	//	//ct_t ctReal2Canvas_;	// 실좌표계 -> Canvas(ex, Scanner) 좌표계

	//public:
	//	//CCoordSystem() = default;
	//	//CCoordSystem(const CCoordSystem&) = default;
	//	//CCoordSystem& operator = (const CCoordSystem&) = default;
	//	//CQCoordSystem(CQCoordSystem&&) = default;
	//	//CQCoordSystem& operator = (CQCoordSystem&&) = default;

	//	GTL__DYNAMIC_VIRTUAL_BASE(CCoordSystem)
	//};

	extern GTL_SHAPE_API void Canvas_Spline(ICanvas& canvas, int degree, std::span<point_t const> pts, std::span<double const> knots, bool bLoop);

	//=============================================================================================================================
	// ICanvas : Interface of Canvas
	class ICanvas {
	protected:
		std::unique_ptr<ICoordTrans> rCT_, rCTI_;
		point_t ptLast_{};

	public:

		/// @brief for laser on/off
		double min_jump_length_ = 0.0;

		/// @brief target resolution
		double target_interpolation_inverval_{1.0};

	public:
		ICanvas() = default;
		ICanvas(ICanvas const&) = default;
		ICanvas(ICanvas &&) = default;
		ICanvas(ICoordTrans const& ct) : rCT_ {new_clone(ct)} {
			rCTI_ = ct.GetInverse();
		}
		virtual ~ICanvas() {}
		ICanvas& operator = (ICanvas const&) = default;
		ICanvas& operator = (ICanvas &&) = default;

		virtual void Init() {
			ptLast_ = point_t::All(std::nan(""));
			min_jump_length_ = 0.0;
			target_interpolation_inverval_ = 1.0;
		}

		void SetCT(ICoordTrans const& ct) {
			rCT_ = ct.NewClone();
			rCTI_ = ct.GetInverse();
		}
		ICoordTrans const* GetCT() { return rCT_.get(); }
		ICoordTrans const* GetCTI() { return rCTI_.get(); }

		// Scratching
		point_t Trans(point_t const& pt) { return (*rCT_)(pt); }
		point_t TransI(point_t const& pt) { return (*rCTI_)(pt); }

		virtual void MoveTo_Target(point_t const& ptTargetSystem) = 0;
		virtual void LineTo_Target(point_t const& ptTargetSystem) = 0;

		virtual void PreDraw(s_shape const&) = 0;

		void MoveTo(point_t pt) {
			pt = Trans(pt);
			if ( pt.IsAllValid() and (min_jump_length_ != 0.0) and (ptLast_.Distance(pt) < min_jump_length_) )
				return;
			MoveTo_Target(pt);
			ptLast_ = pt;
		}
		void LineTo(point_t pt) {
			pt = Trans(pt);
			LineTo_Target(pt);
			ptLast_ = pt;
		}

		virtual void Dot(point_t const& pt) {
			MoveTo(pt);
			LineTo(pt);
		}
		virtual void Line(point_t const& pt0, point_t const& pt1) {
			MoveTo(pt0);
			LineTo(pt1);
		}
		virtual void Arc(point_t const& ptCenter, double radius, deg_t t0, deg_t tLength) {
			int n = Round(tLength * std::numbers::pi * radius / target_interpolation_inverval_);
			deg_t t1 = t0+tLength;
			MoveTo(radius * point_t{cos(t0), sin(t0), .0}+ptCenter);
			for (int i = 1; i <= n; i++) {
				deg_t t {std::lerp(t0, t1, (double)i/n)};
				constexpr static auto m2pi = std::numbers::pi*2;
				double c = radius * cos(t);
				double s = radius * sin(t);
				CPoint2d pt(ptCenter.x + c, ptCenter.y+s);
				LineTo(pt);
			}
		}
		virtual void Ellipse(point_t const& ptCenter, double radius1, double radius2, deg_t tFirstAxis, deg_t t0, deg_t tLength) {
			CCoordTrans2d ct;
			ct.Init(1.0, (rad_t)tFirstAxis, point_t{}, ptCenter);

			deg_t t1 = t0 + tLength;
			MoveTo(ct(point_t{cos(t0), sin(t0)}));
			int n = Round(tLength * std::numbers::pi * std::max(radius1, radius2) / target_interpolation_inverval_);
			for (int i = 0; i <= n; i++) {
				deg_t t {std::lerp(t0, t1, (double)i/n)};
				constexpr static auto m2pi = std::numbers::pi*2;
				double c = cos(t);
				double s = sin(t);
				LineTo(ct(point_t{radius1*c, radius2*s}));
			}
		}
		virtual void Spline(int degree, std::span<point_t const> pts, std::span<double const> knots, bool bLoop) {
			Canvas_Spline(*this, degree, pts, knots, bLoop);
		}

	//public:
	//	virtual rad_t CalcArcInterval(double radius, double target_resolution) {
	//		auto scale = ct_(point_t::All(0)).Distance(ct_(point_t::All(1.)));
	//		return rad_t(target_resolution / (radius*scale*std::numbers::pi));
	//	}
	};

	//=============================================================================================================================
	// IQDeviceScanner : Interface of Canvas
	class GTL_SHAPE_CLASS ICanvasScanner : public ICanvas {
		friend class ICanvas;
	protected:
		ct_t::mat_t matTrans_ = ct_t::mat_t::eye();

	public:
		ICanvasScanner() {
		}
		virtual ~ICanvasScanner() {
		}

	public:
		//virtual void PreDraw(s_shape const& shape) {}

		virtual void PreDraw(s_shape const&) override {}

		virtual void MoveTo_Target(point_t const& pt) override {
			// Scanner Jump To
		}
		virtual void LineTo_Target(point_t const& pt) override {
			// Scanner Write To
		}
	protected:
	};


	//=============================================================================================================================
	// ICanvas : Interface of Canvas
	class GTL_SHAPE_CLASS CCanvasMat : public ICanvas {
	public:
		cv::Mat& img_;

		cv::Scalar color_{};
		double line_thickness_ {1.};

		// line thickness
		int line_type_ = cv::LINE_8;

	public:
		CCanvasMat(cv::Mat& img) : img_(img) {
		}
		virtual ~CCanvasMat() {
		}

		virtual void PreDraw(s_shape const& shape) override {
			color_ = ColorS(shape.color);
		}

		// Primative, returns End Position.
		virtual void MoveTo_Target(point_t const& pt) override {}
		virtual void LineTo_Target(point_t const& pt) override {
			cv::line(img_, ptLast_, pt, color_, (int)line_thickness_, line_type_);
			//cv::line(img_, cv::Point(ptLast_.x, ptLast_.y), cv::Point(pt.x, pt.y), color_, (int)line_thickness_, line_type_);
			//cv::line(img_, cv::Point2d(ptLast_.x, ptLast_.y), cv::Point2d(pt.x, pt.y), color_, (int)line_thickness_, line_type_);
		}

		//inline auto ColorS() { return ColorS(cr_); }
		inline cv::Scalar ColorS(color_t cr) { return cv::Scalar(cr.b, cr.g, cr.r); }

		//// additional...
		//virtual void MoveRelTo(const point_t& pt) { MoveTo(m_ptLast+pt); }
		//virtual void LineRelTo(const point_t& pt, bool bShowDirection = false) { LineTo(m_ptLast + pt, bShowDirection); }
		//virtual void ArcRelTo(const point_t& ptCenter, deg_t dTLength) { ArcTo(m_ptLast + ptCenter, dTLength); }
	};


	//=============================================================================================================================
	// ICanvas : Interface of Canvas
	class GTL_SHAPE_CLASS CCanvasMat_RoundDown : public CCanvasMat {
	public:
		using base_t = CCanvasMat;
	public:
		using base_t::base_t;

		// Primative, returns End Position.
		virtual void LineTo_Target(point_t const& pt) override {
			cv::line(img_, cv::Point((int)ptLast_.x, (int)ptLast_.y), cv::Point((int)pt.x, (int)pt.y), color_, (int)line_thickness_, line_type_);
		}
	};



	////-----------------------------------------------------------------------------
	//// ICanvas : Interface of Canvas
	//class AFX_EXT_CLASS_QSHAPE ICanvas {
	//protected:
	//	std::unique_ptr<IQDevice> m_rDevice;

	//protected:
	//	COLORREF m_cr;
	//	QSCoordTrans m_ct;
	//	double m_dErrorTolerance;
	//	point_t m_ptLast;

	////protected:
	////	COLORREF m_crArrow = RGB(255, 255, 255);
	////	bool m_bShowDirection = false;
	////	bool m_bShowOrder = false;
	////	int m_iOrder = 0;
	////	int m_iArrowSize = 5;
	////	int m_iArrowThickness = 1;

	//public:
	//	ICanvas() {
	//	}
	//	virtual ~ICanvas() {}
	//	ICanvas(const ICanvas& ) = default;

	//	virtual bool CreateCanvas(const QSRect& rectSource, const QSRect& rectTarget, bool bKeepAspectRatio = true);
	//	virtual void PreDraw() {
	//		//m_iOrder = 0;
	//		memset(m_ptLast.val, 0xff, sizeof(m_ptLast.val));	// not zero. set INVALID_VALUE
	//	}

	//	//void ShowDirection(bool bShow = true, bool bShowOrder = true, COLORREF crArrow = RGB(255, 0, 0), int iArrowSize = 16, int iArrowThickness = 1) {
	//	//	m_bShowDirection = bShow;
	//	//	m_bShowOrder = bShowOrder;
	//	//	m_crArrow = crArrow;
	//	//	m_iArrowSize = iArrowSize;
	//	//	m_iOrder = 0;
	//	//	m_iArrowThickness = iArrowThickness;
	//	//}
	//	//bool StopDrawingDirection();
	//	//bool ResumeDrawingDirection();
	//	//bool IsDirectionVisible() const { return m_bShowDirection; }

	//public:
	//	virtual void SetColor(COLORREF cr) { m_cr = cr; }

	//	// Primative
	//	void MoveTo(const point_t& pt);
	//	void LineTo(const point_t& pt, bool bShowDirection = FALSE);

	//	virtual void Draw();
	//	//virtual void Dot(const point_t& pt, DWORD dwDuration = 0 /* in usec */);
	//	//virtual void Line(const QSLine& pts, bool bLoop = FALSE);
	//	//virtual void PolyLine(const QSLine& pts, const std::vector<double>& dBulges, bool bLoop);
	//	virtual void Spline(const QSLine& pts, const std::vector<double>& dKnots, bool bLoop);
	//	virtual void Arc(const point_t& ptCenter, double dRadius, double dT0, double dTLength);
	//	virtual void Ellipse(const point_t& ptCenter, double dRadius1, double dRadius2, double dTFirstAxis, double dT0, double dTLength);

	//	//-----------------------
	//	// Hatch :
	//	//
	//	//	linePolygon : 시작과 끝점이 다를 경우, 첫번째 점과 마지막점을 연결하게 됨. 점의 갯수가 최소한 3개 이상 있어야 함.
	//	//
	//	//	eHatching :	SH_NONE
	//	//				SH_L2R
	//	//				SH_T2B
	//	//				SH_LT2RB
	//	//				SH_LB2RT
	//	virtual void Hatch(const TList<QSLine>& lines, DWORD eHatching, double dHatching); 

	//	//-----------------------
	//	// Text :
	//	//	pt : 기준 점
	//	//
	//	//	dwAlign :	DT_TOP
	//	//				DT_LEFT
	//	//				DT_CENTER
	//	//				DT_RIGHT
	//	//				DT_VCENTER
	//	//				DT_BOTTOM
	//	//
	//	//	lf : 크기는 무시됨. (기본 72 point(720) 으로 만들어야 함)
	//	//
	//	virtual void Text(LPCWSTR pszText, const point_t& pt, DWORD dwAlign /*DT_...*/, double dHeight, double dLineSpacingFactor, const LOGFONT& lf, DWORD eHatching, double dHatching);
	//};

	////-----------------------------------------------------------------------------
	//// CCanvasLineExtractor
	//class AFX_EXT_CLASS_QSHAPE CQCanvasLineExtractor : public ICanvas {
	//public:
	//	QSLines& m_lines;
	//
	//public:
	//	CQCanvasLineExtractor(QSLines& lines) : m_lines(lines) {
	//	}
	//	virtual ~CQCanvasLineExtractor() {}
	//
	//public:
	//	virtual void MoveToImpl(const point_t& pt) {
	//		if (m_lines.empty() || !m_lines.back().empty())
	//			m_lines.push_back(QSLine());
	//	}
	//	virtual void LineToImpl(const point_t& pt) {
	//		if (m_lines.empty())
	//			m_lines.push_back(QSLine());
	//
	//		QSLine& line = m_lines.back();
	//		if (!line.size())
	//			line.push_back(m_ct(m_ptLast));
	//		line.push_back(m_ct(pt));
	//	}
	//};

	////-----------------------------------------------------------------------------
	//// CCanvasMat
	//class CCanvaMat : public ICanvas {
	//public:
	//	cv::Size m_sizeEffective;
	//	cv::Mat m_mat;
	//protected:
	//	cv::Scalar m_crBGR;

	//public:
	//	CCanvaMat() {
	//	}
	//	CCanvaMat(const CCanvaMat& B) = delete;
	//	virtual ~CCanvaMat() {}

	//	virtual bool CreateCanvas(const CRect2i& rectSource, const CRect2i& rectTarget, bool bKeepAspectRatio = true);
	//};

	//-----------------------------------------------------------------------------
	//
	//GTL_API std::vector<point_t> AddLaserOffsetToLine(std::span<point_t const> pts, double dThickness, bool bLoop);


}


#pragma pack(pop)
