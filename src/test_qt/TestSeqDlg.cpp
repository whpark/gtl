#include "pch.h"
#include "TestSeqDlg.h"
#include "gtl/reflection_glaze.h"

using gtl::qt::ToQString;
using gtl::qt::ToString;
using namespace std::literals;

seq_t g_driver(nullptr);

template < typename ... TArgs>
void Log(fmt::format_string<TArgs...> format_string, TArgs&& ... args) {
	OutputDebugStringA(fmt::format(format_string, std::forward<TArgs>(args) ...).c_str());
}

xTestSeqDlg::xTestSeqDlg(QWidget* parent) : QDialog(parent), base_seq_t(g_driver, "main") {
	ui.setupUi(this);

	ui.txtMainThreadID->setText(ToQString(fmt::format("{}", std::this_thread::get_id())));

	connect(&m_timer, &QTimer::timeout, [this]() { Dispatch(); });
	m_timer.start(5ms);

	connect(ui.btnSeq1, &QPushButton::clicked, [this]() {
		//m_top.AddSequence([this](Generator<int>& self) -> Generator<int> { return Seq1(self); });
		using namespace std::placeholders;
		//m_driver.CreateChildSequence("ChildSequence"s, [this]{ return this->Seq1(); });
		//m_driver.CreateChildSequence("ChildSequence"s, std::bind(&this_t::Seq1, this, std::make_shared<gtl::seq::sParam>()));
		CreateChildSequence(m_driver, "MainSequence"s, &this_t::Seq1);
	});

	connect(ui.btnSeqContinue, &QPushButton::clicked, [this]() {
		if (auto* seq = m_driver.FindChildDFS("SuspendString")) {
			seq->ReserveResume();
		}
	});

	std::jthread j([this]{
		CreateChildSequence(m_driver, "MainSequence"s, &this_t::Seq1);		// seq injection from other thread
	});
	j.join();

	//connect(this, &xTestSeqDlg::sigSequence, this, [this]() { Dispatch();}, Qt::QueuedConnection);
	//m_thread = std::jthread([this](std::stop_token st) {
	//	Log("{}\r\n", std::this_thread::get_id());
	//	while (!st.stop_requested()) {
	//		emit sigSequence();
	//		std::this_thread::yield();
	//		std::this_thread::sleep_for(5ms);
	//	}
	//});
}

xTestSeqDlg::~xTestSeqDlg() {
	if (m_thread.joinable()) {
		m_thread.request_stop();
		m_thread.join();
	}
}

void xTestSeqDlg::Dispatch() {
	//m_timer.stop();
	static auto t0 = gtl::seq::clock_t::now();
	auto t1 = gtl::seq::clock_t::now();
	// log in ms
	if (auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0); dur > 10ms) {
		Log("-- Dispatch --, {}\n", dur);
	}
	t0 = t1;

	//auto tNext = base_seq_t::Dispatch();
	auto tNext = m_driver.Dispatch();
	//if (tNext < gtl::seq::clock_t::time_point::max()) {
	//	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(tNext - t1);
	//	m_timer.start(std::max(1ms, dur));
	//}
}

seq_t Suspend3(std::string const&) {
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

	co_yield {10ms};
}

seq_t xTestSeqDlg::Suspend(seq_param_t param) {
	auto sl = std::source_location::current();

	//static std::atomic<int>	counter{};
	static int counter{};
	auto i = counter++;
	Log("{} <<<<<<<< {} \n", sl.function_name(), i);
	int loop{};
	for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
		Log("{} await < {}, loop {}\n", sl.function_name(), i, loop++);
		//co_await std::suspend_always();
		co_yield {100ms};
		Log("{} await > {}\n", sl.function_name(), i);
	}
	Log("{} >>>>>>>> {} \n", sl.function_name(), i);

	co_yield {0ms};
}

seq_t xTestSeqDlg::SuspendAny(int) {
	auto sl = std::source_location::current();

	//static std::atomic<int>	counter{};
	static int counter{};
	auto i = counter++;
	Log("{} <<<<<<<< {} \n", sl.function_name(), i);
	int loop{};
	for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
		Log("{} await < {}, loop {}\n", sl.function_name(), i, loop++);
		//co_await std::suspend_always();
		co_yield {100ms};
		Log("{} await > {}\n", sl.function_name(), i);
	}
	Log("{} >>>>>>>> {} \n", sl.function_name(), i);

	co_yield {0ms};
}

seq_t xTestSeqDlg::SuspendHandler(seq_param_t param) {
	glz::json_t in;
	glz::read_json(in, param->in);
	glz::json_t out;
	gtl::xFinalAction result([&] { glz::write_json(out, param->out); });

	auto sl = std::source_location::current();
	auto t0 = std::chrono::steady_clock::now();
	//static std::atomic<int>	counter{};
	static int counter{};
	auto i = counter++;
	Log("{} <<<<<<<< {} \n", sl.function_name(), i);
	int loop{};
	auto duration = std::chrono::milliseconds{in["duration"].as<int>()};
	if (duration <= 1000ms)
		duration = 1000ms;
	for (auto t = t0; t - t0 < duration; t = std::chrono::steady_clock::now()) {
		Log("{} await < {}, loop {}\n", sl.function_name(), i, loop++);
		//co_await std::suspend_always();
		co_yield {100ms};
		Log("{} await > {}\n", sl.function_name(), i);
	}
	out["result"] = std::format("time : {}",
		std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0));
	Log("{} >>>>>>>> {} \n", sl.function_name(), i);
}

seq_t xTestSeqDlg::Seq1(seq_param_t param) {
	static std::atomic<int> counter_{};
	int counter = (counter_ += 3)-3;
	Log("Seq1 Begin\n");
	//co_await step1();
	ui.txtMessage1->setText(ToQString(fmt::format("{}", ++counter)));
	//CreateSequence("child2", self, &this_t::Suspend);
	glz::json_t in;
	in["duration"] = 1500;
	auto param2 = std::make_shared<base_seq_t::sParam>();
	glz::write_json(in, param2->in);
	CreateChildSequence("child2", &this_t::SuspendHandler, param2);
	co_yield {10ms};
	glz::json_t out;
	glz::read_json(out, param2->out);
	ui.txtMessage1->setText(ToQString(fmt::format("{}", gtl::ValueOr(out["result"], "no result"s))));
	//co_yield 10ms;

	//for (auto t0 = std::chrono::steady_clock::now(); std::chrono::steady_clock::now() - t0 < 1000ms; ) {
	//	co_await std::suspend_always();
	//}
	ui.txtMessage2->setText(ToQString(fmt::format("{}", ++counter)));
	
	m_driver.GetCurrentSequence()->CreateChildSequence<std::string>("SuspendString", Suspend3, "sdfasdf"s);//, self.CreateSequence(Suspend), self.CreateSequence(Suspend), self.CreateSequence(Suspend);
	co_yield {10ms};
	//co_await self.CreateSequenceParams<std::string>(Suspend3, std::string{});//, self.CreateSequence(Suspend), self.CreateSequence(Suspend), self.CreateSequence(Suspend);

	ui.txtMessage2->setText(ToQString(fmt::format("{}-2", counter)));
	CreateChildSequence("child3", &this_t::Suspend);
	//CreateSequence("child4", self, &this_t::Suspend);
	//CreateSequence("child5", self, &this_t::Suspend);
	//CreateSequence("child6", self, &this_t::Suspend);
	co_yield {10ms};//std::suspend_always{};

	ui.txtMessage3->setText(ToQString(fmt::format("{}", ++counter)));

	Log("Seq1 END\n");
	co_return ;
}
