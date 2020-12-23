//////////////////////////////////////////////////////////////////////
//
// convert_utf.h:
//
// PWH
// 2020.12.22.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING_CONVERT_UTF
#define GTL_HEADER__STRING_CONVERT_UTF

#include <experimental/generator>

#include "string_primitives.h"

namespace gtl {
#pragma pack(push, 8)

	namespace utf_const {
		constexpr static inline uint8_t const bitmask_3bit = 0b0000'0111;
		constexpr static inline uint8_t const bitmask_4bit = 0b0000'1111;
		constexpr static inline uint8_t const bitmask_6bit = 0b0011'1111;
		constexpr static inline std::pair const fSurrogateW1{ (char16_t)0xd800u, (char16_t)0xdbffu };	// 0xd800u ~ 0xdbffu;
		constexpr static inline std::pair const fSurrogateW2{ (char16_t)0xdc00u, (char16_t)0xdfffu };	// 0xd800u ~ 0xdbffu;
		constexpr static inline int const nBitSurrogate{ 10 };
	}

	//-------------------------------------------------------------
	// experimental
	//
	template < gtlc::string_elem_utf tchar >
	std::experimental::generator<tchar> SeqUTF(std::basic_string_view<char32_t> sv) {
		if constexpr (sizeof(tchar) == sizeof(char8_t)) {
			for (auto const c : sv) {
				if (c <= 0x7f) {
					co_yield c;
				}
				else if (c <= 0x7ff) {
					co_yield 0xc0 + (c >> 6);
					co_yield 0x80 + (c & utf_const::bitmask_6bit);
				}
				else if (c <= 0xffff) {
					co_yield 0xe0 + (c >> 12);
					co_yield 0x80 + ((c >> 6) & utf_const::bitmask_6bit);
					co_yield 0x80 + ((c >> 0) & utf_const::bitmask_6bit);
				}
				else if (c <= 0x10'ffff) {
					co_yield 0xf0 + (c >> 18);
					co_yield 0x80 + ((c >> 12) & utf_const::bitmask_6bit);
					co_yield 0x80 + ((c >> 6) & utf_const::bitmask_6bit);
					co_yield 0x80 + ((c >> 0) & utf_const::bitmask_6bit);
				}
				else
					throw std::invalid_argument{ GTL__FUNCSIG "no utf" };
			}
		}
		else if constexpr (sizeof(tchar) == sizeof(char16_t)) {
			for (auto const c : sv) {
				if (c <= 0xffff) {
					[[likely]]
					co_yield c;
				}
				else if ((0x1'0000 <= c) && (c <= 0x10'ffff)) {
					constexpr auto const preH = utf_const::fSurrogateW1.first - 0x1'0000;
					co_yield preH + (c >> 10);
					constexpr auto const preL = utf_const::fSurrogateW2.first;
					co_yield preL + (c & 0b0011'1111);
				}
				else
					co_yield c;
			}
		}
		else if constexpr (sizeof(tchar) == sizeof(char32_t)) {
			for (auto const c : sv) {
				co_yield c;
			}
		}
		co_return;
	}

	//template < gtlc::string_elem_utf tchar >
	//std::experimental::generator<tchar> SeqUTF32(std::basic_string_view<char16_t> sv) {
	//	// utf16
	//	if ((position_[0] < uc::fSurrogateW1.first) || (position_[0] > uc::fSurrogateW2.second)) [[ likely ]] {
	//		code_ = position_[0];
	//		diff_to_next_ = 1;
	//	}
	//	else if (position_[0] <= uc::fSurrogateW2.first) [[unlikely]] { // leading surrogate
	//		if (((position_ + 1) >= end_)
	//			|| ((position_[1] < uc::fSurrogateW2.first) || (position_[1] > uc::fSurrogateW2.second))
	//			)
	//			throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid trailing surrogate" };

	//		constexpr static uint32_t const pre = 0x1'0000 - ((uc::fSurrogateW1.first << uc::nBitSurrogate) + uc::fSurrogateW2.first);
	//		code_ = (pre + (uint32_t)(position_[0] << 10) + position_[1]);
	//		diff_to_next_ = 2;
	//	}
	//	else {
	//		throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid surrogate" };
	//	}
	//}



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


#endif	// GTL_HEADER__STRING_CONVERT_UTF
