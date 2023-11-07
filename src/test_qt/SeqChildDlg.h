#pragma once

#include <QDialog>
#include "ui_SeqChildDlg.h"
#include "gtl/sequence_map.h"

#include "glaze/glaze.hpp"

using seq_t = gtl::seq::xSequence;
using seq_map_t = gtl::seq::TSequenceMap<glz::json_t, glz::json_t>;
using sParam = seq_map_t::sParam;

class xSeqChildDlg : public QDialog, public gtl::seq::TSequenceMap<glz::json_t> {
	Q_OBJECT
public:
	using this_t = xSeqChildDlg;
	using base_t = QDialog;

public:
	xSeqChildDlg(seq_map_t& parentSeqMap, QWidget* parent = nullptr);
	~xSeqChildDlg();

public:
	seq_t SeqShowSomeText(std::shared_ptr<seq_map_t::sParam> param);

private:
	Ui::xSeqChildDlgClass ui;
};
