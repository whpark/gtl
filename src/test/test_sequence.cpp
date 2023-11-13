#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"
#include "gtl/sequence.h"
#include "gtl/sequence_any.h"
#include "gtl/sequence_map.h"

using namespace std::literals;
using namespace gtl::literals;
namespace chrono = std::chrono;

namespace gtl::seq::test {

	using seq_t = gtl::seq::TSequence<std::string>;
	using seq_map_t = gtl::seq::TSequenceMap<seq_t::result_t>;
	using coro_t = seq_t::coro_t;

	coro_t Sequence1(seq_t& seq) {
		namespace chrono = std::chrono;
		auto t0 = chrono::steady_clock::now();

		// do print something
		fmt::print("step1\n");

		// Wait For 1s
		co_await seq.WaitFor(40ms);

		// do print something
		auto t1 = chrono::steady_clock::now();
		fmt::print("step2 : {:>8}\n", chrono::duration_cast<chrono::milliseconds>(t1 - t0));

		co_await seq.WaitUntil(gtl::seq::clock_t::now() + 1ms);

		auto t2 = chrono::steady_clock::now();
		fmt::print("step3 : {:>8}\n", chrono::duration_cast<chrono::milliseconds>(t2 - t1));

		co_return fmt::format("{} ended. take {}", seq.GetName(), chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t0));
	}

	coro_t TopSeq(seq_t&);
	coro_t Child1(seq_t&);
	coro_t Child1_1(seq_t&);
	coro_t Child1_2(seq_t&);
	coro_t Child2(seq_t&);


	coro_t TopSeq(seq_t& seq) {
		auto sl = std::source_location::current();
		auto funcname = seq.GetName();// sl.function_name();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		fmt::print("{}: Creating Child1\n", funcname);
		auto t0 = clock_t::now();
		seq.CreateChildSequence("Child1", &Child1);

		co_await seq.WaitForChild();

		// step 2
		auto t1 = clock_t::now();
		fmt::print("{}: Child 1 Done, {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1-t0));

		auto t2 = clock_t::now();
		fmt::print("{}: WaitFor 100ms, {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t2 - t1));
		co_await seq.WaitFor(100ms);

		// step 3
		fmt::print("{}: End\n", funcname);

		co_return "";
	}

	coro_t Child1(seq_t& seq) {
		auto sl = std::source_location::current();
		auto funcname = seq.GetName();// sl.function_name();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		fmt::print("{}: Creating Child1_1, Child1_2\n", funcname);
		auto t0 = clock_t::now();
		seq.CreateChildSequence("Child1_1", &Child1_1);
		seq.CreateChildSequence("Child1_2", &Child1_2);

		co_await seq.WaitForChild();

		auto t1 = clock_t::now();
		fmt::print("{}: Child1_1, Child1_2 Done. {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1 - t0));

		// step 3
		fmt::print("{}: End\n", funcname);
	
		co_return "";
	}

	coro_t Child1_1(seq_t& seq) {
		auto sl = std::source_location::current();
		auto funcname = seq.GetName();// sl.function_name();

		auto t0 = clock_t::now();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		for (int i = 0; i < 5; i++) {
			auto t1 = clock_t::now();
			fmt::print("{}: doing some job... and wait for 200ms : {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1-t0));
			co_await seq.WaitFor(200ms);
		}
		fmt::print("{}: End. Creating Child1_1, Child1_2\n", funcname);
	
		co_return "";
	}

	coro_t Child1_2(seq_t& seq) {
		auto* self = seq_t::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = seq.GetName();// sl.function_name();

		auto t0 = clock_t::now();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		for (int i = 0; i < 5; i++) {
			auto t1 = clock_t::now();
			fmt::print("{}: doing some job... and wait for 200ms : {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1 - t0));
			co_await self->WaitFor(200ms);
		}
		fmt::print("{}: End. Creating Child1_1, Child1_2\n", funcname);
	
		co_return "";
	}

	coro_t Child2(seq_t& seq) {
		co_return "";
	}

	TEST(gtl_sequence, normal) {

		seq_t driver;

		fmt::print("Begin\n");

		// start simple sequence
		auto future = driver.CreateChildSequence("SimpleSequence", &Sequence1);
		do {
			auto t = driver.Dispatch();
			if (driver.IsDone())
				break;
			if (auto ts = t - gtl::seq::clock_t::now(); ts > 3s)
				t = gtl::seq::clock_t::now() + 3s;
			std::this_thread::sleep_until(t);
		} while (!driver.IsDone());
		fmt::print("Sequence1 result : {}\n", future.get());

		fmt::print("\n");

		// start tree sequence
		driver.CreateChildSequence("TreeSequence", &TopSeq);
		do {
			auto t = driver.Dispatch();
			if (driver.IsDone())
				break;
			if (auto ts = t - gtl::seq::clock_t::now(); ts > 3s)
				t = gtl::seq::clock_t::now() + 3s;
			std::this_thread::sleep_until(t);
		} while (!driver.IsDone());

		fmt::print("End\n");
	}

}	// namespace gtl::seq::test


namespace gtl::seq::test2 {

	using seq_t = gtl::seq::TSequence<std::string>;
	using seq_map_t = gtl::seq::TSequenceMap<seq_t::result_t>;
	using coro_t = seq_t::coro_t;

	//-----
	class CApp : public seq_map_t {
	public:
		CApp(seq_t& driver) : seq_map_t("top", driver) {
		}
		void Run() {
			auto* driver = GetSequenceDriver();
			do {
				auto t = driver->Dispatch();
				if (driver->IsDone())
					break;
				if (auto ts = t - gtl::seq::clock_t::now(); ts > 3s)
					t = gtl::seq::clock_t::now() + 3s;
				std::this_thread::sleep_until(t);
			} while (true);
		}
	};

	auto ms(auto dur) {
		return std::chrono::duration_cast<std::chrono::milliseconds>(dur);
	}

	//-----
	class C1 : public seq_map_t {
	public:
		using this_t = C1;
		using base_t = seq_map_t;

		C1(unit_id_t const& id, seq_map_t& parent) : seq_map_t(id, parent) {
			Bind("task1", &this_t::Task1);
			Bind("task2", &this_t::Task2);
		}

	protected:

		coro_t Task1(seq_t& seq, param_t param) {
			auto t0 = clock_t::now();
			auto funcname = "C1::Task1"; //std::source_location::current().function_name();

			fmt::print("{}: Begin\n", funcname);

			// call this->task2
			auto future = CreateChildSequence("task2", fmt::format("Greeting from {}", funcname));
			co_await WaitForChild();
			fmt::print("{}: child done: {}\n", funcname, future.get());

			fmt::print("{}: End {}\n", funcname, ms(clock_t::now() - t0));

			co_return "";
		}
		coro_t Task2(seq_t& seq, param_t param) {
			auto t0 = clock_t::now();
			auto funcname = "C1::Task2"; //std::source_location::current().function_name();

			fmt::print("{}: Begin param: {}\n", funcname, param);

			// call c2::taskA
			auto future = CreateChildSequence("c2", "taskA", fmt::format("Greeting from {}", funcname));
			co_await WaitForChild();
			fmt::print("{}: child done: {}\n", funcname, future.get());

			auto str = fmt::format("{}: End - {}", funcname, ms(clock_t::now() - t0));
			fmt::print("{}\n", str);

			co_return std::move(str);
		}

	};


	//-----
	class C2 : public seq_map_t {
	public:
		using this_t = C2;
		using base_t = seq_map_t;

		C2(unit_id_t const& id, seq_map_t& parent) : seq_map_t(id, parent) {
			Bind("taskA", &this_t::TaskA);
			Bind("taskB", &this_t::TaskB);
		}

	protected:
		coro_t TaskA(seq_t& seq, param_t param) {
			auto t0 = clock_t::now();
			auto funcname = "C2::TaskA"; //std::source_location::current().function_name();

			fmt::print("{}: Begin param: {}\n", funcname, param);

			// call c2::taskA
			auto future = CreateChildSequence("taskB", fmt::format("Greeting from {}", funcname));
			co_await WaitForChild();
			fmt::print("{}: child done: {}\n", funcname, future.get());

			auto str = fmt::format("{}: End - {}", funcname, ms(clock_t::now() - t0));
			fmt::print("{}\n", str);

			co_return std::move(str);
		}
		coro_t TaskB(seq_t& seq, param_t param) {
			auto t0 = clock_t::now();
			auto funcname = "C2::TaskB"; //std::source_location::current().function_name();

			fmt::print("{}: Begin param: {}\n", funcname, param);

			co_await WaitFor(100ms);

			auto str = fmt::format("{}: End - {}", funcname, ms(clock_t::now() - t0));
			fmt::print("{}\n", str);

			co_return std::move(str);
		}
	};


	TEST(gtl_sequence, normal) {


		fmt::print("start\n");

		seq_t driver;
		CApp app(driver);


		C1 c1("c1", app);
		C2 c2("c2", app);

		c1.CreateRootSequence("task1");	// c1::task1 -> c1::task2 -> c2::taskA -> c2::taskB

		app.Run();

		fmt::print("done\n");


	}


	gtl::seq::v01::TCoroutineHandle<std::string> SeqReturningString(gtl::seq::v01::xSequenceAny& seq) {
		auto t0 = chrono::steady_clock::now();
		auto str = fmt::format("{} ended. take {}", seq.GetName(), chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - t0));

		co_return std::move(str);
	}

	gtl::seq::v01::TCoroutineHandle<int> SeqReturningInt(gtl::seq::v01::xSequenceAny& seq) {

		co_return 3141592;
	}

	TEST(gtl_sequence, any_return_type) {
		gtl::seq::v01::xSequenceAny driver;

		auto f1 = driver.CreateChildSequence("SeqReturningString", &SeqReturningString);
		auto f2 = driver.CreateChildSequence("SeqReturningInt", &SeqReturningInt);

		do {
			auto t = driver.Dispatch();
			if (driver.IsDone())
				break;
			if (auto ts = t - gtl::seq::clock_t::now(); ts > 3s)
				t = gtl::seq::clock_t::now() + 3s;
			std::this_thread::sleep_until(t);
		} while (!driver.IsDone());

		fmt::print("Result of SeqReturningString : {}\n", f1.get());
		fmt::print("Result of SeqReturningInt : {}\n", f2.get());
	}

}	// namespace gtl::seq::test

