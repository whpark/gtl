﻿#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/misc.h"
#include <QTimer>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
	class ProgressDlgClass;
} // namespace Ui
QT_END_NAMESPACE

namespace gtl::qt {
	class GTL__QT_CLASS xProgressDlg;
}

/// @brief Progress Dialog
class gtl::qt::xProgressDlg  : public QDialog {
	Q_OBJECT
public:
	using this_t = xProgressDlg;
	using base_t = QDialog;

public:
	std::wstring m_title;
	std::wstring m_message;
	std::chrono::system_clock::time_point m_tStart;
	int m_iPercent{};
	bool m_bUserAbort{};
	bool m_bDone{};
	bool m_bError{};

	std::unique_ptr<std::jthread> m_rThreadWorker;

protected:
	QTimer m_timerUI;
	gtl::callback_progress_t m_callback;

public:
	xProgressDlg(QWidget *parent);
	~xProgressDlg();

	void Init();
	bool UpdateProgress(int iPercent, bool bDone, bool bError) {
		m_iPercent = iPercent;
		m_bDone = bDone;
		m_bError = bError;
		return !m_bUserAbort;
	}
	gtl::callback_progress_t GetCallback() const { return m_callback; }

protected:
	void OnTimer_UpdateUI();
	void OnButton(QAbstractButton *button);

private:
	std::unique_ptr<Ui::ProgressDlgClass> ui;
};
