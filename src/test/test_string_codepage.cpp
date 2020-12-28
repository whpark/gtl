#include "pch.h"

#include "gtl/gtl.h"

using namespace std::literals;
using namespace gtl::literals;


TEST(gtl_string, codepage) {
#pragma warning(push)
#pragma warning(disable: 4566)

	// unicode <=> mbcs
	std::u16string str1 = gtl::ToStringU16("가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍", {.from = gtl::eCODEPAGE::KO_KR_949});
	EXPECT_TRUE(str1 == u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"s);

	std::string str2 = gtl::ToStringA(u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍", {.to = gtl::eCODEPAGE::KO_KR_949});
	EXPECT_TRUE(str2 == "가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"s);

	std::string str3 = gtl::ToStringA(u"adfasdf가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍", {.to = gtl::eCODEPAGE::KO_KR_949});
	EXPECT_TRUE(str3 == "adfasdf가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"s);

#define TEST_STRING "asdfasdf가나다라마adrg바sfdgdh사아자차카타dd파하긎긣꿳fghs뎓뫓멙뻍😊❤✔"

	std::u8string stru8(_u8(TEST_STRING));
	std::u16string stru16(_u(TEST_STRING));
	std::u32string stru32(_U(TEST_STRING));

	// unicode <=> unicode
	std::u16string str4 = gtl::ToStringU16(_u8(TEST_STRING));
	EXPECT_TRUE(str4 ==  _u(TEST_STRING));
	std::u32string str5 = gtl::ToStringU32(_u8(TEST_STRING));
	EXPECT_TRUE(str5 ==  _U(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( _u8(TEST_STRING))  == _u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( _u8(TEST_STRING))  ==  _u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( _u8(TEST_STRING))  ==  _U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   _u8(TEST_STRING))  ==  _W(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( _u(TEST_STRING))  == _u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( _u(TEST_STRING))  ==  _u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( _u(TEST_STRING))  ==  _U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   _u(TEST_STRING))  ==  _W(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( _U(TEST_STRING))  == _u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( _U(TEST_STRING))  ==  _u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( _U(TEST_STRING))  ==  _U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   _U(TEST_STRING))  ==  _W(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( _W(TEST_STRING))  == _u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( _W(TEST_STRING))  ==  _u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( _W(TEST_STRING))  ==  _U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   _W(TEST_STRING))  ==  _W(TEST_STRING));

#pragma warning(pop)

}
