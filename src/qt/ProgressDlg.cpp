#include "pch.h"
#include "ui_ProgressDlg.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/ProgressDlg.h"

using namespace std::literals;

gtl::qt::xProgressDlg::xProgressDlg(QWidget* parent) : QDialog(parent) {
	ui = std::make_unique<Ui::ProgressDlgClass>();
	ui->setupUi(this);

	Init();
}

gtl::qt::xProgressDlg::~xProgressDlg() {
}

void gtl::qt::xProgressDlg::Init() {
	if (!m_title.empty())
		this->setWindowTitle(ToQString(m_title));
	ui->labelMessage->setText(ToQString(m_message));
	ui->progressBar->setRange(0, 100);

	m_calback = [&](int iPercent, bool bDone, bool bError)->bool{
		m_iPercent = iPercent;
		m_bError = bError;
		m_bDone = bDone;
		return !m_bUserAbort;
	};

	m_tStart  = std::chrono::system_clock::now();
	m_timerUI.start(100ms);
}

void gtl::qt::xProgressDlg::OnTimer_UpdateUI() {
	if (ui and ui->progressBar) {
		auto& prog = *ui->progressBar;
		if (prog.value() != m_iPercent)
			prog.setValue(m_iPercent);
		auto ts = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now() - m_tStart);
		auto str = ToQString(std::format(L"{:.3f} 초, {} %", ts.count(), m_iPercent));
		if (ui->edtStatus->text() != str)
			ui->edtStatus->setText(str);
	}

	if (m_bDone) {
		if (m_bUserAbort)
			emit rejected();
		else if (m_bError)
			emit rejected();
		else
			emit accepted();
		return;
	}
}

void gtl::qt::xProgressDlg::OnAbort() {
	m_bUserAbort = true;
	reject();
}
