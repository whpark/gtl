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
		constexpr static inline uint32_t const unicode_max = 0x10'ffff;
	}


	/// @brief UTFn -> UTFn coroutine. (experimental)
	/// @param svFrom source text
	/// @return converted text
	template < gtlc::string_elem_utf tchar_return, gtlc::string_elem_utf tchar >
	std::experimental::generator<tchar_return> SeqUTF(std::basic_string_view<tchar> svFrom) {
		namespace uc = gtl::utf_const;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		if constexpr (std::is_same_v<tchar_return, tchar>) {
			for (auto c : svFrom)
				co_yield c;
		}
		else if constexpr (std::is_same_v<tchar_return, char32_t>) {
			auto const* end = svFrom.data() + svFrom.size();
			for (auto const* pos = svFrom.data(); pos < end; pos++) {
				auto const c = pos[0];

				char32_t v{};

				if constexpr (std::is_same_v<tchar, char8_t>) {
					// utf8
					if (c <= 0x7f)
						v = c;
					else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7f
						if ( (pos+1 >= end)
							|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
							)
							throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
						v = ((char32_t)*pos & uc::bitmask_6bit) << 6;
						v |= (char32_t)*++pos & uc::bitmask_6bit;
					}
					else if ((c & 0b1111'0000) == 0b1110'0000) {	// ~0x7ff
						if ((pos+2 >= end)
							|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
							|| ((pos[2] & 0b1100'0000) != 0b1000'0000)
							)
							throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
						v = ((char32_t)*pos & uc::bitmask_4bit) << 12;
						v |= ((char32_t)*++pos & uc::bitmask_6bit) << 6;
						v |= (*++pos & uc::bitmask_6bit);
					}
					else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0xffff
						if ((pos+3 >= end)
							|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
							|| ((pos[2] & 0b1100'0000) != 0b1000'0000)
							|| ((pos[3] & 0b1100'0000) != 0b1000'0000)
							)
							throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
						v = ((char32_t)*pos & uc::bitmask_3bit) << 18;
						v |= ((char32_t)*++pos & uc::bitmask_6bit) << 12;
						v |= ((char32_t)*++pos & uc::bitmask_6bit) << 6;
						v |= (*++pos & uc::bitmask_6bit);
					}

				}
				else if constexpr (std::is_same_v<tchar, char16_t>) {
					// utf16
					if ((pos[0] < uc::fSurrogateW1.first) or (pos[0] > uc::fSurrogateW2.second)) [[ likely ]] {
						v = pos[0];
					}
					else if (pos[0] <= uc::fSurrogateW2.first) [[unlikely]] { // leading surrogate
						if (((pos + 1) >= end)
							|| ((pos[1] < uc::fSurrogateW2.first) || (pos[1] > uc::fSurrogateW2.second))
							)
							throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid trailing surrogate" };

						constexpr static uint32_t const pre = 0x1'0000 - ((uc::fSurrogateW1.first << uc::nBitSurrogate) + uc::fSurrogateW2.first);
						v = pre + ((uint32_t)(pos[0]) << 10) + pos[1];
						pos++;
					}
					else {
						throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid surrogate" };
					}

				}
				else if constexpr (std::is_same_v<tchar, char32_t>) {
					// utf32
					v = *pos;
					static_assert(false);
				} else {
					static_assert(false);
				}

				co_yield v;

			}
		}
		else {
			for (auto const c : SeqUTF<char32_t, tchar>(svFrom)) {
				if constexpr (sizeof(tchar_return) == sizeof(char8_t)) {
					if (c <= 0x7f) {
						co_yield c;
					}
					else if (c <= 0x7ff) {
						co_yield 0xc0 + (c >> 6);
						co_yield 0x80 + (c & uc::bitmask_6bit);
					}
					else if (c <= 0xffff) {
						co_yield 0xe0 + (c >> 12);
						co_yield 0x80 + ((c >> 6) & uc::bitmask_6bit);
						co_yield 0x80 + ((c >> 0) & uc::bitmask_6bit);
					}
					else if (c <= uc::unicode_max) {
						co_yield 0xf0 + (c >> 18);
						co_yield 0x80 + ((c >> 12) & uc::bitmask_6bit);
						co_yield 0x80 + ((c >> 6) & uc::bitmask_6bit);
						co_yield 0x80 + ((c >> 0) & uc::bitmask_6bit);
					}
					else
						throw std::invalid_argument{ GTL__FUNCSIG "not a utf char" };
				}
				else if constexpr (sizeof(tchar_return) == sizeof(char16_t)) {
					if (c < uc::fSurrogateW1.first) [[likely]] {
						co_yield c;
					}
					else if (c < uc::fSurrogateW2.second) {
						throw std::invalid_argument{ GTL__FUNCSIG "not a utf char" };
					}
					if (c <= 0xffff) {
						co_yield c;
					}
					else if (c <= 0x10'ffff) {
						constexpr auto const preH = uc::fSurrogateW1.first - 0x1'0000;
						co_yield preH + (c >> 10);
						constexpr auto const preL = uc::fSurrogateW2.first;
						co_yield preL + (c & 0b0011'1111);
					}
					else {
						throw std::invalid_argument{ GTL__FUNCSIG "not a utf char" };
					}
				}
				//else if constexpr (sizeof(tchar_return) == sizeof(char32_t)) {
				//	co_yield c;
				//}
			}
		}

		co_return;
	}





#if 0
	template < typename > requires(false)
	[[deprecated]] inline std::u8string ConvUTF16_UTF8(std::u16string_view svFrom) {
		std::wstring_convert<std::codecvt<char16_t, char8_t, std::mbstate_t>, char16_t> conversion;
		return (std::u8string&)conversion.to_bytes(svFrom.data());
	}
	template < typename > requires(false)
	[[deprecated]] inline std::u16string ConvUTF8_UTF16(std::u8string_view svFrom) {
		std::wstring_convert<std::codecvt<char16_t, char8_t, std::mbstate_t>, char16_t> conversion;
		return conversion.from_bytes((char const*)svFrom.data());
	}
	template < typename > requires(false)
	[[deprecated]] inline std::u32string ConvUTF8_UTF32(std::u8string_view sv) {
		std::wstring_convert<std::codecvt<char32_t, char8_t,std::mbstate_t>, char32_t> conversion;
		return conversion.from_bytes((char const*)sv.data());
	}
	template < typename > requires(false)
	[[deprecated]] inline std::u8string ConvUTF32_UTF8(std::u32string_view sv) {
		std::wstring_convert<std::codecvt<char32_t, char8_t,std::mbstate_t>, char32_t> conversion;
		return (std::u8string&)conversion.to_bytes(sv.data());
	}
#endif

#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__STRING_CONVERT_UTF
