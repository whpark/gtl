#pragma once

#include "gtl/gtl.h"
#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/reflection_glaze.h"
#include <QGraphicsView>
#include <QGraphicsScene>

namespace gtl::qt {
	class GTL__QT_CLASS xMatViewGVCanvas;
} // namespace gtl::qt

class gtl::qt::xMatViewGVCanvas : public QGraphicsView {
	friend class xMatViewGV;

	Q_OBJECT

public:
	using this_t = xMatViewGVCanvas;
	using base_t = QGraphicsView;

	using string_t = std::string;
	//using xCoordTrans = gtl::xCoordTrans2d;

public:
	std::unique_ptr<QGraphicsScene> m_scene;
	QPixmap m_pixmap;
	cv::Mat m_img;

public:
	struct S_OPTION {
		bool bDrawPixelValue{true};			// draw pixel value on image
		bool bSmoothInterpolation{false};	// smooth interpolation
		double dPanningSpeed{2.0};			// Image Panning Speed. 1.0 is same with mouse move.
		cv::Vec3b crBackground{161, 114, 230};	// rgb

		bool operator == (S_OPTION const&) const = default;
		bool operator != (S_OPTION const&) const = default;

		// Sync with json
		GLZ_LOCAL_META(S_OPTION, bDrawPixelValue, bSmoothInterpolation, dPanningSpeed, crBackground);
	};

	std::string m_strCookie;
	std::function<bool(bool bStore, std::string_view cookie, S_OPTION&)> m_fnSyncSetting;

protected:
	//QColor m_crBackground{"black"};
	mutable struct {
		bool bInSelectionMode{};
		bool bRectSelected{};
		std::optional<QPoint> ptAnchor;			// Anchor Point in Screen Coordinate
		QPoint ptOffset0;			// offset backup
		QPointF ptSel0, ptSel1;	// Selection Point in Image Coordinate

		void Clear() {
			bInSelectionMode = {};
			bRectSelected = {};
			ptAnchor.reset();
			ptOffset0 = {};
			ptSel0 = ptSel1 = {};
		}
	} m_mouse;
	mutable struct {
		QPointF pt0, pt1;
		std::chrono::steady_clock::time_point t0, t1;
		void Clear() {
			pt0 = pt1 = {};
			t0 = t1 = {};
		}
	} m_smooth_scroll;

	S_OPTION m_option;

public:
	xMatViewGVCanvas(QWidget *parent = nullptr) : QGraphicsView(parent) { Init(); }
	xMatViewGVCanvas(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) { Init(); }

	~xMatViewGVCanvas();

	void Init();
	void SetBackgroundColor(QColor crBackground);
	QColor GetBackgroundColor() const { return QColor(m_option.crBackground[0], m_option.crBackground[1], m_option.crBackground[2]); }

public:
	bool SetImage(cv::Mat const& img, bool bCopy = false);

	std::optional<xRect2i> GetSelectionRect() const {
		if (!m_mouse.bRectSelected)
			return {};
		xRect2i rect(gtl::Floor(xPoint2i(m_mouse.ptSel0.x(), m_mouse.ptSel0.y())), gtl::Floor(xPoint2i(m_mouse.ptSel1.x(), m_mouse.ptSel1.y())));
		rect.NormalizeRect();
		return rect;
	}
	std::optional<std::pair<xPoint2d, xPoint2d>> GetSelectionPoints() const {
		if (!m_mouse.bRectSelected)
			return {};
		return std::pair<xPoint2d, xPoint2d>{{m_mouse.ptSel0.x(), m_mouse.ptSel0.y()}, {m_mouse.ptSel1.x(), m_mouse.ptSel1.y()}};
	}
	void SetSelectionRect(xRect2i const& rect);
	void ClearSelectionRect();

	bool LoadOption() { return m_fnSyncSetting and m_fnSyncSetting(false, m_strCookie, m_option) and SetOption(m_option, false); }
	bool SaveOption() { return m_fnSyncSetting and m_fnSyncSetting(true, m_strCookie, m_option); }
	S_OPTION const& GetOption() const { return m_option; }
	bool SetOption(S_OPTION const& option, bool bStore = true);

	//bool ShowToolBar(bool bShow);
	//bool IsToolBarShown() const { return m_toolBar->IsShown(); }

	//virtual void OnClose(wxCloseEvent& event) override;

	// ClientRect, ImageRect, ScrollRange
	bool ZoomInOut(double step, QPointF ptAnchor);
	bool SetZoom(double scale, QPointF ptAnchor);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	//virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
	virtual void drawForeground(QPainter *painter, const QRectF &rect) override;

public:
signals:
	// signal fire when mouse move
	void sigMouseMoved(int x, int y);
	void sigMouseSelected(QRect rect);
	void sigZoom(double zoom);
};
