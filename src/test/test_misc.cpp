#include "pch.h"

#include "gtl/gtl.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::misc {
	using namespace gtl;

	TEST(misc, xTrigger) {
		int i{};
		{
			xTrigger trigger{[&]{ i = 3;} };

		}
		EXPECT_EQ(i, 3);
	}
}

