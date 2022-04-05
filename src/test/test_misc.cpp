#include "pch.h"

#include "gtl/gtl.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

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

		gtl::uptr_deque<int> lst;

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

		std::sort(lst.base_t::begin(), lst.base_t::end(), [](auto const& a, auto const& b) { return *a<*b;});

		EXPECT_EQ(lst[0], 0);
		EXPECT_EQ(lst[10], 10);

	}
}

