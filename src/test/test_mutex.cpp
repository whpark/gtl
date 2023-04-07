#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/mutex.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::mutex {

	gtl::recursive_shared_mutex mtx;

	void reader_thread(int id) {
		std::shared_lock lock(mtx);
		std::cout << "Reader " << id << " attempting to acquire shared lock...\n";
		std::shared_lock lock2(mtx);
		std::cout << "Reader " << id << " acquired shared lock.\n";
		// simulate some work
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	void writer_thread(int id) {
		std::unique_lock lock(mtx);
		std::cout << "Writer " << id << " attempting to acquire exclusive lock...\n";
		std::unique_lock lock2(mtx);
		std::cout << "Writer " << id << " acquired exclusive lock.\n";
		// simulate some work
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	TEST(mutex, recursive_shared_mutex) {
		// create 5 reader threads and 2 writer threads
		std::vector<std::thread> threads;
		for (int i = 1; i <= 5; ++i) {
			threads.emplace_back(reader_thread, i);
		}
		for (int i = 1; i <= 2; ++i) {
			threads.emplace_back(writer_thread, i);
		}

		// wait for all threads to finish
		for (auto& t : threads) {
			t.join();
		}
	}

	class ThreadSafeCounter {
	public:
		ThreadSafeCounter() = default;

		// Multiple threads/readers can read the counter's value at the same time.
		unsigned int get() const {
			std::shared_lock lock(mutex_);
			return value_;
		}

		// Only one thread/writer can increment/write the counter's value.
		void increment() {
			std::unique_lock lock(mutex_);
			++value_;
		}

		// Only one thread/writer can reset/write the counter's value.
		void reset() {
			std::unique_lock lock(mutex_);
			value_ = 0;
		}

	private:
		mutable std::shared_mutex mutex_;
		unsigned int value_ = 0;
	};

	TEST(mutex, recursive_shared_mutex2) {
		ThreadSafeCounter counter;

		auto increment_and_print = [&counter]() {
			for (int i = 0; i < 3; i++) {
				std::this_thread::sleep_for(10ms);
				counter.increment();
				std::cout << std::this_thread::get_id() << ' ' << counter.get() << '\n';

				// Note: Writing to std::cout actually needs to be synchronized as well
				// by another std::mutex. This has been omitted to keep the example small.
			}
		};

		std::jthread thread1(increment_and_print);
		std::jthread thread2(increment_and_print);
		std::jthread thread3(increment_and_print);
		std::jthread thread4(increment_and_print);

		thread1.join();
		thread2.join();
		thread3.join();
		thread4.join();
	}

	class xTest {
	public:
		std::unique_ptr<int> i;

		xTest() : i{std::make_unique<int>(0)} {};

		int Get() const {
			return *i;
		}

		int Mutate() {
			(*i) ++;
			return *i;
		}

		int Mutate2() {
			(*i) += 2;
			return *i;
		}
	};

	TEST(mutex, MutexLocker) {

		gtl::TMutexLocker<xTest> a;

		std::jthread t1 {
			[&a]{ for (int i{}; i < 100; i++) { auto v = a.Lock(); v->Mutate(); } }
		};

		std::jthread t2 {
			[&a]{ for (int i{}; i < 100; i++) { auto v = a.Lock(); v->Mutate2(); } }
		};

		t1.join();
		t2.join();

		EXPECT_EQ(a.Lock()->Get(), 100+(100*2));

		auto a0 = a.Release();

		EXPECT_EQ(a0.Get(), 100+(100*2));

		EXPECT_TRUE(a.Lock()->i == nullptr);

	}

}	// namespace gtl::test::mutex
