#include "pch.h"
#include "MatViewToolbar.h"

namespace gtl::qt {

	xMatViewToolbar::xMatViewToolbar(QWidget* parent)
		: QWidget(parent) {
		ui.setupUi(this);
	}

	xMatViewToolbar::~xMatViewToolbar() {
	}

} // namespace gtl::qt
