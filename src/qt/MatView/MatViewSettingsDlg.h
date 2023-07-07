#pragma once

#include <QDialog>
#include "ui_MatViewSettingsDlg.h"

class xMatViewSettingsDlg : public QDialog
{
	Q_OBJECT

public:
	xMatViewSettingsDlg(QWidget *parent = nullptr);
	~xMatViewSettingsDlg();

private:
	Ui::MatViewSettingsDlgClass ui;
};
