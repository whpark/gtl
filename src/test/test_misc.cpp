#include "pch.h"

#include "gtl/gtl.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

static_assert(std::is_convertible_v<std::unique_ptr<int>, std::shared_ptr<int>>);
static_assert(!std::is_convertible_v<std::shared_ptr<int>, std::unique_ptr<int>>);
static_assert(!std::is_same_v<std::unique_ptr<int>, std::shared_ptr<int>>);

namespace gtl::test::misc {

	class xTestVirtualBase {
	public:
		std::vector<int> data;

		//xTestVirtualBase() = default;
		//xTestVirtualBase(xTestVirtualBase const&) = default;
		//xTestVirtualBase(xTestVirtualBase &&) = default;

		//virtual ~xTestVirtualBase() = default;
	};

	class xTestVirtualDerived : public xTestVirtualBase {
	public:
		std::vector<int> data2;

		virtual ~xTestVirtualDerived() = default;
	};

	void Func1(int, int, double) {}
	void Func1(int, double, int) {}


	TEST(misc, xFinalAction) {

		auto* p = std::bit_cast<int64_t*>(nullptr);

		int i{};
		{
			xFinalAction fa{[&]{ i = 3;} };

		}
		EXPECT_EQ(i, 3);


		//Func1(4, 5, 6);

		xTestVirtualBase ob1;
		ob1.data.push_back(1);
		ob1.data.push_back(2);
		ob1.data.push_back(3);
		auto ob2 = std::move(ob1);
		auto ob3 = ob1;

		EXPECT_EQ(ob1.data.size(), 0);
		EXPECT_EQ(ob2.data.size(), 3);

	}

	TEST(misc, matrix) {
		TMatrix<double, 3, 3> m1 { 1., 0., 0., 0., 2., 0., 0., 0., 3.}, m2{ 3., 0., 0., 0., 3., 0., 0., 0., 3.}, m4 { 3., 0., 0., 0., 6., 0., 0., 0., 9.};
		auto m3 = m1 * m2;
		EXPECT_EQ(m3, m4);
	}

	TEST(misc, ptr_container) {

		gtl::TConcurrentUPtrDeque<int> lst;

		lst.push_back(std::make_unique<int>(3));
		auto b = lst.begin();
		auto e = lst.end();
		auto rb = lst.rbegin();
		auto re = lst.rend();
		for (auto const& i : lst) {
			EXPECT_EQ(i, 3);
		}
		auto const& const_lst = lst;
		for (auto const& i : const_lst) {
			EXPECT_EQ(i, 3);
		}

		lst.clear();
		lst.push_back(std::make_unique<int>(10));
		lst.push_back(std::make_unique<int>(9));
		lst.push_back(std::make_unique<int>(8));
		lst.push_back(std::make_unique<int>(7));
		lst.push_back(std::make_unique<int>(6));
		lst.push_back(std::make_unique<int>(5));
		lst.push_back(std::make_unique<int>(4));
		lst.push_back(std::make_unique<int>(3));
		lst.push_back(std::make_unique<int>(2));
		lst.push_back(std::make_unique<int>(1));
		lst.push_back(std::make_unique<int>(0));

		//lst.push_front(std::make_unique<int>(0));

		std::sort(lst.base_t::begin(), lst.base_t::end(), [](auto const& a, auto const& b) { return *a<*b;});
		EXPECT_EQ(lst[0], 0);
		EXPECT_EQ(lst[10], 10);
		lst.clear();

		// test thread
		{
			constexpr size_t N = 100;
			gtl::TConcurrentUPtrDeque<int> lst;
			std::latch latchQueue(N);
			auto Queue = [&](size_t index) {
				latchQueue.count_down();
				latchQueue.wait();
				std::unique_lock lock(lst);
				for (int i = 0; i < N; i++) {
					lst.push_back(std::make_unique<int>((int)(index*N + i)));
				}
			};
			std::vector<std::thread> threadsQueue;
			threadsQueue.reserve(N);
			for (size_t i{}; i < N; i++) {
				threadsQueue.emplace_back(Queue, i);
			}
			for (auto& t : threadsQueue)
				t.join();

			std::latch latch(N);
			std::vector<std::thread> threads;
			threads.reserve(N);
			size_t sum{};
			for (size_t i{}; i < N; i++) {
				threads.emplace_back([&]{
					latch.count_down();
					latch.wait();
					std::unique_lock lock(lst);
					for (auto i : lst) {
						sum += i;
					}
				});
			}

			for (auto& t : threads)
				t.join();

			auto n = N*N-1;
			EXPECT_EQ(sum, ((1+n)*n/2)*N);
		}

		{
			constexpr size_t N = 100;
			gtl::TConcurrentUPtrDeque<int> lst;
			std::latch latchQueue(N);
			auto Queue = [&](size_t index) {
				latchQueue.count_down();
				latchQueue.wait();
				//std::unique_lock lock(lst);
				for (int i = 0; i < N; i++) {
					lst.push_back(std::make_unique<int>(index*N + i));
				}
			};

			std::vector<std::thread> threadsQueue;
			threadsQueue.reserve(N);
			for (size_t i{}; i < N; i++) {
				threadsQueue.emplace_back(Queue, i);
			}
			for (auto& t : threadsQueue)
				t.join();
			EXPECT_EQ(lst.size(), N*N);

			std::latch latch(N);
			std::vector<std::thread> threads;
			threads.reserve(N);
			size_t sum{};
			for (size_t i{}; i < N; i++) {
				threads.emplace_back([&]{
					latch.count_down();
					latch.wait();
					std::unique_lock lock(lst);
					for (auto i : lst) {
						sum += i;
					}
					});
			}

			for (auto& t : threads)
				t.join();

			auto n = N*N-1;
			EXPECT_EQ(sum, ((1+n)*n/2)*N);
		}
	}

	TEST(misc, StopWatch) {
		{
			gtl::TStopWatch<wchar_t> sw(std::wcout);
			sw.Lap(L"asdfasdf{}, {}", 1, L"asdf");
			sw.Lap(L"asdfasdf{}, {}", 2, L"asdf");
			sw.Lap(L"asdfasdf{}, {}", 3, L"가나다");
		}

		{
			gtl::TStopWatch sw(std::cout);
			sw.Lap("asdfasdf{}, {}", 1, "asdf");
			sw.Lap("asdfasdf{}, {}", 2, "asdf");
			sw.Lap("asdfasdf{}, {}", 3, "가나다");
		}
	}
}

