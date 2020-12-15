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


#if 0	// NOT available. Template Argument Deduction for Function Argument is NOT, YET!
	template < gtlc::string_elem tchar >
		requires (std::is_same_v<std::remove_const_t<tchar>, tchar>)
	class tstr_buf {
	public:
		tchar* const psz_;
		//[[no_unique_address]]std::enable_if_t<bSized, tchar const* const> pszEnd_;
		tchar const* const pszEnd_;
		bool const bSized_;

	public:
		tstr_buf() = delete;
		tstr_buf(tstr_buf const&) = default;
		tstr_buf(tstr_buf &&) = default;
		tstr_buf& operator = (tstr_buf const&) = default;
		tstr_buf& operator = (tstr_buf &&) = default;

		constexpr tstr_buf(tchar* const& psz) : psz_(psz), pszEnd_(nullptr), bSized_(false) {}

		constexpr tstr_buf(tchar* const& psz, size_t size) : psz_(psz), pszEnd_(), bSized_(false) {}

		template < size_t size >
		constexpr tstr_buf(tchar(&sz)[size]) : psz_(sz), pszEnd_(sz+size), bSized_(true) {} 

		template < typename tstring_buf >
		requires (gtlc::string_buffer_fixed_c<tstring_buf, tchar>
			and !std::is_convertible_v<tstring_buf, std::basic_string<tchar>>
			and !std::is_convertible_v<tstring_buf, std::basic_string_view<tchar>>)
		constexpr tstr_buf(tstring_buf const& sv) : psz_(std::data(sv)), pszEnd_(std::data(sv)+std::size(sv)), bSized_(true) {}

	};

	template < gtlc::string_elem tchar >
	class tstr_view {
	public:
		tchar const* const psz_;
		//[[no_unique_address]]std::enable_if_t<bSized, tchar const* const> pszEnd_;
		tchar const* const pszEnd_;
		bool const bSized_{};

	public:
		tstr_view() = delete;
		tstr_view(tstr_view const&) = default;
		tstr_view(tstr_view &&) = default;
		tstr_view& operator = (tstr_view const&) = default;
		tstr_view& operator = (tstr_view &&) = default;

		constexpr tstr_view(tchar const * const& psz) : psz_(psz), pszEnd_(nullptr), bSized_(false) {}

		constexpr tstr_view(tchar const * const& psz, size_t size) : psz_(psz), pszEnd_(psz_+size), bSized_(true) {}

		template < size_t size >
		constexpr tstr_view(tchar const (&sz)[size]) : psz_(sz), pszEnd_(sz+size), bSized_(true) {} 

		template < gtlc::string_buffer_fixed_c<tchar> tstring_buf >
		constexpr tstr_view(tstring_buf const& sv) : psz_(std::data(sv)), pszEnd_(std::data(sv)+std::size(sv)), bSized_(true) {}

	};

	template < typename tchar, size_t size >
	tstr_view(tchar const[size]) -> tstr_view<tchar>;
#endif

	//-----------------------------------------------------------------------------
	/// @brief  misc. GetSpaceString()
	/// @return " \r\t\n"
	template < gtlc::string_elem tchar > [[nodiscard]] constexpr std::basic_string_view<tchar> GetSpaceString();
	constexpr inline [[nodiscard]] std::basic_string_view<char>     GetSpaceStringA()				{ return GetSpaceString<char>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<char8_t>  GetSpaceStringU8()				{ return GetSpaceString<char8_t>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<char16_t> GetSpaceStringU16()				{ return GetSpaceString<char16_t>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<char32_t> GetSpaceStringU32()				{ return GetSpaceString<char32_t>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<wchar_t>  GetSpaceStringW()				{ return GetSpaceString<wchar_t>(); }

	constexpr static inline [[nodiscard]] bool is_space(int const c);


	//-----------------------------------------------------------------------------
	/// @brief TrimLeft, TrimRight, Trim
	/// @tparam tchar 
	/// @param str : string
	/// @param pszTrim : chars to trim
	template < gtlc::string_elem tchar > void TrimRight(std::basic_string<tchar>& str,			std::basic_string_view<tchar> svTrim);
	template < gtlc::string_elem tchar > void TrimLeft(std::basic_string<tchar>& str,			std::basic_string_view<tchar> svTrim);
	template < gtlc::string_elem tchar > void Trim(std::basic_string<tchar>& str,				std::basic_string_view<tchar> svTrim);
	template < gtlc::string_elem tchar > void TrimRight(std::basic_string<tchar>& str,			tchar const cTrim);
	template < gtlc::string_elem tchar > void TrimLeft(std::basic_string<tchar>& str,			tchar const cTrim);
	template < gtlc::string_elem tchar > void Trim(std::basic_string<tchar>& str,				tchar const cTrim);
	template < gtlc::string_elem tchar > void TrimLeft(std::basic_string<tchar>& str);
	template < gtlc::string_elem tchar > void TrimRight(std::basic_string<tchar>& str);
	template < gtlc::string_elem tchar > void Trim(std::basic_string<tchar>& str);


	//-----------------------------------------------------------------------------
	/// @brief TrimLeftView, TrimRightView, TrimView
	/// @tparam tchar 
	/// @param str : string
	/// @param pszTrim : chars to trim
	/// @return Trimmed string_view
	template < gtlc::string_elem tchar > [[nodiscard]]        std::basic_string_view<tchar> TrimRightView(std::basic_string_view<tchar> sv,	std::basic_string_view<tchar> svTrim);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimLeftView(std::basic_string_view<tchar> sv,	std::basic_string_view<tchar> svTrim);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimView(std::basic_string_view<tchar> sv,		std::basic_string_view<tchar> svTrim);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimRightView(std::basic_string_view<tchar> sv,	tchar const cTrim);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimLeftView(std::basic_string_view<tchar> sv,	tchar const cTrim);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimView(std::basic_string_view<tchar> sv,		tchar const cTrim);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimLeftView(std::basic_string_view<tchar> sv);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimRightView(std::basic_string_view<tchar> sv);
	template < gtlc::string_elem tchar > [[nodiscard]] inline std::basic_string_view<tchar> TrimView(std::basic_string_view<tchar> sv);


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
	template < gtlc::string_elem tchar >				constexpr		 [[nodiscard]] GTL_DEPR_SEC size_t tszlen(tchar const*const& psz);
	template < gtlc::string_elem tchar >				constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, tchar const* const pszMax);
	template < gtlc::string_elem tchar >				constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, size_t sizeOfBuf);
	template < gtlc::contiguous_string_container tstring_buf >	constexpr inline [[nodiscard]] size_t tszlen(tstring_buf const& v);

	// todo : documents...

	/// @brief utf8/16/32/wchar_t ver. of strcpy_s
	/// @param pszDest 
	/// @param sizeDest 
	/// @param pszSrc 
	/// @return 0 : ok.
	///  EINVAL : if !pszDest || !pszSrc
	///  ERANGE : if sizeDest is smaller
	template < gtlc::string_elem tchar >
	constexpr		 GTL_DEPR_SEC errno_t tszcpy(tchar* const& pszDest, size_t sizeDest, tchar const*const& pszSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline GTL_DEPR_SEC errno_t tszcpy(tstring_buf& szDest, tchar const* const& pszSrc);


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
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string_view<tchar> svSrc);

	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string<tchar> const& strSrc);

	// tszncpy
	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszncpy(tchar*const& pszDest, size_t sizeDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf& szDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);

	// tszncpy (sv)
	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszncpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string<tchar> const& svSrc, size_t nCount = _TRUNCATE);

	// tszcat
	template < gtlc::string_elem tchar >
	constexpr		 GTL_DEPR_SEC errno_t tszcat(tchar* pszDest, size_t sizeDest, tchar const* pszSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline GTL_DEPR_SEC errno_t tszcat(tstring_buf& szDest, tchar const* pszSrc);

	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszcat(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string<tchar> const& strSrc);


	/// @brief Remove Charactors from str.
	/// @param psz 
	/// @param chRemove 
	/// @return str length
	template < gtlc::string_elem tchar >
	constexpr GTL_DEPR_SEC	size_t tszrmchar(tchar* const& psz, tchar chRemove);
	template < gtlc::string_elem tchar >
	constexpr size_t tszrmchar(tchar* const psz, tchar const* const pszMax, tchar chRemove);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline size_t tszrmchar(tstring_buf &sz, tchar chRemove);


	template < gtlc::string_elem tchar > constexpr inline GTL_DEPR_SEC [[nodiscard]] int tszcmp(tchar const* pszA, tchar const* pszB);
	template < gtlc::string_elem tchar > constexpr inline GTL_DEPR_SEC [[nodiscard]] int tszncmp(tchar const* pszA, tchar const* pszB, size_t nCount);
	template < gtlc::string_elem tchar > constexpr inline GTL_DEPR_SEC [[nodiscard]] int tszicmp(tchar const* pszA, tchar const* pszB);
	template < gtlc::string_elem tchar > constexpr inline GTL_DEPR_SEC [[nodiscard]] int tsznicmp(tchar const* pszA, tchar const* pszB, size_t nCount);

	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tszncmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tszicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tsznicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount);

	template < gtlc::contiguous_string_container tstring_buf >	errno_t tszupr(tstring_buf& buf);
	template < gtlc::string_elem tchar >			 	errno_t tszupr(tchar* const psz, size_t size);
	template < gtlc::contiguous_string_container tstring_buf >	errno_t tszlwr(tstring_buf& buf);
	template < gtlc::string_elem tchar >			 	errno_t tszlwr(tchar* const& psz, size_t size);


	//template < gtlc::string_elem tchar > GTL_DEPR_SEC [[nodiscard]] tchar* tszrev(tchar* psz);
	template < gtlc::string_elem tchar > 			  [[nodiscard]] tchar* tszrev(tchar* psz, tchar const* const pszEnd);
	template < gtlc::contiguous_string_container tstring_buf > std::remove_cvref_t<decltype(tstring_buf{}[0])>* tszrev(tstring_buf& buf);

#if 0     // use string_view, or stl
	template < gtlc::string_elem tchar > GTL_DEPR_SEC [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar c);
	template < gtlc::string_elem tchar > GTL_DEPR_SEC [[nodiscard]] tchar* tszsearch(tchar* psz, tchar c);
	template < gtlc::string_elem tchar > GTL_DEPR_SEC [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* pszSub);
	template < gtlc::string_elem tchar > GTL_DEPR_SEC [[nodiscard]] tchar* tszsearch(tchar* psz, tchar* pszSub);

	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar c);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar c);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar const* pszSub, tchar const* const pszSubEnd);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar* pszSub, tchar const* const pszSubEnd);


	/// @brief  tszsearch_oneof
	/// @param psz    : string
	/// @param pszSet : chars to find
	/// @return position of found char.
	template < gtlc::string_elem tchar >
	GTL_DEPR_SEC tchar* tszsearch_oneof(tchar* psz, tchar const * const pszSet);
	template < gtlc::string_elem tchar >
	GTL_DEPR_SEC tchar const* tszsearch_oneof(tchar const* psz, tchar const* const pszSet);

	template < gtlc::string_elem tchar >
	constexpr tchar* tszsearch_oneof(tchar* psz, tchar* pszEnd, tchar const * const pszSet, tchar const* const pszSetEnd);
	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(tchar const* psz, tchar const* pszEnd, tchar const * const pszSet, tchar const* const pszSetEnd);
	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svSet);
#endif

	/// <summary>
	/// digit contants to integral type value.
	///  - radix detecting (c++ notation)
	///	   ex "0b1001'1100", "0xABCD1234", "0123456"
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
	template < std::integral tvalue = int, gtlc::string_elem tchar > GTL_DEPR_SEC [[nodiscard]] inline tvalue tsztoi(tchar const* const& psz, tchar** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);
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
