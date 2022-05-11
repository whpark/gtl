#include "pch.h"

//#include "gtl/mutex.h"
import gtl;

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::recursive_shared_mutex {

	TEST(thread, mutex) {

		//gtl::recursive_shared_mutex mutex;
		gtl::xSimpleLog log;

		//std::vector<std::unique_ptr<std::pair<std::thread::id, int> > > vecP;
		gtl::TConcurrentUPtrVector<std::pair<std::thread::id, int>> vecP;

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
}

