#pragma once

#include <QDialog>
#include "ui_TestSeqDlg.h"
#include "gtl/sequence.h"

class xTestSeqDlg : public QDialog {
	Q_OBJECT
public:
	using this_t = xTestSeqDlg;
	using base_t = QDialog;

public:
	xTestSeqDlg(QWidget* parent = nullptr);
	~xTestSeqDlg();

	gtl::xSequence m_main;
	std::jthread m_thread;

	gtl::xSequence Seq1(gtl::xSequence& self);

Q_SIGNALS:
	void sigSequence();

private:
	Ui::xTestSeqDlgClass ui;
};
