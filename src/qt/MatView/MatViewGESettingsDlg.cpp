#include "pch.h"
#include <QColorDialog>
#include "MatViewSettingsDlg.h"

namespace gtl::qt {

	xMatViewSettingsDlg::xMatViewSettingsDlg(QWidget* parent)
		: QDialog(parent) {
		ui.setupUi(this);
		connect(ui.btnBackgroundColor, &QPushButton::clicked, this, &xMatViewSettingsDlg::OnBackgroundColor);
		connect(ui.btnOK, &QPushButton::clicked, this, &this_t::OnBtnOK);
		connect(ui.btnCancel, &QPushButton::clicked, this, &this_t::OnBtnCancel);
	}

	xMatViewSettingsDlg::~xMatViewSettingsDlg() {
	}

	bool xMatViewSettingsDlg::UpdateData(bool bSaveAndValidate) {
		if (bSaveAndValidate) {
			m_option.bDrawPixelValue = ui.chkDrawPixelValue->isChecked();
			m_option.bSmoothInterpolation = ui.chkSmoothInterpolation->isChecked();
			m_option.dMouseSpeed = ui.spinMouseSpeed->value();
			auto cr = QColor(ui.edtColorBackground->text());
			m_option.crBackground = cv::Vec3b(cr.red(), cr.green(), cr.blue());
		}
		else {
			ui.chkDrawPixelValue->setChecked(m_option.bDrawPixelValue);
			ui.chkSmoothInterpolation->setChecked(m_option.bSmoothInterpolation);
			ui.spinMouseSpeed->setValue(m_option.dMouseSpeed);
			ui.edtColorBackground->setText(QColor(m_option.crBackground[0], m_option.crBackground[1], m_option.crBackground[2]).name());
		}
		return false;
	}

	void xMatViewSettingsDlg::OnBtnOK() {
		UpdateData(true);
		accept();
	}

	void xMatViewSettingsDlg::OnBtnCancel() {
		reject();
	}

	void xMatViewSettingsDlg::OnBackgroundColor() {
		QColor color = QColorDialog::getColor(QColor(ui.edtColorBackground->text()), this, tr("Select Color"));
		if (color.isValid()) {
			m_option.crBackground[0] = color.red();
			m_option.crBackground[1] = color.green();
			m_option.crBackground[2] = color.blue();
			ui.edtColorBackground->setText(color.name());
		}
	}

} // namespace gtl::qt
