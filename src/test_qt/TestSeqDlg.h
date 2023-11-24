#pragma once

#include <QDialog>
#include "ui_TestSeqDlg.h"
#include "gtl/sequence.h"
#include "gtl/sequence_map.h"
#include "glaze/glaze.hpp"


class xTestSeqDlg : public QDialog, public gtl::seq::TSequenceMap<glz::json_t> {
	Q_OBJECT
public:
	using this_t = xTestSeqDlg;
	using base_t = QDialog;
	using seq_map_t = gtl::seq::TSequenceMap<glz::json_t>;
	using seq_t = seq_map_t::seq_t;
	using seq_result_t = seq_t::result_t;
	using seq_param_t = seq_map_t::param_t;

public:
	xTestSeqDlg(QWidget* parent = nullptr);
	~xTestSeqDlg();

	QTimer m_timer;
	std::jthread m_thread;
	void Dispatch();

	coro_t Seq1(seq_t&, seq_param_t param = {});
	coro_t Suspend(seq_t&, seq_param_t param = {});
	coro_t SuspendHandler(seq_t&, seq_param_t param = {});
	coro_t SuspendAny(seq_t&, int);

Q_SIGNALS:
	void sigSequence();

private:
	Ui::xTestSeqDlgClass ui;
};
