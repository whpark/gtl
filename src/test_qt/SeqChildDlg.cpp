#include "pch.h"
#include "SeqChildDlg.h"

using namespace std::literals;

xSeqChildDlg::xSeqChildDlg(seq_map_t& parentSeqMap, QWidget* parent) : QDialog(parent), seq_map_t("ChildSeq", parentSeqMap) {
	ui.setupUi(this);


	Bind("SeqShowSomeText", this, &xSeqChildDlg::SeqShowSomeText);
	//Connect("SeqShowSomeText", [this](auto&& param) { return SeqShowSomeText(std::move(param)); });

	connect(ui.btnContinue, &QPushButton::clicked, [this] { if (auto* seq = GetSequenceDriver()->FindChildDFS("SeqShowSomeText")) seq->ReserveResume(); });
}

xSeqChildDlg::~xSeqChildDlg() {
}

xSeqChildDlg::seq_t xSeqChildDlg::SeqShowSomeText(seq_param_t param) {
	auto t0 = gtl::seq::clock_t::now();
	show();
	setFocus();
	ui.txt1->setText("");
	ui.txt2->setText("");
	ui.txt3->setText("");

	auto* seq = GetSequenceDriver()->GetCurrentSequence();

	ui.txt1->setText(fmt::format("{} ...", seq->GetName()).c_str());
	for (int i{}; i < 100; i++) {
		ui.txt2->setText(fmt::format("{} working {}", seq->GetName(), i).c_str());
		co_await seq->WaitFor(1ms);
	}

	ui.txt1->setText("Suspended");

	co_await std::suspend_always{};

	ui.txt1->setText("Resumed");

	for (int i{}; i < 1000; i++) {
		ui.txt3->setText(fmt::format("{} working {}", seq->GetName(), i).c_str());
		co_await seq->WaitFor(10us);
	}

	ui.txt1->setText("END");

	co_return seq_param_t{fmt::format("{} take {}", seq->GetName(), std::chrono::duration_cast<std::chrono::milliseconds>(gtl::seq::clock_t::now() - t0))};
}
