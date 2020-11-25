//////////////////////////////////////////////////////////////////////
//
// string_primitives.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__BASIC_STRING
#define GTL_HEADER__BASIC_STRING

#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"

#if !defined(__cpp_lib_concepts)
#	error ERROR! Supports C++v20 only.
#endif

//-----------------------------------------------------------------------------
// Text Constant macros
#define _A(x) x
#define ___WIDE_TEXT(x) L##x
#define _W(x) ___WIDE_TEXT(x)
#define ___UTF8_TEXT(x) u8##x
#define _u8(x) ___UTF8_TEXT(x)
#define ___UTF16_TEXT(x) u##x
#define _u(x) ___UTF16_TEXT(x)
#define ___UTF32_TEXT(x) U##x
#define _U(x) ___UTF32_TEXT(x)

#define _EOL			"\r\n"
#define EOL				_t(_EOL)
#define EOLA			_A(_EOL)
#define EOLW			_W(_EOL)
#define EOLu8			_u8(_EOL)
#define SPACE_STRING	" \t\r\n"


namespace gtl {
#pragma pack(push, 8)

#define GTL_DEPR_SEC [[deprecated("NOT Secure")]]

	// todo : hpp 쪽으로 implementation 분리.
	// todo : string primitives argument 에 string_view 추가

	//-----------------------------------------------------------------------------
	/// @brief pre-defines : basic_string_t
	template < gtlc::string_elem tchar, class ttraits = std::char_traits<tchar>, class tallocator = std::allocator<tchar> >
	using basic_string_t = std::basic_string<tchar, ttraits, tallocator>;

	//-----------------------------------------------------------------------------
	/// @brief  misc. GetSpaceString()
	/// @return " \r\t\n"
	template < gtlc::string_elem tchar > [[nodiscard]] constexpr tchar const* GetSpaceString();
	constexpr inline [[nodiscard]] char const*		GetSpaceStringA()				{ return GetSpaceString<char>(); }
	constexpr inline [[nodiscard]] char8_t const*	GetSpaceStringU8()				{ return GetSpaceString<char8_t>(); }
	constexpr inline [[nodiscard]] char16_t const*	GetSpaceStringU16()				{ return GetSpaceString<char16_t>(); }
	constexpr inline [[nodiscard]] char32_t const*	GetSpaceStringU32()				{ return GetSpaceString<char32_t>(); }
	constexpr inline [[nodiscard]] wchar_t const*	GetSpaceStringW()				{ return GetSpaceString<wchar_t>(); }

	constexpr static inline [[nodiscard]] bool is_space(int const c);


	//-----------------------------------------------------------------------------
	/// @brief TrimLeft, TrimRight, Trim
	/// @tparam tchar 
	/// @param str : string
	/// @param pszTrim : chars to trim
	template < gtlc::string_elem tchar > void TrimRight(basic_string_t<tchar>& str, const tchar* pszTrim);
	template < gtlc::string_elem tchar > void TrimLeft(basic_string_t<tchar>& str, const tchar* pszTrim);
	template < gtlc::string_elem tchar > void Trim(basic_string_t<tchar>& str, const tchar* pszTrim);
	template < gtlc::string_elem tchar > void TrimRight(basic_string_t<tchar>& str, const tchar cTrim);
	template < gtlc::string_elem tchar > void TrimLeft(basic_string_t<tchar>& str, const tchar cTrim);
	template < gtlc::string_elem tchar > void Trim(basic_string_t<tchar>& str, const tchar cTrim);
	template < gtlc::string_elem tchar > void TrimLeft(basic_string_t<tchar>& str);
	template < gtlc::string_elem tchar > void TrimRight(basic_string_t<tchar>& str);
	template < gtlc::string_elem tchar > void Trim(basic_string_t<tchar>& str);


	/// @brief ToLower, ToUpper, ToDigit, IsSpace ... (locale irrelavant)
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToLower(tchar c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToUpper(tchar c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsDigit(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsOdigit(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsXdigit(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsSpace(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsNotSpace(tchar const c/* Locale Irrelavant */);


	/// @brief  tszlen (string length). you cannot input string literal. you don't need tszlen("testlen");. ==> just call "testlen"sv.size();
	/// @param psz : null terminating char_t* var. (no string literals)
	/// @return string length
	/// @param pszMax : (end+1) of the allocated buffer.
	/// @return string length. if not reached, SIZE_MAX.
	template < gtlc::string_elem tchar >					constexpr		 [[nodiscard]] GTL_DEPR_SEC size_t tszlen(tchar const*const& psz);
	template < gtlc::string_elem tchar >					constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, tchar const* const pszMax);
	template < gtlc::string_elem tchar >					constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, size_t sizeOfBuf);
	template < gtlc::string_container_fixed tcontainer >	constexpr inline [[nodiscard]] size_t tszlen(tcontainer const& v);


	// todo : documents...

	/// <summary>
	/// tszcpy : utf8/16/32/wchar_t ver. of strcpy_s
	/// </summary>
	/// <param name="pszDest"></param>
	/// <param name="sizeDest"></param>
	/// <param name="pszSrc"></param>
	/// <returns>
	/// 0 : ok.
	/// EINVAL : if !pszDest || !pszSrc
	/// ERANGE : if sizeDest is smaller
	/// </returns>
	template < gtlc::string_elem tchar >
	constexpr		 GTL_DEPR_SEC errno_t tszcpy(tchar* const& pszDest, size_t sizeDest, tchar const*const& pszSrc);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline GTL_DEPR_SEC errno_t tszcpy(tcontainer& szDest, tchar const* const pszSrc);


	/// <summary>
	/// tszcpy : utf8/16/32/wchar_t ver. of strcpy_s
	/// </summary>
	/// <param name="pszDest">dest. buffer</param>
	/// <param name="sizeDest">size of dest. including null terminating char.</param>
	/// <param name="szDest">
	/// dest.
	///  ex) char szDest[30];</param>
	///      std::array<char16_t> szDest;</param>
	///      std::vector<char> szDest;</param>
	/// <param name="svSrc">source string. do not need to be NULL terminated string</param>
	/// <param name="strSrc">source string.</param>
	/// <returns>
	/// 0 : ok.
	/// EINVAL : if !pszDest
	/// ERANGE : if sizeDest is not enough.
	/// </returns>
	template < gtlc::string_elem tchar >
	constexpr errno_t tszcpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc);

	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline errno_t tszcpy(tcontainer& szDest, std::basic_string_view<tchar> svSrc);

	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline errno_t tszcpy(tcontainer& szDest, std::basic_string<tchar> const& strSrc);

	// tszncpy
	template < gtlc::string_elem tchar >
	constexpr			GTL_DEPR_SEC errno_t tszncpy(tchar*const& pszDest, size_t sizeDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline	GTL_DEPR_SEC errno_t tszncpy(tcontainer& szDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);

	// tszncpy (sv)
	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszncpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline errno_t tszncpy(tcontainer &szDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline errno_t tszncpy(tcontainer &szDest, std::basic_string<tchar> const& svSrc, size_t nCount = _TRUNCATE);

	// tszcat
	template < gtlc::string_elem tchar >
	constexpr		 GTL_DEPR_SEC errno_t tszcat(tchar* pszDest, size_t sizeDest, tchar const* pszSrc);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline GTL_DEPR_SEC errno_t tszcat(tcontainer& szDest, tchar const* pszSrc);

	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszcat(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline errno_t tszcat(tcontainer &szDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline errno_t tszcat(tcontainer &szDest, std::basic_string<tchar> const& strSrc);


	/// @brief Remove Charactors from str.
	/// @param psz 
	/// @param chRemove 
	/// @return str length
	template < gtlc::string_elem tchar >
	constexpr GTL_DEPR_SEC	size_t tszrmchar(tchar* const& psz, tchar chRemove);
	template < gtlc::string_elem tchar >
	constexpr size_t tszrmchar(tchar* const psz, tchar const* const pszMax, tchar chRemove);
	template < gtlc::string_elem tchar, gtlc::string_container_fixed_c<tchar> tcontainer >
	constexpr inline size_t tszrmchar(tcontainer &sz, tchar chRemove);


	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] GTL_DEPR_SEC int tszcmp(tchar const* pszA, tchar const* pszB);

	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] int tszncmp(tchar const* pszA, tchar const* pszB, size_t nCount);

	template < gtlc::string_elem tchar >
	int tszicmp(tchar const* pszA, tchar const* pszB);

	template < gtlc::string_elem tchar >
	int tsznicmp(tchar const* pszA, tchar const* pszB, size_t nCount);

	template < gtlc::string_elem tchar >					errno_t tszupr(tchar* sz, size_t nCount);
	template < gtlc::string_elem tchar, int sizeBuffer >	errno_t tszupr(tchar (&sz)[sizeBuffer]);
	template < gtlc::string_elem tchar, int sizeBuffer >	errno_t tszupr(std::array<tchar, sizeBuffer>& sz);

	template < gtlc::string_elem tchar >					errno_t tszlwr(tchar* sz, size_t nCount);
	template < gtlc::string_elem tchar, int sizeBuffer >	errno_t tszlwr(tchar(&sz)[sizeBuffer]);
	template < gtlc::string_elem tchar, int sizeBuffer >	errno_t tszlwr(std::array<tchar, sizeBuffer>& sz);

	template < gtlc::string_elem tchar > [[nodiscard]] tchar* tszrev(tchar* psz);
	template < gtlc::string_elem tchar > [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar c);
	template < gtlc::string_elem tchar > [[nodiscard]] tchar* tszsearch(tchar* psz, tchar c);
	template < gtlc::string_elem tchar > [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* pszSub);
	template < gtlc::string_elem tchar > [[nodiscard]] tchar* tszsearch(tchar* psz, tchar* pszSub);


	/// <summary>
	/// digit contants to integral type value.
	///  - radix detecting (c++ notation)
	///	   ex "0b1001'1100", "0xABCD1234", "0b1234568"
	///  - digit seperator (such as ',' or '\'' for c++ notation.)
	///  - only throws when (radix > 36)
	/// </summary>
	/// <param name="psz"></param>
	/// <param name="pszEnd"></param>
	/// <param name="ppszStopped">where conversion stopped</param>
	/// <param name="radix">radix</param>
	/// <param name="cSplitter">digit splitter. such as ',' (thousand sepperator) or '\'' (like c++v14 notation)</param>
	/// <returns>number value. (no overflow checked)</returns>
	template < std::integral tvalue = int, gtlc::string_elem tchar >			[[nodiscard]]		 tvalue tsztoi(std::basic_string_view<tchar> svNumberString, tchar** ppszStopped = nullptr, int radix = 0, int cSplitter = 0);
	template < std::integral tvalue = int, gtlc::string_elem tchar >			[[nodiscard]] inline tvalue tsztoi(std::basic_string<tchar> const& str, tchar** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);
	template < std::integral tvalue = int, gtlc::string_elem tchar >			[[nodiscard]] inline tvalue tsztoi(tchar const*& psz, tchar** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);
	template < std::integral tvalue = int, gtlc::string_elem tchar, int size >	[[nodiscard]] inline tvalue tsztoi(tchar const (&sz)[size], tchar** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);

	template < std::floating_point tvalue = double, gtlc::string_elem tchar = char16_t >
	[[deprecated("NOT STANDARD CONVERTING !")]] constexpr tvalue _tsztod(tchar const* psz, tchar const* pszEnd, tchar** ppszStopped = nullptr, tchar cSplitter = 0);

	template < std::floating_point tvalue = double, gtlc::string_elem tchar > inline [[nodiscard]] tvalue tsztod(std::basic_string_view<tchar> sv, tchar** ppszStopped = nullptr);
	template < std::floating_point tvalue = double, gtlc::string_elem tchar > inline [[nodiscard]] tvalue tsztod(std::basic_string<tchar> const& str, tchar** ppszStopped = nullptr);
	template < std::floating_point tvalue = double, gtlc::string_elem tchar > inline [[nodiscard]] tvalue tsztod(tchar const* psz, tchar** ppszStopped = nullptr);


	/// @brief Compare strings (containing numbers)
	template < gtlc::string_elem tchar, bool bIgnoreCase >
	constexpr int CompareStringContainingNumbers(const tchar* pszA, const tchar* pszB);

	template < gtlc::string_elem tchar >
	constexpr inline int tdszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar >
	constexpr inline int tdszcmp(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB);
	template < gtlc::string_elem tchar >
	constexpr inline int tdszicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar >
	constexpr inline int tdszicmp(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB);



	/// @brief  tszsearch_oneof
	/// @param psz    : string
	/// @param pszSet : chars to find
	/// @return position of found char.
	template < gtlc::string_elem tchar >
	tchar* tszsearch_oneof(tchar* psz, tchar const * const pszSet);
	template < gtlc::string_elem tchar >
	tchar const* tszsearch_oneof(tchar const* psz, tchar const* const pszSet);


	/// @brief tszto_number
	/// @param psz 
	/// @param pszEnd 
	/// @param radix 
	/// @return number
	template < gtlc::arithmetic tvalue, gtlc::string_elem tchar>
	tvalue tszto(const tchar* psz, tchar const* pszEnd, tchar** pszStopped = nullptr, int radix = 0, int cSplitter = 0);


	/// @brief Compare two Strings. according to number (only for '0'-'9' are taken as number. no '-' sign, neither '.' for floating point 
	///  ex)
	///      "123" > "65"         // 123 > 65
	///      "abc123" > "abc6"    // 123 > 65 ("abc" == "abc")
	///      "-100" > "-125"      // '-' is just taken as part of string. ===> 100 > 125
	///      "00001" < "0000003"  // 1 < 3
	///      "01" < "001"         // if same (1 == 1) ===> longer gets winner.
	/// @param pszA
	/// @param pszB
	/// @return 
	template < gtlc::string_elem tchar, bool bIgnoreCase = false >
	constexpr int/*std::strong_ordering*/ CompareStringContainingNumbers(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar >
	constexpr int CompareStringContainingNumbers(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB, bool bIgnoreCase);
	//template < gtlc::string_elem tchar, bool bIgnoreCase = false >
	//constexpr inline int/*std::strong_ordering*/ CompareStringContainingNumbers(tchar const* pszA, tchar const* pszB) {
	//	return CompareStringContainingNumbers<tchar, bIgnoreCase>({pszA, pszA+tszlen(pszA)}, {pszB, pszB+tszlen(pszB)});
	//}

#pragma pack(pop)
};	// namespace gtl;

#include "string_primitives.hpp"


#endif	// GTL_HEADER__BASIC_STRING
