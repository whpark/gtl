#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QDialog>
#include "ui_MatViewDlg.h"

namespace gtl::qt {

	class GTL__QT_CLASS xMatViewDlg : public QDialog {
		Q_OBJECT

	public:
		using this_t = xMatViewDlg;
		using base_t = QDialog;

	public:
		xMatViewDlg(QWidget* parent = nullptr);
		~xMatViewDlg();

	private:
		Ui::MatViewDlgClass ui;
	};

} // namespace gtl::qt
