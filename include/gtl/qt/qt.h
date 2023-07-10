#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QApplication>

namespace gtl::qt {

	class xWaitCursor {
	public:
		xWaitCursor() {
			QApplication::setOverrideCursor(Qt::WaitCursor);
		}
		~xWaitCursor() {
			QApplication::restoreOverrideCursor();
		}
	};


} // namespace gtl::qt
