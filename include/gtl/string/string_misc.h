//////////////////////////////////////////////////////////////////////
//
// string_misc.ixx:
//
// PWH
// 2020.12.16.
//
//////////////////////////////////////////////////////////////////////


#include "gtl/_lib_gtl.h"
#include "gtl/_default.h"
//#include "string_primitives.h"
//#include "convert_codepage.h"
//#include "convert_codepage_kssm.h"



//#ifndef GTL_HEADER__BASIC_STRING_MISC
//#define GTL_HEADER__BASIC_STRING_MISC

namespace gtl {
#pragma pack(push, 8)


	//-----------------------------------------------------------------------------
	/// @brief  misc. GetSpaceString()
	/// @return " \r\t\n"
	template < gtlc::string_elem tchar > [[nodiscard]] constexpr std::basic_string_view<tchar> GetSpaceString();
	constexpr inline [[nodiscard]] std::basic_string_view<char>     GetSpaceStringA()	{ return GetSpaceString<char>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<char8_t>  GetSpaceStringU8()	{ return GetSpaceString<char8_t>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<char16_t> GetSpaceStringU16()	{ return GetSpaceString<char16_t>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<char32_t> GetSpaceStringU32()	{ return GetSpaceString<char32_t>(); }
	constexpr inline [[nodiscard]] std::basic_string_view<wchar_t>  GetSpaceStringW()	{ return GetSpaceString<wchar_t>(); }


	/// @brief ToLower, ToUpper, ToDigit, IsSpace ... (locale irrelavant)
	template < gtlc::string_elem tchar > /*constexpr */inline [[nodiscard]] tchar ToLower(tchar c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > /*constexpr */inline [[nodiscard]] tchar ToUpper(tchar c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > /*constexpr */inline               void MakeLower(tchar& c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > /*constexpr */inline               void MakeUpper(tchar& c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsDigit(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsOdigit(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsXdigit(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsSpace(tchar const c/* Locale Irrelavant */);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsNotSpace(tchar const c/* Locale Irrelavant */);


	/// @brief Compare strings (containing numbers)
	template < gtlc::string_elem tchar >
	constexpr inline int tdszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar >
	constexpr inline int tdszcmp(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB);
	template < gtlc::string_elem tchar >
	constexpr inline int tdszicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar >
	constexpr inline int tdszicmp(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB);


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


	/// @brief Split Strings into ...
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string<tchar>> Split(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svDelimiters);
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string<tchar>> Split(std::basic_string_view<tchar> sv, tchar cDelimiter);
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string_view<tchar>> SplitView(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svDelimiters);
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string_view<tchar>> SplitView(std::basic_string_view<tchar> sv, tchar cDelimiter);


	/// @brief Translate Escape Sequence Characters
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] std::optional<std::basic_string<tchar>> TranslateEscapeSequence(std::basic_string_view<tchar> sv, tchar cFill = (tchar)'?', tchar cTerminating = 0);

	template < gtlc::string_elem tchar_t >
	std::vector<std::basic_string<tchar_t>> ConvDataToHexString(std::span<uint8_t> data, size_t nCol = 16, int cDelimiter = ' ', bool bAddText = false, int cDelimiterText = '|');

#pragma pack(pop)
};	// namespace gtl;

#include "string_misc.hpp"


//#endif	// GTL_HEADER__BASIC_STRING_MISC
