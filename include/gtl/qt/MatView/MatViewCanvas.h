#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QOpenGLWidget>

namespace gtl::qt {
	class GTL__QT_CLASS xMatViewCanvas;
}

class gtl::qt::xMatViewCanvas : public QOpenGLWidget {
	friend class xMatView;
	Q_OBJECT

public:
	using this_t = xMatViewCanvas;
	using base_t = QOpenGLWidget;

protected:
	std::function<void(xMatViewCanvas*)> m_fnInitializeGL;
	std::function<void(xMatViewCanvas*)> m_fnPaintGL;
	std::function<void(xMatViewCanvas*, QMouseEvent* event)> m_fnMousePress;
	std::function<void(xMatViewCanvas*, QMouseEvent* event)> m_fnMouseRelease;
	std::function<void(xMatViewCanvas*, QMouseEvent* event)> m_fnMouseDoubleClick;
	std::function<void(xMatViewCanvas*, QMouseEvent* event)> m_fnMouseMove;
	std::function<void(xMatViewCanvas*, QWheelEvent* event)> m_fnWheel;

public:
	xMatViewCanvas(QWidget* parent): QOpenGLWidget(parent) {}
	~xMatViewCanvas() {}

protected:
	void initializeGL() override {
		base_t::initializeGL();
		if (m_fnInitializeGL)
			m_fnInitializeGL(this);
	}
	void paintGL() override {
		if (m_fnPaintGL)
			m_fnPaintGL(this);
	}

	void mousePressEvent(QMouseEvent *event) override {
		if (m_fnMousePress)
			m_fnMousePress(this, event);
	}
	void mouseReleaseEvent(QMouseEvent *event) override {
		if (m_fnMouseRelease)
			m_fnMouseRelease(this, event);
	}
	void mouseDoubleClickEvent(QMouseEvent* event) override {
		if (m_fnMouseDoubleClick)
			m_fnMouseDoubleClick(this, event);
	}
	void mouseMoveEvent(QMouseEvent *event) override {
		if (m_fnMouseMove)
			m_fnMouseMove(this, event);
	}
	void wheelEvent(QWheelEvent* event) override {
		if (m_fnWheel)
			m_fnWheel(this, event);
	}
};
