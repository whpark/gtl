#pragma once

#include <QDialog>
#include "ui_MatViewSettingsDlg.h"
#include "gtl/qt/MatView/MatViewCanvas.h"

namespace gtl::qt {

	class xMatViewSettingsDlg : public QDialog {
		Q_OBJECT

	public:
		using this_t = xMatViewSettingsDlg;
		using base_t = QDialog;

	public:
		xMatViewCanvas::S_OPTION m_option;

	public:
		xMatViewSettingsDlg(QWidget* parent = nullptr);
		~xMatViewSettingsDlg();

		bool UpdateData(bool bSaveAndValidate = true);

	protected slots:
		void OnBackgroundColor();
		void OnBtnOK();
		void OnBtnCancel();

	private:
		Ui::MatViewSettingsDlgClass ui;
	};

} // namespace gtl::qt

