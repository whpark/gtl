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
		auto* self = xSequence::GetCurrentSequence();

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
		auto* self = xSequence::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		// step 1
		fmt::print("{}: Start\n", funcname);
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
		auto* self = xSequence::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		// step 1
		fmt::print("{}: Start\n", funcname);
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
		auto* self = xSequence::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		auto t0 = clock_t::now();

		// step 1
		fmt::print("{}: Start\n", funcname);
		for (int i = 0; i < 5; i++) {
			auto t1 = clock_t::now();
			fmt::print("{}: doing some job... and wait for 200ms : {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1-t0));
			co_await self->WaitFor(200ms);
		}
		fmt::print("{}: Creating Child1_1, Child1_2\n", funcname);
	}

	seq_t Child1_2() {
		auto* self = xSequence::GetCurrentSequence();

		auto sl = std::source_location::current();
		auto funcname = sl.function_name();

		auto t0 = clock_t::now();

		// step 1
		fmt::print("{}: Start\n", funcname);
		for (int i = 0; i < 5; i++) {
			auto t1 = clock_t::now();
			fmt::print("{}: doing some job... and wait for 200ms : {}\n", funcname, chrono::duration_cast<chrono::milliseconds>(t1 - t0));
			co_await self->WaitFor(200ms);
		}
		fmt::print("{}: Creating Child1_1, Child1_2\n", funcname);
	}

	seq_t Child2() {
		co_return;
	}

	TEST(gtl_sequence, normal) {

		gtl::seq::xSequence driver;

		fmt::print("start\n");

		//// start simple sequence
		//driver.CreateChildSequence("SimpleSequence", &Sequence1);
		//do {
		//	auto t = driver.Dispatch();
		//	if (driver.IsDone())
		//		break;
		//	if (auto ts = t - gtl::seq::clock_t::now(); ts > 3s)
		//		t = gtl::seq::clock_t::now() + 3s;
		//	std::this_thread::sleep_until(t);
		//} while (!driver.IsDone());

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

		fmt::print("done\n");
	}

}	// namespace gtl::seq::test
