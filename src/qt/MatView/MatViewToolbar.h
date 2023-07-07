#pragma once

#include <QWidget>
#include "ui_MatViewToolbar.h"

class xMatViewToolbar : public QWidget {
	Q_OBJECT

public:
	xMatViewToolbar(QWidget* parent = nullptr);
	~xMatViewToolbar();

private:
	Ui::MatViewToolbarClass ui;
};
