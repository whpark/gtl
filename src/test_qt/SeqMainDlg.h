#pragma once

#include <QDialog>
#include "ui_SeqMainDlg.h"

#include "gtl/sequence_map.h"

using seq_t = gtl::seq::xSequence;

class xSeqMainDlg : public QDialog, public gtl::seq::TSequenceMap<> {
	Q_OBJECT

public:
	using this_t = xSeqMainDlg;
	using base_t = QDialog;
	using seq_map_t = gtl::seq::TSequenceMap<>;
	using seq_param_t = seq_map_t::param_t;

public:
	xSeqMainDlg(QWidget *parent = nullptr);
	~xSeqMainDlg();

	QTimer m_timer;
	gtl::seq::xSequence m_driver;

public:
	seq_t Seq1(std::shared_ptr<seq_map_t::sParam> param);
	seq_t Seq2(std::shared_ptr<seq_map_t::sParam> param);

private:
	Ui::xSeqMainDlgClass ui;
};
