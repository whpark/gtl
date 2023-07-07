#pragma once

#include <QDialog>
#include "ui_MatViewDlg.h"

class xMatViewDlg : public QDialog {
	Q_OBJECT

public:
	xMatViewDlg(QWidget* parent = nullptr);
	~xMatViewDlg();

private:
	Ui::MatViewDlgClass ui;
};
