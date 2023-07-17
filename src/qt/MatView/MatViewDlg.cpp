#include "pch.h"

#include "ui_MatViewDlg.h"
#include "gtl/qt/MatView/MatViewDlg.h"

namespace gtlq = gtl::qt;

gtlq::xMatViewDlg::xMatViewDlg(QWidget* parent) : QDialog(parent), ui(std::make_unique<Ui::MatViewDlgClass>()) {
	ui->setupUi(this);
	m_view = ui->view;
}

gtlq::xMatViewDlg::~xMatViewDlg() {
}
