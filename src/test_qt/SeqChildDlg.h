#pragma once

#include <QDialog>
#include "ui_SeqChildDlg.h"
#include "gtl/sequence_map.h"

using seq_t = gtl::seq::xSequence;

class xSeqChildDlg : public QDialog, public gtl::seq::TSequenceMap<> {
	Q_OBJECT
public:
	using this_t = xSeqChildDlg;
	using base_t = QDialog;
	using seq_map_t = gtl::seq::TSequenceMap<>;

public:
	xSeqChildDlg(seq_map_t& parentSeqMap, QWidget* parent = nullptr);
	~xSeqChildDlg();

public:
	seq_t SeqShowSomeText(std::shared_ptr<seq_map_t::sParam> param);

private:
	Ui::xSeqChildDlgClass ui;
};
