#include "pch.h"

#include "gtl/gtl.h"

TEST(gtl_string, tszto) {

	auto ExpectEQ = []<typename T1, typename T2 = T1>(T1 && a, T2 && b) {
		EXPECT_EQ(std::forward<T1>(a), std::forward<T2>(b));
	};


	using namespace std::literals;
	using namespace gtl::literals;
	auto a = gtl::tsztoi("12345"sv);

	std::string str;
	gtl::CStringA str2;
	std::string_view sv = str;
	sv = (std::string_view)str2;

	EXPECT_EQ(123456, gtl::tsztoi((std::string_view)gtl::TString<char>("123456")));
	EXPECT_EQ(123456, gtl::tsztoi((std::string_view)gtl::TString<char>("123456")));
	EXPECT_EQ(123456, gtl::tsztoi("123456"sv));
	EXPECT_EQ(123456, gtl::tsztoi("123456"sv));
	EXPECT_EQ(0xABCD'EFab'1234'9853I64, gtl::tsztoi<int64_t>("0xABCD'EFab'1234'9853"sv, (char**)nullptr, 0, '\''));
	EXPECT_EQ(0xABCDI64, gtl::tsztoi("0xABCD'EFab'1234'9853"sv));

	EXPECT_EQ(1.1, gtl::tsztod("1.1"sv));
	EXPECT_EQ(+1.1, gtl::tsztod("+1.1"sv));
	EXPECT_EQ(1.3e3, gtl::tsztod("1.3e3"sv));
	EXPECT_EQ(-1.1, gtl::tsztod("-1.1"sv));
	EXPECT_EQ(-1.3e3, gtl::tsztod("-1.3e3"sv));
	EXPECT_EQ(1.3e-3, gtl::tsztod("1.3e-3"sv));
	EXPECT_EQ(-1.3e-3, gtl::tsztod("-1.3e-3"sv));

	std::regex rx(R"(^([+-]?(?:[[:d:]]+\.?|[[:d:]]*\.[[:d:]]+))(?:[Ee][+-]?[[:d:]]+)?$)");
	std::regex reg{ R"x(^([+\-]?(?:[[:d:]]+\.?|[[:d:]]*\.[[:d:]]+))(?:[Ee][+\-]?[[:d:]]+)?$)x" };
	//std::regex_starts_with
	//static constexpr auto pattern = ctll::fixed_string{ R"()" };
	static constexpr auto pattern = ctll::fixed_string{ R"x(^[\-+]?[0-9]*\.?[0-9]+([eE][\-+]?[0-9]+)?$)x" };
	//auto str_regex = ctre::starts_with<pattern>("-0.32840e9agdrgai");




}
