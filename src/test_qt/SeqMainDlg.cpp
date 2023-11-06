#include "pch.h"
#include "SeqMainDlg.h"

using namespace std::literals;

xSeqMainDlg::xSeqMainDlg(QWidget* parent) : QDialog(parent), m_driver(""), seq_map_t("MainSeq", m_driver) {
	ui.setupUi(this);

	connect(ui.btnCreateSequence1, &QPushButton::clicked, [this] { CreateChildSequence(m_driver, "Seq1"); });
	connect(ui.btnCreateSequence2, &QPushButton::clicked, [this] {
		CreateChildSequence(m_driver, "Seq2");
		m_driver.CreateChildSequence<std::shared_ptr<seq_map_t::sParam>>(
			"Sequence2-1"s,
			[this](auto p) { return this->Seq2(std::move(p)); },
			std::make_shared<seq_map_t::sParam>());
		using namespace std::placeholders;
		m_driver.CreateChildSequence<std::shared_ptr<seq_map_t::sParam>>(
			"Sequence2-2"s,
			std::bind(&this_t::Seq2, this, _1),
			std::make_shared<seq_map_t::sParam>());
		//m_driver.CreateChildSequence("Sequence2-3"s, [this]() { return this->Seq2(std::make_shared<base_seq_t::sParam>()); });
		m_driver.CreateChildSequence("Sequence2-3"s, std::bind(&this_t::Seq2, this, std::make_shared<seq_map_t::sParam>()));
	});
	connect(&m_timer, &QTimer::timeout, [this] {
		using namespace std::chrono;
		m_timer.stop();
		auto t0 = gtl::seq::clock_t::now();
		auto t = m_driver.Dispatch();
		auto dur = duration_cast<milliseconds>(t - t0);
		dur = std::clamp(dur, 0ms, 10ms);
		m_timer.start(dur);
	});

	Connect("Seq1", &xSeqMainDlg::Seq1, this);
	Connect("Seq2", &xSeqMainDlg::Seq2, this);

	m_timer.start(10ms);
}

xSeqMainDlg::~xSeqMainDlg() {
}

seq_t xSeqMainDlg::Seq1(std::shared_ptr<seq_map_t::sParam> param) {

	CreateChildSequence("ChildSeq", "SeqShowSomeText");
	ui.txt1->setText("ChildSeq::SeqShowSomeText - Started");
	co_yield{ 10ms };
	ui.txt1->setText("ChildSeq::SeqShowSomeText - END");

	co_return;
}

seq_t xSeqMainDlg::Seq2(std::shared_ptr<seq_map_t::sParam> param) {
	auto t0 = std::chrono::steady_clock::now();

	ui.txt2->setText(gtl::qt::ToQString(fmt::format("seq : {} STARTED", m_driver.GetCurrentSequence()->GetName())));

	auto t1 = t0 + 1s;
	auto counter = 0;
	for (auto t = std::chrono::steady_clock::now(); t < t1; t = std::chrono::steady_clock::now()) {
		ui.txt3->setText(gtl::qt::ToQString(fmt::format("seq : {} running {}", m_driver.GetCurrentSequence()->GetName(), counter++)));
		co_yield { 1ms };
	}

	ui.txt2->setText(gtl::qt::ToQString(fmt::format("seq : {} STOPPED", m_driver.GetCurrentSequence()->GetName())));

	co_return;
}
