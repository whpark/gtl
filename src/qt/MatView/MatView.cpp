#include "pch.h"

#include "gtl/mat_gl.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/util.h"
#include "gtl/qt/MatView/MatView.h"
#include "ui_MatView.h"

#include <opencv2/cvconfig.h>
#ifdef HAVE_CUDA
#include <opencv2/cudawarping.hpp>
#endif

#include "MatViewSettingsDlg.h"

#include "spdlog/stopwatch.h"

namespace gtlq = gtl::qt;

namespace gtl::qt {

	static double const dZoomLevels[] = {
		1./8192, 1./4096, 1./2048, 1./1024,
		1./512, 1./256, 1./128, 1./64, 1./32, 1./16, 1./8, 1./4., 1./2.,
		3./4, 1, 1.25, 1.5, 1.75, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10,
		12.5, 15, 17.5, 20, 25, 30, 35, 40, 45, 50, 60, 70, 80, 90, 100,
		125, 150, 175, 200, 250, 300, 350, 400, 450, 500,
		600, 700, 800, 900, 1'000,
		//1250, 1500, 1750, 2000, 2500, 3000, 3500, 4000, 4500, 5000,
		//6000, 7000, 8000, 9000, 10000,
		//12500, 15000, 17500, 20000, 25000, 30000, 35000, 40000, 45000, 50000,
		//60000, 70000, 80000, 90000, 100000,
		//125000, 150000, 175000, 200000, 250000, 300000, 350000, 400000, 450000, 500000,
		//600000, 700000, 800000, 900000, 1000000,
		//1250000, 1500000, 1750000, 2000000, 2500000, 3000000, 3500000, 4000000, 4500000, 5000000,
		//6000000, 7000000, 8000000, 9000000, 10000000,
		//12500000, 15000000, 17500000, 20000000, 25000000, 30000000, 35000000, 40000000, 45000000, 50000000,
		//60000000, 70000000, 80000000, 90000000
	};

	xMatView::xMatView(QWidget* parent) : QWidget(parent), ui(std::make_unique<Ui::MatViewClass>()) {
		ui->setupUi(this);

		ui->cmbZoomMode->setCurrentIndex(std::to_underlying(m_eZoom));

		// openGL object
		//ui->view = std::make_unique<xMatViewCanvas>(this);
		if (auto* view = ui->view) {
			view->m_fnInitializeGL =	[this](auto* p) { this->InitializeGL(p); };
			view->m_fnPaintGL =			[this](auto* p) { this->PaintGL(p); };
			view->m_fnMousePress =		[this](auto* p, auto* e) { this->OnView_mousePressEvent(p, e); };
			view->m_fnMouseRelease =	[this](auto* p, auto* e) { this->OnView_mouseReleaseEvent(p, e); };
			view->m_fnMouseMove =		[this](auto* p, auto* e) { this->OnView_mouseMoveEvent(p, e); };
			view->m_fnWheel =			[this](auto* p, auto* e) { this->OnView_wheelEvent(p, e); };
			view->setMouseTracking(true);
		}
		//ui->view->setObjectName("view");
		//QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		//sizePolicy.setHorizontalStretch(0);
		//sizePolicy.setVerticalStretch(0);
		//sizePolicy.setHeightForWidth(false);
		//ui->view->setSizePolicy(sizePolicy);
		//ui->gridLayout->addWidget(ui->view.get(), 0, 0, 1, 1);

		if (m_fnSyncSetting) {
			m_fnSyncSetting(false, m_strCookie, m_option);
		}

		connect(ui->cmbZoomMode, &QComboBox::currentIndexChanged, this, &this_t::OnCmbZoomMode_currentIndexChanged);
		connect(ui->spinZoom, &QDoubleSpinBox::valueChanged, this, &this_t::OnSpinZoom_valueChanged);
		connect(ui->btnZoomIn, &QPushButton::clicked, this, &this_t::OnBtnZoomIn_clicked);
		connect(ui->btnZoomOut, &QPushButton::clicked, this, &this_t::OnBtnZoomOut_clicked);
		connect(ui->btnZoomFit, &QPushButton::clicked, this, &this_t::OnBtnZoomFit_clicked);
		connect(ui->btnSettings, &QPushButton::clicked, this, &this_t::OnBtnSettings_clicked);
		connect(ui->sbHorz, &QScrollBar::valueChanged, this, &this_t::OnSbHorz_valueChanged);
		connect(ui->sbHorz, &QScrollBar::sliderMoved, this, &this_t::OnSbHorz_valueChanged);
		connect(ui->sbVert, &QScrollBar::valueChanged, this, &this_t::OnSbVert_valueChanged);
		connect(ui->sbVert, &QScrollBar::sliderMoved, this, &this_t::OnSbVert_valueChanged);
		connect(&m_smooth_scroll.timer, &QTimer::timeout, this, &this_t::OnSmoothScroll_timeout);
		connect(ui->view, &QOpenGLWidget::resized, this, &this_t::OnView_resized);
	}

	xMatView::~xMatView() {
		m_gl.Clear();
	}

	bool xMatView::SetImage(cv::Mat const& img, bool bCenter, eZOOM eZoomMode, bool bCopy) {
		xWaitCursor wc;

		// original image
		if (bCopy)
			img.copyTo(m_imgOriginal);
		else
			m_imgOriginal = img;

		m_img = m_imgOriginal;
		BuildPyramid();

		// check (opengl) texture format
		auto [eColorType, eFormat, ePixelType] = GetGLImageFormatType(m_img.type());
		if (eColorType == 0)
			return false;

		m_mouse.Clear();
		m_smooth_scroll.Clear();

		//if (img.empty() and !m_img.isContinuous()) {
		//	return false;
		//}

		if (eZoomMode != eZOOM::none) {
			ui->cmbZoomMode->setCurrentIndex(std::to_underlying(eZoomMode));
		}
		UpdateCT(bCenter, eZoomMode);
		UpdateScrollBars();

		if (ui->view)
			ui->view->update();
		return true;
	}

	bool xMatView::SetZoomMode(eZOOM eZoomMode, bool bCenter) {
		ui->cmbZoomMode->setCurrentIndex(std::to_underlying(eZoomMode));
		m_eZoom = eZoomMode;
		UpdateCT(bCenter, eZoomMode);
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
		return true;
	}

	void xMatView::SetSelectionRect(xRect2i const& rect) {
		m_mouse.bRectSelected = true;
		m_mouse.ptSel0 = rect.pt0();
		m_mouse.ptSel1 = rect.pt1();
		if (ui->view)
			ui->view->update();
	}
	void xMatView::ClearSelectionRect() {
		m_mouse.bRectSelected = false;
	}

	bool xMatView::SetOption(S_OPTION const& option, bool bStore) {
		if (&m_option != &option)
			m_option = option;

		BuildPyramid();

		UpdateCT(false, eZOOM::none);
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();

		if (bStore) {
			return m_fnSyncSetting and m_fnSyncSetting(true, m_strCookie, m_option);
		}
		return true;
	}

	bool xMatView::ShowToolBar(bool bShow) {
		if (!ui->toolbar)
			return false;
		if (bShow)
			ui->toolbar->show();
		else
			ui->toolbar->hide();
		return true;
	}


	bool xMatView::UpdateCT(bool bCenter, eZOOM eZoom) {
		if (m_img.empty())
			return false;
		if (eZoom == eZOOM::none)
			eZoom = m_eZoom;
		ui->cmbZoomMode->setCurrentIndex(std::to_underlying(eZoom));

		xRect2i rectClient;
		rectClient = GetViewRect();
		xSize2i sizeClient;
		sizeClient = rectClient.GetSize();

		// scale
		double dScale{-1.0};
		switch (eZoom) {
			using enum eZOOM;
		case one2one:		dScale = 1.0; break;
		case fit2window:	dScale = std::min((double)sizeClient.cx / m_img.cols, (double)sizeClient.cy / m_img.rows); break;
		case fit2width:		dScale = (double)sizeClient.cx / m_img.cols; break;
		case fit2height:	dScale = (double)sizeClient.cy / m_img.rows; break;
		//case free:			dScale = m_ctScreenFromImage.m_scale; break;
		}
		if (dScale > 0)
			m_ctScreenFromImage.m_scale = dScale;

		// constraints. make image put on the center of the screen
		if ( bCenter or gtl::IsValueOneOf(eZoom, eZOOM::fit2window, eZOOM::fit2width, eZOOM::fit2height) ) {
			xCoordTrans ct2 = m_ctScreenFromImage;
			ct2.m_origin = xPoint2d(m_img.size())/2.;
			ct2.m_offset = xRect2d(rectClient).CenterPoint();

			xPoint2d ptOrigin = {};
			xPoint2i ptLT = ct2(ptOrigin);

			if (bCenter or eZoom == eZOOM::fit2window) {
				m_ctScreenFromImage.m_origin = {};
				m_ctScreenFromImage.m_offset = ptLT;
			}
			else if (eZoom == eZOOM::fit2width) {
				m_ctScreenFromImage.m_origin.x = 0;
				m_ctScreenFromImage.m_offset.x = ptLT.x;
			}
			else if (eZoom == eZOOM::fit2height) {
				m_ctScreenFromImage.m_origin.y = 0;
				m_ctScreenFromImage.m_offset.y = ptLT.y;
			}
		}

		// panning constraints.
		xPoint2d pt0 = m_ctScreenFromImage(xPoint2d(0,0));
		xPoint2d pt1 = m_ctScreenFromImage(xPoint2d(m_img.cols, m_img.rows));
		xRect2i rectImageScreen(pt0, pt1);
		rectImageScreen.NormalizeRect();
		if (m_option.bExtendedPanning) {
			int marginX {std::min(m_option.nScrollMargin, rectImageScreen.Width())};
			int marginY {std::min(m_option.nScrollMargin, rectImageScreen.Height())};
			// make any part of image stays inside the screen
			if (rectImageScreen.right < rectClient.left) {
				m_ctScreenFromImage.m_offset.x += rectClient.left - rectImageScreen.right + marginX;
			}
			else if (rectImageScreen.left > rectClient.right) {
				m_ctScreenFromImage.m_offset.x += rectClient.right - rectImageScreen.left - marginX;
			}
			if (rectImageScreen.bottom < rectClient.top) {
				m_ctScreenFromImage.m_offset.y += rectClient.top - rectImageScreen.bottom + marginY;
			}
			else if (rectImageScreen.top > rectClient.bottom) {
				m_ctScreenFromImage.m_offset.y += rectClient.bottom - rectImageScreen.top - marginY;
			}
		} else {
			// default panning. make image stays inside the screen
			if (rectImageScreen.Width() <= rectClient.Width()) {
				auto pt = m_ctScreenFromImage(xPoint2d(m_img.cols/2, 0));
				m_ctScreenFromImage.m_offset.x += rectClient.CenterPoint().x - pt.x;
			}
			if (rectImageScreen.Width() > rectClient.Width()) {
				if (rectImageScreen.left > rectClient.left)
					m_ctScreenFromImage.m_offset.x += rectClient.left - rectImageScreen.left;
				else if (rectImageScreen.right < rectClient.right)
					m_ctScreenFromImage.m_offset.x += rectClient.right - rectImageScreen.right;
			}
			if (rectImageScreen.Height() <= rectClient.Height()) {
				auto pt = m_ctScreenFromImage(xPoint2d(0, m_img.rows/2));
				m_ctScreenFromImage.m_offset.y += rectClient.Height() / 2 - pt.y;
			}
			if (rectImageScreen.Height() > rectClient.Height()) {
				if (rectImageScreen.top > rectClient.top)
					m_ctScreenFromImage.m_offset.y += rectClient.top - rectImageScreen.top;
				else if (rectImageScreen.bottom < rectClient.bottom)
					m_ctScreenFromImage.m_offset.y += rectClient.bottom - rectImageScreen.bottom;
			}
		}

		return true;
	}

	xMatView::S_SCROLL_GEOMETRY xMatView::GetScrollGeometry() {
		xRect2i rectClient{GetViewRect()};
		auto pt0 = m_ctScreenFromImage(xPoint2d(0,0));
		auto pt1 = m_ctScreenFromImage(xPoint2d(m_img.cols, m_img.rows));
		xRect2i rectImageScreen(pt0, pt1);
		rectImageScreen.NormalizeRect();
		xRect2i rectScrollRange(rectClient);
		if (m_option.bExtendedPanning) {
			rectScrollRange.pt0() -= rectImageScreen.GetSize();
			rectScrollRange.pt1() += rectClient.GetSize();
			int nScrollMarginX = std::min(m_option.nScrollMargin, rectImageScreen.Width());
			int nScrollMarginY = std::min(m_option.nScrollMargin, rectImageScreen.Height());
			rectScrollRange.DeflateRect(nScrollMarginX, nScrollMarginY);
		} else {
			rectScrollRange |= rectImageScreen;
		}
		return {rectClient, rectImageScreen, rectScrollRange};
	}

	bool xMatView::UpdateScrollBars() {
		if (m_img.empty())
			return false;
		auto [rectClient, rectImageScreen, rectScrollRange] = GetScrollGeometry();

		if (QScrollBar* sb = ui->sbHorz; sb and sb->isVisible()) {
			int p = m_option.bExtendedPanning
				? rectScrollRange.Width() - (m_ctScreenFromImage.m_offset.x + std::max(0, rectImageScreen.Width() - m_option.nScrollMargin) + rectClient.Width())
				: -m_ctScreenFromImage.m_offset.x;
			if (rectScrollRange.Width() > 0) {
				p = std::clamp(p, 0, rectScrollRange.Width());
				sb->setRange(0, std::max(0, rectScrollRange.Width() - rectClient.Width()));
				sb->setPageStep(rectClient.Width());
				sb->setSliderPosition(p);
			}
		}

		if (QScrollBar* sb = ui->sbVert; sb and sb->isVisible()) {
			int p = m_option.bExtendedPanning
				? rectScrollRange.Height() - (m_ctScreenFromImage.m_offset.y + std::max(0, rectImageScreen.Height() - m_option.nScrollMargin) + rectClient.Height())
				: -m_ctScreenFromImage.m_offset.y;
			if (rectScrollRange.Height() > 0) {
				p = std::clamp(p, 0, rectScrollRange.Height());
				sb->setRange(0, std::max(0, rectScrollRange.Height()-rectClient.Height()));
				sb->setPageStep(rectClient.Height());
				sb->setSliderPosition(p);
			}
		}

		if (m_ctScreenFromImage.m_scale != ui->spinZoom->value()*1.e-2) {
			m_bSkipSpinZoomEvent = true;
			ui->spinZoom->setValue(m_ctScreenFromImage.m_scale*1.e2);
			m_bSkipSpinZoomEvent = false;
		}

		return true;
	}

	bool xMatView::ZoomInOut(double step, xPoint2i ptAnchor, bool bCenter) {
		auto scale = m_ctScreenFromImage.m_scale;
		if (step > 0) {
			for (auto dZoom : dZoomLevels) {
				if (dZoom > scale) {
					scale = dZoom;
					break;
				}
			}
		}
		else {
			for (auto pos = std::rbegin(dZoomLevels); pos != std::rend(dZoomLevels); pos++) {
				auto dZoom = *pos;
				if (dZoom < scale) {
					scale = dZoom;
					break;
				}
			}
		}
		return SetZoom(scale, ptAnchor, bCenter);
	}

	bool xMatView::SetZoom(double scale, xPoint2i ptAnchor, bool bCenter) {
		if (m_img.empty())
			return false;
		// Backup Image Position
		xPoint2d ptImage = m_ctScreenFromImage.TransI(xPoint2d(ptAnchor));
		// Get Scroll Amount
		if (m_ctScreenFromImage.m_scale == scale)
			return true;
		else
			m_ctScreenFromImage.m_scale = scale;

		auto rectClient = GetViewRect();
		auto dMinZoom = std::min(rectClient.Width()/4. / m_img.cols, rectClient.Height()/4. / m_img.rows);
		dMinZoom = std::min(dMinZoom, 0.5);
		m_ctScreenFromImage.m_scale = std::clamp(m_ctScreenFromImage.m_scale, dMinZoom, 1.e3);
		m_ctScreenFromImage.m_offset += ptAnchor - m_ctScreenFromImage(ptImage);
		// Anchor point
		auto eZoom = m_eZoom;
		if (eZoom != eZOOM::mouse_wheel_locked)
			eZoom = eZOOM::free;
		ui->cmbZoomMode->setCurrentIndex(std::to_underlying(eZoom));
		//OnCmbZoomMode_currentIndexChanged(std::to_underlying(eZoom));
		UpdateCT(bCenter);
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
		return true;
	}

	bool xMatView::ScrollTo(xPoint2d pt, std::chrono::milliseconds tsScroll) {
		if (tsScroll == 0ms) {
			m_smooth_scroll.timer.stop();	// 이미 동작하고 있는 Scroll 중지
			m_ctScreenFromImage.m_offset = pt;
			UpdateCT(false);
			UpdateScrollBars();
			if (ui->view)
				ui->view->update();
		}
		else {
			m_smooth_scroll.pt0 = m_ctScreenFromImage.m_offset;
			m_smooth_scroll.pt1 = pt;
			m_smooth_scroll.t0 = std::chrono::steady_clock::now();
			if (tsScroll < 0ms)
				tsScroll = m_option.tsScroll;
			m_smooth_scroll.t1 = m_smooth_scroll.t0 + tsScroll;
			m_smooth_scroll.timer.start(10ms);
		}
		return true;
	}

	bool xMatView::Scroll(xPoint2d delta, std::chrono::milliseconds tsScroll) {
		return ScrollTo(m_ctScreenFromImage.m_offset + delta, tsScroll);
	}

	void xMatView::PurgeScroll(bool bUpdate) {
		if (!m_smooth_scroll.timer.isActive())
			return;
		m_smooth_scroll.timer.stop();
		m_ctScreenFromImage.m_offset = m_smooth_scroll.pt1;
		m_smooth_scroll.Clear();
		if (bUpdate) {
			UpdateCT(false);
			UpdateScrollBars();
			if (ui->view)
				ui->view->update();
		}
	}

	bool xMatView::KeyboardNavigate(int key, bool ctrl, bool alt, bool shift) {
		if (ctrl or alt or m_img.empty())
			return false;

		// if scroll is not finished, use target scroll position
		PurgeScroll();

		xCoordTrans ctS2I;
		m_ctScreenFromImage.GetInv(ctS2I);
		xRect2i rectClient{GetViewRect()};

		switch (key) {
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_Up:
		case Qt::Key_Down:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
			{
				auto ptShift = ctS2I(xPoint2d{100., 100.}) - ctS2I(xPoint2d{0., 0.});
				auto ptShiftPage = ctS2I(rectClient.pt1()) - ctS2I(rectClient.pt0());
				ptShiftPage *= 0.9;

				auto tsScroll = -1ms;

				xPoint2d delta;
				if (key == Qt::Key_Left)			{ delta.x += ptShift.x;	tsScroll = 0ms; }
				else if (key == Qt::Key_Right)		{ delta.x -= ptShift.x;	tsScroll = 0ms; }
				else if (key == Qt::Key_Up)			{ delta.y += ptShift.y;	tsScroll = 0ms; }
				else if (key == Qt::Key_Down)		{ delta.y -= ptShift.y;	tsScroll = 0ms; }
				else if (key == Qt::Key_PageUp)		{ delta.y += ptShiftPage.y; }
				else if (key == Qt::Key_PageDown)	{ delta.y -= ptShiftPage.y; }

				Scroll(delta, 0ms);
			}
			break;

		case Qt::Key_Space :
			{
				int heightMin { (int) (ctS2I.Trans((double)rectClient.Height()) * 5.0 / 10) };
				int deltaDefaultScreen{ rectClient.Height() * 95 / 100 };
				double deltaImage { ctS2I.Trans((double)deltaDefaultScreen) };
				int heightMax { (int) (ctS2I.Trans((double)rectClient.Height()) * 9.5 / 10) };
				xPoint2i ptImgBottom = ctS2I(xPoint2i{rectClient.left, rectClient.bottom});
				xPoint2i ptImgTop = ctS2I(xPoint2i{rectClient.left, rectClient.top});
				auto y0 = std::min(ptImgBottom.y + heightMax, m_img.rows - 1);

				auto CheckIfBlank = [&](int y) -> bool {
					constexpr static int margin = 5;
					cv::Rect rc(margin, y, m_img.cols-2*margin, 1);
					if (!gtl::IsROI_Valid(rc, m_img.size()))
						return false;
					auto imgRow = m_img(rc);
					if (imgRow.empty())
						return true;
					cv::Scalar mean, stddev;
					cv::meanStdDev(imgRow, mean, stddev);
					for (int i{}, nChannel = std::min(3, imgRow.channels()); i < nChannel; i++) {
						constexpr auto threshold = 4.0;
						if (stddev[i] > threshold)
							return false;
					}
					return true;
				};

				{
					if (CheckIfBlank(ptImgBottom.y)) {
						auto yE = m_img.rows;
						for (auto y = ptImgBottom.y; y < yE; y++) {
							if (!CheckIfBlank(y)) {
								deltaImage = y-ptImgTop.y;
								break;
							}
						}
					}
					else {
						deltaImage = heightMax;
						auto y2 = ptImgTop.y + heightMin*1/2;
						auto y = ptImgBottom.y;
						for (; y >= y2; y--) {
							if (CheckIfBlank(y)) {
								deltaImage = y - ptImgTop.y;
								break;
							}
						}
						if (y < y2) {	// not found
							auto yE = ptImgBottom.y + heightMax;
							for (; y < yE; y++) {
								if (CheckIfBlank(y)) {
									deltaImage = y - ptImgBottom.y;
									break;
								}
							}
						}
					}
				}

				deltaImage *= m_ctScreenFromImage.m_scale;
				Scroll({0., -deltaImage});
			}
			break;

		case Qt::Key_Home :
			ScrollTo({});
			break;

		case Qt::Key_End :
			{
				auto pt0 = m_ctScreenFromImage(xPoint2d{0.0, (double)0.0});
				auto pt1 = m_ctScreenFromImage(xPoint2d{0.0, (double)m_img.rows});
				auto p = (pt0.y - pt1.y) - rectClient.Height();
				ScrollTo({0., p});
			}
			break;

		default:
			return false;
		}
		return true;
	}

	void xMatView::BuildPyramid() {
		// Build Pyramid Image for down sampling { cv::InterpolationFlags::INTER_AREA }
		if (m_pyramid.threadPyramidMaker.joinable()) {
			m_pyramid.threadPyramidMaker.request_stop();
			m_pyramid.threadPyramidMaker.join();
		}
		m_pyramid.imgs.clear();
		m_pyramid.imgs.push_front(m_img);
		const uint minArea = 1'000 * 1'000;
		if (m_option.bPyrImageDown and m_option.eZoomOut == eZOOM_OUT::area and ((uint64_t)m_img.cols * m_img.rows) > minArea) {
			m_pyramid.threadPyramidMaker = std::jthread([this](std::stop_token stop) {
				cv::Mat imgPyr = m_pyramid.imgs[0];
				while (!stop.stop_requested() and ((uint64_t)imgPyr.cols * imgPyr.rows) > minArea) {
					cv::pyrDown(imgPyr, imgPyr);
					{
						std::unique_lock lock{ m_pyramid.mtx };
						m_pyramid.imgs.emplace_front(imgPyr);
					}
				}
			});
		}
	}

	xRect2i xMatView::GetViewRect() {
		if (!ui->view)
			return {};
		auto rect = ToCoord(ui->view->rect());
		rect.MoveToXY(0, 0);
		for (auto r = devicePixelRatio(); auto& v : rect.arr())
			v *= r;
		return rect;
	}

	void xMatView::keyPressEvent(QKeyEvent* event) {
		// Print Char Pressed
		auto l = std::source_location::current();
		OutputDebugString(std::format(L"{}: {}\n", gtl::ToStringW(l.function_name()), event->key()).c_str());
		auto bControl = event->modifiers() & Qt::ControlModifier;
		auto bAlt =	event->modifiers() & Qt::AltModifier;
		auto bShift = event->modifiers() & Qt::ShiftModifier;
		auto code = event->key();

		if ( (code == Qt::Key_Escape)
			and (m_mouse.bRectSelected or m_mouse.bInSelectionMode)
			)
		{
			m_mouse.bRectSelected = m_mouse.bInSelectionMode = false;
			if (ui->view)
				ui->view->update();
			event->accept();
			return ;
		}

		if ( bControl and bAlt and !bShift and (code == 'M') ) {
			auto show = !ui->toolbar->isVisible();
			ShowToolBar(show);
			event->accept();
			return ;
		}

		if (m_option.bKeyboardNavigation
			and KeyboardNavigate(code, bControl, bAlt, bShift))
		{
			event->accept();
			return;
		}

		event->ignore();
	}

	void xMatView::OnView_mousePressEvent(xMatViewCanvas* view, QMouseEvent* event) {
		if (!view)
			return;
		xPoint2i ptView = ToCoord(event->pos() * devicePixelRatio());
		if (event->button() == Qt::MouseButton::LeftButton) {
			if (m_option.bPanningLock and (m_eZoom == eZOOM::fit2window))
				return;
			if (mouseGrabber())
				return;
			event->accept();
			view->grabMouse();
			m_mouse.ptAnchor = ptView;
			m_mouse.ptOffset0 = m_ctScreenFromImage.m_offset;
		}
		else if (event->button() == Qt::MouseButton::RightButton) {
			if (m_mouse.bInSelectionMode) {
				m_mouse.bInSelectionMode = false;
				m_mouse.bRectSelected = true;
				auto pt = m_ctScreenFromImage.TransI(ptView);
				m_mouse.ptSel1.x = std::clamp<int>(pt.x, 0, m_img.cols);
				m_mouse.ptSel1.y = std::clamp<int>(pt.y, 0, m_img.rows);
			} else {
				m_mouse.bRectSelected = false;
				m_mouse.bInSelectionMode = true;
				auto pt = m_ctScreenFromImage.TransI(ptView);
				m_mouse.ptSel0.x = std::clamp<int>(pt.x, 0, m_img.cols);
				m_mouse.ptSel0.y = std::clamp<int>(pt.y, 0, m_img.rows);
				m_mouse.ptSel1 = m_mouse.ptSel0;
			}
			view->update();
		}
	}

	void xMatView::OnView_mouseReleaseEvent(xMatViewCanvas* view, QMouseEvent* event) {
		if (!view)
			return;
		if (event->button() == Qt::MouseButton::LeftButton) {
			if (!mouseGrabber())
				return;
			event->accept();
			view->releaseMouse();
			m_mouse.ptAnchor.reset();
		}
		else if (event->button() == Qt::MouseButton::RightButton) {
		}
	}

	void xMatView::OnView_mouseMoveEvent(xMatViewCanvas* view, QMouseEvent* event) {
		static std::locale l("en_US.UTF-8");
		if (!view)
			return;
		event->accept();
		xPoint2d ptView = ToCoord(event->pos()*devicePixelRatio());
		if (m_mouse.ptAnchor) {
			if (!m_option.bPanningLock) {
				switch (m_eZoom) {
				case eZOOM::one2one: break;
				case eZOOM::mouse_wheel_locked: break;
				case eZOOM::free: break;
				default :
					m_eZoom = eZOOM::free;
					ui->cmbZoomMode->setCurrentIndex(std::to_underlying(m_eZoom));
					break;
				}
			}
			auto dPanningSpeed = m_mouse.bInSelectionMode ? 1.0 : m_option.dPanningSpeed;
			auto ptOffset = (ptView - *m_mouse.ptAnchor) * dPanningSpeed;
			if (m_eZoom == eZOOM::fit2width)
				ptOffset.x = 0;
			if (m_eZoom == eZOOM::fit2height)
				ptOffset.y = 0;
			m_ctScreenFromImage.m_offset = m_mouse.ptOffset0 + ptOffset;
			UpdateCT();
			UpdateScrollBars();
			view->update();
		}

		// Selection Mode
		if (m_mouse.bInSelectionMode) {
			auto pt = m_ctScreenFromImage.TransI(ptView);
			m_mouse.ptSel1.x = std::clamp<int>(pt.x, 0, m_img.cols);
			m_mouse.ptSel1.y = std::clamp<int>(pt.y, 0, m_img.rows);
			view->update();
		}

		// status
		{
			auto ptImage = gtl::Floor(m_ctScreenFromImage.TransI(xPoint2d(ptView)));
			std::wstring status;

			// Current Position
			int nx{}, ny{};
			{
				for (auto v = m_imgOriginal.cols; v; v/= 10, nx++);
				nx = nx*4/3;
				for (auto v = m_imgOriginal.rows; v; v/= 10, ny++);
				ny = ny*4/3;
				// print ptImage.x and ptImage.y with thousand comma separated
				if (!m_imgOriginal.empty())
					status += fmt::format(L"(w{} h{}) ",
										  AddThousandCommaW((int)m_imgOriginal.cols),
										  AddThousandCommaW((int)m_imgOriginal.rows));
				status += fmt::format(L"[x{:>{}} y{:>{}}]",
									  AddThousandCommaW((int)ptImage.x), nx,
									  AddThousandCommaW((int)ptImage.y), ny);
			}

			// image value
			{
				int n = m_imgOriginal.channels();
				if (xRect2i(0, 0, m_img.cols, m_img.rows).PtInRect(ptImage)) {
					int depth = m_imgOriginal.depth();
					auto cr = GetMatValue(m_imgOriginal.ptr(ptImage.y), depth, n, ptImage.y, ptImage.x);
					auto strValue = std::format(L" [{:3}", cr[0]);
					for (int i{1}; i < n; i++)
						strValue += std::format(L",{:3}", cr[i]);
					status += strValue + L"]";
				}
				else {
					auto strValue = std::format(L" [{:3}", ' ');
					for (int i{1}; i < n; i++)
						strValue += std::format(L" {:3}", ' ');
					status += strValue + L"]";
				}
			}

			// Selection
			if (m_mouse.bInSelectionMode or m_mouse.bRectSelected) {
				gtl::xSize2i size = m_mouse.ptSel1 - m_mouse.ptSel0;
				status += fmt::format(L" (x{0} y{1} w{2} h{3})",
					AddThousandCommaW((int)m_mouse.ptSel0.x), AddThousandCommaW((int)m_mouse.ptSel0.y),
					AddThousandCommaW(std::abs(size.cx)), AddThousandCommaW(std::abs(size.cy)));
			}


			if (auto str = ToQString(status); str != ui->edtInfo->text()) {
				ui->edtInfo->setText(str);
				ui->edtInfo->setSelection(0, 0);
			}
		}
	}

	void xMatView::OnView_wheelEvent(xMatViewCanvas* view, QWheelEvent* event) {
		if (!view)
			return;
		if ((m_eZoom == eZOOM::mouse_wheel_locked) or (m_option.bZoomLock and m_eZoom != eZOOM::free)) {
			return;
		}
		event->accept();
		ZoomInOut(event->angleDelta().y(), ToCoord(event->position()*devicePixelRatio()), false);
	}

	void xMatView::OnCmbZoomMode_currentIndexChanged(int index) {
		//static int count{};
		//OutputDebugStringA(std::format("{} : index : {}, count {}\n", std::source_location{}.function_name(), index, count++).c_str());
		{
			eZOOM cur = (eZOOM)index;
			using enum eZOOM;
			if (m_eZoom == cur and gtl::IsValueOneOf(m_eZoom, fit2window, one2one, mouse_wheel_locked, free))
				return;
			m_eZoom = cur;
		}

		//// Scroll Bar Visibility
		//bool bHorz{true}, bVert{true};
		//ui->view->AlwaysShowScrollbars(bHorz, bVert);

		UpdateCT(true);
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
	}

	void xMatView::OnSpinZoom_valueChanged(double value) {
		auto scale = value * 1.e-2;
		static auto const dMinZoom = dZoomLevels[0];
		static auto const dMaxZoom = dZoomLevels[std::size(dZoomLevels)-1];
		scale = std::clamp<double>(scale, dMinZoom, dMaxZoom);
		if (m_bSkipSpinZoomEvent)
			return;
		if ( m_option.bZoomLock and gtl::IsValueNoneOf(m_eZoom, eZOOM::free, eZOOM::mouse_wheel_locked) )
			return;
		if (m_ctScreenFromImage.m_scale == scale)
			return;
		auto rect{GetViewRect()};
		auto ptImage = m_ctScreenFromImage.TransI(rect.CenterPoint());
		m_ctScreenFromImage.m_scale = scale;
		auto pt2 = m_ctScreenFromImage(ptImage);
		m_ctScreenFromImage.m_offset += rect.CenterPoint() - m_ctScreenFromImage(ptImage);
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
	}

	void xMatView::OnBtnZoomIn_clicked() {
		if ( m_option.bZoomLock and gtl::IsValueNoneOf(m_eZoom, eZOOM::free, eZOOM::mouse_wheel_locked) )
			return;
		ZoomInOut(100, GetViewRect().CenterPoint(), false);
	}

	void xMatView::OnBtnZoomOut_clicked() {
		if ( m_option.bZoomLock and gtl::IsValueNoneOf(m_eZoom, eZOOM::free, eZOOM::mouse_wheel_locked) )
			return;
		ZoomInOut(-100, GetViewRect().CenterPoint(), false);
	}

	void xMatView::OnBtnZoomFit_clicked() {
		if ( m_option.bZoomLock and gtl::IsValueNoneOf(m_eZoom, eZOOM::free, eZOOM::mouse_wheel_locked) )
			return;
		UpdateCT(true, eZOOM::fit2window);
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
	}

	void xMatView::OnBtnSettings_clicked() {
		xMatViewSettingsDlg dlg(m_option, this);
		if (dlg.exec() != QDialog::Accepted)
			return;
		SetOption(dlg.m_option);
	}

	void xMatView::OnSmoothScroll_timeout() {
		auto t = std::chrono::steady_clock::now();
		if (m_smooth_scroll.t0 < t and t < m_smooth_scroll.t1) {
			auto& ss = m_smooth_scroll;

			double len = (ss.t1-ss.t0).count();
			double pos = (t-ss.t0).count();
			double s = ::sin(pos / len * std::numbers::pi/2.);
			m_ctScreenFromImage.m_offset = ss.pt0 + (ss.pt1-ss.pt0) * s;
		}
		if (t >= m_smooth_scroll.t1) {
			m_smooth_scroll.timer.stop();
			m_ctScreenFromImage.m_offset = m_smooth_scroll.pt1;
		}
		UpdateCT(false);
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
	}

	void xMatView::OnView_resized() {
		UpdateCT();
		UpdateScrollBars();
	}

	void xMatView::OnSbHorz_valueChanged(int pos) {
		auto [rectClient, rectImageScreen, rectScrollRange] = GetScrollGeometry();
		int range = rectScrollRange.Width();
		int page =  rectClient.Width();
		pos = std::clamp(pos, 0, range);
		m_ctScreenFromImage.m_offset.x = - pos;
		if (m_option.bExtendedPanning)
			m_ctScreenFromImage.m_offset.x += rectScrollRange.Width() - std::max(0, rectImageScreen.Width() - m_option.nScrollMargin) - rectClient.Width();
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
	}
	void xMatView::OnSbVert_valueChanged(int pos) {
		auto [rectClient, rectImageScreen, rectScrollRange] = GetScrollGeometry();
		int range = rectScrollRange.Height();
		int page =  rectClient.Height();
		pos = std::clamp(pos, 0, range);
		m_ctScreenFromImage.m_offset.y = - pos;
		if (m_option.bExtendedPanning)
			m_ctScreenFromImage.m_offset.y += rectScrollRange.Height() - std::max(0, rectImageScreen.Height() - m_option.nScrollMargin) - rectClient.Height();
		UpdateScrollBars();
		if (ui->view)
			ui->view->update();
	}

	void xMatView::InitializeGL(xMatViewCanvas* view) {
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			const GLubyte* msg = glewGetErrorString(err);
			QString str((char const*)msg);
			QMessageBox::critical(this, "openGL", str);
			return;
			//throw std::exception(reinterpret_cast<const char*>(msg));
		}

		// OpenGL Version in number
		int nMajor = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &nMajor);
		if (nMajor < 3) {
			QMessageBox::critical(this, "openGL", "OpenGL 3.0 or higher is required.");
			return;
		}

		//QString strVersion = QString::fromUtf8((char const*)glGetString(GL_VERSION));
		// openGL Version Major

		// Vertex Shader Source
		static char const* vertexShaderSource = 
R"(
	#version 330 core
	layout(location = 0) in vec2 inPosition;
	layout(location = 1) in vec2 inTexCoord;
	out vec2 TexCoord;
	void main() {
		gl_Position = vec4(inPosition, 0.0, 1.0);
		TexCoord = inTexCoord;
	}
)";

		// Fragment Shader Source
		static char const* fragmentShaderSource =
R"(
	#version 330 core
	in vec2 TexCoord;
	out vec4 FragColor;
	uniform sampler2D textureSampler;
	void main() {
		FragColor = texture(textureSampler, TexCoord);
	}
)";

		// todo: TEMP
		if (!m_gl.gl)
			m_gl.gl = std::make_unique<QOpenGLExtraFunctions>(view->context());
		if (!m_gl.shaderProgram) {

			m_gl().glGenVertexArrays(1, &m_gl.VAO);
			m_gl().glBindVertexArray(m_gl.VAO);

			// Create Vertex Buffer Object (VBO)
			m_gl().glGenBuffers(1, &m_gl.VBO);
			m_gl().glBindBuffer(GL_ARRAY_BUFFER, m_gl.VBO);

			// Define vertices and texture coordinates for a quad
			float vertices[] = {
				// Position    // Texture Coordinates
				0.0f, 1.0f,  0.0f, 0.0f,
				1.0f, 0.0f,  1.0f, 0.0f,
				1.0f, 1.0f,  1.0f, 1.0f,
				0.0f, 1.0f,  0.0f, 1.0f,
			};
			m_gl().glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			// Create and compile the vertex shader
			auto vertexShader = m_gl().glCreateShader(GL_VERTEX_SHADER);
			m_gl().glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
			m_gl().glCompileShader(vertexShader);

			// Create and compile the fragment shader
			GLuint fragmentShader = m_gl().glCreateShader(GL_FRAGMENT_SHADER);
			m_gl().glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
			m_gl().glCompileShader(fragmentShader);

			// Create and link the shader program
			m_gl.shaderProgram = m_gl().glCreateProgram();
			m_gl().glAttachShader(m_gl.shaderProgram, vertexShader);
			m_gl().glAttachShader(m_gl.shaderProgram, fragmentShader);
		}
		if (m_gl.shaderProgram) {
			m_gl().glLinkProgram(m_gl.shaderProgram);
			m_gl().glUseProgram(m_gl.shaderProgram);

			m_gl().glUniform1i(m_gl().glGetUniformLocation(m_gl.shaderProgram, "textureSampler"), 0);
		}

		// Specify vertex attribute data
		m_gl().glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		m_gl().glEnableVertexAttribArray(0);
		m_gl().glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		m_gl().glEnableVertexAttribArray(1);

		//// get max bitmap size
		//GLint maxTexSize;
		//glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
		//OutputDebugStringA((std::format("GL_MAX_TEXTURE_SIZE: {}\n", maxTexSize)).c_str());

		//// get opengl Version
		//QString strVersion = QString::fromUtf8((char const*)glGetString(GL_VERSION));
		//QString strVendor = QString::fromUtf8((char const*)glGetString(GL_VENDOR));
		//QString strRenderer = QString::fromUtf8((char const*)glGetString(GL_RENDERER));
		//QString strGLSL = QString::fromUtf8((char const*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		//OutputDebugStringA((std::format("OpenGL Version: {}\n", strVersion.toStdString())).c_str());

	}

	bool xMatView::PutMatAsTexture(GLuint textureID, cv::Mat const& img, int width, gtl::xRect2i const& rect, gtl::xRect2i const& rectClient) {
		if (!textureID or img.empty() or !img.isContinuous())
			return false;

		if (!m_gl.gl or !m_gl.shaderProgram or !m_gl.VAO or !m_gl.VBO) {
			return gtl::PutMatAsTexture(textureID, img, width, rect);
		}

		if (rectClient.Width() <= 0 or rectClient.Height() <= 0)
			return false;

		glBindTexture(GL_TEXTURE_2D, textureID);
		if (img.step%4)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		else
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		// Create the texture
		auto [eColorType, eFormat, ePixelType] = GetGLImageFormatType(img.type());
		glTexImage2D(GL_TEXTURE_2D, 0, eColorType, img.cols, img.rows, 0, eFormat, ePixelType, img.ptr());

		m_gl().glUniform1i(m_gl().glGetUniformLocation(m_gl.shaderProgram, "textureSampler"), 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Set texture clamping method
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		// patch
		m_gl().glBindVertexArray(m_gl.VAO);
		m_gl().glBindBuffer(GL_ARRAY_BUFFER, m_gl.VBO);

		gtl::TRect2<float> rc((float)rect.left/rectClient.Width(), (float)rect.top/rectClient.Height(), (float)rect.right/rectClient.Width(), (float)rect.bottom/rectClient.Height());
		for (auto& v : rc.arr()) {
			v = v*2 - 1;
		}

		// Define vertices and texture coordinates for a quad
		auto r = (float)width/img.cols;
		float vertices[] = {
			// Position		// Texture Coordinates
			//-r, -1.0f,		rc.right, rc.top,
			//r, -1.0f,		rc.left, rc.top,
			//r, 1.0f,		rc.left, rc.bottom,
			//-r, 1.0f,		rc.right, rc.bottom,
			rc.left, -rc.top, 	0, 0,
			rc.right, -rc.top, 	r, 0,
			rc.right, -rc.bottom, r, 1,
			rc.left, -rc.bottom, 0, 1,
		};

		m_gl().glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		m_gl().glBindVertexArray(0);

		//glBegin(GL_QUADS);
		//glTexCoord2f(0.f, 0.f);	glVertex2i(rect.left,   rect.top);
		//glTexCoord2f(0.f, 1.f);	glVertex2i(rect.left,   rect.bottom);
		//glTexCoord2f(r, 1.f);	glVertex2i(rect.right,  rect.bottom);
		//glTexCoord2f(r, 0.f);	glVertex2i(rect.right,  rect.top);
		//glEnd();

		return true;
	}


	void xMatView::PaintGL(xMatViewCanvas* view) {
		//auto t0 = std::chrono::steady_clock::now();

		if (!view)
			return;
		using namespace gtl;

		//================
		// openGL
		//view->makeCurrent();

		// Client Rect
		xRect2i rectClient;
		rectClient = GetViewRect();
		xSize2i const sizeView = rectClient.GetSize();
		glViewport(0, 0, sizeView.cx, sizeView.cy);

		glMatrixMode(GL_PROJECTION);     // Make a simple 2D projection on the entire window
		glLoadIdentity();
		glOrtho(0.0, sizeView.cx, sizeView.cy, 0.0, 0.0, 100.0);

		glMatrixMode(GL_MODELVIEW);    // Set the matrix mode to object modeling
		cv::Scalar cr = m_option.crBackground;
		glClearColor(cr[0]/255.f, cr[1]/255.f, cr[2]/255.f, 1.0f);
		glClearDepth(0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the window

		auto* context = view->context();
		if (!context)
			return;
		auto* surface = view->context()->surface();

		gtl::xFinalAction faSwapBuffer{[&]{
			//OutputDebugStringA("Flush\n");
			glFlush();
			context->swapBuffers(surface);
		}};

		//event.Skip();
		if (!view or !context)
			return;

		//// Show zoom Scale
		//base_t::m_spinCtrlZoom->SetValue(m_ctScreenFromImage.m_scale);

		if (rectClient.IsRectEmpty())
			return;

		//==================
		// Get Image - ROI
		auto const& ct = m_ctScreenFromImage;

		// Position
		xRect2i rectImage;
		rectImage.pt0() = ct.TransInverse(xPoint2d(rectClient.pt0())).value_or(xPoint2d{});
		rectImage.pt1() = ct.TransInverse(xPoint2d(rectClient.pt1())).value_or(xPoint2d{});
		rectImage.NormalizeRect();
		rectImage.InflateRect(1, 1);
		rectImage &= xRect2i{ 0, 0, m_img.cols, m_img.rows };
		if (rectImage.IsRectEmpty())
			return;
		cv::Rect roi(rectImage);
		xRect2i rectTarget;
		rectTarget.pt0() = ct(rectImage.pt0());
		rectTarget.pt1() = ct.Trans(xPoint2d(rectImage.pt1()));
		rectTarget.NormalizeRect();
		if (rectTarget.right == rectTarget.left)
			rectTarget.right = rectTarget.left+1;
		if (rectTarget.bottom == rectTarget.top)
			rectTarget.bottom = rectTarget.top+1;
		if (!gtl::IsROI_Valid(roi, m_img.size()))
			return;

		// img (roi)
		cv::Rect rcTarget(cv::Point2i{}, cv::Size2i(rectTarget.Width(), rectTarget.Height()));
		cv::Rect rcTargetC(rcTarget);	// 4 byte align
		if (rcTarget.width*m_img.elemSize() % 4)
			rcTargetC.width = gtl::AdjustAlign32(rcTargetC.width);
		// check target image size
		if ((uint64_t)rcTargetC.width * rcTargetC.height > 1ull *1024*1024*1024)
			return;

		cv::Mat img(rcTargetC.size(), m_img.type());
		//img = m_option.crBackground;
		int eInterpolation = cv::INTER_LINEAR;
		try {
			if (ct.m_scale < 1.) {
				static std::unordered_map<eZOOM_OUT, cv::InterpolationFlags> const mapInterpolation = {
					{eZOOM_OUT::nearest, cv::InterpolationFlags::INTER_NEAREST}, {eZOOM_OUT::area, cv::InterpolationFlags::INTER_AREA},
				};
				if (auto pos = mapInterpolation.find(m_option.eZoomOut); pos != mapInterpolation.end())
					eInterpolation = pos->second;

				// resize from pyramid image
				cv::Size2d size {ct.m_scale*m_img.cols, ct.m_scale*m_img.rows};
				cv::Mat imgPyr;
				//{
				//	auto t = std::chrono::steady_clock::now();
				//	OutputDebugStringA(std::format("=======================\n1 {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(t-t0)).c_str());
				//	t0 = t;
				//}
				{
					std::unique_lock lock{m_pyramid.mtx};
					for (auto const& img : m_pyramid.imgs) {
						if (img.cols < size.width)
							continue;
						imgPyr = img;
						break;
					}
				}
				//{
				//	auto t = std::chrono::steady_clock::now();
				//	OutputDebugStringA(std::format("=======================\n2 {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(t-t0)).c_str());
				//	t0 = t;
				//}
				if (!imgPyr.empty()) {

					xWaitCursor wc;

					double scaleP = m_img.cols < m_img.rows ? (double)imgPyr.rows / m_img.rows : (double)imgPyr.cols / m_img.cols;
					double scale = imgPyr.cols < imgPyr.rows ? (double)size.height / imgPyr.rows : (double)size.width / imgPyr.cols;
					cv::Rect roiP(roi);
					roiP.x *= scaleP;
					roiP.y *= scaleP;
					roiP.width *= scaleP;
					roiP.height *= scaleP;
					roiP = gtl::GetSafeROI(roiP, imgPyr.size());
					if (!roiP.empty()) {
						//cv::resize(imgPyr(roiP), img(rcTarget), rcTarget.size(), 0., 0., eInterpolation);
						cv::Mat imgSrc(imgPyr(roiP));
						cv::Mat imgDest;
						if (IsGPUEnabled()) {
							try {
								cv::cuda::GpuMat dst;
								cv::cuda::resize(cv::cuda::GpuMat(imgSrc), dst, rcTarget.size(), 0., 0., eInterpolation);
								dst.download(imgDest);
							}
							catch (...) {
								//TRACE((GTL__FUNCSIG " - Error\n").c_str());
							}
						}
						//#endif
						if (imgDest.empty())
							cv::resize(imgSrc, img(rcTarget), rcTarget.size(), 0., 0., eInterpolation);
						else {
							if (imgDest.size() == rcTarget.size())
								imgDest.copyTo(img(rcTarget));
						}
					}
				}
				//{
				//	auto t = std::chrono::steady_clock::now();
				//	OutputDebugStringA(std::format("=======================\n3 {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(t-t0)).c_str());
				//	t0 = t;
				//}
			}
			else if (ct.m_scale > 1.) {
				static std::unordered_map<eZOOM_IN, cv::InterpolationFlags> const mapInterpolation = {
					{eZOOM_IN::nearest, cv::InterpolationFlags::INTER_NEAREST}, {eZOOM_IN::linear, cv::InterpolationFlags::INTER_LINEAR},
					{eZOOM_IN::bicubic, cv::InterpolationFlags::INTER_CUBIC}, {eZOOM_IN::lanczos4, cv::InterpolationFlags::INTER_LANCZOS4},
				};
				if (auto pos = mapInterpolation.find(m_option.eZoomIn); pos != mapInterpolation.end())
					eInterpolation = pos->second;
				cv::resize(m_img(roi), img(rcTarget), rcTarget.size(), 0., 0., eInterpolation);
			}
			else {
				m_img(roi).copyTo(img(rcTarget));
			}
		} catch (std::exception& e) {
			OutputDebugStringA(std::format("cv::{}.......\n", e.what()).c_str());
		} catch (...) {
			OutputDebugStringA("cv::.......\n");
		}

		if (!img.empty()) {
			if (m_option.bDrawPixelValue) {
				auto ctCanvas = m_ctScreenFromImage;
				ctCanvas.m_offset -= m_ctScreenFromImage(roi.tl());
				DrawPixelValue(img, m_imgOriginal, roi, ctCanvas, 8*devicePixelRatio());
			}

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_TEXTURE_2D);
			GLuint textures[2]{};
			glGenTextures(std::size(textures), textures);
			if (!textures[0]) {
				OutputDebugStringA("glGenTextures failed\n");
				return;
			}
			gtl::xFinalAction finalAction([&] {
				glDisable(GL_TEXTURE_2D);
				glDeleteTextures(std::size(textures), textures);
			});

			// Use the shader program
			if (m_gl.shaderProgram) {
				m_gl().glUseProgram(m_gl.shaderProgram);
			}

			PutMatAsTexture(textures[0], img, rcTarget.width, rectTarget, rectClient);

			// Draw Selection Rect
			if (m_mouse.bInSelectionMode or m_mouse.bRectSelected) {
				xRect2i rect;
				rect.pt0() = m_ctScreenFromImage(m_mouse.ptSel0);
				rect.pt1() = m_ctScreenFromImage(m_mouse.ptSel1);
				rect.NormalizeRect();
				rect &= rectClient;
				if (!rect.IsRectEmpty()) {
					cv::Mat rectangle(16, 16, CV_8UC4);
					rectangle = cv::Scalar(255, 255, 127, 128);
					PutMatAsTexture(textures[1], rectangle, rectangle.cols, rect, rectClient);
				}
			}
		}

	}

} // namespace gtl::qt
