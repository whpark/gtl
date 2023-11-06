#pragma once

#include <QDialog>
#include "ui_TestSeqDlg.h"
#include "gtl/sequence.h"

using seq_t = gtl::seq::xSequence;

class xTestSeqDlg : public QDialog, public gtl::seq::v01::TSequence<xTestSeqDlg> {
	Q_OBJECT
public:
	using this_t = xTestSeqDlg;
	using base_seq_t = gtl::seq::v01::TSequence<xTestSeqDlg>;
	using base_t = QDialog;
	using seq_param_t = std::shared_ptr<base_seq_t::sParam>;

public:
	xTestSeqDlg(QWidget* parent = nullptr);
	~xTestSeqDlg();

	QTimer m_timer;
	std::jthread m_thread;
	void Dispatch();

	seq_t Seq1(seq_param_t param = {});
	seq_t Suspend(seq_param_t param = {});
	seq_t SuspendHandler(seq_param_t param = {});
	seq_t SuspendAny(int);

Q_SIGNALS:
	void sigSequence();

private:
	Ui::xTestSeqDlgClass ui;
};
