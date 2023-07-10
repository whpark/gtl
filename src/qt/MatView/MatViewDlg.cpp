#include "pch.h"
#include "gtl/qt/MatView/MatViewDlg.h"
#include "ui_MatViewDlg.h"

namespace gtl::qt {

	xMatViewDlg::xMatViewDlg(QWidget* parent)
		: QDialog(parent) {
		ui = std::make_unique<Ui::MatViewDlgClass>();
		ui->setupUi(this);
		m_widgetMatView = ui->widgetMatView;
	}

	xMatViewDlg::~xMatViewDlg() {
	}

	bool xMatViewDlg::SetImage(cv::Mat const& img, bool bCopy) {
		if (ui->widgetMatView->SetImage(img, bCopy)) {
			ui->widgetMatView->update();
			return true;
		}
		return false;
	}

}	// namespace gtl::qt
