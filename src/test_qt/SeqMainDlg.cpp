#include "pch.h"
#include "SeqMainDlg.h"

using namespace std::literals;

using seq_t = xSeqMainDlg::seq_t;;

xSeqMainDlg::xSeqMainDlg(QWidget* parent) : QDialog(parent), m_driver(""), seq_map_t("MainSeq", m_driver) {
	ui.setupUi(this);

	connect(ui.btnCreateSequence1, &QPushButton::clicked, [this] { try { CreateRootSequence("Seq1"); } catch (std::exception&) {} });
	connect(ui.btnCreateSequence2, &QPushButton::clicked, [this] {
		CreateRootSequence("Seq2");
		m_driver.CreateChildSequence<seq_param_t>(
			"Sequence2-1"s,
			0,
			[this](auto& seq, auto p) { return this->Seq2(seq, std::move(p)); },
			{});
		using namespace std::placeholders;
		m_driver.CreateChildSequence<seq_param_t>(
			"Sequence2-2"s,
			0,
			std::bind(&this_t::Seq2, this, _1, _2),
			{});
		//m_driver.CreateChildSequence("Sequence2-3"s, [this]() { return this->Seq2(std::make_shared<base_seq_t::sParam>()); });
		m_driver.CreateChildSequence<seq_param_t>("Sequence2-3"s, 0, std::bind(&this_t::Seq2, this, _1, _2), seq_param_t{});

		//CreateSequence(&m_driver, "Sequence2-4", this, &this_t::Seq2, {});

	});

	// Dispatch
	connect(&m_timer, &QTimer::timeout, [this] {
		using namespace std::chrono;
		m_timer.stop();
		auto t0 = gtl::seq::clock_t::now();
		auto t = m_driver.Dispatch();
		auto dur = duration_cast<milliseconds>(t - t0);
		dur = std::clamp(dur, 0ms, 100ms);
		m_timer.start(dur);
	});

	Bind("Seq1", &xSeqMainDlg::Seq1, 1);
	Bind("Seq2", &xSeqMainDlg::Seq2, 1);

	m_timer.start(100ms);
}

xSeqMainDlg::~xSeqMainDlg() {
}

xSeqMainDlg::coro_t xSeqMainDlg::Seq1(seq_t& seq, seq_param_t param) {
	auto future = CreateChildSequence("ChildSeq", "SeqShowSomeText", {});
	ui.txt1->setText("ChildSeq::SeqShowSomeText - Started");
	co_await seq.WaitForChild();

	if (!future.valid()) {
		QMessageBox::critical(this, "Error", "future is invalid");
		co_return {};
	}
	ui.txt1->setText(gtl::qt::ToQString(fmt::format("ChildSeq::SeqShowSomeText - result : {}", future.get())));

	co_return {};
}

xSeqMainDlg::coro_t xSeqMainDlg::Seq2(seq_t& seq, seq_param_t param) {
	auto t0 = std::chrono::steady_clock::now();

	ui.txt2->setText(gtl::qt::ToQString(fmt::format("seq : {} STARTED", m_driver.GetCurrentSequence()->GetName())));

	auto t1 = t0 + 1s;
	auto counter = 0;
	for (auto t = std::chrono::steady_clock::now(); t < t1; t = std::chrono::steady_clock::now()) {
		ui.txt3->setText(gtl::qt::ToQString(fmt::format("seq : {} running {}", m_driver.GetCurrentSequence()->GetName(), counter++)));
		co_await seq.WaitFor(1ms);
	}

	ui.txt2->setText(gtl::qt::ToQString(fmt::format("seq : {} STOPPED", m_driver.GetCurrentSequence()->GetName())));

	co_return {};
}
