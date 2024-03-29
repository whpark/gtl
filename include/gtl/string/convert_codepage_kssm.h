﻿//////////////////////////////////////////////////////////////////////
//
// convert_codepage_kssm.h: Korean JOHAB code (KSSM) (text file is in utf-8)
//
// PWH
// 2020.11.14.
// 2020.11.26. 자모코드 추가
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL__HEADER__STRING_CONVERT_CODEPAGE_KSSM
#define GTL__HEADER__STRING_CONVERT_CODEPAGE_KSSM

#include "gtl/_config.h"
#include "gtl/iconv_wrapper.h"

#include "string_primitives.h"
#include "HangeulCodeMap.h"

namespace gtl::charset::KSSM {
#pragma pack(push, 8)

	enum eKSSM : charKSSM_t {
		maskIni = 0b1'11111'00000'00000,	// 초성
		maskMid = 0b1'00000'11111'00000,	// 중성
		maskFin = 0b1'00000'00000'11111,	// 종성

		// 초성
		SyllableIni_Fill	=  1 << 10,	// 채움
		SyllableIni_G		=  2 << 10,	// ㄱ
		SyllableIni_GG		=  3 << 10,	// ㄲ
		SyllableIni_N		=  4 << 10,	// ㄴ
		SyllableIni_D		=  5 << 10,	// ㄷ
		SyllableIni_DD		=  6 << 10,	// ㄸ
		SyllableIni_L		=  7 << 10,	// ㄹ
		SyllableIni_M		=  8 << 10,	// ㅁ
		SyllableIni_B		=  9 << 10,	// ㅂ
		SyllableIni_BB		= 10 << 10,	// ㅃ
		SyllableIni_S		= 11 << 10,	// ㅅ
		SyllableIni_SS		= 12 << 10,	// ㅆ
		SyllableIni_O		= 13 << 10,	// ㅇ
		SyllableIni_J		= 14 << 10,	// ㅈ
		SyllableIni_JJ		= 15 << 10,	// ㅉ
		SyllableIni_CH		= 16 << 10,	// ㅊ
		SyllableIni_K		= 17 << 10,	// ㅋ
		SyllableIni_T		= 18 << 10,	// ㅌ
		SyllableIni_P		= 19 << 10,	// ㅍ
		SyllableIni_H		= 20 << 10,	// ㅎ


		// 중성. U : Up, D : Down, R : Right, L : Left, H : Horizontal bar, V : Vertical bar
		SyllableMid_FILL	=  2 <<  5,	// 채움		00010
		SyllableMid_R		=  3 <<  5,	// ㅏ		00011
		SyllableMid_RV		=  4 <<  5,	// ㅐ		00100
		SyllableMid_RR		=  5 <<  5,	// ㅑ		00101
		SyllableMid_RRV		=  6 <<  5,	// ㅒ		00110
		SyllableMid_L		=  7 <<  5,	// ㅓ		00111
		SyllableMid_LV		= 10 <<  5,	// ㅔ		01010
		SyllableMid_LL		= 11 <<  5,	// ㅕ		01011
		SyllableMid_LLV		= 12 <<  5,	// ㅖ		01100
		SyllableMid_U		= 13 <<  5,	// ㅗ		01101
		SyllableMid_UR		= 14 <<  5,	// ㅘ		01110
		SyllableMid_URV		= 15 <<  5,	// ㅙ		01111
		SyllableMid_UV		= 18 <<  5,	// ㅚ		10010
		SyllableMid_UU		= 19 <<  5,	// ㅛ		10011
		SyllableMid_D		= 20 <<  5,	// ㅜ		10100
		SyllableMid_DL		= 21 <<  5,	// ㅝ		10101
		SyllableMid_DLV		= 22 <<  5,	// ㅞ		10110
		SyllableMid_DV		= 23 <<  5,	// ㅟ		10111
		SyllableMid_DD		= 26 <<  5,	// ㅠ		11010
		SyllableMid_H		= 27 <<  5,	// ㅡ		11011
		SyllableMid_HV		= 28 <<  5,	// ㅢ		11100
		SyllableMid_V		= 29 <<  5,	// ㅣ		11101


		// 종성
		SyllableFin_FILL	=  1,	// 채움
		SyllableFin_G		=  2,	// ㄱ
		SyllableFin_GG		=  3,	// ㄲ
		SyllableFin_GS		=  4,	// ㄳ
		SyllableFin_N		=  5,	// ㄴ
		SyllableFin_NJ		=  6,	// ㄵ
		SyllableFin_NH		=  7,	// ㄶ
		SyllableFin_D		=  8,	// ㄷ
		SyllableFin_L		=  9,	// ㄹ
		SyllableFin_LG		= 10,	// ㄺ
		SyllableFin_LM		= 11,	// ㄻ
		SyllableFin_LB		= 12,	// ㄼ
		SyllableFin_LS		= 13,	// ㄽ
		SyllableFin_LT		= 14,	// ㄾ
		SyllableFin_LP		= 15,	// ㄿ
		SyllableFin_LH		= 16,	// ㅀ
		SyllableFin_M		= 17,	// ㅁ
		SyllableFin_B		= 19,	// ㅂ
		SyllableFin_BS		= 20,	// ㅄ
		SyllableFin_S		= 21,	// ㅅ
		SyllableFin_SS		= 22,	// ㅆ
		SyllableFin_O		= 23,	// ㅇ
		SyllableFin_J		= 24,	// ㅈ
		SyllableFin_CH		= 25,	// ㅊ
		SyllableFin_K		= 26,	// ㅋ
		SyllableFin_T		= 27,	// ㅌ
		SyllableFin_P		= 28,	// ㅍ
		SyllableFin_H		= 29,	// ㅎ

	};


#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM)

	///============================================
	// KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	//

#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @tparam tchar_from char16_t for UTF16, or charKSSM_t for KSSM
	/// @tparam tchar_to 
	/// @return Converted String.
	template < typename tchar_to, typename tchar_from >
	requires ( (std::is_same_v<tchar_from, char16_t> && std::is_same_v<tchar_to, charKSSM_t>)
			  || (std::is_same_v<tchar_from, charKSSM_t> && std::is_same_v<tchar_to, char16_t>)
			  )
	std::basic_string<tchar_to> _Conv_KSSM_UTF16(std::basic_string_view<tchar_from> const& svFrom, tchar_to cErrorFillCode = L'?') {
		Ticonv<tchar_to, tchar_from> conv;
		conv.SetDiscardIsseq(true);
		return conv.Convert(svFrom).value_or(std::basic_string<tchar_to>{cErrorFillCode});
	}

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @tparam tchar_from char16_t for UTF16, or charKSSM_t for KSSM
	/// @tparam tchar_to 
	/// @return Converted Char.
	template < typename tchar_to, typename tchar_from >
	requires ( (std::is_same_v<tchar_from, char16_t> && std::is_same_v<tchar_to, charKSSM_t>)
			  || (std::is_same_v<tchar_from, charKSSM_t> && std::is_same_v<tchar_to, char16_t>)
			  )
	tchar_to _Conv_KSSM_UTF16(tchar_from c, tchar_to cErrorFillCode = L'?') {
		Ticonv<tchar_to, tchar_from> conv;
		conv.SetDiscardIsseq(true);
		if (auto r = conv.Convert(std::basic_string_view<tchar_from>(&c, 1)); r and r->size()) {
			return r->front();
		} else {
			return cErrorFillCode;
		}
	}


#else

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @return Code Converting MAP
	inline std::map<char16_t, charKSSM_t> const& GetHangeulCodeMap(char16_t, charKSSM_t) {
		return mapUTF16toKSSM_g;
	}

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @return Code Converting MAP
	inline std::map<charKSSM_t, char16_t> const& GetHangeulCodeMap(charKSSM_t, char16_t) {
		return mapKSSMtoUTF16_g;
	}


	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @tparam tchar_from char16_t for UTF16, or charKSSM_t for KSSM
	/// @tparam tchar_to 
	/// @return Converted String.
	template < typename tchar_to, typename tchar_from >
	requires ( (std::is_same_v<tchar_from, char16_t> && std::is_same_v<tchar_to, charKSSM_t>)
			  || (std::is_same_v<tchar_from, charKSSM_t> && std::is_same_v<tchar_to, char16_t>)
			  )
	std::basic_string<tchar_to> _Conv_KSSM_UTF16(std::basic_string_view<tchar_from> const& svFrom, tchar_to cErrorFillCode = L'?') {
		std::basic_string<tchar_to> str;
		str.reserve(svFrom.size());
		static auto const& map1to2 = GetHangeulCodeMap(tchar_from{}, tchar_to{});
		static auto const& map2to1 = GetHangeulCodeMap(tchar_to{}, tchar_from{});

		for (auto c : svFrom) {
			if constexpr (std::is_same_v<tchar_from, charKSSM_t>) {
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
	/// @tparam tchar_from char16_t for UTF16, or charKSSM_t for KSSM
	/// @tparam tchar_to 
	/// @return Converted Char.
	template < typename tchar_to, typename tchar_from >
	requires ( (std::is_same_v<tchar_from, char16_t> && std::is_same_v<tchar_to, charKSSM_t>)
			  || (std::is_same_v<tchar_from, charKSSM_t> && std::is_same_v<tchar_to, char16_t>)
			  )
	tchar_to _Conv_KSSM_UTF16(tchar_from c, tchar_to cErrorFillCode = L'?') {
		static auto const& map1to2 = GetHangeulCodeMap(tchar_from{}, tchar_to{});
		static auto const& map2to1 = GetHangeulCodeMap(tchar_to{}, tchar_from{});

		if constexpr (std::is_same_v<tchar_from, charKSSM_t>) {
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

#endif

	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param svFrom : source String (KSSM)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return KSSM -> UTF16
	inline std::u16string ConvKSSM_UTF16(std::basic_string_view<charKSSM_t> svFrom, char16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<char16_t, charKSSM_t>(svFrom, cErrorFillCode);
	}
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param svFrom : source String (UTF16)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return UTF16 -> KSSM
	inline std::basic_string<charKSSM_t> ConvUTF16_KSSM(std::u16string_view svFrom, charKSSM_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<charKSSM_t, char16_t>(svFrom, cErrorFillCode);
	}
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param svFrom : source char (KSSM)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return KSSM -> UTF16
	inline char16_t ConvKSSM_UTF16(charKSSM_t const cSource, char16_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<char16_t, charKSSM_t>(cSource, cErrorFillCode);
	}
	/// @brief KSSM <=> Codepage 949 (Korean Text Conversion. 조합형 <=> 완성형949(UTF16))
	/// @param svFrom : source char (UTF16)
	/// @param cErrorFillCode : Error Fill Code.
	/// @return UTF16 -> KSSM
	inline charKSSM_t ConvUTF16_KSSM(char16_t const cSource, charKSSM_t cErrorFillCode = L'?') {
		return _Conv_KSSM_UTF16<charKSSM_t, char16_t>(cSource, cErrorFillCode);
	}

#pragma pack(pop)
};	// namespace gtl::charset::KSSM;


#endif	// (GTL__STRING_SUPPORT_CODEPAGE_KSSM)

#endif	// GTL__HEADER__STRING_CONVERT_CODEPAGE_KSSM
