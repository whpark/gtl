#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/string.h"

#pragma warning(disable:4566)	// character encoding

using namespace std::literals;
using namespace gtl::literals;

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


TEST(gtl_string, TString) {
	using namespace gtl;

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

	constexpr char32_t szCH[] =
		U"Aa Bb Cc Dd Ee Ff Gg Hh Ii Jj Kk Ll Mm Nn Oo Pp Qq Rr Ss Tt Uu Vv Ww Xx Yy Zz "
		U"Àà Áá Ǎǎ Ãã Ȧȧ Ââ Ää Åå Āā Ąą Ăă Ćć Ĉĉ Čč Ċċ Çç Ďď Ḑḑ Éé Ěě Ēē Èè Ęę Ẽẽ Ėė Êê "
		U"Ëë Ĝĝ Ǵǵ Ǧǧ Ģģ Ğğ Ȟȟ Ĥĥ Īī Įį Íí Ǐǐ Ïï Ĩĩ İi Îî Ɨɨ Iı Ĵĵ Ǩǩ Ḱḱ Ķķ Ĺĺ Ļļ Łł "
		U"Ḿḿ Ňň Ńń Ññ Ņņ Õõ Ǒǒ Öö Őő Óó Òò Øø Ōō Ǫǫ Řř Ŕŕ Ȓȓ Şş Śś Šš Șș Ŝŝ Ťť Țț Ŧŧ "
		U"Ŭŭ Üü Ūū Ǔǔ Ųų Ůů Űű Ũũ Úú Ùù Ṽṽ Ẃẃ Ẋẋ Ȳȳ Ỹỹ Ÿÿ Ýý Žž Źź"
		;

	for (char32_t const* pos = szCH; pos < szCH+std::size(szCH); pos+=3) {
		if (pos+1 >= szCH+std::size(szCH))
			break;

		if (IsSpace(*pos))
			continue;

		char32_t c1 = *pos;
		char32_t c2 = *(pos+1);

		CStringU16 str1;
		str1 = std::u32string_view(pos, pos+1);
		CStringU16 str2;
		str2 = std::u32string_view(pos+1, pos+2);

		EXPECT_TRUE(str1.size() == 1);
		EXPECT_TRUE(str2.size() == 1);

		CStringU16 str1L = str1.GetLower();
		CStringU16 str2U = str2.GetUpper();
		if (str1 != str2U) {
			int i{};
		}
		if (str2 != str1L) {
			int i{};
		}
		EXPECT_TRUE(str1L == str2);
		EXPECT_TRUE(str2U == str1);

	}
	
}
