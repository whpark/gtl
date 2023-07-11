#pragma once

#include <QWidget>
#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/reflection_glaze.h"

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
	using string_t = std::wstring;
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

		struct glaze {
			using T = S_OPTION_flags;
			static constexpr auto value = glz::flags(GLZ_FOR_EACH(GLZ_X,
				bZoomLock, bPanningLock, bExtendedPanning, bKeyboardNavigation, bDrawPixelValue, bPyrImageDown));
		};
	};
	struct S_OPTION : public S_OPTION_flags{
		double dPanningSpeed{2.0};			// Image Panning Speed. 1.0 is same with mouse move.
		int nScrollMargin{5};				// bExtendedPanning, px margin to scroll
		std::chrono::milliseconds tsScroll{250ms};	// Smooth Scroll. duration
		eZOOM_IN eZoomIn{eZOOM_IN::nearest};
		eZOOM_OUT eZoomOut{eZOOM_OUT::area};
		cv::Vec3b crBackground{161, 114, 230};	// rgb

		// Sync with json
		GLZ_LOCAL_META_DERIVED(S_OPTION, S_OPTION_flags, bZoomLock, bPanningLock);

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

	cv::Mat m_imgOriginal;	// original image
	mutable cv::Mat m_imgScreen;	// image for screen
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
		void Clear() {
			pt0 = pt1 = {};
			t0 = t1 = {};
		}
	} m_smooth_scroll;

	S_OPTION m_option;
	eZOOM m_eZoom{eZOOM::fit2window};
	xCoordTrans m_ctScreenFromImage;

public:
	xMatView(QWidget* parent = nullptr);
	~xMatView();

	bool Init();

protected:
	void OnSettings();

private:
	std::unique_ptr<Ui::MatViewClass> ui;
};
