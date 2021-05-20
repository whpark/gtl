#include "pch.h"

//#include "gtl/mutex.h"
#include "gtl/gtl.h"
import gtl;

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::recursive_shared_mutex {
	

	//std::vector<std::unique_ptr<pair<thread::id, int>>> vecP;

	//const int maxP = 1'000;
	//vecP.reserve(maxP);

	//atomic<int> nReader = 0;
	//atomic<int> nWriter = 0;
	//int nMaxReader = 0;
	//int nMaxWriter = 0;

	//auto Producer = [&]() {
	//	auto id = this_thread::get_id();
	//	for (int i = 0; i < maxP; i++) {
	//		std::shared_lock lock6(vecP);

	//		std::scoped_lock lock(vecP);
	//		nWriter++;
	//		if (nMaxWriter < nWriter) nMaxWriter = nWriter;
	//		vecP.push_back(std::make_unique<pair<thread::id, int>>(id, i));
	//		nWriter--;

	//		std::shared_lock lock2(vecP);
	//		std::shared_lock lock3(vecP);
	//		std::unique_lock lock4(vecP);
	//		std::unique_lock lock5(vecP);

	//		std::this_thread::sleep_for(1ms);
	//	}
	//};
	//auto Adder = [&]() {
	//	do {
	//		double dSum = 0;
	//		{
	//			std::shared_lock lock(vecP);
	//			nReader++;
	//			if (nMaxReader < nReader) nMaxReader = nReader;

	//			if (nWriter)
	//				throw std::runtime_error("failed");

	//			for (int i = 0; i < vecP.size(); i++) {
	//				dSum += vecP[i]->second;
	//			}
	//			nReader--;

	//			std::this_thread::sleep_for(1ms);
	//		}
	//	} while (vecP.size() < maxP);
	//};

	//std::vector<std::unique_ptr<std::thread>> threads;
	//for (int i = 0; i < 10; i++) {
	//	if (i < 3)
	//		threads.push_back(make_unique<thread>(Producer));
	//	//else
	//	//	threads.push_back(make_unique<thread>(Adder));
	//}

	//for (auto& rThread : threads) {
	//	if (rThread)
	//		rThread->join();
	//}

}
