#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/iconv_wrapper.h"

using namespace std::literals;
using namespace gtl::literals;

#pragma warning(disable:4566)	// character encoding


TEST(gtl_string, codepage) {
#pragma warning(push)
#pragma warning(disable: 4566)

	gtl::g_eCodepageMBCS = gtl::GetHostCodepage();

	// unicode <=> mbcs
	auto rstrA = gtl::ToString_iconv<char, wchar_t, "CP949">(L"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv);
	std::u16string str1 = gtl::ToStringU16(*rstrA, {.from = gtl::eCODEPAGE::KO_KR_949});
	EXPECT_TRUE(str1 == u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"s);

	std::string str2 = gtl::ToStringA(u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍", {.to = gtl::eCODEPAGE::KO_KR_949});

	EXPECT_TRUE(str2 == (*gtl::ToString_iconv<char, wchar_t, "CP949">(L"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv)));

	std::string str3 = gtl::ToStringA(u"adfasdf가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍", {.to = gtl::eCODEPAGE::KO_KR_949});
	EXPECT_TRUE(str3 == *gtl::ToString_iconv<char>(u"adfasdf가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv, "CP949"));

#define TEST_STRING "asdfasdf가나다라마adrg바sfdgdh사아자차카타dd파하긎긣꿳fghs뎓뫓멙뻍😊❤✔"

	std::u8string stru8(TEXT_u8(TEST_STRING));
	std::u16string stru16(TEXT_u(TEST_STRING));
	std::u32string stru32(TEXT_U(TEST_STRING));

	// unicode <=> unicode
	std::u16string str4 = gtl::ToStringU16(TEXT_u8(TEST_STRING));
	EXPECT_TRUE(str4 ==  TEXT_u(TEST_STRING));
	std::u32string str5 = gtl::ToStringU32(TEXT_u8(TEST_STRING));
	EXPECT_TRUE(str5 ==  TEXT_U(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( TEXT_u8(TEST_STRING))  ==  TEXT_u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( TEXT_u8(TEST_STRING))  ==  TEXT_u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( TEXT_u8(TEST_STRING))  ==  TEXT_U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   TEXT_u8(TEST_STRING))  ==  TEXT_W(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( TEXT_u(TEST_STRING))  ==  TEXT_u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( TEXT_u(TEST_STRING))  ==  TEXT_u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( TEXT_u(TEST_STRING))  ==  TEXT_U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   TEXT_u(TEST_STRING))  ==  TEXT_W(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( TEXT_U(TEST_STRING))  ==  TEXT_u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( TEXT_U(TEST_STRING))  ==  TEXT_u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( TEXT_U(TEST_STRING))  ==  TEXT_U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   TEXT_U(TEST_STRING))  ==  TEXT_W(TEST_STRING));

	EXPECT_TRUE(gtl::ToStringU8 ( TEXT_W(TEST_STRING))  ==  TEXT_u8(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU16( TEXT_W(TEST_STRING))  ==  TEXT_u(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringU32( TEXT_W(TEST_STRING))  ==  TEXT_U(TEST_STRING));
	EXPECT_TRUE(gtl::ToStringW(   TEXT_W(TEST_STRING))  ==  TEXT_W(TEST_STRING));

#pragma warning(pop)

}


TEST(gtl_string_codepage_Test, iconv_wrapper) {

	gtl::g_eCodepageMBCS = gtl::GetHostCodepage();

	// unicode <=> mbcs
	auto r1 = gtl::ToString_iconv<char16_t>(gtl::ToStringA(U"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv, {.to = gtl::eCODEPAGE::KO_KR_949}), nullptr, "CP949");
	std::u16string str1 = r1.value_or(u"ERROR"s);
	EXPECT_EQ(str1, u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv);

	std::u8string strLong_u8(TEXT_u8(TEST_STRING));
	int n {4};
	for (int i = 0; i < n; i++) {
		strLong_u8 += strLong_u8 + strLong_u8;
	}
	std::u16string strLong_u(TEXT_u(TEST_STRING));
	for (int i = 0; i < n; i++) {
		strLong_u += strLong_u + strLong_u;
	}
	auto r = gtl::ToString_iconv<char16_t>(strLong_u8);
	EXPECT_EQ(strLong_u , gtl::ToString_iconv<char16_t>(strLong_u8));
	EXPECT_TRUE(strLong_u8 == gtl::ToString_iconv<char8_t>(strLong_u));
	EXPECT_EQ(strLong_u , (gtl::ToString_iconv<char16_t, char8_t, "", "", 0>(strLong_u8)));
	EXPECT_TRUE(strLong_u8 == (gtl::ToString_iconv<char8_t, char16_t, "", "", 0>(strLong_u)));

	auto r2 = gtl::ToString_iconv<char>(u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv, "CP949");
	EXPECT_TRUE(r2 == gtl::ToStringA(L"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv, {.to = gtl::eCODEPAGE::KO_KR_949}));

	auto r3 = gtl::ToString_iconv<char>(u"adfasdf가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv, "CP949");
	EXPECT_TRUE(r3 == gtl::ToStringA(L"adfasdf가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍"sv, {.to = gtl::eCODEPAGE::KO_KR_949}));

	std::u8string stru8(TEXT_u8(TEST_STRING));
	std::u16string stru16(TEXT_u(TEST_STRING));
	std::u32string stru32(TEXT_U(TEST_STRING));

	// unicode <=> unicode
	std::u16string str4 = gtl::ToStringU16(TEXT_u8(TEST_STRING));
	EXPECT_TRUE(str4 ==  TEXT_u(TEST_STRING));
	std::u32string str5 = gtl::ToStringU32(TEXT_u8(TEST_STRING));
	EXPECT_TRUE(str5 ==  TEXT_U(TEST_STRING));

	EXPECT_TRUE(gtl::ToString_iconv<char8_t> (TEXT_u8(TEST_STRING) ""sv)  ==  TEXT_u8(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char16_t>(TEXT_u8(TEST_STRING) ""sv), TEXT_u(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char32_t>(TEXT_u8(TEST_STRING) ""sv), TEXT_U(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<wchar_t> (TEXT_u8(TEST_STRING) ""sv), TEXT_W(TEST_STRING));

	EXPECT_TRUE(gtl::ToString_iconv<char8_t> (TEXT_u(TEST_STRING) ""sv)  ==  TEXT_u8(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char16_t>(TEXT_u(TEST_STRING) ""sv), TEXT_u(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char32_t>(TEXT_u(TEST_STRING) ""sv), TEXT_U(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<wchar_t> (TEXT_u(TEST_STRING) ""sv), TEXT_W(TEST_STRING));

	EXPECT_TRUE(gtl::ToString_iconv<char8_t> (TEXT_U(TEST_STRING) ""sv)  ==  TEXT_u8(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char16_t>(TEXT_U(TEST_STRING) ""sv), TEXT_u(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char32_t>(TEXT_U(TEST_STRING) ""sv), TEXT_U(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<wchar_t> (TEXT_U(TEST_STRING) ""sv), TEXT_W(TEST_STRING));

	EXPECT_TRUE(gtl::ToString_iconv<char8_t> (TEXT_W(TEST_STRING) ""sv)  ==  TEXT_u8(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char16_t>(TEXT_W(TEST_STRING) ""sv), TEXT_u(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<char32_t>(TEXT_W(TEST_STRING) ""sv), TEXT_U(TEST_STRING));
	EXPECT_EQ(gtl::ToString_iconv<wchar_t> (TEXT_W(TEST_STRING) ""sv), TEXT_W(TEST_STRING));


}
