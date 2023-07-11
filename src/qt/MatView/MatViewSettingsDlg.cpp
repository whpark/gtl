#include "pch.h"
#include "gtl/qt/qt.h"
#include "MatViewSettingsDlg.h"

namespace gtlq = gtl::qt;
using namespace std::literals;

gtlq::xMatViewSettingsDlg::xMatViewSettingsDlg(xMatView::S_OPTION const& option, QWidget* parent) : QDialog(parent), m_option(option) {
	ui.setupUi(this);

	connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &this_t::OnOK);
	connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &this_t::OnCancel);
	connect(ui.btnBackgroundColor, &QPushButton::clicked, this, &this_t::OnBackgroundColor);

	UpdateData(false);
}

bool gtl::qt::xMatViewSettingsDlg::UpdateData(bool bSaveAndValidate) {
	auto const& b = bSaveAndValidate;
	UpdateWidgetValue(b, ui.chkZoomLock, m_option.bZoomLock);
	UpdateWidgetValue(b, ui.chkPanningLock, m_option.bPanningLock);
	UpdateWidgetValue(b, ui.chkExtendedPanning, m_option.bExtendedPanning);
	UpdateWidgetValue(b, ui.chkKeyboardNavigation, m_option.bKeyboardNavigation);
	UpdateWidgetValue(b, ui.chkDrawPixelValue, m_option.bDrawPixelValue);
	UpdateWidgetValue(b, ui.chkBuildPyramidImage, m_option.bPyrImageDown);
	UpdateWidgetValue(b, ui.spinPanningSpeed, m_option.dPanningSpeed);
	UpdateWidgetValue(b, ui.spinScrollMargin, m_option.nScrollMargin);
	if (b) {
		m_option.tsScroll = std::chrono::milliseconds{(int)ui.spinScrollDuration->value()};
	}
	else {
		ui.spinScrollDuration->setValue(m_option.tsScroll.count());
	}
	if (bSaveAndValidate) {
		auto cr = QColor(ui.edtColorBackground->text());
		m_option.crBackground = cv::Vec3b(cr.red(), cr.green(), cr.blue());
	}
	else {
		ui.edtColorBackground->setText(QColor(m_option.crBackground[0], m_option.crBackground[1], m_option.crBackground[2]).name());
	}

	return false;
}

void gtl::qt::xMatViewSettingsDlg::OnOK() {
	UpdateData(true);
	base_t::accept();
}

void gtl::qt::xMatViewSettingsDlg::OnCancel() {
	base_t::reject();
}

void gtl::qt::xMatViewSettingsDlg::OnBackgroundColor() {
	QColor color = QColorDialog::getColor(QColor(ui.edtColorBackground->text()), this, tr("Select Color"));
	if (color.isValid()) {
		m_option.crBackground[0] = color.red();
		m_option.crBackground[1] = color.green();
		m_option.crBackground[2] = color.blue();
		ui.edtColorBackground->setText(color.name());
	}
}
