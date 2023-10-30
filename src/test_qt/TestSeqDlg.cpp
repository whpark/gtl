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
		m_main.CreateSequence(std::bind(&this_t::Seq1, this, _1));

		//auto& self = m_top.m_sequences.back();
		//std::function<Generator<int>()> fSeq = [this, &self]()->auto { return Seq1(self); };
		//self = fSeq();
		//m_top.AddSequence(fSeq);
		//m_sequences.emplace_back(fSeq());

	});
	connect(this, &xTestSeqDlg::sigSequence, this, [this]() {
		//Log("sigSequence\n");
		m_main.ProcessSingleStep();
	}, Qt::QueuedConnection);

	m_thread = std::jthread([this](std::stop_token st) {
		//ui.txtWorkerThreadID->setText(ToQString(fmt::format("{}", std::this_thread::get_id())));
		Log("{}\r\n", std::this_thread::get_id());
		while (!st.stop_requested()) {
			emit sigSequence();
			std::this_thread::yield();
			std::this_thread::sleep_for(100us);
		}
	});
}

xTestSeqDlg::~xTestSeqDlg() {
	m_thread.request_stop();
	if (m_thread.joinable())
		m_thread.join();
}

gtl::xSequence Suspend3(gtl::xSequence& self, std::string const&) {
	auto sl = std::source_location::current();

	//static std::atomic<int>	counter{};
	static int counter{};
	auto i = counter++;
	Log("{} <<<<<<<< {} \n", sl.function_name(), i);
	for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
		Log("{} await < {}\n", sl.function_name(), i);
		co_await std::suspend_always();
		Log("{} await > {}\n", sl.function_name(), i);
	}
	Log("{} >>>>>>>> {} \n", sl.function_name(), i);

	co_yield true;
}

gtl::xSequence Suspend2(gtl::xSequence& self, int) {
	auto sl = std::source_location::current();

	//static std::atomic<int>	counter{};
	static int counter{};
	auto i = counter++;
	Log("{} <<<<<<<< {} \n", sl.function_name(), i);
	int loop{};
	for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
		Log("{} await < {}, loop {}\n", sl.function_name(), i, loop++);
		//co_await std::suspend_always();
		co_yield true;
		Log("{} await > {}\n", sl.function_name(), i);
	}
	Log("{} >>>>>>>> {} \n", sl.function_name(), i);

	co_yield true;
}

gtl::xSequence Suspend(gtl::xSequence& self) {
	auto sl = std::source_location::current();

	//static std::atomic<int>	counter{};
	static int counter{};
	auto i = counter++;
	Log("{} <<<<<<<< {} \n", sl.function_name(), i);
	int loop{};
	for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
		Log("{} await < {}, loop {}\n", sl.function_name(), i, loop++);
		co_await std::suspend_always();
		Log("{} await > {}\n", sl.function_name(), i);
	}
	Log("{} >>>>>>>> {} \n", sl.function_name(), i);

	co_yield true;
}

gtl::xSequence xTestSeqDlg::Seq1(gtl::xSequence& self) {
	Log("Seq1 Begin\n");
	//co_await step1();
	ui.txtMessage1->setText("1");
	co_await self.CreateSequence(Suspend);

	//for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
	//	co_await std::suspend_always();
	//}
	ui.txtMessage2->setText("2");
	co_await self.CreateSequenceParams<int>(Suspend2, 3);//, self.CreateSequence(Suspend), self.CreateSequence(Suspend), self.CreateSequence(Suspend);
	//co_await self.CreateSequenceParams<std::string>(Suspend3, std::string{});//, self.CreateSequence(Suspend), self.CreateSequence(Suspend), self.CreateSequence(Suspend);

	ui.txtMessage2->setText("2-2");
	self.CreateSequence(Suspend), self.CreateSequence(Suspend), self.CreateSequence(Suspend), self.CreateSequence(Suspend);
	co_await std::suspend_always{};

	ui.txtMessage3->setText("3");

	Log("Seq1 END\n");
	co_return ;
}
