#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

#define	SUPPRESS_DEPRECATED_WARNING //_Pragma ("warning(suppress:4996)")

namespace gtl {

	template < typename tchar, typename _Traits >
	std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& stream, gtl::TString<tchar> const& str) {
		if constexpr (std::is_same_v<tchar, char>) {
			return stream << (std::string&)str;
		}
		else {
			return stream << ToStringA(str);
		}
	}
}


TEST(gtl_string, TString_CodepageConversion) {
	using namespace gtl;

	std::string_view sv{"ACBDEF"};

	auto len0 = gtl::tszlen(u"ABCDEF");
	static_assert(gtl::tszlen(u"ABCDEF") == 6);
	auto len1 = gtl::tszlen(u"ABCDEF"sv);
	static_assert(gtl::tszlen(u"ABCDEF"sv) == 6);

	gtl::xStringU16 str;
	str = u"가나다라😊";
	EXPECT_TRUE(str == u"가나다라😊"sv);

	str = U"나다라😊가나다";	// UTF32 -> UTF16
	EXPECT_TRUE(str == u"나다라😊가나다"sv);
	EXPECT_TRUE(str.Compare(u"나다라😊가나다"sv) == 0);

	str = u8"나라😊가나다";
	EXPECT_TRUE(str == u"나라😊가나다"sv);

	EXPECT_TRUE(2 == str.find(u"😊"));

	EXPECT_TRUE(1 == str.find_first_of(u"다라"));
}

TEST(gtl_string, TString_Trim) {
	using namespace gtl;
	
	gtl::xStringU16 str;
	str = u8"나라😊가나다";

	str += u"  \r\n \t\t   \r\n";
	str = u"  \r\n \t\t   \r\n"sv + str;

	EXPECT_EQ(str.TrimLeftView(),                        u"나라😊가나다" u"  \r\n \t\t   \r\n");
	EXPECT_EQ(str.TrimRightView(), u"  \r\n \t\t   \r\n" u"나라😊가나다");
	EXPECT_EQ(str.TrimView(),                            u"나라😊가나다");

	str = u"  \r\n \t\t   \r\n";
	EXPECT_EQ(str.TrimView(), u"");
}

TEST(gtl_string, TString_Upper_Lower) {
	using namespace gtl;

	constexpr char16_t const szCH[] =
		u"Aa Bb Cc Dd Ee Ff Gg Hh Ii Jj Kk Ll Mm Nn Oo Pp Qq Rr Ss Tt Uu Vv Ww Xx Yy Zz "
		"Àà Áá Ǎǎ Ãã Ȧȧ Ââ Ää Åå Āā Ąą Ăă Ćć Ĉĉ Čč Ċċ Çç Ďď Ḑḑ Éé Ěě Ēē Èè Ęę Ẽẽ Ėė Êê "
		"Ëë Ĝĝ Ǵǵ Ǧǧ Ģģ Ğğ Ȟȟ Ĥĥ Īī Įį Íí Ǐǐ Ïï Ĩĩ "
		"İi "
		"Îî Ɨɨ "
		"Iı "
		"Ĵĵ Ǩǩ Ḱḱ Ķķ Ĺĺ Ļļ Łł "
		"Ḿḿ Ňň Ńń Ññ Ņņ Õõ Ǒǒ Öö Őő Óó Òò Øø Ōō Ǫǫ Řř Ŕŕ Ȓȓ Şş Śś Šš Șș Ŝŝ Ťť Țț Ŧŧ "
		"Ŭŭ Üü Ūū Ǔǔ Ųų Ůů Űű Ũũ Úú Ùù Ṽṽ Ẃẃ Ẋẋ Ȳȳ Ỹỹ "
		//"Ÿÿ "
		"Ýý Žž Źź"
		;

	constexpr char16_t const szCH_Exception[] = {
		//0x69, 0x131, 0x142, 0xF8, 0x167,
		0x3f, 0x49, 0x141, 0xd8, 0x166,
	};

	EXPECT_TRUE(std::setlocale(LC_CTYPE, ".1250"));

	auto const * const end = szCH+std::size(szCH);
	for (auto const* pos = szCH; pos < end; pos+=3) {
		if (pos+1 >= end)
			break;

		if (IsSpace(*pos))
			continue;

		// Exception. NOT Working,
		if (std::find(std::begin(szCH_Exception), std::end(szCH_Exception), *pos) != std::end(szCH_Exception))
			continue;

		//auto mapC = gtl::charset::mapUL_latin_extended_g;

		char32_t c1 = *pos;
		char32_t c2 = *(pos+1);

		xStringU16 str1;
		str1 = std::u16string_view(pos, pos+1);
		xStringU16 str2;
		str2 = std::u16string_view(pos+1, pos+2);

		EXPECT_EQ(str1.size(), 1);
		EXPECT_EQ(str2.size(), 1);

		xStringU16 str1L { str1.GetLower() };
		xStringU16 str2U { str2.GetUpper() };
		EXPECT_EQ(str1L, str2);
		EXPECT_EQ(str2U, str1);

	}

}


TEST(gtl_string, TString_etc) {
	//using namespace gtl;

	EXPECT_TRUE(std::setlocale(LC_ALL, ".949"));

	gtl::xStringU16 str;

	str = u"ABCDEFGHIJKLMNOPQUST";

	// Mid, Left, Right
	EXPECT_TRUE(str.MidView(2, 3) == u"CDE"sv);
	EXPECT_TRUE(str.RightView(3) == u"UST"sv);
	EXPECT_TRUE(str.LeftView(5) == u"ABCDE"sv);

	EXPECT_TRUE(str.Mid(2, 3) == u"CDE"s);
	EXPECT_TRUE(str.Right(3) == u"UST"s);
	EXPECT_TRUE(str.Left(5) == u"ABCDE"s);

	EXPECT_TRUE(str.LeftView(100) == u"ABCDEFGHIJKLMNOPQUST"sv);
	EXPECT_TRUE(str.RightView(100) == u"ABCDEFGHIJKLMNOPQUST"sv);
	EXPECT_TRUE(str.MidView(0, 100) == u"ABCDEFGHIJKLMNOPQUST"sv);

	auto c1 = str.SpanExcluding(u"OKLBA"sv);
	auto c2 = str.SpanIncluding(u"OKLBA"sv);
	EXPECT_TRUE(str.SpanExcluding(u"OKLBA"sv) == u"CDEFGHIJMNPQUST"sv);
	EXPECT_TRUE(str.SpanIncluding(u"OKLBA"sv) == u"ABKLO"sv);

	gtl::xStringU16 strR = str.GetReverse();
	EXPECT_TRUE(strR == u"TSUQPONMLKJIHGFEDCBA"sv);
	strR.MakeReverse();
	EXPECT_TRUE(strR == str);


	str.Replace(u'T', 'Z');
	EXPECT_TRUE(str == u"ABCDEFGHIJKLMNOPQUSZ"sv);
	str.Replace(u"JKL"sv, u"**WXY**"sv);
	EXPECT_TRUE(str == u"ABCDEFGHI**WXY**MNOPQUSZ"sv);

	str = u"ABCDABCABCDABCABCDABC";
	str.Replace(u"ABC", u"*ABC*");
	EXPECT_TRUE(str == u"*ABC*D*ABC**ABC*D*ABC**ABC*D*ABC*"sv);

	auto r = str.GetReplacedOpt(u"*ABC*", u"ABC");
	EXPECT_TRUE(r);
	EXPECT_TRUE(r.value_or(u""s) == u"ABCDABCABCDABCABCDABC"sv);
	r = str.GetReplacedOpt(u"가나다", u"");
	EXPECT_TRUE(!r);
	r = str.GetReplacedOpt(u"*ABC*D*ABC**ABC*D*ABC**ABC*D*ABC*"sv, u"");
	EXPECT_TRUE(r);
	EXPECT_TRUE(r->empty());


	str.Remove('D');
	EXPECT_TRUE(str == u"*ABC**ABC**ABC**ABC**ABC**ABC*"sv);

	{
		auto * buf = str.GetBuffer(32);
		buf[0] = u'가';
		buf[1] = u'나';
		buf[2] = u'다';
		buf[3] = u'라';
		buf[4] = u'마';
		buf[5] = 0;
		str.ReleaseBuffer();
		EXPECT_TRUE(str == u"가나다라마"sv);
	}


	{
		auto* buf = str.GetBuffer(15);
		for (int i = 0; i < str.size(); i++) {
			buf[i] = '0'+i;
		}
		std::basic_string<char16_t> p {buf, buf+str.size()};
		str.ReleaseBuffer();
		EXPECT_TRUE(str == p);
	}

	{

		str.clear();
		str.reserve(32);
		str += 'a';
		str += u'가';
		str += U'나';
		str += L'다';
		str += u"라";
		str += u"마"s;
		str += u"바"sv;
SUPPRESS_DEPRECATED_WARNING
		str += u"사"s.c_str();	// NOT Secure
		str += L"아";
		str += u"자차";
		str += U"카";
		str += u8"타";
SUPPRESS_DEPRECATED_WARNING
		str += u8"파하"s.c_str();	// NOT Secure
		EXPECT_TRUE(str == u"a가나다라마바사아자차카타파하"sv);

		str += str;
		EXPECT_TRUE(str == u"a가나다라마바사아자차카타파하a가나다라마바사아자차카타파하"sv);


		str = u"ABCDEF\n";
		str += u"가나다\n"sv;
		str += L"마바사\n"sv;
		str += u8"아자차\n"sv;
		str += U"아자차\n"sv;
		EXPECT_TRUE(str == u"ABCDEF\n가나다\n마바사\n아자차\n아자차\n"sv);

		str.clear();
		str = str + u"ABCDEF\n";
		str = str + u"ABCDEF\n"s;
		str = str + u"ABCDEF\n"s.c_str();	// NOT Secure
		EXPECT_TRUE(str == u"ABCDEF\nABCDEF\nABCDEF\n");

		str = u"ABCDEF\n";
		gtl::xStringU16 str2 = str + L"가나다\n"sv + u8"마바사\n"sv + U"아자차\n"sv;
		EXPECT_TRUE(str2 == u"ABCDEF\n가나다\n마바사\n아자차\n"sv);

		gtl::xStringU16 str3;
		str3 = str + str2;
		str3 = u"가나다\n"sv + str3;
		str3 = U"마바사\n"sv + str3;
		str3 = L"아자차\n"sv + str3;
		EXPECT_TRUE(str3 == u"아자차\n마바사\n가나다\nABCDEF\nABCDEF\n가나다\n마바사\n아자차\n"sv);

		str = u"가나다\n";
		str += u"라마바\n"sv;
		str += "라마바\n"sv;
		str += str3;
SUPPRESS_DEPRECATED_WARNING
		str += str3.c_str();	// NOT Secure

SUPPRESS_DEPRECATED_WARNING
		str += "가나다\n"s.c_str();	// NOT Secure

		str = u"가나다\n"sv;
		str = u'A' + gtl::TString(u"가나다"s);
		EXPECT_TRUE(str == u"A가나다"sv);
		str = L'가' + gtl::TString(u"가나다"s);
		EXPECT_TRUE(str == u"가가나다"sv);

		str = u"가나다"sv;
		gtl::xStringA strA { L"가"sv};
		auto strA1 = L"가" + str + u"도레미";
		auto strA2 = L"가"s + str + U"마바사";
		auto strA3 = L"가"sv + str + L"솔라시";
		gtl::xStringW strA4 {strA + str};
SUPPRESS_DEPRECATED_WARNING
		auto strA5 = L"가"s.c_str() + str;	// NOT Secure
		EXPECT_TRUE(strA1 == L"가가나다도레미"sv);
		EXPECT_TRUE(strA2 == L"가가나다마바사"sv);
		EXPECT_TRUE(strA3 == L"가가나다솔라시"sv);
		EXPECT_TRUE(strA4 == L"가가나다"sv);
		EXPECT_TRUE(strA5 == L"가가나다"sv);

		gtl::xStringW strAc1 {'A' + str};
		EXPECT_TRUE(strAc1 == L"A가나다"sv);

	}


	{
		str = u"\t가나\t다라\t마바\t";
		auto r = str.SplitView(u'\t');
		EXPECT_TRUE(r.size() == 5);
		if (r.size() >= 5) {
			EXPECT_TRUE(r[0] == u""sv);
			EXPECT_TRUE(r[1] == u"가나"sv);
			EXPECT_TRUE(r[2] == u"다라"sv);
			EXPECT_TRUE(r[3] == u"마바"sv);
			EXPECT_TRUE(r[4] == u""sv);
		}
	}

	{
		str = u"\t가나\t다라\t마바";
		auto r = str.SplitView(u'\t');
		EXPECT_TRUE(r.size() == 4);
		if (r.size() >= 4) {
			EXPECT_TRUE(r[0] == u""sv);
			EXPECT_TRUE(r[1] == u"가나"sv);
			EXPECT_TRUE(r[2] == u"다라"sv);
			EXPECT_TRUE(r[3] == u"마바"sv);
		}
	}

	{
		str = u"\t가나\t다라\t마바\t";
		auto r = str.Split(u'\t');
		EXPECT_TRUE(r.size() == 5);
		if (r.size() >= 5) {
			EXPECT_TRUE(r[0] == u""sv);
			EXPECT_TRUE(r[1] == u"가나"sv);
			EXPECT_TRUE(r[2] == u"다라"sv);
			EXPECT_TRUE(r[3] == u"마바"sv);
			EXPECT_TRUE(r[4] == u""sv);
		}
	}

	{
		str = u"\t가나\t다라\t마바";
		auto r = str.Split(u'\t');
		EXPECT_TRUE(r.size() == 4);
		if (r.size() >= 4) {
			EXPECT_TRUE(r[0] == u""sv);
			EXPECT_TRUE(r[1] == u"가나"sv);
			EXPECT_TRUE(r[2] == u"다라"sv);
			EXPECT_TRUE(r[3] == u"마바"sv);
		}
	}

	{
		str = u"\t가나\t\t\t다라\t마바";
		auto r = str.Split(u'\t');
		EXPECT_TRUE(r.size() == 6);
		if (r.size() >= 6) {
			EXPECT_TRUE(r[0] == u""sv);
			EXPECT_TRUE(r[1] == u"가나"sv);
			EXPECT_TRUE(r[2] == u""sv);
			EXPECT_TRUE(r[3] == u""sv);
			EXPECT_TRUE(r[4] == u"다라"sv);
			EXPECT_TRUE(r[5] == u"마바"sv);
		}
	}

	// fmt::format
	{
		str = u"AB"sv;
		str.Format(u"{}, {}, {}", 1, 2, str);
		EXPECT_TRUE(str == u"1, 2, AB"sv);

	}

	// fmt::format
	{
		str = u"AB"sv;
		str.Format(u"{2}, {1}, {0}", 1, 2, str);
		EXPECT_TRUE(str == u"AB, 2, 1"sv);

	}


	// Escape Sequence


	{
		static_assert(L"\101"sv == L"A"sv);
		static_assert(L"\1011"sv == L"A1"sv);
		static_assert(L"\x000041"sv == L"A"sv);
		EXPECT_TRUE(u"\u0000"s != u""sv);
		EXPECT_TRUE(u"\u0000"sv != u""s);
		static_assert(u"\0"sv != u""sv);

		auto const r = gtl::TranslateEscapeSequence(uR"xx(\101)xx"sv);
		EXPECT_TRUE(r);
		if (r) {
			EXPECT_TRUE(*r == u"A"sv);
		}
		//static_assert(gtl::TranslateEscapeSequence(uR"xx()xx"sv) == u""sv);
	}


}
