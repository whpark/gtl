#pragma once

#include <QWidget>
#include "ui_MatViewToolbar.h"

namespace gtl::qt {

	class xMatViewToolbar : public QWidget, public Ui::MatViewToolbarClass {
		Q_OBJECT

	public:
		xMatViewToolbar(QWidget* parent = nullptr);
		~xMatViewToolbar();

	};

} // namespace gtl::qt
