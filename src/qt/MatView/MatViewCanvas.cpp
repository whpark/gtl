#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/MatView/MatViewCanvas.h"

namespace gtl::qt {

	inline cv::Scalar GetMatValue(uchar const* ptr, int depth, int channel, int row, int col) {
		//if ( mat.empty() or (row < 0) or (row >= mat.rows) or (col < 0) or (col >= mat.cols) )
		//	return;

		cv::Scalar v;
		auto GetValue = [&]<typename T>(T&&){
			for (int i = 0; i < channel; ++i)
				v[i] = ptr[col * channel + i];
		};
		switch (depth) {
		case CV_8U:		GetValue(uint8_t{}); break;
		case CV_8S:		GetValue(int8_t{}); break;
		case CV_16U:	GetValue(uint16_t{}); break;
		case CV_16S:	GetValue(int16_t{}); break;
		case CV_32S:	GetValue(int32_t{}); break;
		case CV_32F:	GetValue(float{}); break;
		case CV_64F:	GetValue(double{}); break;
			//case CV_16F:	GetValue(uint16_t{}); break;
		}

		return v;
	}

	// returns eInternalColorFormat, ColorFormat, PixelType
	std::tuple<GLint, GLenum, GLenum> GetGLImageFormatType(int type) {
		static std::unordered_map<int, std::tuple<GLint, GLenum, GLenum>> const m {
			{ CV_8UC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_UNSIGNED_BYTE	} },
			{ CV_8UC3,	{GL_RGB,		GL_RGB,				GL_UNSIGNED_BYTE	} },
			{ CV_8UC4,	{GL_RGBA,		GL_RGBA,			GL_UNSIGNED_BYTE	} },
			{ CV_16UC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_UNSIGNED_SHORT	} },
			{ CV_16UC3,	{GL_RGB,		GL_RGB,				GL_UNSIGNED_SHORT	} },
			{ CV_16UC4,	{GL_RGBA,		GL_RGBA,			GL_UNSIGNED_SHORT	} },
			{ CV_16SC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_SHORT			} },
			{ CV_16SC3,	{GL_RGB,		GL_RGB,				GL_SHORT			} },
			{ CV_16SC4,	{GL_RGBA,		GL_RGBA,			GL_SHORT			} },
			{ CV_32SC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_INT				} },
			{ CV_32SC3,	{GL_RGB,		GL_RGB,				GL_INT				} },
			{ CV_32SC4,	{GL_RGBA,		GL_RGBA,			GL_INT				} },
			{ CV_32FC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_FLOAT			} },
			{ CV_32FC3,	{GL_RGB,		GL_RGB,				GL_FLOAT			} },
			{ CV_32FC4,	{GL_RGBA,		GL_RGBA,			GL_FLOAT			} },
		};
		if (auto iter = m.find(type); iter != m.end())
			return iter->second;
		return {};
	}


	xMatViewCanvas::~xMatViewCanvas() {
		m_scene.release();
	}

	void xMatViewCanvas::Init() {
	}

	bool xMatViewCanvas::SetImage(cv::Mat const& img) {
		QImage::Format ePixelFormat{QImage::Format_Grayscale8};
		if (img.elemSize1() == 1) {
			switch (img.channels()) {
			case 1: ePixelFormat = QImage::Format_Grayscale8; break;
			case 3: ePixelFormat = QImage::Format_RGB888; break;
			case 4: ePixelFormat = QImage::Format_RGBA8888; break;
			}
		} else {
			return false;
		}

		m_scene = std::make_unique<QGraphicsScene>();
		m_pixmap = QPixmap::fromImage(QImage{ img.data, img.cols, img.rows, (qsizetype)(size_t)img.step, ePixelFormat });
		m_scene->addPixmap(m_pixmap);
		setScene(m_scene.get());
		viewport()->installEventFilter(this);
		viewport()->setMouseTracking(true);

		return true;
	}

	void xMatViewCanvas::wheelEvent(QWheelEvent* event) {
		setTransformationAnchor(base_t::AnchorUnderMouse);

		auto dScale = transform().m11();
		double scalefactor = 1.5;
		if ( (event->angleDelta().y() > 0) and (dScale < 100) ) {
			scale(scalefactor, scalefactor);
		}
		else if ( (event->angleDelta().y() < 0) and (dScale > 0.01) ) {
			scale(1 / scalefactor, 1 / scalefactor);
		}
	}

	void xMatViewCanvas::mousePressEvent(QMouseEvent* event) {
		if (event->button() & Qt::LeftButton) {
			setTransformationAnchor(base_t::NoAnchor);
			m_ptClickL = event->pos();
			m_bClickL = true;
		}
		if (event->button() & Qt::RightButton) {
			if (m_bClickR) {
				m_bClickR = false;
				auto rectF = QRectF(m_ptClickR, m_ptClickR_end).normalized();
				emit sigMouseSelected(Floor(rectF));
			}
			else {
				m_ptClickR_end = m_ptClickR = mapToScene(event->pos());
				m_bClickR = true;
			}
		}

		base_t::mousePressEvent(event);
	}

	void xMatViewCanvas::mouseMoveEvent(QMouseEvent* event) {
		m_pt = event->pos();

		if (m_bClickL) {
			setTransformationAnchor(base_t::NoAnchor);
			QPointF oldp = mapToScene(m_ptClickL);
			QPointF newp = mapToScene(event->pos());
			double speed = m_bClickR ? 1. : 2.;
			QPointF translation = (newp - oldp)*speed;	// increase moving speed

			translate(translation.x(), translation.y());

			if (m_ptClickL != event->pos()) {
				m_ptClickL = event->pos();
			}
		}

		QRectF rectUpdate;
		if (m_bClickR) {
			m_ptClickR_end = mapToScene(event->pos());
			rectUpdate = QRectF(m_ptClickR, m_ptClickR_end).normalized();
			emit sigMouseSelected(Floor(rectUpdate));
		}

		// text
		{
			auto pt = Floor(mapToScene(event->pos()));
			int x = pt.x();
			int y = pt.y();
			if ( (x >= 0) and (x < m_img.cols) and (y >= 0) and (y < m_img.rows) ) {
				auto value = m_img.at<uchar>(y, x);
				std::string str;
				if (m_bClickL) str += "L";
				if (m_bClickR) str += "R";

				emit sigMouseMoved(value, x, y, str);
			}
		}

		if (!Floor(rectUpdate).isEmpty()) {
			m_scene->invalidate();
		}
		base_t::mouseMoveEvent(event);
	}

	void xMatViewCanvas::mouseReleaseEvent(QMouseEvent* event) {
		if (event->button() & Qt::LeftButton)
			m_bClickL = false;
		//if (event->button() & Qt::RightButton)
		//	m_bClickR = false;

		base_t::mouseReleaseEvent(event);
	}

	void xMatViewCanvas::drawBackground(QPainter* painter, const QRectF& rect) {
		base_t::drawBackground(painter, rect);

		QColor cr(215, 220, 240);
		QBrush brush(cr);
		painter->setBrush(brush);
		painter->setPen(cr);
		QRectF r = rect;
		r.setWidth(r.width()+1.);
		r.setHeight(r.height()+1.);
		painter->drawRect(r);
	}

	void xMatViewCanvas::drawForeground(QPainter* painter, const QRectF& rect) {
		base_t::drawForeground(painter, rect);

		// draw grid if zoom scale is greater than 10
		QTransform trans = painter->transform();
		// reset transform
		painter->resetTransform();
		gtl::xFinalAction restoreTransform([&]{
			painter->setTransform(trans);
			});

		auto scale = trans.m11();
		auto roi = Floor(sceneRect());
		// grid
		if (scale >= 4.0) {
			QPen pen;
			pen.setColor(Qt::gray);
			pen.setStyle(Qt::SolidLine);
			painter->setPen(pen);
			for (int i = roi.left(); i <= roi.right(); i ++) {
				auto p0 = trans.map(QPoint{i, roi.top()});
				auto p1 = trans.map(QPoint{i, roi.bottom()});
				painter->drawLine(p0, p1);
			}
			for (int i = roi.top(); i <= roi.bottom(); i ++) {
				auto p0 = trans.map(QPoint{roi.left(), i});
				auto p1 = trans.map(QPoint{roi.right(), i});
				painter->drawLine(p0, p1);
			}
		}

		// pixel value
		if (false and (scale > 20.)) {
			QPen penBlack;
			penBlack.setColor(Qt::black);
			penBlack.setStyle(Qt::SolidLine);
			QPen penWhite;
			penWhite.setColor(Qt::white);
			penWhite.setStyle(Qt::SolidLine);

			auto y0 = std::max(0, roi.top());
			auto y1 = std::min(m_img.rows, roi.bottom()+1);
			auto x0 = std::max(0, roi.left());
			auto x1 = std::min(m_img.cols, roi.right()+1);
			if (m_img.type() == CV_8UC1) {
				for (int y = y0; y < y1; y++) {
					auto* ptr = m_img.ptr<uchar>(y);
					for (int x = x0; x < x1; x++) {
						auto str = std::format("{:X}", ptr[x]);
						painter->setPen((ptr[x] < 128) ? penWhite : penBlack);
						auto r = trans.mapRect(QRectF{(double)x, (double)y, 1.0, 1.0});
						painter->drawText(r, str.c_str(), QTextOption(Qt::AlignCenter));
					}
				}
			} else if (m_img.type() == CV_8UC3) {
				for (int y = y0; y < y1; y++) {
					auto* ptr = m_img.ptr<cv::Vec3b>(y);
					for (int x = x0; x < x1; x++) {
						auto v = ptr[x];
						auto str = std::format("{:02X}.{:02X},{:02X}", v[2], v[1], v[0]);
						painter->setPen((v[0] + v[1] + v[2] < 128*3) ? penWhite : penBlack);
						auto r = trans.mapRect(QRectF{(double)x, (double)y, 1.0, 1.0});
						painter->drawText(r, str.c_str(), QTextOption(Qt::AlignCenter));
					}
				}
			}
		}

		if (m_ptClickR != m_ptClickR_end) {
			// draw grid from m_ptClickR to event->pos()
			// Get Painter
			painter->setRenderHint(QPainter::Antialiasing);
			painter->setPen(QPen(Qt::red, (scale < 3.) ? 1 : 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

			QRectF rectF = QRectF(m_ptClickR, m_ptClickR_end).normalized();
			auto rect = Floor(rectF);
			rect.adjust(0, 0, 1, 1);
			QRect rect2(trans.map(rect.topLeft()), trans.map(rect.bottomRight()));
			painter->drawRect(rect2);
		}
	}

}	// namespace gtl::qt
