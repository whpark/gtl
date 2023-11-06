#include "pch.h"
#include "SeqChildDlg.h"

using namespace std::literals;

xSeqChildDlg::xSeqChildDlg(seq_map_t& parentSeqMap, QWidget* parent) : QDialog(parent), seq_map_t("ChildSeq", parentSeqMap) {
	ui.setupUi(this);


	Connect("SeqShowSomeText", &xSeqChildDlg::SeqShowSomeText, this);
	//Connect("SeqShowSomeText", [this](auto&& param) { return SeqShowSomeText(std::move(param)); });

	connect(ui.btnContinue, &QPushButton::clicked, [this] { if (auto* seq = m_driver->FindChildDFS("SeqShowSomeText")) seq->ReserveResume(); });
}

xSeqChildDlg::~xSeqChildDlg() {
}

seq_t xSeqChildDlg::SeqShowSomeText(std::shared_ptr<seq_map_t::sParam> param) {
	show();
	setFocus();
	ui.txt1->setText("");
	ui.txt2->setText("");
	ui.txt3->setText("");

	auto* curSeq = m_driver->GetCurrentSequence();

	ui.txt1->setText(fmt::format("{} ...", curSeq->GetName()).c_str());
	for (int i{}; i < 100; i++) {
		ui.txt2->setText(fmt::format("{} working {}", curSeq->GetName(), i).c_str());
		co_yield{ 1ms };
	}

	ui.txt1->setText("Suspended");

	co_await std::suspend_always{};

	ui.txt1->setText("Resumed");

	for (int i{}; i < 1000; i++) {
		ui.txt3->setText(fmt::format("{} working {}", curSeq->GetName(), i).c_str());
		co_yield{ 10us };
	}

	ui.txt1->setText("END");
	co_return;
}
