#pragma once

#include <QDialog>
#include "ui_SeqChildDlg.h"
#include "gtl/sequence_map.h"

#include "glaze/glaze.hpp"

class xSeqChildDlg : public QDialog, public gtl::seq::TSequenceMap<std::string> {
	Q_OBJECT
public:
	using this_t = xSeqChildDlg;
	using base_t = QDialog;

	using seq_map_t = gtl::seq::TSequenceMap<std::string>;
	using seq_t = seq_map_t::seq_t;
	using seq_result_t = seq_t::result_t;
	using seq_param_t = seq_map_t::param_t;

public:
	xSeqChildDlg(seq_map_t& parentSeqMap, QWidget* parent = nullptr);
	~xSeqChildDlg();

public:
	coro_t SeqShowSomeText(seq_t&, seq_param_t);

private:
	Ui::xSeqChildDlgClass ui;
};
