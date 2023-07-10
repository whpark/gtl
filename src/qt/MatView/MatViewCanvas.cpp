#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/MatView/MatViewCanvas.h"

namespace gtl::qt {

	static double const dZoomLevels[] = {
		1./8192, 1./4096, 1./2048, 1./1024,
		1./512, 1./256, 1./128, 1./64, 1./32, 1./16, 1./8, 1./4., 1./2.,
		3./4, 1, 1.25, 1.5, 1.75, 2, 2.5, 3, 4, 5, 6, 7, 8, 9, 10,
		12.5, 15, 17.5, 20, 25, 30, 35, 40, 45, 50, 60, 70, 80, 90, 100,
		125, 150, 175, 200, 250, 300, 350, 400, 450, 500,
		600, 700, 800, 900, 1000,
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

	xMatViewCanvas::~xMatViewCanvas() {
		m_scene.release();
	}

	void xMatViewCanvas::Init() {
		SetBackgroundColor(GetBackgroundColor());
	}

	void xMatViewCanvas::SetBackgroundColor(QColor crBackground) {
		m_option.crBackground[0] = crBackground.red();
		m_option.crBackground[1] = crBackground.green();
		m_option.crBackground[2] = crBackground.blue();
		setBackgroundBrush(crBackground);
	}

	bool xMatViewCanvas::SetImage(cv::Mat const& img, bool bCopy) {
		if (bCopy)
			img.copyTo(m_img);
		else
			m_img = img;

		if (!m_img.isContinuous()) {
			assert(false);
			return false;
		}

		auto ePixelFormat = GetImageFormatType(m_img.type());
		if (ePixelFormat == QImage::Format_Invalid) {
			return false;
		}

		m_mouse.Clear();

		m_scene = std::make_unique<QGraphicsScene>();
		m_pixmap = QPixmap::fromImage(QImage{ m_img.data, m_img.cols, m_img.rows, (qsizetype)(size_t)m_img.step, ePixelFormat });
		m_scene->addPixmap(m_pixmap);
		setScene(m_scene.get());
		viewport()->installEventFilter(this);
		viewport()->setMouseTracking(true);

		return true;
	}

	void xMatViewCanvas::SetSelectionRect(xRect2i const& rect) {
		m_mouse.bRectSelected = true;
		m_mouse.ptSel0 = QPointF(rect.left, rect.top);
		m_mouse.ptSel1 = QPointF(rect.right, rect.bottom);

		this->update();
	}
	void xMatViewCanvas::ClearSelectionRect() {
		m_mouse.bRectSelected = false;
		this->update();
	}

	bool xMatViewCanvas::SetOption(S_OPTION const& option, bool bStore) {
		if (&m_option != &option) {
			m_option = option;
			SetBackgroundColor(GetBackgroundColor());
		}

		update();

		if (bStore) {
			return m_fnSyncSetting and m_fnSyncSetting(true, m_strCookie, m_option);
		}
		return true;
	}

	//bool xMatViewCanvas::ShowToolBar(bool bShow) {
	//	if (!m_toolBar)
	//		return false;
	//	m_toolBar->Show(bShow);
	//	m_sizerTop->Layout();
	//	Layout();
	//	return true;
	//}

	bool xMatViewCanvas::ZoomInOut(double step, QPointF ptAnchor) {
		auto scale = transform().m11();
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
		return SetZoom(scale, ptAnchor);
	}

	bool xMatViewCanvas::SetZoom(double dScale, QPointF ptAnchor) {
		if (m_img.empty() or dScale <= 0.0)
			return false;

		auto trans = transform();
		if (dScale == trans.m11())
			return false;
		scale(dScale/trans.m11(), dScale/trans.m22());

		emit sigZoom(dScale);

		return true;
	}

	void xMatViewCanvas::wheelEvent(QWheelEvent* event) {
		setTransformationAnchor(base_t::AnchorUnderMouse);

		auto dScale = transform().m11();
		if ( (event->angleDelta().y() > 0) and (dScale < 100) ) {
			ZoomInOut(1.0, event->position());
		}
		else if ( (event->angleDelta().y() < 0) and (dScale > 0.01) ) {
			ZoomInOut(-1.0, event->position());
		}
	}

	void xMatViewCanvas::mousePressEvent(QMouseEvent* event) {
		if (event->button() & Qt::LeftButton) {
			setTransformationAnchor(base_t::NoAnchor);
			m_mouse.ptAnchor = event->pos();
		}
		if (event->button() & Qt::RightButton) {
			if (m_mouse.bInSelectionMode) {
				m_mouse.bInSelectionMode = false;
				m_mouse.bRectSelected = true;
				auto rectF = QRectF(m_mouse.ptSel0, m_mouse.ptSel1).normalized();
				emit sigMouseSelected(Floor(rectF));
			}
			else {
				m_mouse.bInSelectionMode = true;
				m_mouse.ptSel0 = m_mouse.ptSel1 = mapToScene(event->pos());
			}
		}

		base_t::mousePressEvent(event);
	}

	void xMatViewCanvas::mouseMoveEvent(QMouseEvent* event) {
		if (m_mouse.ptAnchor) {
			setTransformationAnchor(base_t::NoAnchor);
			QPointF oldp = mapToScene(*m_mouse.ptAnchor);
			QPointF newp = mapToScene(event->pos());
			double speed = m_mouse.bInSelectionMode ? 1. : 2.;
			QPointF translation = (newp - oldp)*speed;	// increase moving speed

			translate(translation.x(), translation.y());

			if (*m_mouse.ptAnchor != event->pos()) {
				*m_mouse.ptAnchor = event->pos();
			}
		}

		QRectF rectUpdate;
		if (m_mouse.bInSelectionMode) {
			m_mouse.ptSel1 = mapToScene(event->pos());
			rectUpdate = QRectF(m_mouse.ptSel0, m_mouse.ptSel1).normalized();
			emit sigMouseSelected(Floor(rectUpdate));
		}

		// text
		{
			auto pt = Floor(mapToScene(event->pos()));
			int x = pt.x();
			int y = pt.y();
			if ( (x >= 0) and (x < m_img.cols) and (y >= 0) and (y < m_img.rows)) {
				emit sigMouseMoved(x, y);
			}
		}

		if (m_scene and !Floor(rectUpdate).isEmpty()) {
			m_scene->invalidate();
		}
		base_t::mouseMoveEvent(event);
	}

	void xMatViewCanvas::mouseReleaseEvent(QMouseEvent* event) {
		if (event->button() & Qt::LeftButton)
			m_mouse.ptAnchor.reset();

		base_t::mouseReleaseEvent(event);
	}

	//void xMatViewCanvas::drawBackground(QPainter* painter, const QRectF& rect) {
	//	base_t::drawBackground(painter, rect);
	//
	//	QColor cr = GetBackgroundColor();
	//	QBrush brush(cr);
	//	painter->setBrush(brush);
	//	painter->setPen(cr);
	//	QRectF r = rect;
	//	r.setWidth(r.width()+1.);
	//	r.setHeight(r.height()+1.);
	//	painter->drawRect(r);
	//}

	void xMatViewCanvas::drawForeground(QPainter* painter, const QRectF& rect) {
		base_t::drawForeground(painter, rect);

		// draw grid if zoom scale is greater than 10
		QTransform trans = painter->transform();
		auto scale = trans.m11();
		// get Client rect
		auto rc = this->rect();
		QRectF roiF;
		roiF.setTopLeft(mapToScene(rc.topLeft()));
		roiF.setBottomRight(mapToScene(rc.bottomRight()));
		QRect roi = Floor(roiF);
		if (roi.x() < 0) roi.setX(0);
		if (roi.right() >= m_img.cols) roi.setRight(m_img.cols-1);
		if (roi.y() < 0) roi.setY(0);
		if (roi.bottom() >= m_img.rows) roi.setBottom(m_img.rows-1);

	#ifdef _DEBUG
		{
			auto str = std::format("scene rect: {}, {}, {}, {}\n", roi.x(), roi.y(), roi.width(), roi.height());
			OutputDebugStringA(str.c_str());
		}
	#endif

		// reset transform
		painter->resetTransform();
		gtl::xFinalAction restoreTransform([&]{ painter->setTransform(trans); });

		// sel region
		if (m_mouse.ptSel0 != m_mouse.ptSel1) {
			// draw grid from m_ptClickR to event->pos()
			// Get Painter
			painter->setRenderHint(QPainter::Antialiasing);
			painter->setPen(QPen(Qt::red, (scale < 3.) ? 1 : 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

			QRectF rectF = QRectF(m_mouse.ptSel0, m_mouse.ptSel1).normalized();
			auto rect = Floor(rectF);
			rect.adjust(0, 0, 1, 1);
			QRect rect2(trans.map(rect.topLeft()), trans.map(rect.bottomRight()));
			painter->drawRect(rect2);
		}

		// Grid
		if (!m_option.bDrawPixelValue or scale < 4.0)
			return;
		QPen pen;
		pen.setColor(Qt::gray);
		pen.setStyle(Qt::SolidLine);
		painter->setPen(pen);
		for (int i = roi.left(); i <= roi.right(); i ++) {
			auto p0 = trans.map(QPoint{i, roi.top()});
			auto p1 = trans.map(QPoint{i, roi.bottom()+1});
			painter->drawLine(p0, p1);
		}
		for (int i = roi.top(); i <= roi.bottom(); i ++) {
			auto p0 = trans.map(QPoint{roi.left(), i});
			auto p1 = trans.map(QPoint{roi.right()+1, i});
			painter->drawLine(p0, p1);
		}

		// pixel value
		int heightText = std::max(12, font().pixelSize());
		if (scale < heightText * m_img.channels())
			return;
		auto nChannel = m_img.channels();
		auto depth = m_img.depth();

		QPen penBlack;
		penBlack.setColor(Qt::black);
		penBlack.setStyle(Qt::SolidLine);
		QPen penWhite;
		penWhite.setColor(Qt::white);
		penWhite.setStyle(Qt::SolidLine);

		auto y0 = std::max(0, roi.top());
		auto y1 = std::min(m_img.rows, roi.bottom() + 1);
		auto x0 = std::max(0, roi.left());
		auto x1 = std::min(m_img.cols, roi.right() + 1);

		for (int y{roi.y()}, y1{roi.y() + roi.height()}; y < y1; y++) {
			auto* ptr = m_img.ptr(y);
			int x1{roi.x() + roi.width()};
			//#pragma omp parallel for --------- little improvement
			for (int x{roi.x()}; x < x1; x++) {
				//auto p = SkPoint::Make(pt.x, pt.y);
				auto v = GetMatValue(ptr, depth, nChannel, y, x);
				float one_over_channel = 1.f / nChannel;
				auto avg = (v[0] + v[1] + v[2]) * one_over_channel;
				auto cr = (avg > 128) ? QColor{0, 0, 0} : QColor{255, 255, 255};
				painter->setPen((v[0] + v[1] + v[2] < 128*3) ? penWhite : penBlack);
				auto r = trans.mapRect(QRectF{(qreal)x, (qreal)y, 1.0, 1.0});
				r.setY(r.bottom() - heightText * nChannel);
				r.setHeight(heightText);
				for (int ch{}; ch < nChannel; ch++) {
					auto str = fmt::format("{:3}", v[ch]);
					painter->drawText(r, str.c_str(), QTextOption(Qt::AlignRight|Qt::AlignTop));
					r.setY(r.y() + heightText);
					r.setHeight(heightText);
				}
			}
		}
	}

}	// namespace gtl::qt
