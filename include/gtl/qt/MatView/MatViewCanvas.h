#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/gtl.h"
#include <QGraphicsView>
#include <QGraphicsScene>

namespace gtl::qt {

	class GTL__QT_CLASS xMatViewCanvas : public QGraphicsView {
		Q_OBJECT

	public:
		using this_t = xMatViewCanvas;
		using base_t = QGraphicsView;

		using string_t = std::string;
		//using xCoordTrans = gtl::xCoordTrans2d;

	public:
		std::unique_ptr<QGraphicsScene> m_scene;
		QPixmap m_pixmap;

	protected:
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

	public:
		xMatViewCanvas(QWidget *parent = nullptr) : QGraphicsView(parent) { Init(); }
		xMatViewCanvas(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) { Init(); }

		~xMatViewCanvas();

		void Init();

	public:
		bool SetImage(cv::Mat const& img);

		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
		virtual void drawForeground(QPainter *painter, const QRectF &rect) override;

	public:
	signals:
		// signal fire when mouse move
		void sigMouseMoved(uchar value, int x, int y, std::string str);
		void sigMouseSelected(QRect rect);
	};

} // namespace gtl::qt
