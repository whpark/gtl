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
	struct eCODEPAGE { enum : int {
		Default = 1,	// CP_OEMCP

		UnicodeLE = 1200,
		UnicodeBE = 1201,
		Unicode = (std::endian::native == std::endian::little) ? UnicodeLE : UnicodeBE,

		UTF7 = 65000,
		UTF8 = 65001,

		UTF16LE = UnicodeLE,
		UTF16BE = UnicodeBE,
		UTF16 = (std::endian::native == std::endian::little) ? UTF16LE : UTF16BE,

		UTF32LE = 12000,
		UTF32BE = 12001,
		UTF32 = (std::endian::native == std::endian::little) ? UTF32LE : UTF32BE,

		KO_KR_949 = 949,
		KO_KR_KSSM_1361 = 1361,	// 조합형 KSSM
	}; };

	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar_t1, gtlc::string_elem tchar_t2 >
	basic_string_t<tchar_t2> ToString(std::basic_string_view<tchar_t1> str, int eCodepage = eCODEPAGE::Default) {
		basic_string_t<tchar_t2> result;
		if constexpr (std::is_same_v<tchar_t1, tchar_t2>) {
			result = str;
		} else {
			if constexpr (std::is_same_v<tchar_t1, char>) {
				if constexpr (std::is_same_v<tchar_t2, char>) {
					result = str;//ToStringA(str, eCodepage);
				} else if constexpr (std::is_same_v<tchar_t2, wchar_t>) {
					result = ToStringW(str, eCodepage);
				} else if constexpr (std::is_same_v<tchar_t2, char8_t>) {
					result = ToStringU8(str, eCodepage);
				} else if constexpr (std::is_same_v<tchar_t2, char16_t>) {
					result = ToStringU16(str, eCodepage);
				} else if constexpr (std::is_same_v<tchar_t2, char32_t>) {
					result = ToStringU32(str, eCodepage);
				}
			} else {
				if constexpr (std::is_same_v<tchar_t2, char>) {
					result = ToStringA(str, eCodepage);
				} else if constexpr (std::is_same_v<tchar_t2, wchar_t>) {
					result = ToStringW(str);
				} else if constexpr (std::is_same_v<tchar_t2, char8_t>) {
					result = ToStringU8(str);
				} else if constexpr (std::is_same_v<tchar_t2, char16_t>) {
					result = ToStringU16(str);
				} else if constexpr (std::is_same_v<tchar_t2, char32_t>) {
					result = ToStringU32(str);
				}
			}
		}
		return result;
	}

	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar_t1, gtlc::string_elem tchar_t2 >
	void ConvertString(std::basic_string_view<tchar_t1> strFrom, basic_string_t<tchar_t2>& strTo, int eCodepage = eCODEPAGE::Default) {
		strTo = ToString<tchar_t1, tchar_t2>(strFrom, eCodepage);
	}
	template < gtlc::string_elem tchar_t1, gtlc::string_elem tchar_t2 >
	void ConvertString(basic_string_t<tchar_t1> const& strFrom, basic_string_t<tchar_t2>& strTo, int eCodepage = eCODEPAGE::Default) {
		strTo = ToString<tchar_t1, tchar_t2>(strFrom, eCodepage);
	}
	template < gtlc::string_elem tchar_t1, gtlc::string_elem tchar_t2 >
	void ConvertString(tchar_t1 const* pszFrom, basic_string_t<tchar_t2>& strTo, int eCodepage = eCODEPAGE::Default) {
		strTo = ToString<tchar_t1, tchar_t2>(std::basic_string_view{pszFrom, tszlen(pszFrom)}, eCodepage);
	}

	/// @brief Converts Codepage To StringA (MBCS)
	GTL_API std::string		ToStringA(std::string_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::string		ToStringA(std::wstring_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::string		ToStringA(std::u8string_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::string		ToStringA(std::u16string_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::string		ToStringA(std::u32string_view str, int eCodepage = eCODEPAGE::Default);

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	GTL_API std::wstring	ToStringW(std::string_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::wstring	ToStringW(std::wstring_view str);
	GTL_API std::wstring	ToStringW(std::u8string_view str);
	GTL_API std::wstring	ToStringW(std::u16string_view str);
	GTL_API std::wstring	ToStringW(std::u32string_view str);

	/// @brief Converts Codepage To utf-8
	GTL_API std::u8string	ToStringU8(std::string_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::u8string	ToStringU8(std::wstring_view str);
	GTL_API std::u8string	ToStringU8(std::u8string_view str);
	GTL_API std::u8string	ToStringU8(std::u16string_view str);
	GTL_API std::u8string	ToStringU8(std::u32string_view str);

	/// @brief Converts Codepage To utf-16
	GTL_API std::u16string	ToStringU16(std::string_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::u16string	ToStringU16(std::wstring_view str);
	GTL_API std::u16string	ToStringU16(std::u8string_view str);
	GTL_API std::u16string	ToStringU16(std::u16string_view str);
	GTL_API std::u16string	ToStringU16(std::u32string_view str);

	/// @brief Converts Codepage To utf-32
	GTL_API std::u32string	ToStringU32(std::string_view str, int eCodepage = eCODEPAGE::Default);
	GTL_API std::u32string	ToStringU32(std::wstring_view str);
	GTL_API std::u32string	ToStringU32(std::u8string_view str);
	GTL_API std::u32string	ToStringU32(std::u16string_view str);
	GTL_API std::u32string	ToStringU32(std::u32string_view str);

	/// @brief Unicode <-> KSSM (조합형)
	GTL_API bool ToStringW_KSSM(std::wstring_view, basic_string_t<uint16_t>&);
	GTL_API bool ToStringKSSM_W(std::basic_string_view<uint16_t>, std::wstring&);

	// todo :
	// determin which conversion to take, using parameter.
	GTL_API std::basic_string<uint16_t>	ConvUnicode_KSSM(const std::wstring& strSourceUnicode);
	GTL_API std::wstring				ConvKSSM_Unicode(const std::basic_string<uint16_t>& strSourceKSSM);
	GTL_API uint16_t					ConvUnicode_KSSM(wchar_t cKSSM);
	GTL_API wchar_t						ConvKSSM_Unicode(uint16_t cUnicode);

	// Conversion using move. destroys source.
	template < gtlc::string_elem tchar_t >
	void ConvCodepage_Move(basic_string_t<tchar_t>& strDest, basic_string_t<tchar_t>&& strSource, int eCodepage = 0/*dummy*/)	{ strDest = strSource; }
	template < gtlc::string_elem tchar_t, gtlc::string_elem tchar_other_t> requires (!std::is_same_v<tchar_t, tchar_other_t>)
	void ConvCodepage_Move(basic_string_t<tchar_t>& strDest, const basic_string_t<tchar_other_t>& strSource, int eCodepage = CP_ACP)	{ ConvCodepage(strDest, strSource, eCodepage); }


	///============================================
	// KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	//

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @tparam tchar_t1 char16_t for UTF16, or uint16_t for KSSM
	/// @tparam tchar_t2 
	/// @return Code Converting MAP
	template < typename tchar_t1, typename tchar_t2 >
	requires ( (std::is_same_v<tchar_t1, char16_t> && std::is_same_v<tchar_t2, uint16_t>)
			  || (std::is_same_v<tchar_t1, uint16_t> && std::is_same_v<tchar_t2, char16_t>)
			  )
		std::map<tchar_t1, tchar_t2> const& GetHangeulCodeMap() {
		if constexpr (std::is_same_v<tchar_t1, char16_t>) {
			return GetHangeulCodeMapWtoKSSM();
		} else {
			return GetHangeulCodeMapKSSMtoW();
		}
	}

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @tparam tchar_t1 char16_t for UTF16, or uint16_t for KSSM
	/// @tparam tchar_t2 
	/// @return Converted String.
	template < typename tchar_t1, typename tchar_t2 >
	requires ( (std::is_same_v<tchar_t1, char16_t> && std::is_same_v<tchar_t2, uint16_t>)
			  || (std::is_same_v<tchar_t1, uint16_t> && std::is_same_v<tchar_t2, char16_t>)
			  )
		basic_string_t<tchar_t2> _Conv_KSSM_UTF16(basic_string_t<tchar_t1> const& strSource, tchar_t2 cErrorFillCode = L'?') {
		basic_string_t<tchar_t2> str;
		str.reserve(strSource.size());
		static auto const& map1to2 = GetHangeulCodeMap<tchar_t1, tchar_t2>();
		static auto const& map2to1 = GetHangeulCodeMap<tchar_t2, tchar_t1>();

		for (auto c : strSource) {
			if constexpr (std::is_same_v<tchar_t1, uint16_t>) {
				if ( (c < 0x8444) || (c > 0xd3bd) ) {
					str += c;
					continue;
				}
			} else {
				if ( (c < 0x3131) || (c > 0xd7a) ) {
					str += c;
					continue;
				}
			}

			if (auto iter = map1to2.find(c); iter != map1to2.end())
				str += iter->second;
			else {
				if (map2to1.find(c) != map2to1.end())	// 없는데, 코드가 겹칠경우
					str += cErrorFillCode;
				else
					str += c;
			}
		}
		return str;
	}

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @tparam tchar_t1 char16_t for UTF16, or uint16_t for KSSM
	/// @tparam tchar_t2 
	/// @return Converted Char.
	template < typename tchar_t1, typename tchar_t2 >
	requires ( (std::is_same_v<tchar_t1, char16_t> && std::is_same_v<tchar_t2, uint16_t>)
			  || (std::is_same_v<tchar_t1, uint16_t> && std::is_same_v<tchar_t2, char16_t>)
			  )
		tchar_t2 _Conv_KSSM_UTF16(tchar_t1 cSource, tchar_t2 cErrorFillCode = L'?') {
		static auto const& map1to2 = GetHangeulCodeMap<tchar_t1, tchar_t2>();
		static auto const& map2to1 = GetHangeulCodeMap<tchar_t2, tchar_t1>();

		if constexpr (std::is_same_v<tchar_t1, uint16_t>) {
			if ( (c < 0x8444) || (c > 0xd3bd) ) {
				return c;
			}
		} else {
			if ( (c < 0x3131) || (c > 0xd7a3) ) {
				return c;
			}
		}

		if (auto iter = map1to2.find(c); iter != map1to2.end())
			return iter->second;
		else {
			if (map2to1.find(c) != map2to1.end())	// 없는데, 코드가 겹칠경우
				return cErrorFillCode;
			else
				return c;
		}
	}

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param strSource : source String (KSSM)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return KSSM -> UTF16
	std::u16string ConvKSSM_UTF16(const basic_string_t<uint16_t>& strSource, char16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<uint16_t, char16_t>(strSource, cErrorFillCode);
	}
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param strSource : source String (UTF16)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return UTF16 -> KSSM
	basic_string_t<uint16_t> ConvUTF16_KSSM(const std::u16string& strSource, uint16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<char16_t, uint16_t>(strSource, cErrorFillCode);
	}
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param strSource : source char (KSSM)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return KSSM -> UTF16
	char16_t ConvKSSM_UTF16(uint16_t const cSource, char16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<uint16_t, char16_t>(cSource, cErrorFillCode);
	}
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param strSource : source char (UTF16)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return UTF16 -> KSSM
	uint16_t ConvUTF16_KSSM(char16_t const cSource, uint16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<char16_t, uint16_t>(cSource, cErrorFillCode);
	}

	GTL_API bool IsUTF8String(std::string_view str, int* pOutputBufferCount = nullptr, bool* pbIsMSBSet = nullptr);

#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__STRING_CONVERT_CODEPAGE
