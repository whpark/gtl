#include "pch.h"

#include "gtl/gtl.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::misc {
	using namespace gtl;

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
}

