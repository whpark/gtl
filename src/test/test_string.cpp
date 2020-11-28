#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

TEST(gtl_string, TString) {

	std::string_view sv{"ACBDEF"};

	auto len0 = gtl::tszlen(u"ABCDEF");
	static_assert(gtl::tszlen(u"ABCDEF") == 6);
	auto len1 = gtl::tszlen(u"ABCDEF"sv);
	static_assert(gtl::tszlen(u"ABCDEF"sv) == 6);

	gtl::CStringU16 str;
	str = u"가나다라😊";
	EXPECT_TRUE(str == u"가나다라😊"sv);

	str = U"나다라😊가나다";	// UTF32 -> UTF16
	EXPECT_TRUE(str == u"나다라😊가나다"sv);
	EXPECT_TRUE(str.Compare(u"나다라😊가나다"sv) == 0);

	str = u8"나라😊가나다";
	EXPECT_TRUE(str == u"나라😊가나다"sv);

	EXPECT_TRUE(2 == str.find(u"😊"));

	EXPECT_TRUE(1 == str.find_first_of(u"다라"));

	str += u"  \r\n \t\t   \r\n";
	str = u"  \r\n \t\t   \r\n" + str;

	EXPECT_TRUE(str.TrimLeftView() ==                        u"나라😊가나다" u"  \r\n \t\t   \r\n");
	EXPECT_TRUE(str.TrimRightView() == u"  \r\n \t\t   \r\n" u"나라😊가나다");
	EXPECT_TRUE(str.TrimView() ==                            u"나라😊가나다");

	str = u"  \r\n \t\t   \r\n";
	EXPECT_TRUE(str.TrimView() == u"");
}
