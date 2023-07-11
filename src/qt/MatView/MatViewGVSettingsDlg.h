#pragma once

#include <QDialog>
#include "ui_MatViewSettingsDlg.h"
#include "gtl/qt/MatView/MatViewGVCanvas.h"

namespace gtl::qt {

	class xMatViewGVSettingsDlg : public QDialog {
		Q_OBJECT

	public:
		using this_t = xMatViewGVSettingsDlg;
		using base_t = QDialog;

	public:
		xMatViewGVCanvas::S_OPTION m_option;

	public:
		xMatViewGVSettingsDlg(QWidget* parent = nullptr);
		~xMatViewGVSettingsDlg();

		bool UpdateData(bool bSaveAndValidate = true);

	protected slots:
		void OnBackgroundColor();
		void OnBtnOK();
		void OnBtnCancel();

	private:
		Ui::MatViewSettingsDlgClass ui;
	};

} // namespace gtl::qt

