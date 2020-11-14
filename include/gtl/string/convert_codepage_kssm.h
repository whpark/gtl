//////////////////////////////////////////////////////////////////////
//
// convert_codepage_kssm.h:
//
// PWH
// 2020.11.14.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING_CONVERT_CODEPAGE_KSSM
#define GTL_HEADER__STRING_CONVERT_CODEPAGE_KSSM

#include "basic_string.h"

namespace gtl {
#pragma pack(push, 8)

	///============================================
	// KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	//

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @return Code Converting MAP
	GTL_API std::map<wchar_t, uint16_t> GetHangeulCodeMap(wchar_t, uint16_t);
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @return Code Converting MAP
	GTL_API std::map<uint16_t, wchar_t> GetHangeulCodeMap(uint16_t, wchar_t);

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
		static auto const& map1to2 = GetHangeulCodeMap(tchar_t1{}, tchar_t2{});
		static auto const& map2to1 = GetHangeulCodeMap(tchar_t2{}, tchar_t1{});

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
	std::optional<tchar_t2> _Conv_KSSM_UTF16(tchar_t1 c, tchar_t2 cErrorFillCode = L'?') {
		static auto const& map1to2 = GetHangeulCodeMap(tchar_t1{}, tchar_t2{});
		static auto const& map2to1 = GetHangeulCodeMap(tchar_t2{}, tchar_t1{});

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
	std::optional<char16_t> ConvKSSM_UTF16(uint16_t const cSource, char16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<uint16_t, char16_t>(cSource, cErrorFillCode);
	}
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param strSource : source char (UTF16)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return UTF16 -> KSSM
	std::optional<uint16_t> ConvUTF16_KSSM(char16_t const cSource, uint16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<char16_t, uint16_t>(cSource, cErrorFillCode);
	}

#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__STRING_CONVERT_CODEPAGE_KSSM
