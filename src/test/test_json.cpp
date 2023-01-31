#include "pch.h"

#include "gtl/gtl.h"
#include "gtl/concepts.h"
#include "gtl/json_sync.h"

#if (GTL__BOOST_JSON__AS_STANDALONE)
#include "boost/json/src.hpp"   // impl
//namespace boost {
//	void throw_exception(std::exception const&) {
//	}
//}
#endif

using namespace std::literals;

namespace gtl {
	template <typename tchar_t>
	class TStringProxy {
		std::basic_string_view<char> m_view;
	public:
		TStringProxy(tchar_t const* psz) {}
		TStringProxy(std::basic_string<tchar_t> const& str) {}
		TStringProxy(std::basic_string_view<tchar_t> sv) {}
	};

	// additional deduction guide
	//template < typename tchar_t >
	//TStringProxy(std::basic_string<tchar_t> const& str) -> TStringProxy<tchar_t>;

	template < typename tchar_t >
	void TestFunction(TStringProxy<tchar_t> str) {

	}
	
	template < typename tchar_t >
	void TestFunction2(std::basic_string_view<tchar_t> sv) {
	}


	template < gtlc::tstring_like tstr_t >
	void TestFunction3(tstr_t const& str, tstr_t& str2) {
		using tchar_t = typename tstr_t::value_type;
		tchar_t aa{};
		//static_assert(std::is_same_v<tchar_t, wchar_t>);
	}

	template < gtlc::tstring_like tstr_t, gtlc::tstring_like tstr2 >
	void TestFunction4(tstr_t const& str, tstr2 const& str2) {
		using tchar_t = typename tstr_t::value_type;
		tchar_t aa{};
		static_assert(std::is_same_v<typename tstr_t::value_type, typename tstr2::value_type>);
	}

	template < gtlc::tstring_like tstr_t, gtlc::tstring_like tstr2 >
	tstr_t TestFunction5(tstr_t const& str, tstr2 const& str2) {
		using tchar_t = typename tstr_t::value_type;
		tchar_t aa{};
		//static_assert(std::is_same_v<tchar_t, wchar_t>);
		return str;
	}

	TEST(json_sync, basic) {
		std::wstring wstr(L"abasdfjklds"s);
		std::wstring_view wsv(L"abasdfjklds"sv);
		std::string str("abasdfjklds"s);
		std::string_view sv("abasdfjklds"sv);
		TestFunction(TStringProxy(wstr));
		TestFunction2(L"sdfsdf"sv);
		TestFunction3(wstr, wstr);
		TestFunction3(str, str);
		TestFunction3(sv, sv);
		TestFunction4(wstr, wsv);
		TestFunction4(L"Sdfsdf"s, L"Sdfasdfkjlsd"s);
		TestFunction4(str, sv);

		auto s = TestFunction5(str, sv);

	}

}
