#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_test_qt.h"
#include "gtl/qt/MatView/MatViewGVDlg.h"
#include "gtl/qt/MatView/MatView.h"
#include "gtl/qt/MatView/MatViewDlg.h"

namespace gtl::qt { class test_qt; }

class gtl::qt::test_qt : public QMainWindow {
	Q_OBJECT

public:
	test_qt(QWidget* parent = nullptr);
	~test_qt();

	std::unique_ptr<gtl::qt::xMatViewGVDlg> m_dlgMatViewGV;
	std::unique_ptr<gtl::qt::xMatView> m_ctrlMatView;
	std::unique_ptr<gtl::qt::xMatViewDlg> m_dlgMatView;

private:
	Ui::test_qtClass ui;
};
