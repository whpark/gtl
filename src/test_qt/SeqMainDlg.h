#pragma once

#include <QDialog>
#include "ui_SeqMainDlg.h"

#include "gtl/sequence_map.h"

#include "glaze/glaze.hpp"

class xSeqMainDlg : public QDialog, public gtl::seq::TSequenceMap<std::string> {
	Q_OBJECT

public:
	using this_t = xSeqMainDlg;
	using base_t = QDialog;

	using seq_map_t = gtl::seq::TSequenceMap<std::string>;
	using seq_t = seq_map_t::seq_t;
	using seq_result_t = seq_t::result_t;
	using seq_param_t = seq_map_t::param_t;

public:
	xSeqMainDlg(QWidget *parent = nullptr);
	~xSeqMainDlg();

	QTimer m_timer;
	seq_t m_driver;

public:
	seq_t Seq1(seq_param_t param);
	seq_t Seq2(seq_param_t param);

private:
	Ui::xSeqMainDlgClass ui;
};
