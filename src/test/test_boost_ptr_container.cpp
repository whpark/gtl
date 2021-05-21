
#include "pch.h"

#include <compare>

#include "gtl/dynamic.h"
#include "gtl/reflection.h"
#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/ptr_container/ptr_vector.hpp"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::boost_ptr_container {

	struct ITT {
		GTL__VIRTUAL_DYNAMIC_INTERFACE(ITT);

		auto operator <=> (ITT const&) const = default;

	};

	struct ttt : public ITT {
		int i, j, k;

		auto operator <=> (ttt const&) const = default;
		ttt(int i={}, int j={}, int k={}) : i(i), j(j), k(k) {}

		GTL__VIRTUAL_DYNAMIC_DERIVED(ttt);

	};

	struct tt2 : public ttt {
		double l{}, m{};

		tt2() = default;
		tt2(tt2 const&) = default;
		tt2(ttt const& base, double l, double m) : ttt(base), l(l), m(m) {}
		tt2& operator = (tt2 const&) = default;

		auto operator <=> (tt2 const&) const = default;

		GTL__VIRTUAL_DYNAMIC_DERIVED(tt2);

	};

	TEST(ptr_container, test_deque) {
		boost::ptr_deque<ttt> lst;
		lst.push_back(std::make_unique<ttt>(382, 3, 32));
		lst.push_back(std::make_unique<ttt>(1, 2, 3));
		lst.push_back(std::make_unique<ttt>(0, 31, 33));
		lst.push_back(std::make_unique<tt2>(ttt{1, 2, 3}, 1.0, 2.0));
		lst.push_back(std::make_unique<tt2>(ttt{4, 5, 6}, 1.0, 2.0));
		{
			boost::ptr_deque<ttt> lst2 = lst;
			auto& a0 = lst2[0];
			auto& a1 = lst2[1];
			auto& a2 = lst2[2];
			auto& a3 = lst2[3];
			auto& a4 = lst2[4];

			int k = a0.i;
		}

		lst.sort();
		auto r = lst.pop_back();
		//std::unique_ptr<ttt> r2 = std::move(lst.pop_back());

		EXPECT_EQ(lst.size(), 4);
		EXPECT_EQ(r->i, 382);
		EXPECT_EQ(r->j, 3);
		EXPECT_EQ(r->k, 32);
	}

}
