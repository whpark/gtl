//////////////////////////////////////////////////////////////////////
//
// old_format.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "basic_string.h"


//-----------------------------------------------------------------------------
// Format
//
namespace gtl::old_format {
	//-----------------------------------------------------------------------------
	// Format()
	// 참고 : https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
	//

	template < gtlc::arithmetic T_NUMBER >
	constexpr const char* GetDefaultFormatSpecifierA(T_NUMBER = 0) {
		if constexpr (std::is_integral_v<T_NUMBER>) {
			if constexpr (std::is_signed_v<T_NUMBER>) {
				if constexpr (sizeof(T_NUMBER) == sizeof(std::int64_t)) {
					return "%I64d";
				} else {
					return "%d";
				}
			} else {
				if constexpr (sizeof(T_NUMBER) == sizeof(std::uint64_t)) {
					return "%I64u";
				} else {
					return "%u";
				}
			}
		} else {
			return "%g";
		}
	}

	template < gtlc::arithmetic T_NUMBER >
	constexpr const wchar_t* GetDefaultFormatSpecifierW(T_NUMBER = 0) {
		if constexpr (std::is_integral_v<T_NUMBER>) {
			if constexpr (std::is_signed_v<T_NUMBER>) {
				if constexpr (sizeof(T_NUMBER) == sizeof(std::int64_t)) {
					return L"%I64d";
				} else {
					return L"%d";
				}
			} else {
				if constexpr (sizeof(T_NUMBER) == sizeof(std::uint64_t)) {
					return L"%I64u";
				} else {
					return L"%u";
				}
			}
		} else {
			return L"%g";
		}
	}

	template < gtlc::string_elem tchar_t, gtlc::arithmetic T_NUMBER >
	constexpr const tchar_t* GetDefaultFormatSpecifier(T_NUMBER) {
		if constexpr (std::is_same_v<tchar_t, char> || std::is_same_v<tchar_t, char8_t>) {
			return GetDefaultFormatSpecifierA<T_NUMBER>();
		} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
			return GetDefaultFormatSpecifierW<T_NUMBER>();
		} else {
			static_assert(false, "char or wchar_t only.");
		}
	}

	template < typename ... Args >
	int tsz_snprintf(char* const _Buffer, size_t const _BufferCount, char const* const _Format, Args&& ... args) {
		return std::snprintf(_Buffer, _BufferCount, _Format, args ...);
	}
	template < typename ... Args >
	int tsz_snprintf(wchar_t* const _Buffer, size_t const _BufferCount, wchar_t const* const _Format, Args&& ... args) {
		//#pragma warning(suppress:4996)	// 아래 한 줄만 에러 막기.
		return _snwprintf(_Buffer, _BufferCount, _Format, args ...);
	}
	template < typename ... Args >
	int tsz_snprintf(char8_t* const _Buffer, size_t const _BufferCount, char8_t const* const _Format, Args&& ... args) {
		return std::snprintf((char*)_Buffer, _BufferCount, (char*)_Format, args ...);
	}

#if 0	// no need
	inline int tsz_vsnprintf(char *buffer, size_t _Count, const char *format, va_list argptr) {
		return ::vsnprintf(buffer, _Count, format, argptr);
	}
	inline int tsz_vsnprintf(wchar_t *buffer, size_t _Count, const wchar_t *format, va_list argptr) {
	#pragma warning(suppress:4996)	// 아래 한 줄만 에러 막기.
		return ::_vsnwprintf(buffer, _Count, format, argptr);
	}
	inline int tsz_vsnprintf(char8_t *buffer, size_t _Count, const char8_t *format, va_list argptr) {
		return ::vsnprintf((char*)buffer, _Count, (char*)format, argptr);
	}

	template < gtlc::string_elem tchar_t >
	basic_string_t<tchar_t> TFormatV(const tchar_t* pszFormat, va_list argList) {
		basic_string_t<tchar_t> str;
		int len = tsz_vsnprintf(nullptr, 0, pszFormat, argList);
		if (len < 0)
			throw std::invalid_argument("formatting error");
		if (len) {
			//str.reserve(len+1);	// for NULL-Terminating char... VS2017 still uses it.
			str.resize(len);
			tsz_vsnprintf(str.data(), str.size(), pszFormat, argList);
		}
		return str;
	}
#endif

	template < gtlc::string_elem tchar_t, typename ... Args/*, ENABLE_IF(std::is_integral_v<tchar_t>)*/ >
	basic_string_t<tchar_t> TFormat(const tchar_t* pszFormat, Args&& ... args) {
		basic_string_t<tchar_t> str;
		int len = tsz_snprintf(nullptr, 0, pszFormat, args ...);
		if (len < 0)
			throw std::invalid_argument("formatting error");
		if (len) {
			//str.reserve(len+1);	// for NULL-Terminating char... VS2017 still uses it.
			str.resize(len);
			tsz_snprintf(str.data(), str.size()+1, pszFormat, args ... );
		}
		return str;
	}


	template < gtlc::string_elem tchar_t, typename tchar2_t = tchar_t >
	const tchar2_t* FilterStringTypeToCSTR(const gtl::TString<tchar2_t>& str) {
		static_assert(std::is_same_v<tchar_t, tchar2_t>, "Check String Type...");
		return str.c_str();
	}
	template < gtlc::string_elem tchar_t, typename tchar2_t = tchar_t >
	const tchar2_t* FilterStringTypeToCSTR(const basic_string_t<tchar2_t>& str) {
		static_assert(std::is_same_v<tchar_t, tchar2_t>, "Check String Type...");
		return str.c_str();
	}

	template < gtlc::string_elem tchar_t > class TStrView;
	template < gtlc::string_elem tchar_t, typename tchar2_t = tchar_t >
	const tchar2_t* FilterStringTypeToCSTR(const TStrView<tchar2_t>& str);

	template < gtlc::string_elem tchar_t > class tstr_view;
	template < gtlc::string_elem tchar_t, typename tchar2_t = tchar_t >
	const tchar2_t* FilterStringTypeToCSTR(const tstr_view<tchar2_t>& str);

	template < gtlc::string_elem tchar_t, typename TYPE >
	const TYPE& FilterStringTypeToCSTR(const TYPE& v) {
		return v; 
	}

	template < gtlc::string_elem tchar_t, typename ... Args >
	basic_string_t<tchar_t> TFFormat(const tchar_t* pszFormat, Args&& ... args) {
		return TFormat(pszFormat, FilterStringTypeToCSTR<tchar_t>(args)...);
	}

	// for Wrong Paramters

	template < typename ... Args > [[nodiscard]] basic_string_t<char>		format(const char* pszFormat, Args&& ... args)		{ return TFFormat(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] basic_string_t<wchar_t>	format(const wchar_t* pszFormat, Args&& ... args)	{ return TFFormat(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] basic_string_t<char8_t>	format(const char8_t* pszFormat, Args&& ... args)	{ return TFFormat(pszFormat, args ...); }
	template < typename ... Args > [[nodiscard]] TString<char>				Format(const char* pszFormat, Args&& ... args)		{ return TString<char>(TFFormat(pszFormat, args ...)); }
	template < typename ... Args > [[nodiscard]] TString<wchar_t>			Format(const wchar_t* pszFormat, Args&& ... args)	{ return TString<wchar_t>(TFFormat(pszFormat, args ...)); }
	template < typename ... Args > [[nodiscard]] TString<char8_t>			Format(const char8_t* pszFormat, Args&& ... args)	{ return TString<char8_t>(TFFormat(pszFormat, args ...)); }

}

