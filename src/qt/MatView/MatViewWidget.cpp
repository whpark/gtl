#include "pch.h"
#include "gtl/qt/MatView/MatViewWidget.h"

namespace gtl::qt {

	xMatViewWidget::xMatViewWidget(QWidget* parent) : QWidget(parent) {
		ui.setupUi(this);
	}

	xMatViewWidget::~xMatViewWidget() {
	}

} // namespace gtl::qt
