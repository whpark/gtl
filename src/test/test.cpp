#include "pch.h"

#include "gtl/gtl.h"

using namespace std::literals;
using namespace gtl::literals;

namespace {
	using namespace std;

	template<class... Ts>
	struct overload : Ts... {
		using Ts::operator()...;
	};
	template<class... Ts>
	overload(Ts...) -> overload<Ts...>;

};

TEST(gtl_string, tszto) {

	auto ExpectEQ = []<typename T1, typename T2 = T1>(T1 && a, T2 && b) {
		EXPECT_EQ(std::forward<T1>(a), std::forward<T2>(b));
	};

	using namespace gtl;

	EXPECT_EQ(7, gtl::tszlen("dsafdjk"));
	EXPECT_EQ(3, gtl::tszlen(U"가나다"));
	EXPECT_EQ(4, gtl::tszlen(u"가나다라"));
	EXPECT_EQ(4, gtl::tszlen(L"가나다라"));
	EXPECT_EQ(8, gtl::tszlen("가나다라"));

	char16_t sz[30];
	char16_t szS[]{u"가나다라마바사"};
	tszcpy(sz, szS);

	EXPECT_TRUE(CompareStringIncludingNumber(u"", u"") == 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"0", u"") > 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"", u"0") < 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"1", u"1") == 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"1", u"2") < 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"2", u"1") > 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"01", u"001") < 0);	// 같을 경우, 길이가 길수록 큰 값
	EXPECT_TRUE(CompareStringIncludingNumber(u"10", u"100") < 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"abcdef0123456789aaaa", u"abcdef0000123456789aaaa") == 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"abcdef0123456789aaaa", u"abcdef000012345678aaaa") > 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"abcdef0123456789aaaa", u"abcdef00001234567891aaaa") < 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"abcdef0123456789aaaa", u"abcdef0000123456788aaaa") > 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"0123456789", u"0123456789") == 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"0123456789", u"012345678") > 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"0123456789", u"01234567891") < 0);
	EXPECT_TRUE(CompareStringIncludingNumber(u"0123456789", u"0123456788") > 0);

	auto a1 = gtl::tsztoi("12345"sv);
	auto a2 = gtl::tsztoi("12345"s);
	auto a3 = gtl::tsztoi("12345");
	auto a4 = gtl::tsztoi(CString("12345"));
	EXPECT_TRUE(ToStringU16("가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍") == u"가나다라마바사아자차카타파하긎긣꿳뎓뫓멙뻍");

	std::string str;
	gtl::CStringA str2;
	std::string_view sv { str2 };

	ToString<char>("sas"s, str);

	auto aa = overload{
		[]() { std::cout << "()" << std::endl; },
		[](int) { std::cout << "(int)" << std::endl; },
		[](float) { std::cout << "(float)" << std::endl; },
	};

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
	static constexpr auto pattern = ctll::fixed_string{ R"x(^[\-+]?[0-9]*\.?[0-9]+([eE][\-+]?[0-9]+)?$)x" };
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