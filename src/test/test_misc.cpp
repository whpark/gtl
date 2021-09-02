#include "pch.h"

#include "gtl/gtl.h"

import gtl;

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

namespace gtl::test::misc {
	using namespace gtl;

	TEST(misc, xFinalAction) {
		int i{};
		{
			xFinalAction fa{[&]{ i = 3;} };

		}
		EXPECT_EQ(i, 3);
	}
}

