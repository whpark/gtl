#include "pch.h"
#include "TestSeqDlg.h"

using gtl::qt::ToQString;
using gtl::qt::ToString;
using namespace std::literals;

template < typename ... TArgs>
void Log(fmt::format_string<TArgs...> format_string, TArgs&& ... args) {
	OutputDebugStringA(fmt::format(format_string, std::forward<TArgs>(args) ...).c_str());
}

xTestSeqDlg::xTestSeqDlg(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);

	ui.txtMainThreadID->setText(ToQString(fmt::format("{}", std::this_thread::get_id())));

	connect(ui.btnSeq1, &QPushButton::clicked, [this]() {
		ui.txtMessage1->clear();
		ui.txtMessage2->clear();
		ui.txtMessage3->clear();
		//m_top.AddSequence([this](Generator<int>& self) -> Generator<int> { return Seq1(self); });
		using namespace std::placeholders;
		m_top.AddSequence(std::bind(&this_t::Seq1, this, _1));

		//auto& self = m_top.m_sequences.back();
		//std::function<Generator<int>()> fSeq = [this, &self]()->auto { return Seq1(self); };
		//self = fSeq();
		//m_top.AddSequence(fSeq);
		//m_sequences.emplace_back(fSeq());

	});
	connect(this, &xTestSeqDlg::sigSequence, this, [this]() {
		m_top.DoSequenceLoop();
	}, Qt::QueuedConnection);

	m_thread = std::jthread([this](std::stop_token st) {
		//ui.txtWorkerThreadID->setText(ToQString(fmt::format("{}", std::this_thread::get_id())));
		Log("{}\r\n", std::this_thread::get_id());
		while (!st.stop_requested()) {
			emit sigSequence();
			//std::this_thread::yield();
			std::this_thread::sleep_for(10ms);
		}
	});
}

xTestSeqDlg::~xTestSeqDlg() {
	m_thread.request_stop();
	if (m_thread.joinable())
		m_thread.join();
}

Generator<int> Suspend(Generator<int>& self) {
	//static std::atomic<int>	counter{};
	static int counter{};
	auto i = counter++;
	Log("Suspend <<<<<<<<< {} \n", i);
	for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
		co_await std::suspend_always();
	}
	Log("Suspend >>>>>>>>> {} \n", i);
}
Generator<int> xTestSeqDlg::Seq1(Generator<int>& self) {
	Log("Seq1 Begin\n");
	//co_await step1();
	ui.txtMessage1->setText("1");
	//co_await self.AddSequence(Suspend);

	//for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
	//	co_await std::suspend_always();
	//}
	ui.txtMessage2->setText("2");
	co_await self.AddSequence(Suspend), self.AddSequence(Suspend), self.AddSequence(Suspend), self.AddSequence(Suspend);

	ui.txtMessage2->setText("2-2");
	co_await self.AddSequence(Suspend), self.AddSequence(Suspend), self.AddSequence(Suspend), self.AddSequence(Suspend);

	ui.txtMessage3->setText("3");

	Log("Seq1 END\n");
	co_return ;
}

Generator<double> xTestSeqDlg::SubSeq() {
	co_yield 1.1;
	co_return ;
}
