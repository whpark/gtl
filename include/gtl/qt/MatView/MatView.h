#pragma once

//#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QWidget>
#include <QTimer>
#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/reflection_glaze.h"

#include "MatViewCanvas.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MatViewClass; };
QT_END_NAMESPACE

namespace gtl::qt {
	using namespace std::literals;
	class GTL__QT_CLASS xMatView;
}

class gtl::qt::xMatView : public QWidget {
	Q_OBJECT

public:
	using this_t = xMatView;
	using base_t = QWidget;

	using string_t = std::wstring;
	using xCoordTrans = gtl::xCoordTrans2d;

public:
	// ZOOM
	enum class eZOOM : int8_t { none = -1, one2one, fit2window, fit2width, fit2height, mouse_wheel_locked, free };	// lock : 
	enum class eZOOM_IN : uint8_t { nearest, linear, bicubic, lanczos4/* EDSR, ESPCN, FSRCNN, LapSRN */};
	enum class eZOOM_OUT : uint8_t { nearest, area, };

	struct S_OPTION {
		bool bZoomLock{false};				// if eZoom is NOT free, zoom is locked
		bool bPanningLock{true};			// image can be moved only eZoom == free
		bool bExtendedPanning{true};		// image can move out of screen
		bool bKeyboardNavigation{};			// page up/down, scroll down to next blank of image
		bool bDrawPixelValue{true};			// draw pixel value on image
		bool bPyrImageDown{true};			// build cache image for down sampling. (such as mipmap)
		double dPanningSpeed{2.0};			// Image Panning Speed. 1.0 is same with mouse move.
		int nScrollMargin{5};				// bExtendedPanning, px margin to scroll
		std::chrono::milliseconds tsScroll{250ms};	// Smooth Scroll. duration
		eZOOM_IN eZoomIn{eZOOM_IN::nearest};
		eZOOM_OUT eZoomOut{eZOOM_OUT::area};
		cv::Vec3b crBackground{0, 0, 0};	// rgb	//{161, 114, 230}

		// Sync with json
		GLZ_LOCAL_META(S_OPTION, bZoomLock, bPanningLock, bExtendedPanning, bKeyboardNavigation, bDrawPixelValue,
			bPyrImageDown, dPanningSpeed, nScrollMargin, tsScroll, eZoomIn, eZoomOut, crBackground);
	};

	std::string m_strCookie;
	std::function<bool(bool bStore, std::string_view cookie, S_OPTION&)> m_fnSyncSetting;

	struct S_SCROLL_GEOMETRY {
		xRect2i rectClient, rectImageScreen, rectScrollRange;
	};

protected:
	//// gl
	//std::unique_ptr<wxGLContext> m_context;
	////GLuint m_textureID{};

	struct {
		std::unique_ptr<QOpenGLExtraFunctions> gl{};
		GLuint shaderProgram{};
		GLuint VBO{}, VAO{};
		QOpenGLExtraFunctions& operator () () { return *gl; }
		QOpenGLExtraFunctions const& operator () () const { return *gl; }

		void Clear() {
			if (gl) {
				if (auto r = std::exchange(shaderProgram, 0)) {
					gl->glDeleteProgram(r);
				}
				if (auto r = std::exchange(VBO, 0)) {
					gl->glDeleteBuffers(1, &r);
				}
				if (auto r = std::exchange(VAO, 0)) {
					gl->glDeleteVertexArrays(1, &r);
				}
			}
			gl.reset();
		}
	} m_gl;

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
		void Clear() {
			bInSelectionMode = {};
			bRectSelected = {};
			ptAnchor.reset();
			ptOffset0 = {};
			ptSel0 = ptSel1 = {};
		}
	} m_mouse;
	mutable struct {
		xPoint2d pt0, pt1;
		std::chrono::steady_clock::time_point t0, t1;
		QTimer timer;
		void Clear() {
			pt0 = pt1 = {};
			t0 = t1 = {};
			timer.stop();
		}
	} m_smooth_scroll;

	//xMatViewCanvas* m_view{};

	S_OPTION m_option;
	eZOOM m_eZoom{eZOOM::fit2window};
	xCoordTrans m_ctScreenFromImage;
	mutable bool m_bSkipSpinZoomEvent{};

public:
	xMatView(QWidget* parent = nullptr);
	~xMatView();

public:
	cv::Mat GetImage() { return m_imgOriginal; }
	cv::Mat const& GetImage() const { return m_imgOriginal; }
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
	bool IsToolBarShown() const;

	//virtual void OnClose(wxCloseEvent& event) override;

	// ClientRect, ImageRect, ScrollRange
	S_SCROLL_GEOMETRY GetScrollGeometry();
	bool UpdateCT(bool bCenter = false, eZOOM eZoom = eZOOM::none);
	bool UpdateScrollBars();
	bool ZoomInOut(double step, xPoint2i ptAnchor, bool bCenter);
	bool SetZoom(double scale, xPoint2i ptAnchor, bool bCenter);
	bool ScrollTo(xPoint2d pt, std::chrono::milliseconds tsScroll = -1ms);
	bool Scroll(xPoint2d delta, std::chrono::milliseconds tsScroll = -1ms);
	void PurgeScroll(bool bUpdate = true);
	bool KeyboardNavigate(int key, bool ctrl = false, bool alt = false, bool shift = false);

protected:
	void BuildPyramid();
	xRect2i GetViewRect();
	void InitializeGL(xMatViewCanvas* view);
	void PaintGL(xMatViewCanvas* view);
	bool PutMatAsTexture(GLuint textureID, cv::Mat const& img, int width, gtl::xRect2i const& rect, gtl::xRect2i const& rectClient);

protected:
	virtual void keyPressEvent(QKeyEvent *event) override;
	void OnView_mousePressEvent(xMatViewCanvas* view, QMouseEvent *event);
	void OnView_mouseReleaseEvent(xMatViewCanvas* view, QMouseEvent *event);
	void OnView_mouseMoveEvent(xMatViewCanvas* view, QMouseEvent *event);
	void OnView_wheelEvent(xMatViewCanvas* view, QWheelEvent* event);

protected:
	// slots
	void OnCmbZoomMode_currentIndexChanged(int index);
	void OnSpinZoom_valueChanged(double value);
	void OnBtnZoomIn_clicked();
	void OnBtnZoomOut_clicked();
	void OnBtnZoomFit_clicked();
	void OnBtnCountColor_clicked();	// NOT Connected with button
	void OnBtnSettings_clicked();
	void OnSmoothScroll_timeout();
	void OnView_resized();
	void OnSbHorz_valueChanged(int value);
	//void OnSbHorz_sliderMoved(int value);
	void OnSbVert_valueChanged(int value);
	//void OnSbVert_sliderMoved(int value);

private:
	std::unique_ptr<Ui::MatViewClass> ui;
};
