#include "pch.h"
#include "gtl/qt/MatView/MatViewGVDlg.h"
#include "ui_MatViewGVDlg.h"

namespace gtl::qt {

	xMatViewGVDlg::xMatViewGVDlg(QWidget* parent)
		: QDialog(parent) {
		ui = std::make_unique<Ui::MatViewGVDlgClass>();
		ui->setupUi(this);
		m_widgetMatView = ui->widgetMatView;
	}

	xMatViewGVDlg::~xMatViewGVDlg() {
	}

	bool xMatViewGVDlg::SetImage(cv::Mat const& img, bool bCopy) {
		if (ui->widgetMatView->SetImage(img, bCopy)) {
			ui->widgetMatView->update();
			return true;
		}
		return false;
	}

}	// namespace gtl::qt
