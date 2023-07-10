#pragma once

#include <QDialog>
#include "ui_MatViewSettingsDlg.h"
#include "gtl/qt/MatView/MatView.h"

namespace gtl::qt {

	class xMatViewSettingsDlg : public QDialog {
		Q_OBJECT

	public:
		using this_t = xMatViewSettingsDlg;
		using base_t = QDialog;

	public:
		xMatView::S_OPTION m_option;

	public:
		xMatViewSettingsDlg(QWidget* parent = nullptr);
		~xMatViewSettingsDlg();

		bool UpdateData(bool bSaveAndValidate = true);

	protected slots:
		void OnBackgroundColor();

	private:
		Ui::MatViewSettingsDlgClass ui;
	};

} // namespace gtl::qt

