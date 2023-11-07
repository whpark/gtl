#pragma once

#include <QDialog>
#include "ui_SeqMainDlg.h"

#include "gtl/sequence_map.h"

#include "glaze/glaze.hpp"

using seq_t = gtl::seq::xSequence;
using seq_map_t = gtl::seq::TSequenceMap<glz::json_t, glz::json_t>;
using sParam = seq_map_t::sParam;

class xSeqMainDlg : public QDialog, public seq_map_t {
	Q_OBJECT

public:
	using this_t = xSeqMainDlg;
	using base_t = QDialog;
	using seq_map_t = seq_map_t;

public:
	xSeqMainDlg(QWidget *parent = nullptr);
	~xSeqMainDlg();

	QTimer m_timer;
	seq_t m_driver;

public:
	seq_t Seq1(std::shared_ptr<sParam> param);
	seq_t Seq2(std::shared_ptr<sParam> param);

private:
	Ui::xSeqMainDlgClass ui;
};
