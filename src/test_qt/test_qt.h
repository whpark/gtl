#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_test_qt.h"
#include "gtl/qt/MatView/MatViewGVDlg.h"
#include "gtl/qt/MatView/MatView.h"
#include "gtl/qt/MatView/MatViewDlg.h"
#include "gtl/qt/QPathCompleter.h"
#include "gtl/qt/QGlazeModel.h"

namespace gtl::qt { class test_qt; }

class gtl::qt::test_qt : public QMainWindow {
	Q_OBJECT

public:
	using this_t = test_qt;
	using base_t = QMainWindow;

public:
	test_qt(QWidget* parent = nullptr);
	~test_qt();

	QGlazeModel m_modelGlaze;
	QPathCompleter m_completer;
	std::unique_ptr<gtl::qt::xMatViewGVDlg> m_dlgMatViewGV;
	//std::unique_ptr<gtl::qt::xMatView> m_ctrlMatView;
	std::unique_ptr<gtl::qt::xMatViewDlg> m_dlgMatView;

protected:
	void OnLoadImage();
	void OnSetEnvVar();

private:
	Ui::test_qtClass ui;
};
