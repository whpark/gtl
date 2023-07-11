#pragma once

#include <QWidget>
#include "ui_MatViewGVToolbar.h"

namespace gtl::qt {
	class xMatViewGVToolbar;
} // namespace gtl::qt

class gtl::qt::xMatViewGVToolbar : public QWidget, public Ui::MatViewGVToolbarClass {
	Q_OBJECT

public:
	xMatViewGVToolbar(QWidget* parent = nullptr);
	~xMatViewGVToolbar();

};
