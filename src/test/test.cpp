#include "pch.h"

#include "gtl/gtl.h"

using namespace std::literals;
using namespace gtl::literals;

using namespace gtl;

namespace {
	using namespace std;

	template<class... Ts>
	struct overload : Ts... {
		using Ts::operator()...;
	};
	template<class... Ts>
	overload(Ts...) -> overload<Ts...>;

};

#if 0
template < typename tchar >				void TestFuncAsConst(tchar const* psz, size_t size) {}

template < typename tchar, int size >	void TestFuncAsConst(tchar const (&sz)[size]) {}
template < typename tchar, int size >	void TestFuncAsConst(std::array<tchar, size> const& sz) {}
template < typename tchar >				[[deprecated("NotSecure")]]void TestFuncAsConst(tchar const*const& psz) {}
template < typename tchar >				void TestFuncAsConst(std::basic_string_view<tchar> sv) {}
template < typename tchar >				void TestFuncAsConst(std::basic_string<tchar> const& str) {}
char const* SomeFunc() { return nullptr; }

template < typename tchar, int size >	void TestFuncAsNonConst(tchar (&sz)[size]) {}
template < typename tchar, int size >	void TestFuncAsNonConst(std::array<tchar, size>& sz) {}
template < typename tchar >				[[deprecated("NotSecure")]]void TestFuncAsNonConst(tchar*const& psz) {}
//template < typename tchar >				void TestFuncAsNonConst(std::basic_string_view<tchar> sv) {}
template < typename tchar >				void TestFuncAsNonConst(std::basic_string<tchar>& str) {}

void TestFuncVar() {
	if constexpr (false) {
		TestFuncAsConst("abc");

		char buf[100];
		TestFuncAsConst(buf);

		std::array<char, 32> bufa;
		TestFuncAsConst(bufa);

		char const* psz1 = "asdf";
		TestFuncAsConst(psz1);

		TestFuncAsConst(SomeFunc());

		constexpr char const * psz2 = "asdf";
		TestFuncAsConst(psz2);


		TestFuncAsConst("abc"sv);
		TestFuncAsConst("abc"s);
		std::string str {"abcdef"};
		TestFuncAsConst(str);
	}

	if constexpr (false) {
		TestFuncAsNonConst("abc");

		char buf[100];
		TestFuncAsNonConst(buf);

		std::array<char, 32> bufa;
		TestFuncAsNonConst(bufa);

		char const* psz1 = "asdf";
		TestFuncAsNonConst(psz1);

		TestFuncAsNonConst(SomeFunc());

		constexpr char const * psz2 = "asdf";
		TestFuncAsNonConst(psz2);


		//TestFuncAsNonConst("abc"sv);
		//TestFuncAsNonConst("abc"s);
		std::string str {"abcdef"};
		TestFuncAsNonConst(str);
	}
}
#endif

namespace template_instantiation {

	template < typename T, typename T2 >
	concept addable =
	requires (T2 a) { a+a; a*a;};

	template < typename T >
	void func() {
		if constexpr (addable<int, T>) {
			static_assert(true);
		}
		else {
			static_assert(false);
		}
	}

	template <> void func<int>() {
	}
	template void func<float>();
	//template void func<std::string_view>();

};

TEST(gtl_string, tszlen) {
	{
		char buf0[100];
		std::array<char, 10> buf1;
		std::vector<char> buf2;

		auto const* v0 = data(buf0);
		auto const* v1 = data(buf1);
		auto const* v2 = data(buf2);
		auto s1 = std::size(buf1);
		auto s2 = std::size(buf2);
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
	constexpr auto l0 = tszlen(sz3);	// will generate compiler warning ("NOT Secure")
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
	EXPECT_EQ(7, tszlen(szConstArray));

}

TEST(gtl_string, tszcpy) {

	// tszcpy char*
	{
		std::vector<char> buf;
		buf.resize(10);
		EXPECT_TRUE(EINVAL == tszcpy((char*)nullptr, buf.size(), "ABCDEF"sv));
		EXPECT_TRUE(EINVAL == tszcpy(buf.data(), 0, "ABCDEF"sv));
		EXPECT_TRUE(EINVAL == tszcpy(buf.data(), buf.size(), (char*)nullptr));	// will generate compiler warning ("NOT Secure")
		auto const* psz1 = "ABCDEF";
		EXPECT_TRUE(     0 == tszcpy(buf.data(), buf.size(), psz1));			// will generate compiler warning ("NOT Secure")
		auto const* pszLong = "long string................";
		EXPECT_TRUE(ERANGE == tszcpy(buf.data(), buf.size(), pszLong));			// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(     0 == tszlen(buf.data(), buf.size()));

		EXPECT_TRUE(     0 == tszcpy(buf, psz1));								// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(buf.data() == "ABCDEF"sv);

		std::array<char, 32> buf2;
		EXPECT_TRUE(     0 == tszcpy(buf2, psz1));								// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(buf2.data() == "ABCDEF"sv);

		char buf3[32]{};
		EXPECT_TRUE(     0 == tszcpy(buf3, psz1));								// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(buf3 == "ABCDEF"sv);

		EXPECT_TRUE(ERANGE == tszcpy(buf, pszLong));							// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(     0 == tszlen(buf));
	}

	// tszcpy from string_view
	{
		constexpr bool b = gtlc::string_container_fixed_c<std::basic_string_view<char>, char>;

		char16_t buf[32] {};
		EXPECT_TRUE(     0 == tszcpy(buf, size(buf), u"가나다라마바사"sv));
		EXPECT_TRUE(     0 == tszcpy(buf, u"가나다라마바사"sv));
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
		EXPECT_TRUE(     0 == tszcpy(buf, u"가나다라마바사"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"s);
	}
}

TEST(gtl_string, tszncpy) {
	std::vector<char16_t> buf;
	EXPECT_TRUE(EINVAL == tszncpy(buf.data(), buf.size(), u"가나다라마바사", _TRUNCATE));	// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(EINVAL == tszncpy(buf.data(), buf.size(), u"가나다라마바사", 4));			// will generate compiler warning ("NOT Secure")
	buf.resize(10);
	EXPECT_TRUE(ERANGE == tszncpy(buf.data(), buf.size(), u"가나다라마바사", 10));			// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(EINVAL == tszncpy(buf.data(),          0, u"가나다라마바사", 2));			// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == tszncpy(buf.data(), buf.size(), u"가나다라마바사", 9));			// will generate compiler warning ("NOT Secure")

	buf.resize(10);
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사", _TRUNCATE));	// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다라마바사", 8*sizeof(buf[0])));

	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다", _TRUNCATE));			// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다", 4*sizeof(buf[0])));

	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다", 5));					// will generate compiler warning ("NOT Secure")
	EXPECT_TRUE(     0 == memcmp(buf.data(), u"가나다", 3*sizeof(buf[0])));

	buf.resize(5);
	EXPECT_TRUE(     0 == tszncpy(buf, u"가나다라마바사", 4));			// will generate compiler warning ("NOT Secure")
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
		EXPECT_TRUE(EINVAL == tszcat(buf.data(), 1, u"가나다라마바사"));						// will generate compiler warning ("NOT Secure")
		EXPECT_TRUE(EINVAL == tszcat(buf, (char16_t*)nullptr));								// will generate compiler warning ("NOT Secure")
		buf.resize(32);
		EXPECT_TRUE(EINVAL == tszcat(buf.data(), -1, u"가나다"));								// will generate compiler warning ("NOT Secure")

		EXPECT_TRUE(     0 == tszcat(buf, u"가나다라마바사"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(     0 == tszcat(buf, u"ABCDEFG"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사ABCDEFG"sv);

		buf[0] = 0;
		EXPECT_TRUE(     0 == tszcat(buf, u"가나다라마바사"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(     0 == tszcat(buf, u"ABCDEFG"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사ABCDEFG"sv);
	}
	{
		std::array<char16_t, 32> buf;
		EXPECT_TRUE(ERANGE == tszcat(buf.data(), 1, u"가나다라마바사"sv));
		EXPECT_TRUE(EINVAL == tszcat(buf.data(), -1, u"가나다"sv));

		buf[0] = 0;
		EXPECT_TRUE(     0 == tszcat(buf, u"가나다라마바사"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(     0 == tszcat(buf, u"ABCDEFG"sv));
		EXPECT_TRUE(buf.data() == u"가나다라마바사ABCDEFG"sv);

		buf[0] = 0;
		EXPECT_TRUE(     0 == tszcat(buf, u"가나다라마바사"s));
		EXPECT_TRUE(buf.data() == u"가나다라마바사"sv);
		EXPECT_TRUE(     0 == tszcat(buf, u"ABCDEFG"s));
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
		EXPECT_TRUE(12 == tszrmchar(buf, buf+std::size(buf), 'h'));
		EXPECT_TRUE(buf == "ABCDEFG01234"sv);
		EXPECT_TRUE(12 == tszrmchar(buf, buf+std::size(buf), 'Z'));
	}
	{
		char buf[32]{"AhBhCDEFGh0123h4"};
		EXPECT_TRUE(12 == tszrmchar(buf, 'h'));
		EXPECT_TRUE(buf == "ABCDEFG01234"sv);
	}
	{
		std::array<char, 32> buf{"AhBhCDEFGh0123h4"};
		EXPECT_TRUE(12 == tszrmchar(buf, 'h'));
		EXPECT_TRUE(buf.data() == "ABCDEFG01234"sv);
	}
	{
		std::vector<char> buf;
		buf.resize(32);
		tszcpy(buf, "AhBhCDEFGh0123h4"sv);
		EXPECT_TRUE(12 == tszrmchar(buf, 'h'));
		EXPECT_TRUE(buf.data() == "ABCDEFG01234"sv);
	}

	{
		char16_t buf[32]{u"가나다ABC나0나12345나6789❤나💕나😁😉"};
		EXPECT_TRUE(22 == tszrmchar(buf, buf+std::size(buf), u'나'));
		EXPECT_TRUE(buf == u"가다ABC0123456789❤💕😁😉"sv);
	}

	{
		char32_t buf[32]{U"가나다❤ABC0❤12345❤6789❤💕😁😉"};
		EXPECT_TRUE(19 == tszrmchar(buf, U'❤'));
		EXPECT_TRUE(buf == U"가나다ABC0123456789💕😁😉"sv);
	}

}

TEST(gtl_string, tszto) {

	auto ExpectEQ = []<typename T1, typename T2 = T1>(T1 && a, T2 && b) {
		EXPECT_EQ(std::forward<T1>(a), std::forward<T2>(b));
	};


	char16_t sz[30];
	char16_t szS[]{u"가나다라마바사"};
	tszcpy(sz, std::basic_string_view{szS, szS+std::size(szS)});
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
	auto a4 = gtl::tsztoi(CString("12345"));
	EXPECT_TRUE(ToStringU16("가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍") == u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍");

	std::string str;
	gtl::CStringA str2;
	std::string_view sv { str2 };

	ToString<char>("sas"s, str);

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


//template <typename X> struct Identity {
//	using type = X;
//};
//template <typename T> struct Corner1 {
//	Corner1(typename Identity<T>::type, int) { }
//};
//Corner1(double, int)->Corner1<double>;
//
//void main043435() {
//	Corner1 corner1(3.14, 1729);
//}
//
//template <typename T> struct Corner2 {
//	Corner2(T, long) { }
//	Corner2(typename Identity<T>::type, unsigned long) { }
//};
//Corner2(double, unsigned long)->Corner2<double>;
//void main21212() {
//	Corner2 corner2(3.14, 1729ul);
//}