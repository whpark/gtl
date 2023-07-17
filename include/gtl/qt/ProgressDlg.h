#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/misc.h"
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
	gtl::callback_progress_t m_calback;
	int m_iPercent{};
	bool m_bUserAbort{};
	bool m_bDone{};
	bool m_bError{};

	std::unique_ptr<std::jthread> m_rThreadWorker;

protected:
	QTimer m_timerUI;

public:
	xProgressDlg(QWidget *parent);
	~xProgressDlg();

	void Init();

protected:
	void OnTimer_UpdateUI();
	void OnButton(QAbstractButton *button);

private:
	std::unique_ptr<Ui::ProgressDlgClass> ui;
};
