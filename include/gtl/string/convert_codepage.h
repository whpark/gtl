//////////////////////////////////////////////////////////////////////
//
// convert_codepage.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING_CONVERT_CODEPAGE
#define GTL_HEADER__STRING_CONVERT_CODEPAGE

#include "basic_string.h"

namespace gtl {
#pragma pack(push, 8)

	//------------------------------------------------------------------------------------------
	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	struct eCODEPAGE { enum : uint32_t {
		Default = 0xFFFF'FFFF,	// CP_OEMCP

		UnicodeLE = 1200,
		UnicodeBE = 1201,
		Unicode = (std::endian::native == std::endian::little) ? UnicodeLE : UnicodeBE,

		UTF7 = 65000,
		UTF8 = 65001,

		UTF16LE = UnicodeLE,
		UTF16BE = UnicodeBE,
		UTF16 = (std::endian::native == std::endian::little) ? UTF16LE : UTF16BE,
		_UTF16_other = (UTF16 == UTF16BE) ? UTF16LE : UTF16BE,

		UTF32LE = 12000,
		UTF32BE = 12001,
		UTF32 = (std::endian::native == std::endian::little) ? UTF32LE : UTF32BE,
		_UTF32_other = (UTF32 == UTF32BE) ? UTF32LE : UTF32BE,

		KO_KR_949 = 949,
		KO_KR_JOHAB_KSSM_1361 = 1361,	// Á¶ÇÕÇü KSSM
	}; };

	struct S_CODEPAGE_OPTION {
		uint32_t from {eCODEPAGE::Default};
		uint32_t to {eCODEPAGE::Default};
	};

	/// @brief Converts Codepage To StringA (MBCS)
	GTL_API std::string		ToStringA(std::string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::string		ToStringA(std::wstring_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::string		ToStringA(std::u8string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::string		ToStringA(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::string		ToStringA(std::u32string_view strFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	GTL_API std::wstring	ToStringW(std::string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::wstring	ToStringW(std::wstring_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::wstring	ToStringW(std::u8string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::wstring	ToStringW(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::wstring	ToStringW(std::u32string_view strFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-8
	GTL_API std::u8string	ToStringU8(std::string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u8string	ToStringU8(std::wstring_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u8string	ToStringU8(std::u8string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u8string	ToStringU8(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u8string	ToStringU8(std::u32string_view strFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-16
	GTL_API std::u16string	ToStringU16(std::string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u16string	ToStringU16(std::wstring_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u16string	ToStringU16(std::u8string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u16string	ToStringU16(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u16string	ToStringU16(std::u32string_view strFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-32
	GTL_API std::u32string	ToStringU32(std::string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u32string	ToStringU32(std::wstring_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u32string	ToStringU32(std::u8string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u32string	ToStringU32(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage = {});
	GTL_API std::u32string	ToStringU32(std::u32string_view strFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar_t1, gtlc::string_elem tchar_t2 >
	basic_string_t<tchar_t2> ToString(std::basic_string_view<tchar_t1> str, S_CODEPAGE_OPTION codepage = {}) {
		basic_string_t<tchar_t2> result;
		if constexpr (std::is_same_v<tchar_t2, char>) {
			result = ToStringA(str, codepage);
		} else if constexpr (std::is_same_v<tchar_t2, wchar_t>) {
			result = ToStringW(str, codepage);
		} else if constexpr (std::is_same_v<tchar_t2, char8_t>) {
			result = ToStringU8(str, codepage);
		} else if constexpr (std::is_same_v<tchar_t2, char16_t>) {
			result = ToStringU16(str, codepage);
		} else if constexpr (std::is_same_v<tchar_t2, char32_t>) {
			result = ToStringU32(str, codepage);
		}
		return result;
	}
	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar_t1, gtlc::string_elem tchar_t2 >
	void ToString(std::basic_string_view<tchar_t1> strFrom, std::basic_string<tchar_t2>& strTo, S_CODEPAGE_OPTION codepage = {}) {
		strTo = ToString<tchar_t1, tchar_t2>(strFrom);
	}

	GTL_API std::string ConvUTF16_MBCS(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u16string ConvMBCS_UTF16(std::string_view strFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u8string ConvUTF16_UTF8(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u16string ConvUTF8_UTF16(std::u8string_view strFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u32string ConvUTF16_UTF32(std::u16string_view strFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u16string ConvUTF32_UTF16(std::u32string_view strFrom, S_CODEPAGE_OPTION codepage);

	GTL_API bool IsUTF8String(std::string_view str, int* pOutputBufferCount = nullptr, bool* pbIsMSBSet = nullptr);


	//-------------------------------------------------------------

	//template < typename > requires(false)
	//[[deprecated]] inline std::u8string ConvUTF16_UTF8(std::u16string_view svFrom) {
	//	std::wstring_convert<std::codecvt<char16_t, char8_t, std::mbstate_t>, char16_t> conversion;
	//	return (std::u8string&)conversion.to_bytes(svFrom.data());
	//}
	//template < typename > requires(false)
	//[[deprecated]] inline std::u16string ConvUTF8_UTF16(std::u8string_view svFrom) {
	//	std::wstring_convert<std::codecvt<char16_t, char8_t, std::mbstate_t>, char16_t> conversion;
	//	return conversion.from_bytes((char const*)svFrom.data());
	//}
	//template < typename > requires(false)
	//[[deprecated]] inline std::u32string ConvUTF8_UTF32(std::u8string_view sv) {
	//	std::wstring_convert<std::codecvt<char32_t, char8_t,std::mbstate_t>, char32_t> conversion;
	//	return conversion.from_bytes((char const*)sv.data());
	//}
	//template < typename > requires(false)
	//[[deprecated]] inline std::u8string ConvUTF32_UTF8(std::u32string_view sv) {
	//	std::wstring_convert<std::codecvt<char32_t, char8_t,std::mbstate_t>, char32_t> conversion;
	//	return (std::u8string&)conversion.to_bytes(sv.data());
	//}


#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__STRING_CONVERT_CODEPAGE
