#pragma once

#include <QDialog>
#include "ui_TestSeqDlg.h"
#include "gtl/sequence.h"

class xTestSeqDlg : public QDialog, public gtl::seq::v01::TSequence<xTestSeqDlg> {
	Q_OBJECT
public:
	using this_t = xTestSeqDlg;
	using base_seq_t = gtl::seq::v01::TSequence<xTestSeqDlg>;
	using sequence_t = gtl::seq::v01::sSequence;
	using base_t = QDialog;

public:
	xTestSeqDlg(QWidget* parent = nullptr);
	~xTestSeqDlg();

	QTimer m_timer;
	void Dispatch();

	sequence_t Seq1(sequence_t& self);
	sequence_t Suspend(sequence_t& self);
	sequence_t Suspend2(sequence_t& self, int);

Q_SIGNALS:
	void sigSequence();

private:
	Ui::xTestSeqDlgClass ui;
};
