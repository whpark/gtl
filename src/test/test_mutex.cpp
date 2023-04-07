#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/mutex.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::mutex {

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
