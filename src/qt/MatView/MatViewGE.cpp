#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/MatView/MatViewWidget.h"
#include "ui_MatViewWidget.h"
#include "MatViewSettingsDlg.h"

namespace gtl::qt {

	xMatViewWidget::xMatViewWidget(QWidget* parent) : QWidget(parent), ui(std::make_unique<Ui::MatViewWidgetClass>()) {
		ui->setupUi(this);
		m_view = ui->view;

		connect(ui->toolbar->spinZoom, &QDoubleSpinBox::valueChanged, this, &this_t::OnSpinZoomValueChanged);
		connect(ui->toolbar->btnZoomIn, &QPushButton::clicked, this, &this_t::OnBtnZoomIn);
		connect(ui->toolbar->btnZoomOut, &QPushButton::clicked, this, &this_t::OnBtnZoomOut);
		connect(ui->toolbar->btnZoomFit, &QPushButton::clicked, this, &this_t::OnBtnZoomFit);
		connect(ui->toolbar->btnSettings, &QPushButton::clicked, this, &this_t::OnBtnSettings);

		connect(m_view, &xMatViewCanvas::sigMouseMoved, this, &xMatViewWidget::OnSigMouseMoved);
		connect(m_view, &xMatViewCanvas::sigMouseSelected, this, &xMatViewWidget::OnSigMouseSelected);
		connect(m_view, &xMatViewCanvas::sigZoom, this, &xMatViewWidget::OnSigZoom);


	}

	xMatViewWidget::~xMatViewWidget() {
	}

	void xMatViewWidget::OnSigMouseMoved(int x, int y) {
		if (!m_view or m_view->m_img.empty())
			return;
		auto status = std::format("{},{}", x, y);
		if (x < 0 or y < 0 or x >= m_view->m_img.cols or y >= m_view->m_img.rows) {
			ui->toolbar->edtInfo->setText(ToQString(status));
			return;
		}
		auto const& img = m_view->m_img;
		if (m_view->m_mouse.bInSelectionMode or m_view->m_mouse.bRectSelected) {
			auto size = m_view->m_mouse.ptSel1 - m_view->m_mouse.ptSel0;
			status += std::format(" w{}, h{}", size.x(), size.y());
		}
		auto value = gtl::GetMatValue(img.ptr(y), img.depth(), img.channels(), y, x);
		status += "(";
		for (int i{}; i < img.channels(); i++) {
			status += std::format(" {}", value[i]);
		}
		status += ")";

		//if (status != m_textStatus->GetValue().ToStdWstring()) {
		ui->toolbar->edtInfo->setText(ToQString(status));
		//}
	}

	void xMatViewWidget::OnSigMouseSelected(QRect rect) {

	}

	void xMatViewWidget::OnSigZoom(double zoom) {
		ui->toolbar->spinZoom->setValue(zoom);
	}

	void xMatViewWidget::OnSpinZoomValueChanged(double value) {
		m_view->SetZoom(value, m_view->rect().center());
	}

	void xMatViewWidget::OnBtnZoomIn() {
		m_view->ZoomInOut(1, {});
	}

	void xMatViewWidget::OnBtnZoomOut() {
		m_view->ZoomInOut(-1, {});
	}

	void xMatViewWidget::OnBtnZoomFit() {
		if (m_view->m_img.empty() or m_view->m_img.size().area() <= 0)
			return;
		auto rect = m_view->rect();
		double dScaleX = (double)rect.width() / m_view->m_img.cols;
		double dScaleY = (double)rect.height() / m_view->m_img.rows;
		double dScale = std::min(dScaleX, dScaleY);
		m_view->SetZoom(dScale, {});
	}

	void xMatViewWidget::OnBtnSettings() {
		xMatViewSettingsDlg dlg;
		dlg.m_option = m_view->m_option;
		dlg.UpdateData(false);
		auto r = dlg.exec();
		if (r != QDialog::Accepted)
			return;
		dlg.UpdateData(true);
		m_view->SetOption(dlg.m_option);
	}

} // namespace gtl::qt
