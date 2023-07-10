#pragma once

#define NOMINMAX
#pragma warning(push)
#pragma warning(disable: 5104 5105)
//#include <windows.h>
#pragma warning(pop)

#include "gtl/gtl.h"

#include <QtCore/qglobal.h>
#include <QtWidgets>
#include <QApplication>

inline QPoint Floor(QPointF pt) {
	return QPoint{ (int)pt.x(), (int)pt.y() };
}
inline QRect Floor(QRectF rect) {
	return QRect{ Floor(rect.topLeft()), Floor(rect.bottomRight()) };
}
