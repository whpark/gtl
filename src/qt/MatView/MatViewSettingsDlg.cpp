#include "pch.h"
#include <QColorDialog>
#include "MatViewSettingsDlg.h"

namespace gtl::qt {

	xMatViewSettingsDlg::xMatViewSettingsDlg(QWidget* parent)
		: QDialog(parent) {
		ui.setupUi(this);
		connect(ui.btnBackgroundColor, &QPushButton::clicked, this, &xMatViewSettingsDlg::OnBackgroundColor);
	}

	xMatViewSettingsDlg::~xMatViewSettingsDlg() {
	}

	bool xMatViewSettingsDlg::UpdateData(bool bSaveAndValidate) {
		if (bSaveAndValidate) {
			m_option.bZoomLock = ui.chkZoomLock->isChecked();
			m_option.bPanningLock = ui.chkPanningLock->isChecked();
			m_option.bExtendedPanning = ui.chkExtendedPanning->isChecked();
			m_option.bKeyboardNavigation = ui.chkKeyboardNavigation->isChecked();
			m_option.bDrawPixelValue = ui.chkDrawPixelValue->isChecked();
			m_option.bSmoothInterpolation = ui.chkSmoothInterpolation->isChecked();
		}
		else {
			ui.chkZoomLock->setChecked(m_option.bZoomLock);
			ui.chkPanningLock->setChecked(m_option.bPanningLock);
			ui.chkExtendedPanning->setChecked(m_option.bExtendedPanning);
			ui.chkKeyboardNavigation->setChecked(m_option.bKeyboardNavigation);
			ui.chkDrawPixelValue->setChecked(m_option.bDrawPixelValue);
			ui.chkSmoothInterpolation->setChecked(m_option.bSmoothInterpolation);
		}
		return false;
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
