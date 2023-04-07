#include "pch.h"

#include "gtl/gtl.h"

#include "gtl/mutex.h"
#include "gtl/log.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::lock {

	TEST(thread, mutex) {

		//gtl::recursive_shared_mutex mutex;
		gtl::xSimpleLog log;

		//std::vector<std::unique_ptr<std::pair<std::thread::id, int> > > vecP;
		gtl::TConcurrentUPtrVector<std::pair<std::thread::id, int>> vecP;	// has gtl::recursive_shared_mutex

		int const maxP = 1'000;
		vecP.reserve(maxP);

		std::atomic<int> nReader = 0;
		std::atomic<int> nWriter = 0;
		int nMaxReader = 0;
		int nMaxWriter = 0;
		size_t const nThread{10};

		std::latch latch(nThread);
		auto Producer = [&]() {
			latch.count_down();
			auto id = std::this_thread::get_id();
			for (int i = 0; i < maxP; i++) {
				//std::shared_lock lock6(vecP);

				std::scoped_lock lock(vecP);
				nWriter++;
				if (nMaxWriter < nWriter) nMaxWriter = nWriter;
				vecP.push_back(std::make_unique<std::pair<std::thread::id, int>>(id, i));
				nWriter--;

				std::shared_lock lock2(vecP);
				std::shared_lock lock3(vecP);
				std::unique_lock lock4(vecP);
				std::unique_lock lock5(vecP);

				//std::this_thread::sleep_for(1ms);
			}
		};
		auto Adder = [&]() {
			latch.arrive_and_wait();
			do {
				double dSum = 0;
				{
					std::shared_lock lock(vecP);
					nReader++;
					if (nMaxReader < nReader) nMaxReader = nReader;

					if (nWriter)
						throw std::runtime_error("failed");

					for (int i = 0; i < vecP.size(); i++) {
						dSum += vecP[i].second;
					}
					nReader--;

					//std::this_thread::sleep_for(1ms);
				}
			} while (vecP.size() < maxP);
		};

		std::vector<std::unique_ptr<std::thread> > threads;
		size_t iThread{};
		for (; iThread < 3; iThread++)
			threads.push_back(std::make_unique<std::thread>(Producer));
		for (; iThread < nThread; iThread++)
			threads.push_back(std::make_unique<std::thread>(Adder));

		for (auto& rThread : threads) {
			if (rThread)
				rThread->join();
		}

	}




	gtl::recursive_shared_mutex mtx;

	void reader_thread(int id) {
		std::shared_lock lock(mtx);
		fmt::print("Reader {} attempting to acquire shared lock...\n", id);
		std::shared_lock lock2(mtx);
		fmt::print("Reader {} acquired shared lock.\n", id);
		// simulate some work
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	void writer_thread(int id) {
		std::unique_lock lock(mtx);
		fmt::print("Writer {} attempting to acquire exclusive lock...\n", id);
		std::unique_lock lock2(mtx);
		fmt::print("Writer {} acquired exclusive lock.\n", id);
		// simulate some work
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
				fmt::print("{} {}\n", std::this_thread::get_id(), counter.get());

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

}	// namespace gtl::test::recursive_shared_mutex

