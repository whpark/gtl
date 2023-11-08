#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"
#include "gtl/sequence.h"
#include "gtl/sequence_map.h"

using namespace std::literals;
using namespace gtl::literals;
namespace chrono = std::chrono;

namespace gtl::seq::test {

	using seq_t = gtl::seq::xSequence;
	using seq_map_t = gtl::seq::TSequenceMap<std::string, std::string>;

	seq_t Sequence1() {
		auto* self = seq_t::GetCurrentSequence();

		namespace chrono = std::chrono;
		auto t0 = chrono::steady_clock::now();

		// do print something
		fmt::print("step1\n");

		// Wait For 1s
		co_await self->WaitFor(40ms);

		// do print something
		auto t1 = chrono::steady_clock::now();
		fmt::print("step2 : {:>8}\n", chrono::duration_cast<chrono::milliseconds>(t1 - t0));

		co_await self->WaitUntil(gtl::seq::clock_t::now() + 1ms);

		auto t2 = chrono::steady_clock::now();
		fmt::print("step3 : {:>8}\n", chrono::duration_cast<chrono::milliseconds>(t2 - t1));

		co_return;
	}

	seq_t TopSeq();
	seq_t Child1();
	seq_t Child1_1();
	seq_t Child1_2();
	seq_t Child2();


	seq_t TopSeq() {
		auto* self = seq_t::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		fmt::print("{}: Creating Child1\n", funcname);
		auto t0 = clock_t::now();
		self->CreateChildSequence("Child1", &Child1);

		co_await self->WaitForChild();

		// step 2
		auto t1 = clock_t::now();
		fmt::print("{}: Child 1 Done, {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1-t0));

		auto t2 = clock_t::now();
		fmt::print("{}: WaitFor 100ms, {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t2 - t1));
		co_await self->WaitFor(100ms);

		// step 3
		fmt::print("{}: End\n", funcname);
	}

	seq_t Child1() {
		auto* self = seq_t::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		fmt::print("{}: Creating Child1_1, Child1_2\n", funcname);
		auto t0 = clock_t::now();
		self->CreateChildSequence("Child1_1", &Child1_1);
		self->CreateChildSequence("Child1_2", &Child1_2);

		co_await self->WaitForChild();

		auto t1 = clock_t::now();
		fmt::print("{}: Child1_1, Child1_2 Done. {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1 - t0));

		// step 3
		fmt::print("{}: End\n", funcname);
	}

	seq_t Child1_1() {
		auto* self = seq_t::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		auto t0 = clock_t::now();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		for (int i = 0; i < 5; i++) {
			auto t1 = clock_t::now();
			fmt::print("{}: doing some job... and wait for 200ms : {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1-t0));
			co_await self->WaitFor(200ms);
		}
		fmt::print("{}: End. Creating Child1_1, Child1_2\n", funcname);
	}

	seq_t Child1_2() {
		auto* self = seq_t::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		auto t0 = clock_t::now();

		// step 1
		fmt::print("{}: Begin\n", funcname);
		for (int i = 0; i < 5; i++) {
			auto t1 = clock_t::now();
			fmt::print("{}: doing some job... and wait for 200ms : {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1 - t0));
			co_await self->WaitFor(200ms);
		}
		fmt::print("{}: End. Creating Child1_1, Child1_2\n", funcname);
	}

	seq_t Child2() {
		co_return;
	}

	TEST(gtl_sequence, normal) {

		seq_t driver;

		fmt::print("Begin\n");

		// start simple sequence
		driver.CreateChildSequence("-------------\nSimpleSequence", &Sequence1);
		do {
			auto t = driver.Dispatch();
			if (driver.IsDone())
				break;
			if (auto ts = t - gtl::seq::clock_t::now(); ts > 3s)
				t = gtl::seq::clock_t::now() + 3s;
			std::this_thread::sleep_until(t);
		} while (!driver.IsDone());

		fmt::print("\n");

		// start tree sequence
		driver.CreateChildSequence("-------------\nTreeSequence", &TopSeq);
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

	using seq_t = gtl::seq::xSequence;
	using seq_map_t = gtl::seq::TSequenceMap<std::string, std::string>;

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


	//-----
	class C1 : public seq_map_t {
	public:
		using this_t = C1;
		using base_t = seq_map_t;

		C1(unit_id_t const& id, seq_map_t& parent) : seq_map_t(id, parent) {
			Bind("task1", this, &this_t::Task1);
			Bind("task2", this, &this_t::Task2);
		}

	protected:
		seq_t Task1(std::shared_ptr<seq_map_t::sParam> param) {
			//auto sl = std::source_location::current();
			auto funcname = "Task1";// sl.function_name();

			fmt::print("{}: Begin\n", funcname);

			auto param2 = std::make_shared<seq_map_t::sParam>();
			param2->in = "Greeting from c1::Task1 ==> c1::Task2";
			CreateChildSequence("task2", param2);
			co_await WaitForChild();

			fmt::print("{}: End. {}\n", funcname, param2->out);

			co_return;
		}
		seq_t Task2(std::shared_ptr<seq_map_t::sParam> param) {
			//auto sl = std::source_location::current();
			auto funcname = "Task2";// sl.function_name();

			fmt::print("{}: Begin - {}\n", funcname, param->in);

			auto param2 = std::make_shared<seq_map_t::sParam>();
			param2->in = "Greeting from c1::Task2 ==> c2::TaskA";
			CreateChildSequence("c2", "taskA", param2);
			co_await WaitForChild();

			fmt::print("{}: End\n", funcname);

			param->out = "OK";

			co_return;
		}

	};


	//-----
	class C2 : public seq_map_t {
	public:
		using this_t = C2;
		using base_t = seq_map_t;

		C2(unit_id_t const& id, seq_map_t& parent) : seq_map_t(id, parent) {
			Bind("taskA", this, &this_t::TaskA);
			Bind("taskB", this, &this_t::TaskB);
		}

	protected:
		seq_t TaskA(std::shared_ptr<seq_map_t::sParam> param) {
			//auto sl = std::source_location::current();
			auto funcname = "TaskA";// sl.function_name();

			fmt::print("{}: Begin - {}\n", funcname, param->in);

			auto param2 = std::make_shared<seq_map_t::sParam>();
			param2->in = "Greeting from c2::TaskA ==> c2::TaskB";
			CreateChildSequence("c2", "taskB", param2);
			co_await WaitForChild();

			fmt::print("{}: End\n", funcname);

			//param->out = "OK";

			co_return;
		}
		seq_t TaskB(std::shared_ptr<seq_map_t::sParam> param) {
			//auto sl = std::source_location::current();
			auto funcname = "TaskB";// sl.function_name();

			fmt::print("{}: Begin - {}\n", funcname, param->in);

			auto param2 = std::make_shared<seq_map_t::sParam>();
			param2->in = "Greeting from c1::Task2 ==> c2::TaskA";
			co_await WaitFor(100ms);

			fmt::print("{}: End\n", funcname);

			//param->out = "OK";

			co_return;
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

}	// namespace gtl::seq::test

