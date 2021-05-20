
#include "pch.h"

//#include "gtl/reflection.h"

#include "gtl/gtl.h"
import gtl;

#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/ptr_container/ptr_vector.hpp"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::boost_ptr_container {

	struct ttt {
		int i, j, k;

		auto operator <=> (ttt const&) const = default;
		ttt(int i={}, int j={}, int k={}) : i(i), j(j), k(k) {}

		virtual ttt* new_clone() {
			return new ttt{*this};
			//return nullptr;
		}
	};

	TEST(ptr_container, test_deque) {
		boost::ptr_deque<ttt> lst;
		lst.push_back(std::make_unique<ttt>(382, 3, 32));
		lst.push_back(new ttt{1, 2, 3});
		lst.push_back(new ttt{0, 31, 33});
		lst.sort();
		auto r = lst.pop_back();
		//std::unique_ptr<ttt> r2 = std::move(lst.pop_back());

		EXPECT_EQ(lst.size(), 2);
		EXPECT_EQ(r->i, 382);
		EXPECT_EQ(r->j, 3);
		EXPECT_EQ(r->k, 32);
	}

}
