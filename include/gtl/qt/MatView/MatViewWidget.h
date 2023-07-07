#pragma once

#include <QWidget>
#include "ui_MatViewWidget.h"

class xMatViewWidget : public QWidget {
	Q_OBJECT

public:
	xMatViewWidget(QWidget* parent = nullptr);
	~xMatViewWidget();

private:
	Ui::MatViewWidgetClass ui;
};
