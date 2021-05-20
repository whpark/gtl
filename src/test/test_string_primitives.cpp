#include "pch.h"

#include "gtl/gtl.h"
import gtl;

using namespace std::literals;
using namespace gtl::literals;

using namespace gtl;



#define	SUPPRESS_DEPRECATED_WARNING _Pragma ("warning(suppress:4996)")

//#if 0
//template < typename tchar >				void TestFuncAsConst(tchar const* psz, size_t size) {}
//
//template < typename tchar, int size >	void TestFuncAsConst(tchar const (&sz)[size]) {}
//template < typename tchar, int size >	void TestFuncAsConst(std::array<tchar, size> const& sz) {}
//template < typename tchar >				[[deprecated("NotSecure")]]void TestFuncAsConst(tchar const*const& psz) {}
//template < typename tchar >				void TestFuncAsConst(std::basic_string_view<tchar> sv) {}
//template < typename tchar >				void TestFuncAsConst(std::basic_string<tchar> const& str) {}
//char const* SomeFunc() { return nullptr; }
//
//template < typename tchar, int size >	void TestFuncAsNonConst(tchar (&sz)[size]) {}
//template < typename tchar, int size >	void TestFuncAsNonConst(std::array<tchar, size>& sz) {}
//template < typename tchar >				[[deprecated("NotSecure")]]void TestFuncAsNonConst(tchar*const& psz) {}
////template < typename tchar >				void TestFuncAsNonConst(std::basic_string_view<tchar> sv) {}
//template < typename tchar >				void TestFuncAsNonConst(std::basic_string<tchar>& str) {}
//
//void TestFuncVar() {
//	if constexpr (false) {
//		TestFuncAsConst("abc");
//
//		char buf[100];
//		TestFuncAsConst(buf);
//
//		std::array<char, 32> bufa;
//		TestFuncAsConst(bufa);
//
//		char const* psz1 = "asdf";
//		TestFuncAsConst(psz1);
//
//		TestFuncAsConst(SomeFunc());
//
//		constexpr char const * psz2 = "asdf";
//		TestFuncAsConst(psz2);
//
//
//		TestFuncAsConst("abc"sv);
//		TestFuncAsConst("abc"s);
//		std::string str {"abcdef"};
//		TestFuncAsConst(str);
//	}
//
//	if constexpr (false) {
//		TestFuncAsNonConst("abc");
//
//		char buf[100];
//		TestFuncAsNonConst(buf);
//
//		std::array<char, 32> bufa;
//		TestFuncAsNonConst(bufa);
//
//		char const* psz1 = "asdf";
//		TestFuncAsNonConst(psz1);
//
//		TestFuncAsNonConst(SomeFunc());
//
//		constexpr char const * psz2 = "asdf";
//		TestFuncAsNonConst(psz2);
//
//
//		//TestFuncAsNonConst("abc"sv);
//		//TestFuncAsNonConst("abc"s);
//		std::string str {"abcdef"};
//		TestFuncAsNonConst(str);
//	}
//}
//#endif

//#if 0
//namespace template_instantiation {
//
//	template < typename T, typename T2 >
//	concept addable =
//	requires (T2 a) { a+a; a*a;};
//
//	template < typename T >
//	void func() {
//		if constexpr (addable<int, T>) {
//			static_assert(true);
//		}
//		else {
//			static_assert(false);
//		}
//	}
//
//	template <> void func<int>() {
//	}
//	template void func<float>();
//	//template void func<std::string_view>();
//
//};
//#endif


TEST(gtl_string, tszlen) {
	{
		char buf0[100];
		std::array<char, 10> buf1;
		std::vector<char> buf2;

		auto const* v0 = std::data(buf0);
		auto const* v1 = std::data(buf1);
		auto const* v2 = std::data(buf2);
		auto s0 = std::size(buf0);
		auto s1 = std::size(buf1);
		auto s2 = std::size(buf2);

		EXPECT_EQ(std::size("ABCDEF"), 7);
	}

	//tszlen
	EXPECT_EQ(5, "asdfe"sv.size());
	gtl::tszlen("123456789012");
	EXPECT_EQ(12, gtl::tszlen("123456789012"));
	EXPECT_EQ(11, (gtl::tszlen(u"asdfjaskdlf", u"asdfjaskdlf"sv.size()+1)));
	EXPECT_EQ(3, gtl::tszlen(U"가나다"));
	EXPECT_EQ(4, gtl::tszlen(u"가나다라"));
	EXPECT_EQ(4, gtl::tszlen(L"가나다라"));
	EXPECT_EQ(8, gtl::tszlen("가나다라"));
	constexpr char16_t const* sz3 = u"가나다";
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	constexpr auto l0 = tszlen(sz3);
	static_assert(l0 == 3);
	constexpr auto l1 = gtl::tszlen(u"가나다");
	static_assert(l1 == 3);

	char16_t sz[128] {u"abcdefghijklmnopqrstu_________________________"};
	EXPECT_EQ(68-22, tszlen(sz));

	std::vector<char> szVec{ {'1', '2', '3', '4', '5', 0}};
	EXPECT_EQ(szVec.size()-1, tszlen(szVec));
	std::array<char, 10> szArray {"ABCDEFG"};
	EXPECT_EQ(7, tszlen(szArray));

	std::array<char const, 10> szConstArray {"ABCDEFG"};
	EXPECT_EQ("ABCDEFG"sv.size(), tszlen(szConstArray));

}


TEST(gtl_string, tszcpy) {

	// tszcpy
	{
		std::vector<char> buf;
		buf.resize(10);
		char const* pszSrc = "ABCDEF";
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(EINVAL == tszcpy((char*)nullptr, buf.size(), pszSrc));
		EXPECT_TRUE(EINVAL == tszcpy(buf.data(), 0, "ABCDEF"sv));
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(EINVAL == tszcpy(buf.data(), buf.size(), (char*)nullptr));
		auto const* psz1 = "ABCDEF";
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(     0 == tszcpy(buf.data(), buf.size(), psz1));
		auto const* pszLong = "long string................";
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(ERANGE == tszcpy(buf.data(), buf.size(), pszLong));
		EXPECT_TRUE(     0 == tszlen(buf.data(), buf.size()));

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(     0 == tszcpy(buf, psz1));
		EXPECT_TRUE(buf.data() == "ABCDEF"sv);

		std::array<char, 32> buf2;
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(     0 == tszcpy(buf2, psz1));
		EXPECT_TRUE(buf2.data() == "ABCDEF"sv);

		char buf3[32]{};
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(     0 == tszcpy(buf3, psz1));
		EXPECT_TRUE(buf3 == "ABCDEF"sv);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(ERANGE == tszcpy(buf, pszLong));
		EXPECT_TRUE(     0 == tszlen(buf));
	}

	// tszcpy from string_view
	{
		constexpr bool b = gtlc::contiguous_type_string_container<std::basic_string_view<char>, char>;

		char16_t buf[32] {};
		EXPECT_TRUE(  0 == tszcpy(buf, std::size(buf), u"가나다라마바사"sv));
		EXPECT_TRUE(  0 == tszcpy(buf, u"가나다라마바사"sv));
		EXPECT_TRUE(buf == u"가나다라마바사"sv);
	}
	{
		std::vector<char> buf;
		EXPECT_TRUE(EINVAL == tszcpy(buf, "abcdef"sv));

		buf.resize(10);
		EXPECT_TRUE(     0 == tszcpy(buf, "abcdef"sv));
		EXPECT_TRUE(     0 == memcmp(buf.data(), "abcdef"sv.data(), 7*sizeof(buf[0])));
		EXPECT_TRUE(EINVAL == tszcpy(buf.data(), 0, "abcdef"sv));
		EXPECT_TRUE(ERANGE == tszcpy(buf, "long string...... ............"sv));
	}
	{
		std::vector<char16_t> buf;
		EXPECT_TRUE(EINVAL == tszcpy(buf, u"abcdef"sv));

		buf.resize(10);
		EXPECT_TRUE(     0 == tszcpy(buf, u"가나다라마바"sv));
		EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다라마바"sv.data(), 7*sizeof(buf[0])));
		EXPECT_TRUE(EINVAL == tszcpy(buf.data(), 0, u"abcdef"sv));
		EXPECT_TRUE(ERANGE == tszcpy(buf, u"long string...... ............"sv));
	}
	{
		std::vector<char32_t> buf;
		EXPECT_TRUE(EINVAL == tszcpy(buf, U"abcdef"sv));

		buf.resize(10);
		EXPECT_TRUE(     0 == tszcpy(buf, U"가나다라마바"sv));
		EXPECT_TRUE(     0 == memcmp(buf.data(), U"가나다라마바"sv.data(), 7*sizeof(buf[0])));
		EXPECT_TRUE(EINVAL == tszcpy(buf.data(), 0, U"abcdef"sv));
		EXPECT_TRUE(ERANGE == tszcpy(buf, U"long string...... ............"sv));
	}
	{
		std::array<char16_t, 32> buf;
		EXPECT_TRUE(         0 == tszcpy(buf, u"가나다라마바사"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"s);
	}
}

TEST(gtl_string, tszncpy) {
	std::vector<char16_t> buf;
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(EINVAL == tszncpy(buf.data(), buf.size(), u"가나다라마바사", _TRUNCATE));
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(EINVAL == tszncpy(buf.data(), buf.size(), u"가나다라마바사", 4));
	buf.resize(10);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(ERANGE == tszncpy(buf.data(), buf.size(), u"가나다라마바사", 10));
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(EINVAL == tszncpy(buf.data(),          0, u"가나다라마바사", 2));
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == tszncpy(buf.data(), buf.size(), u"가나다라마바사", 9));

	buf.resize(10);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사", _TRUNCATE));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다라마바사", 8*sizeof(buf[0])));

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다", _TRUNCATE));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다", 4*sizeof(buf[0])));

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다", 5));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다", 3*sizeof(buf[0])));

	buf.resize(5);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사", 4));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다라", 5*sizeof(buf[0])));

	EXPECT_TRUE(EINVAL == tszncpy(buf.data(), 0, u"가나다라마바사"sv));
	buf.resize(1);
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사"sv));
	EXPECT_TRUE(ERANGE == tszncpy(buf, u"가나다라마바사"sv, 1));
	buf.resize(32);
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사"sv, 3));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다", 4*sizeof(buf[0])));

	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사"sv, _TRUNCATE));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다라마바사", 8*sizeof(buf[0])));

	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다"sv, _TRUNCATE));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다", 4*sizeof(buf[0])));

	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다"sv, 5));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다", 3*sizeof(buf[0])));

	buf.resize(5);
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사"sv, 4));
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다라", 5*sizeof(buf[0])));
}

TEST(gtl_string, tszcat) {
	{
		std::vector<char16_t> buf;
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(EINVAL == tszcat(buf.data(), 1, u"가나다라마바사"));
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(EINVAL == tszcat(buf, (char16_t*)nullptr));
		buf.resize(32);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(EINVAL == tszcat(buf.data(), -1, u"가나다"));

		EXPECT_TRUE(         0 == tszcat(buf, u"가나다라마바사"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(         0 == tszcat(buf, u"ABCDEFG"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사ABCDEFG"sv);

		buf[0] = 0;
		EXPECT_TRUE(         0 == tszcat(buf, u"가나다라마바사"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(         0 == tszcat(buf, u"ABCDEFG"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사ABCDEFG"sv);
	}
	{
		std::array<char16_t, 32> buf;
		EXPECT_TRUE(ERANGE == tszcat(buf.data(), 1, u"가나다라마바사"sv));
		EXPECT_TRUE(EINVAL == tszcat(buf.data(), -1, u"가나다"sv));

		buf[0] = 0;
		EXPECT_TRUE(         0 == tszcat(buf, u"가나다라마바사"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(         0 == tszcat(buf, u"ABCDEFG"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사ABCDEFG"sv);

		buf[0] = 0;
		EXPECT_TRUE(         0 == tszcat(buf, u"가나다라마바사"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(         0 == tszcat(buf, u"ABCDEFG"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사ABCDEFG"sv);
	}
	{
		char16_t buf[32];
		EXPECT_TRUE(ERANGE == tszcat(buf, 1, u"가나다라마바사"sv));
		EXPECT_TRUE(EINVAL == tszcat(buf, -1, u"가나다"sv));

		buf[0] = 0;
		EXPECT_TRUE(     0 == tszcat(buf, u"가나다라마바사"sv));
		EXPECT_TRUE(   buf == u"가나다라마바사"sv);
		EXPECT_TRUE(     0 == tszcat(buf, u"ABCDEFG"sv));
		EXPECT_TRUE(   buf == u"가나다라마바사ABCDEFG"sv);

		buf[0] = 0;
		EXPECT_TRUE(     0 == tszcat(buf, u"가나다라마바사"s));
		EXPECT_TRUE(   buf == u"가나다라마바사"sv);
		EXPECT_TRUE(     0 == tszcat(buf, u"ABCDEFG"s));
		EXPECT_TRUE(   buf == u"가나다라마바사ABCDEFG"sv);
	}

}

TEST(gtl_string, tszrmchar) {
	EXPECT_TRUE(     0 == tszrmchar((char*)nullptr, (char*)nullptr, '1'));

	{
		char buf[32]{"AhBhCDEFGh0123h4"};
		EXPECT_TRUE("ABCDEFG01234"sv.size() == tszrmchar(buf, buf+std::size(buf), 'h'));
		EXPECT_TRUE("ABCDEFG01234"sv        == buf);
		EXPECT_TRUE("ABCDEFG01234"sv.size() == tszrmchar(buf, buf+std::size(buf), 'Z'));
	}
	{
		char buf[32]{"AhBhCDEFGh0123h4"};
		EXPECT_TRUE("ABCDEFG01234"sv.size() == tszrmchar(buf, 'h'));
		EXPECT_TRUE("ABCDEFG01234"sv        == buf);
	}
	{
		std::array<char, 32> buf{"AhBhCDEFGh0123h4"};
		EXPECT_TRUE("ABCDEFG01234"sv.size() == tszrmchar(buf, 'h'));
		EXPECT_TRUE("ABCDEFG01234"sv        == buf.data());
	}
	{
		std::vector<char> buf;
		buf.resize(32); 
		tszcpy(buf, "AhBhCDEFGh0123h4"sv);
		EXPECT_TRUE("ABCDEFG01234"sv.size() == tszrmchar(buf, 'h'));
		EXPECT_TRUE("ABCDEFG01234"sv        == buf.data());
	}

#pragma warning(push)
#pragma warning(disable:4566)
	{
		char16_t buf[32]{u"가나다ABC나0나12345나6789❤나💕나😁😉"};
		EXPECT_TRUE(u"가다ABC0123456789❤💕😁😉"sv.size() == tszrmchar(buf, buf+std::size(buf), u'나'));
		EXPECT_TRUE(u"가다ABC0123456789❤💕😁😉"sv == buf);
	}

	{
		char32_t buf[32]{U"가나다❤ABC0❤12345❤6789❤💕😁😉"};
		EXPECT_TRUE(U"가나다ABC0123456789💕😁😉"sv.size() == tszrmchar(buf, U'❤'));
		EXPECT_TRUE(U"가나다ABC0123456789💕😁😉"sv == buf);
	}
#pragma warning(pop)
}


TEST(gtl_string, tszcmp) {

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	constexpr auto e = tszcmp<char>(nullptr, nullptr);
	static_assert(e == 0);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	constexpr auto e2 = tszcmp("abcdef", "abcdefa");
	static_assert(e2 < 0);

	static_assert(tszcmp("abcdef"sv, "abcdef"sv) == 0);
	static_assert(tszncmp("abcdef"sv, "abcdef"sv, 4) == 0);
	//static_assert(tszicmp("ABCDEF"sv, "abcdef"sv) == 0);
	//static_assert(tsznicmp("ABCDEF"sv, "abcdef"sv, 4) == 0);

	//char ptr[] = "abcdef";
	//constexpr auto e3 = tszcmp(ptr, "abcdefa");
	//static_assert(e3 < 0);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszcmp<char>(nullptr, nullptr) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszcmp<char>(nullptr, "") == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszcmp<char>("", nullptr) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszcmp("", "") == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszcmp("abcdef", "abcdefa") < 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszcmp("abcdefa", "abcdef") > 0);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp<char>(nullptr, nullptr) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp<char>(nullptr, "") == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp<char>("", nullptr) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("", "") == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("aBCdef", "abcdEFa") < 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("abcDefa", "aBCDef") > 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("abCdef", "AbCdefA") < 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("aBcDEFa", "abcdef") > 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("aBCdef", "abcdEF") == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("abcDef", "aBCDef") == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("abCdef", "AbCdef") == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszicmp("aBcDEF", "abcdef") == 0);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp<char>(nullptr, nullptr, 0) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp<char>(nullptr, "", 0) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp<char>("", nullptr, 0) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp<char>(nullptr, nullptr, 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp<char>(nullptr, "", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp<char>("", nullptr, 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp("", "", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp("abcdef", "abcdefa", 10) < 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp("abcdefa", "abcdef", 10) > 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp("", "", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp("abcdef", "abcdefa", 4) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tszncmp("abcdefa", "abcdef", 4) == 0);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp<char>(nullptr, nullptr, 0) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp<char>(nullptr, "", 0) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp<char>("", nullptr, 0) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp<char>(nullptr, nullptr, 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp<char>(nullptr, "", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp<char>("", nullptr, 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("", "", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcdef", "abcdefa", 10) < 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcdefa", "abcdef", 10) > 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("", "", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcdef", "abcdefa", 4) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcdefa", "abcdef", 4) == 0);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBCdef", "abcdEFa", 10) < 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcDefa", "aBCDef", 10) > 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abCdef", "AbCdefA", 10) < 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBcDEFa", "abcdef", 10) > 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBCdef", "abcdEF", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcDef", "aBCDef", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abCdef", "AbCdef", 10) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBcDEF", "abcdef", 10) == 0);

	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBCdef", "abcdEFa", 4) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcDefa", "aBCDef", 4) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abCdef", "AbCdefA", 4) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBcDEFa", "abcdef", 4) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBCdef", "abcdEF", 3) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abcDef", "aBCDef", 3) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("abCdef", "AbCdef", 3) == 0);
	SUPPRESS_DEPRECATED_WARNING													// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(tsznicmp("aBcDEF", "abcdef", 3) == 0);




	EXPECT_TRUE(tszcmp(std::string_view{nullptr, 0}, std::string_view{nullptr, 0}) == 0);
	EXPECT_TRUE(tszcmp({nullptr, 0}, ""sv) == 0);
	EXPECT_TRUE(tszcmp(""sv, {nullptr, 0}) == 0);
	EXPECT_TRUE(tszcmp(""sv, ""sv) == 0);
	EXPECT_TRUE(tszcmp("abcdef"sv, "abcdefa"sv) < 0);
	EXPECT_TRUE(tszcmp("abcdefa"sv, "abcdef"sv) > 0);

	EXPECT_TRUE(tszicmp(std::string_view{nullptr, 0}, {nullptr, 0}) == 0);
	EXPECT_TRUE(tszicmp({nullptr, 0}, ""sv) == 0);
	EXPECT_TRUE(tszicmp(""sv, {nullptr, 0}) == 0);
	EXPECT_TRUE(tszicmp(""sv, ""sv) == 0);
	EXPECT_TRUE(tszicmp("aBCdef"sv, "abcdEFa"sv) < 0);
	EXPECT_TRUE(tszicmp("abcDefa"sv, "aBCDef"sv) > 0);
	EXPECT_TRUE(tszicmp("abCdef"sv, "AbCdefA"sv) < 0);
	EXPECT_TRUE(tszicmp("aBcDEFa"sv, "abcdef"sv) > 0);
	EXPECT_TRUE(tszicmp("aBCdef"sv, "abcdEF"sv) == 0);
	EXPECT_TRUE(tszicmp("abcDef"sv, "aBCDef"sv) == 0);
	EXPECT_TRUE(tszicmp("abCdef"sv, "AbCdef"sv) == 0);
	EXPECT_TRUE(tszicmp("aBcDEF"sv, "abcdef"sv) == 0);

	EXPECT_TRUE(tszncmp<char>({nullptr, 0}, {nullptr, 0}, 0) == 0);
	EXPECT_TRUE(tszncmp({nullptr, 0}, ""sv, 0) == 0);
	EXPECT_TRUE(tszncmp(""sv, {nullptr, 0}, 0) == 0);
	EXPECT_TRUE(tszncmp<char>({nullptr, 0}, {nullptr, 0}, 10) == 0);
	EXPECT_TRUE(tszncmp({nullptr, 0}, ""sv, 10) == 0);
	EXPECT_TRUE(tszncmp(""sv, {nullptr, 0}, 10) == 0);
	EXPECT_TRUE(tszncmp(""sv, ""sv, 10) == 0);
	EXPECT_TRUE(tszncmp("abcdef"sv, "abcdefa"sv, 10) < 0);
	EXPECT_TRUE(tszncmp("abcdefa"sv, "abcdef"sv, 10) > 0);
	EXPECT_TRUE(tszncmp(""sv, ""sv, 10) == 0);
	EXPECT_TRUE(tszncmp("abcdef"sv, "abcdefa"sv, 4) == 0);
	EXPECT_TRUE(tszncmp("abcdefa"sv, "abcdef"sv, 4) == 0);

	EXPECT_TRUE(tsznicmp<char>({nullptr, 0}, {nullptr, 0}, 0) == 0);
	EXPECT_TRUE(tsznicmp({nullptr, 0}, ""sv, 0) == 0);
	EXPECT_TRUE(tsznicmp(""sv, {nullptr, 0}, 0) == 0);
	EXPECT_TRUE(tsznicmp<char>({nullptr, 0}, {nullptr, 0}, 10) == 0);
	EXPECT_TRUE(tsznicmp({nullptr, 0}, ""sv, 10) == 0);
	EXPECT_TRUE(tsznicmp(""sv, {nullptr, 0}, 10) == 0);
	EXPECT_TRUE(tsznicmp(""sv, ""sv, 10) == 0);
	EXPECT_TRUE(tsznicmp("abcdef"sv, "abcdefa"sv, 10) < 0);
	EXPECT_TRUE(tsznicmp("abcdefa"sv, "abcdef"sv, 10) > 0);
	EXPECT_TRUE(tsznicmp(""sv, ""sv, 10) == 0);
	EXPECT_TRUE(tsznicmp("abcdef"sv, "abcdefa"sv, 4) == 0);
	EXPECT_TRUE(tsznicmp("abcdefa"sv, "abcdef"sv, 4) == 0);

	EXPECT_TRUE(tsznicmp("aBCdef"sv, "abcdEFa"sv, 10) < 0);
	EXPECT_TRUE(tsznicmp("abcDefa"sv, "aBCDef"sv, 10) > 0);
	EXPECT_TRUE(tsznicmp("abCdef"sv, "AbCdefA"sv, 10) < 0);
	EXPECT_TRUE(tsznicmp("aBcDEFa"sv, "abcdef"sv, 10) > 0);
	EXPECT_TRUE(tsznicmp("aBCdef"sv, "abcdEF"sv, 10) == 0);
	EXPECT_TRUE(tsznicmp("abcDef"sv, "aBCDef"sv, 10) == 0);
	EXPECT_TRUE(tsznicmp("abCdef"sv, "AbCdef"sv, 10) == 0);
	EXPECT_TRUE(tsznicmp("aBcDEF"sv, "abcdef"sv, 10) == 0);

	EXPECT_TRUE(tsznicmp("aBCdef"sv, "abcdEFa"sv, 4) == 0);
	EXPECT_TRUE(tsznicmp("abcDefa"sv, "aBCDef"sv, 4) == 0);
	EXPECT_TRUE(tsznicmp("abCdef"sv, "AbCdefA"sv, 4) == 0);
	EXPECT_TRUE(tsznicmp("aBcDEFa"sv, "abcdef"sv, 4) == 0);
	EXPECT_TRUE(tsznicmp("aBCdef"sv, "abcdEF"sv, 3) == 0);
	EXPECT_TRUE(tsznicmp("abcDef"sv, "aBCDef"sv, 3) == 0);
	EXPECT_TRUE(tsznicmp("abCdef"sv, "AbCdef"sv, 3) == 0);
	EXPECT_TRUE(tsznicmp("aBcDEF"sv, "abcdef"sv, 3) == 0);
}


TEST(gtl_string, tszupr_lwr) {

	char16_t str[64] { u"ABCDEFG abcdefg 012345678 가나다라마바사"};
	tszupr(str);
	EXPECT_TRUE(str == u"ABCDEFG ABCDEFG 012345678 가나다라마바사"sv);
	tszlwr(str);
	EXPECT_TRUE(str == u"abcdefg abcdefg 012345678 가나다라마바사"sv);

	EXPECT_TRUE(EINVAL == tszupr((char*)nullptr, 0));
	char16_t* ptr = str;
	EXPECT_TRUE(ERANGE == tszupr(ptr, -1));
}

TEST(gtl_string, tszto) {

	auto ExpectEQ = []<typename T1, typename T2 = T1>(T1 && a, T2 && b) {
		EXPECT_EQ(std::forward<T1>(a), std::forward<T2>(b));
	};


	char16_t sz[30];
	char16_t szS[]{u"가나다라마바사"};
	tszcpy(sz, std::basic_string_view{szS});
	static_assert(std::size(szS) == 7+1);

	EXPECT_TRUE(CompareStringContainingNumbers(u""sv, u""sv) == 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"0"sv, u""sv) > 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u""sv, u"0"sv) < 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"1"sv, u"1"sv) == 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"1"sv, u"2"sv) < 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"2"sv, u"1"sv) > 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"01"sv, u"001"sv) < 0);	// 같을 경우, 길이가 길수록 큰 값
	EXPECT_TRUE(CompareStringContainingNumbers(u"10"sv, u"100"sv) < 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"abcdef0123456789aaaa"sv, u"abcdef0000123456789aaaa"sv) < 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"abcdef0123456789aaaa"sv, u"abcdef000012345678aaaa"sv) > 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"abcdef0123456789aaaa"sv, u"abcdef00001234567891aaaa"sv) < 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"abcdef0123456789aaaa"sv, u"abcdef0000123456788aaaa"sv) > 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"0123456789"sv, u"0123456789"sv) == 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"0123456789"sv, u"012345678"sv) > 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"0123456789"sv, u"01234567891"sv) < 0);
	EXPECT_TRUE(CompareStringContainingNumbers(u"0123456789"sv, u"0123456788"sv) > 0);

	auto a1 = gtl::tsztoi("12345"sv);
	auto a2 = gtl::tsztoi("12345"s);
	auto a3 = gtl::tsztoi("12345");
	CString str;
	str = "12345";
	auto a4 = gtl::tsztoi(CString("12345"sv));
	//auto aa = overload{
	//	[]() { std::cout << "()" << std::endl; },
	//	[](int) { std::cout << "(int)" << std::endl; },
	//	[](float) { std::cout << "(float)" << std::endl; },
	//};

	char* psz{};
	auto v0 = gtl::tsztoi<int>("123456"sv, (char**)nullptr);
	//auto v1 = gtl::tsztoi<int>(std::string_view("123456"s), nullptr, 0, 0);
	//auto v2 = gtl::tsztoi((std::string_view)std::basic_string<char>("123456"), nullptr, 0, 0);
	//EXPECT_EQ(123456, v2);
	//EXPECT_EQ(123456, gtl::tsztoi("123456"s));
	//EXPECT_EQ(123456, gtl::tsztoi("123456"sv));
	//EXPECT_EQ(123456, gtl::tsztoi(gtl::TString<char>("123456")));
	//EXPECT_EQ(123456, gtl::tsztoi((std::string_view)gtl::TString<char>("123456")));
	//EXPECT_EQ(0xABCD'EFab'1234'9853I64, gtl::tsztoi<int64_t>("0xABCD'EFab'1234'9853"sv, (char**)nullptr, 0, '\''));
	//EXPECT_EQ(0xABCDI64, gtl::tsztoi("0xABCD'EFab'1234'9853"sv));

	//EXPECT_EQ(1.1, (gtl::tsztod<double, char>("1.1")));
	//EXPECT_EQ(1.1, (gtl::tsztod<double, char>("1.1"s)));
	//EXPECT_EQ(1.1, (gtl::tsztod<double, char>("1.1"sv)));
	//EXPECT_EQ(1.1, (gtl::tsztod<double, char>(gtl::CStringA("1.1"))));
	//EXPECT_EQ(1.1, (gtl::tsztod<double, char>(gtl::CStringA("1.1"))));
	//EXPECT_EQ(1.1, gtl::tsztod((std::string_view)"1.1"));
	//EXPECT_EQ(1.1, gtl::tsztod((std::string_view)"1.1"s));
	//EXPECT_EQ(+1.1, gtl::tsztod("+1.1"sv));
	//EXPECT_EQ(1.3e3, gtl::tsztod("1.3e3"sv));
	//EXPECT_EQ(-1.1, gtl::tsztod("-1.1"sv));
	//EXPECT_EQ(-1.3e3, gtl::tsztod("-1.3e3"sv));
	//EXPECT_EQ(1.3e-3, gtl::tsztod("1.3e-3"sv));
	//EXPECT_EQ(-1.3e-3, gtl::tsztod("-1.3e-3"sv));

	std::regex rx(R"(^([+-]?(?:[[:d:]]+\.?|[[:d:]]*\.[[:d:]]+))(?:[Ee][+-]?[[:d:]]+)?$)");
	std::regex reg{ R"x(^([+\-]?(?:[[:d:]]+\.?|[[:d:]]*\.[[:d:]]+))(?:[Ee][+\-]?[[:d:]]+)?$)x" };
	//std::regex_starts_with
	//static constexpr auto pattern = ctll::fixed_string{ R"()" };
	//static constexpr auto pattern = ctll::fixed_string{ R"x(^[\-+]?[0-9]*\.?[0-9]+([eE][\-+]?[0-9]+)?$)x" };
	//auto str_regex = ctre::starts_with<pattern>("-0.32840e9agdrgai");



}


TEST(gtl_string, etc) {

	char16_t buf[64] {u"0123456789"};
	tszrev(buf);

	EXPECT_TRUE(buf == u"9876543210"sv);

}
