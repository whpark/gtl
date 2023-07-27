#pragma once

#include <QDialog>
#include "ui_MatViewSettingsDlg.h"

#include "gtl/qt/MatView/MatView.h"

namespace gtl::qt {
	class xMatViewSettingsDlg;
} // namespace gtl::qt

class gtl::qt::xMatViewSettingsDlg : public QDialog {
	Q_OBJECT

public:
	using this_t = xMatViewSettingsDlg;
	using base_t = QDialog;

	xMatView::S_OPTION m_option;

public:
	xMatViewSettingsDlg(xMatView::S_OPTION option, QWidget* parent = nullptr);
	~xMatViewSettingsDlg() {}

	bool UpdateData(bool bSaveAndValidate = true);

protected:
	void OnOK();
	void OnCancel();
	void OnBackgroundColor();

private:
	Ui::MatViewSettingsDlgClass ui;
};
