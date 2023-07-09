#pragma once

#include <QWidget>
#include "ui_MatViewToolbar.h"

namespace gtl::qt {

	class xMatViewToolbar : public QWidget {
		Q_OBJECT

	public:
		xMatViewToolbar(QWidget* parent = nullptr);
		~xMatViewToolbar();

	private:
		Ui::MatViewToolbarClass ui;
	};

} // namespace gtl::qt
