#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_test_qt.h"
#include "gtl/qt/MatView/MatViewDlg.h"

class test_qt : public QMainWindow {
	Q_OBJECT

public:
	test_qt(QWidget* parent = nullptr);
	~test_qt();

	std::unique_ptr<gtl::qt::xMatViewDlg> m_dlgMatView;

private:
	Ui::test_qtClass ui;
};
