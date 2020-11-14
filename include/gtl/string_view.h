//////////////////////////////////////////////////////////////////////
//
// string_view.h
//
// PWH
// 2019.01.08.
// 2019.07.24. QL -> GTL
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING_VIEW
#define GTL_HEADER__STRING_VIEW

#include "gtl/_default.h"
#include "gtl/string.h"

namespace gtl {
#pragma pack(push, 8)

	//------------------------------------------------------------------------------------------
	// TStrView : std::string_view-like wrapper. but always zero-terminated-string
	//
	template < gtlc::string_elem tchar_t >
	class TStrView {
	public:
		using value_type = tchar_t;
		using size_type = intptr_t;
		using index_type = intptr_t;
		using gtl_tstring_t = std::bool_constant<true>;

	protected:
		const tchar_t* m_psz = nullptr;
		size_type m_size = 0;

	public:
		// Constructor
		TStrView() = default;
		TStrView(const TStrView&) = default;
		TStrView(TStrView&&) = default;
		TStrView& operator = (const TStrView&) = default;
		TStrView& operator = (TStrView&&) = default;

		constexpr TStrView(const tchar_t* psz) : m_psz(psz) {
			m_size = psz ? tszlen(psz) : 0;
		}
		TStrView(const std::basic_string_view<tchar_t>& sv) {
			m_psz = sv.data();
			m_size = sv.size();
		}
		TStrView(const t_basic_string<tchar_t>& str) {
			m_psz = str.c_str();
			m_size = str.size();
		}
		TStrView(const TString<tchar_t>& str) {
			m_psz = (const tchar_t*)str;
			m_size = str.GetLength();
		}

		//TStrView& operator = (const tchar_t* psz) {
		//	m_psz = psz;
		//	m_size = psz ? tszlen(psz) : 0;
		//}
		//TStrView& operator = (const std::t_basic_string_view<tchar_t>& sv) {
		//	m_psz = sv.data();
		//	m_size = sv.size();
		//}
		//TStrView& operator = (const t_basic_string<tchar_t>& str) {
		//	m_psz = str.c_str();
		//	m_size = str.size();
		//}
		//TStrView& operator = (const TString<tchar_t>& str) {
		//	m_psz = (const tchar_t*)str;
		//	m_size = str.GetLength();
		//}

	public:
		operator const tchar_t*() const { return m_psz; }
		const tchar_t* c_str() const { return m_psz; }
		const tchar_t* data() const { return m_psz; }

		size_type GetLength() const { return m_size; }
		size_t size() const { return (size_t)m_size; }

		value_type& operator [] (index_type pos) { return m_psz[pos]; }
		const value_type operator [] (index_type pos) const { return m_psz[pos]; }

		value_type& front() { return m_psz[0]; }
		const value_type front() const { return m_psz[0]; }
		value_type& back() { return m_psz[m_size-1]; }
		const value_type back() const { return m_psz[m_size-1]; }

		bool empty() const { return !m_psz || (m_size == 0); }

		void reset() {
			m_psz = nullptr;
			m_size = 0;
		}


	};	// TStrView

	template < typename tchar_t, typename tchar2_t = tchar_t >
	const tchar2_t* FilterStringTypeToCSTR(const TStrView<tchar2_t>& str) {
		static_assert(std::is_same_v<tchar_t, tchar2_t>, "Check String Type...");
		return str.c_str();
	}


	using XStrView = TStrView<TCHAR>;
	using XStrViewA = TStrView<char>;
	using XStrViewW = TStrView<wchar_t>;
	using XStrViewU8 = TStrView<char8_t>;


	//------------------------------------------------------------------------------------------
	// tstr_view : std::string_view wrapper
	//
	template < typename tchar_t >
	class tstr_view : public std::basic_string_view<tchar_t> {
	public:
		using base_t = std::basic_string_view<tchar_t>;
		using ql_tstring_t = std::bool_constant<true>;
	public:
		// Constructor
		using base_t::base_t;
		tstr_view() = default;
		tstr_view(const tstr_view&) = default;
		tstr_view(tstr_view&&) = default;
		tstr_view& operator = (const tstr_view&) = default;
		tstr_view& operator = (tstr_view&&) = default;

		constexpr tstr_view(const tchar_t* psz) : base_t(psz?psz:empty_string()) {
		}
		tstr_view(const t_basic_string<tchar_t>& str) : base_t(str) {
		}
		tstr_view(const TString<tchar_t>& str) : base_t((t_basic_string<tchar_t>&)str) {
		}
		tstr_view& operator = (const tchar_t* psz) {
			base_t::operator()(psz);
			return *this;
		}

		TString<tchar_t> ToString() const { return TString<tchar_t>(base_t::data(), base_t::data()+base_t::size()); }

		constexpr inline const tchar_t* empty_string() const {
			if constexpr (std::is_same_v<tchar_t, char>) { return ""; }
			else if constexpr (std::is_same_v<tchar_t, wchar_t>) { return L""; }
			else if constexpr (std::is_same_v<tchar_t, char8_t>) { return _U8(""); }
			else
				static_assert(false);
		}

	};	// tstr_view

	template < typename tchar_t, typename tchar2_t = tchar_t >
	const tchar2_t* FilterStringTypeToCSTR(const tstr_view<tchar2_t>& str) {
		static_assert(false, "Not Yet Implemented");
	}

	using str_view = tstr_view<TCHAR>;
	using str_viewA = tstr_view<char>;
	using str_viewW = tstr_view<wchar_t>;
	using str_viewU8 = tstr_view<char8_t>;

	template < typename ... Args > [[nodiscard]] t_basic_string<char>		format(str_viewA  pszFormat, Args ... args)	{ return internal::TFFormat<char>(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] t_basic_string<wchar_t>	format(str_viewW  pszFormat, Args ... args)	{ return internal::TFFormat<wchar_t>(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] t_basic_string<char8_t>	format(str_viewU8 pszFormat, Args ... args)	{ return internal::TFFormat<char8_t>(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] TString<char>				Format(str_viewA  pszFormat, Args ... args)	{ return TString<char>(internal::TFFormat<char>(pszFormat.data(), args ...)); }
	template < typename ... Args > [[nodiscard]] TString<wchar_t>			Format(str_viewW  pszFormat, Args ... args)	{ return TString<wchar_t>(internal::TFFormat<wchar_t>(pszFormat.data(), args ...)); }
	template < typename ... Args > [[nodiscard]] TString<char8_t>			Format(str_viewU8 pszFormat, Args ... args)	{ return TString<char8_t>(internal::TFFormat<char8_t>(pszFormat.data(), args ...)); }
	template < typename ... Args > [[nodiscard]] t_basic_string<char>		format(XStrViewA  pszFormat, Args ... args)	{ return internal::TFFormat<char>(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] t_basic_string<wchar_t>	format(XStrViewW  pszFormat, Args ... args)	{ return internal::TFFormat<wchar_t>(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] t_basic_string<char8_t>	format(XStrViewU8 pszFormat, Args ... args)	{ return internal::TFFormat<char8_t>(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] TString<char>				Format(XStrViewA  pszFormat, Args ... args)	{ return TString<char>(internal::TFFormat<char>(pszFormat, args ...)); }
	template < typename ... Args > [[nodiscard]] TString<wchar_t>			Format(XStrViewW  pszFormat, Args ... args)	{ return TString<wchar_t>(internal::TFFormat<wchar_t>(pszFormat, args ...)); }
	template < typename ... Args > [[nodiscard]] TString<char8_t>			Format(XStrViewU8 pszFormat, Args ... args)	{ return TString<char8_t>(internal::TFFormat<char8_t>(pszFormat, args ...)); }


#pragma pack(pop)
};	// namespace gtl;


//template < typename tchar_t >
//using std::is_convertible<tstr_view<tchar_t>, TString<tchar_t> >::value = true;

#endif // GTL_HEADER__STRING_VIEW
