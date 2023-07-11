#include "pch.h"
#include "gtl/qt/MatView/MatView.h"
#include "ui_MatView.h"

#include "MatViewSettingsDlg.h"

namespace gtlq = gtl::qt;

gtlq::xMatView::xMatView(QWidget* parent) : QWidget(parent), ui(std::make_unique<Ui::MatViewClass>()) {
	ui->setupUi(this);
	connect(ui->btnSettings, &QPushButton::clicked, this, &xMatView::OnSettings);
}

gtlq::xMatView::~xMatView() {
}

bool gtlq::xMatView::Init() {
	return false;
}

void gtlq::xMatView::OnSettings() {
	xMatViewSettingsDlg dlg(m_option, this);
	if (dlg.exec() != QDialog::Accepted)
		return;
	//SetOption(dlg.m_option);
}
