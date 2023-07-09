#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/gtl.h"
#include "gtl/reflection_glaze.h"
#include <QGraphicsView>

namespace gtl::qt {

	using namespace std::literals;

	class GTL__QT_CLASS xMatView : public QGraphicsView {
		Q_OBJECT

	public:
		using this_t = xMatView;
		using base_t = QGraphicsView;

		using string_t = std::string;
		using xCoordTrans = gtl::xCoordTrans2d;

	public:
		// ZOOM
		enum class eZOOM : int8_t { none = -1, one2one, fit2window, fit2width, fit2height, mouse_wheel_locked, free };	// lock :
		enum class eZOOM_IN : uint8_t { nearest, linear, bicubic, lanczos4/* EDSR, ESPCN, FSRCNN, LapSRN */};
		enum class eZOOM_OUT : uint8_t { nearest, area, };

		struct S_OPTION_flags {
			bool bZoomLock{false};				// if eZoom is NOT free, zoom is locked
			bool bPanningLock{true};			// image can be moved only eZoom == free
			bool bExtendedPanning{true};		// image can move out of screen
			bool bKeyboardNavigation{};			// page up/down, scroll down to next blank of image
			bool bDrawPixelValue{true};			// draw pixel value on image
			bool bPyrImageDown{true};			// build cache image for down sampling. (such as mipmap)
			bool bDrawRuler{true};				// draw ruler

			struct glaze {
				using T = S_OPTION_flags;
				static constexpr auto value = glz::flags(
					GLZ_FOR_EACH(GLZ_X, bZoomLock, bPanningLock, bExtendedPanning, bKeyboardNavigation, bDrawPixelValue,
						bPyrImageDown, bDrawRuler));
			};

		};

		struct S_OPTION : public S_OPTION_flags {
			double dMouseSpeed{2.0};			// Image Panning Speed. 1.0 is same with mouse move.
			int nScrollMargin{5};				// bExtendedPanning, px margin to scroll
			std::chrono::milliseconds tsScroll{250ms};	// Smooth Scroll. duration
			eZOOM_IN eZoomIn{eZOOM_IN::nearest};
			eZOOM_OUT eZoomOut{eZOOM_OUT::area};
			cv::Vec3b crBackground{161, 114, 230};	// rgb

			// Sync with json
			GLZ_LOCAL_META_DERIVED(S_OPTION, S_OPTION_flags, dMouseSpeed, nScrollMargin, tsScroll, eZoomIn, eZoomOut, crBackground);
		};
		std::string m_strCookie;
		std::function<bool(bool bStore, std::string_view cookie, S_OPTION&)> m_fnSyncSetting;

		struct S_SCROLL_GEOMETRY {
			xRect2i rectClient, rectImageScreen, rectScrollRange;
		};

	protected:
		cv::Mat m_imgOriginal;	// original image
		mutable cv::Mat m_img;	// image for screen
		mutable struct {
			std::mutex mtx;
			std::deque<cv::Mat> imgs;
			std::jthread threadPyramidMaker;
		} m_pyramid;
		mutable struct {
			bool bInSelectionMode{};
			bool bRectSelected{};
			std::optional<xPoint2i> ptAnchor;			// Anchor Point in Screen Coordinate
			xPoint2i ptOffset0;			// offset backup
			xPoint2d ptSel0, ptSel1;	// Selection Point in Image Coordinate
		} m_mouse;
		mutable struct {
			xPoint2d pt0, pt1;
			std::chrono::steady_clock::time_point t0, t1;
		} m_smooth_scroll;

		S_OPTION m_option;
		eZOOM m_eZoom{eZOOM::fit2window};
		xCoordTrans m_ctScreenFromImage;

	public:
		xMatView(QWidget *parent = nullptr) : QGraphicsView(parent) { Init(); }
		xMatView(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) { Init(); }

		~xMatView();

		void Init();

	public:
		bool SetImage(cv::Mat const& img, bool bCenter = true, eZOOM eZoomMode = eZOOM::none, bool bCopy = false);
		bool SetZoomMode(eZOOM eZoomMode, bool bCenter = true);
		std::optional<xRect2i> GetSelectionRect() const {
			if (!m_mouse.bRectSelected)
				return {};
			xRect2i rect(xPoint2i(gtl::Floor(m_mouse.ptSel0)), xPoint2i(gtl::Floor(m_mouse.ptSel1)));
			rect.NormalizeRect();
			return rect;
		}
		std::optional<std::pair<xPoint2d, xPoint2d>> GetSelectionPoints() const {
			if (!m_mouse.bRectSelected)
				return {};
			return std::pair<xPoint2d, xPoint2d>{m_mouse.ptSel0, m_mouse.ptSel1};
		}
		void SetSelectionRect(xRect2i const& rect);
		void ClearSelectionRect();

		bool LoadOption() { return m_fnSyncSetting and m_fnSyncSetting(false, m_strCookie, m_option) and SetOption(m_option, false); }
		bool SaveOption() { return m_fnSyncSetting and m_fnSyncSetting(true, m_strCookie, m_option); }
		S_OPTION const& GetOption() const { return m_option; }
		bool SetOption(S_OPTION const& option, bool bStore = true);

		bool ShowToolBar(bool bShow);
		//bool IsToolBarShown() const { return m_toolBar->IsShown(); }

		//virtual void OnClose(wxCloseEvent& event) override;

		// ClientRect, ImageRect, ScrollRange
		S_SCROLL_GEOMETRY GetScrollGeometry();
		bool UpdateCT(bool bCenter = false, eZOOM eZoom = eZOOM::none);
		bool UpdateScrollBars();
		bool ZoomInOut(double step, xPoint2i ptAnchor, bool bCenter);
		bool SetZoom(double scale, xPoint2i ptAnchor, bool bCenter);
		bool ScrollTo(xPoint2d pt, std::chrono::milliseconds tsScroll = -1ms);
		bool Scroll(xPoint2d delta, std::chrono::milliseconds tsScroll = -1ms);
		bool KeyboardNavigate(int key, bool ctrl = false, bool alt = false, bool shift = false);

	protected:
		bool MakeScreenCache(std::stop_token stop);
		//bool OnKeyEvent(wxKeyEvent& event);

	protected:
		//virtual void OnCharHook( wxKeyEvent& event ) override;
		//virtual void OnButtonClick_Hide( wxCommandEvent& event ) override;
		//virtual void OnCombobox_ZoomMode( wxCommandEvent& event ) override;
		//void OnSpinCtrl(double scale);
		//virtual void OnSpinCtrlDouble_ZoomValue( wxSpinDoubleEvent& event ) override;
		//virtual void OnTextEnter_ZoomValue( wxCommandEvent& event ) override;
		//virtual void OnZoomIn( wxCommandEvent& event ) override;
		//virtual void OnZoomOut( wxCommandEvent& event ) override;
		//virtual void OnZoomFit( wxCommandEvent& event ) override;
		//virtual void OnSettings( wxCommandEvent& event ) override;
		//virtual void OnCharHook_View( wxKeyEvent& event ) override { event.Skip();/*OnKeyEvent(event);*/ }
		//virtual void OnLeftDown_View( wxMouseEvent& event ) override;
		//virtual void OnLeftUp_View( wxMouseEvent& event ) override;
		//virtual void OnMiddleDown_View( wxMouseEvent& event ) override;
		//virtual void OnMiddleUp_View( wxMouseEvent& event ) override;
		//virtual void OnMotion_View( wxMouseEvent& event ) override;
		//virtual void OnMouseWheel_View( wxMouseEvent& event ) override;
		//virtual void OnPaint_View( wxPaintEvent& event ) override;
		//virtual void OnRightDown_View( wxMouseEvent& event ) override;
		//virtual void OnRightUp_View( wxMouseEvent& event ) override;
		//virtual void OnSize_View( wxSizeEvent& event ) override;
		//virtual void OnTimerScroll( wxTimerEvent& event ) override;
		//void OnScrollWin_View( wxScrollWinEvent& event);
		//void OnMouseCaptureLost_View(wxMouseCaptureLostEvent& event);
	};

} // namespace gtl::qt
