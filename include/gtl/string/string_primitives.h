//////////////////////////////////////////////////////////////////////
//
// string_primitives.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL__HEADER__BASIC_STRING
#define GTL__HEADER__BASIC_STRING

#include "gtl/_lib_gtl.h"
#include "gtl/_default.h"
#include "gtl/concepts.h"

#if !defined(__cpp_lib_concepts)
#	error ERROR! Supports C++v20 only.
#endif

//-----------------------------------------------------------------------------
// Text Constant macros
#define TEXT_A(x) x
#define ___WIDE_TEXT(x) L##x
#define TEXT_W(x) ___WIDE_TEXT(x)
#define ___UTF8_TEXT(x) u8##x
#define TEXT_u8(x) ___UTF8_TEXT(x)
#define ___UTF16_TEXT(x) u##x
#define TEXT_u(x) ___UTF16_TEXT(x)
#define ___UTF32_TEXT(x) U##x
#define TEXT_U(x) ___UTF32_TEXT(x)

#define _EOL			"\r\n"
#define EOL				TEXT_A(_EOL)
#define EOLA			TEXT_A(_EOL)
#define EOLW			TEXT_W(_EOL)
#define EOLu8			TEXT_u8(_EOL)
#define EOLu			TEXT_u(_EOL)
#define EOLU			TEXT_U(_EOL)
#define SPACE_STRING	" \t\r\n"


namespace gtl {
#pragma pack(push, 8)


	//-----------------------------------------------------------------------------
	/// @brief just char -> tchar_t (this function does not change sv's values.)
	template < typename tcharTarget, typename tcharSource >
	constexpr std::basic_string<tcharTarget> ElevateAnsiToStandard(std::basic_string_view<tcharSource> sv) {
		std::basic_string<tcharTarget> result;
		result.reserve(sv.size());
		for (auto c : sv)
			result += static_cast<tcharTarget>(c);
		return result;
	}


	/// @brief ToLower, ToUpper
	template < gtlc::string_elem tchar > [[nodiscard]] tchar ToLower(tchar c);
	template < gtlc::string_elem tchar > [[nodiscard]] tchar ToUpper(tchar c);
	template < gtlc::string_elem tchar >               void MakeLower(tchar& c);
	template < gtlc::string_elem tchar >               void MakeUpper(tchar& c);
	template < gtlc::string_elem tchar > [[nodiscard]] std::basic_string<tchar> ToLower(std::basic_string_view<tchar> sv);
	template < gtlc::string_elem tchar > [[nodiscard]] std::basic_string<tchar> ToUpper(std::basic_string_view<tchar> sv);
	template < gtlc::string_elem tchar >               void MakeLower(std::basic_string<tchar>& str);
	template < gtlc::string_elem tchar >               void MakeUpper(std::basic_string<tchar>& str);


	/// @brief  tszlen (string length). you cannot input string literal. you don't need tszlen("testlen");. ==> just call "testlen"sv.size();
	/// @param psz : null terminating char_t* var. (no string literals)
	/// @return string length
	/// @param pszMax : (end+1) of the allocated buffer.
	/// @return string length. if not reached, SIZE_MAX.
	template < gtlc::string_elem tchar >						constexpr [[nodiscard]] GTL__DEPR_SEC size_t tszlen(tchar const*const& psz);
	template < gtlc::string_elem tchar >						constexpr [[nodiscard]] size_t tszlen(tchar const* psz, tchar const* const pszMax);
	template < gtlc::string_elem tchar >						constexpr [[nodiscard]] size_t tszlen(tchar const* psz, size_t sizeOfBuf);
	template < gtlc::contiguous_string_container tstring_buf >	constexpr [[nodiscard]] size_t tszlen(tstring_buf const& v);

	// todo : documents...

	/// @brief utf8/16/32/wchar_t ver. of strcpy_s
	/// @param pszDest 
	/// @param sizeDest 
	/// @param pszSrc 
	/// @return 0 : ok.
	///  EINVAL : if !pszDest || !pszSrc
	///  ERANGE : if sizeDest is smaller
	template < gtlc::string_elem tchar >
	constexpr GTL__DEPR_SEC errno_t tszcpy(tchar* const& pszDest, size_t sizeDest, tchar const*const& pszSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr GTL__DEPR_SEC errno_t tszcpy(tstring_buf& szDest, tchar const* const& pszSrc);


	/// @brief tszcpy utf8/16/32/wchar_t ver. of strcpy_s
	/// @param pszDest dest. buffer
	/// @param sizeDest size of dest. including null terminating char.</param>
	/// @param szDest dest. string buffer. 
	///  ex) char szDest[30];</param>
	///      std::array<char16_t> szDest;</param>
	///      std::vector<char> szDest;</param>
	/// @param svSrc source string. do not need to be NULL terminated string</param>
	/// @param strSrc null terminated source string.
	/// @return 
	///  0 : ok.
	///  EINVAL : if !pszDest
	///  ERANGE : if sizeDest is not enough.
	template < gtlc::string_elem tchar >
	constexpr errno_t tszcpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc);

	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr errno_t tszcpy(tstring_buf& szDest, std::basic_string_view<tchar> svSrc);

	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr errno_t tszcpy(tstring_buf& szDest, std::basic_string<tchar> const& strSrc);

	// tszncpy
	template < gtlc::string_elem tchar >
	constexpr errno_t tszncpy(tchar*const& pszDest, size_t sizeDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr errno_t tszncpy(tstring_buf& szDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);

	// tszncpy (sv)
	template < gtlc::string_elem tchar >
	constexpr errno_t tszncpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr errno_t tszncpy(tstring_buf &szDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr errno_t tszncpy(tstring_buf &szDest, std::basic_string<tchar> const& svSrc, size_t nCount = _TRUNCATE);

	// tszcat
	template < gtlc::string_elem tchar >
	constexpr GTL__DEPR_SEC errno_t tszcat(tchar* pszDest, size_t sizeDest, tchar const* pszSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr GTL__DEPR_SEC errno_t tszcat(tstring_buf& szDest, tchar const* pszSrc);

	template < gtlc::string_elem tchar >
	constexpr errno_t tszcat(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr errno_t tszcat(tstring_buf &szDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr errno_t tszcat(tstring_buf &szDest, std::basic_string<tchar> const& strSrc);


	/// @brief Remove Charactors from str.
	/// @param psz 
	/// @param chRemove 
	/// @return str length
	template < gtlc::string_elem tchar >
	constexpr GTL__DEPR_SEC	size_t tszrmchar(tchar* const& psz, tchar chRemove);
	template < gtlc::string_elem tchar >
	constexpr size_t tszrmchar(tchar* const psz, tchar const* const pszMax, tchar chRemove);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr size_t tszrmchar(tstring_buf &sz, tchar chRemove);


	template < gtlc::string_elem tchar > constexpr GTL__DEPR_SEC [[nodiscard]] int tszcmp(tchar const* pszA, tchar const* pszB);
	template < gtlc::string_elem tchar > constexpr GTL__DEPR_SEC [[nodiscard]] int tszncmp(tchar const* pszA, tchar const* pszB, size_t nCount);
	template < gtlc::string_elem tchar > constexpr GTL__DEPR_SEC [[nodiscard]] int tszicmp(tchar const* pszA, tchar const* pszB);
	template < gtlc::string_elem tchar > constexpr GTL__DEPR_SEC [[nodiscard]] int tsznicmp(tchar const* pszA, tchar const* pszB, size_t nCount);

	template < gtlc::string_elem tchar > constexpr [[nodiscard]] int tszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] int tszncmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] int tszicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] int tsznicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount);

	template < gtlc::contiguous_string_container tstring_buf >	errno_t tszupr(tstring_buf& buf);
	template < gtlc::string_elem tchar >			 	errno_t tszupr(tchar* const psz, size_t size);
	template < gtlc::contiguous_string_container tstring_buf >	errno_t tszlwr(tstring_buf& buf);
	template < gtlc::string_elem tchar >			 	errno_t tszlwr(tchar* const& psz, size_t size);


	//template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar* tszrev(tchar* psz);
	template < gtlc::string_elem tchar > 			  [[nodiscard]] tchar* tszrev(tchar* psz, tchar const* const pszEnd);
	template < gtlc::contiguous_string_container tstring_buf > std::remove_cvref_t<decltype(tstring_buf{}[0])>* tszrev(tstring_buf& buf);

#if 0     // use string_view, or stl
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar c);
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar* tszsearch(tchar* psz, tchar c);
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* pszSub);
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar* tszsearch(tchar* psz, tchar* pszSub);

	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar c);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar c);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar const* pszSub, tchar const* const pszSubEnd);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar* pszSub, tchar const* const pszSubEnd);


	/// @brief  tszsearch_oneof
	/// @param psz    : string
	/// @param pszSet : chars to find
	/// @return position of found char.
	template < gtlc::string_elem tchar >
	GTL__DEPR_SEC tchar* tszsearch_oneof(tchar* psz, tchar const * const pszSet);
	template < gtlc::string_elem tchar >
	GTL__DEPR_SEC tchar const* tszsearch_oneof(tchar const* psz, tchar const* const pszSet);

	template < gtlc::string_elem tchar >
	constexpr tchar* tszsearch_oneof(tchar* psz, tchar* pszEnd, tchar const * const pszSet, tchar const* const pszSetEnd);
	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(tchar const* psz, tchar const* pszEnd, tchar const * const pszSet, tchar const* const pszSetEnd);
	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svSet);
#endif

#pragma pack(pop)
};	// namespace gtl;

//#include "string_primitives.hpp"


#endif	// GTL__HEADER__BASIC_STRING
