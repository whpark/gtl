#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QWidget>
#include "ui_MatViewWidget.h"

namespace gtl::qt {

	class GTL__QT_CLASS xMatViewWidget : public QWidget {
		Q_OBJECT

	public:
		xMatViewWidget(QWidget* parent = nullptr);
		~xMatViewWidget();

	private:
		Ui::MatViewWidgetClass ui;
	};

}	// namespace gtl::qt
