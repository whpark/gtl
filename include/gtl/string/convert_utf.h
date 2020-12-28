//////////////////////////////////////////////////////////////////////
//
// convert_utf.h:
//
// PWH
// 2020.12.22.
// 2020.12.28.
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
		constexpr static inline uint16_t const surrogate16h = fSurrogateW1.first - (0x1'0000u >> nBitSurrogate);
		constexpr static inline uint16_t const surrogate16l = fSurrogateW2.first;


		static_assert(std::is_unsigned_v<char8_t>);
		static_assert(std::is_unsigned_v<char16_t>);
		static_assert(std::is_unsigned_v<char32_t>);
	}


	namespace internal {

		//-------------------------------------------------------------------------------------------------------------------------
		/// @brief UTF8 -> UTF16, UTF32
		/// @tparam treturn 
		/// @param pos 
		/// @param end 
		/// @param r 
		template < gtlc::string_elem_utf tchar_to, gtlc::string_elem tchar_from = char8_t,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_to>, ptrdiff_t>
		>
		inline void UTFCharConverter_FromUTF8(tchar_from const*& pos, tchar_from const* const& end, treturn& r) {

			auto MAS = [&pos] (int i, tchar_to mask, int sh) -> tchar_to {	// MaskAndShift
				return (((tchar_to)pos[i]) & mask) << sh;
			};

			namespace uc = utf_const;
			// utf8
			if (pos[0] <= 0x7f) {
				if constexpr (bAddChar) {
					r.push_back(pos[0]);
				}
				else {
					r++;
				}
				pos += 1;
			}
			else if ((pos[0] & 0b1110'0000) == 0b1100'0000) {	// 0~0x7ff
				if constexpr (bCheckError) {
					if ((pos + 1 >= end)
						|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
						)
						throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				}
				if constexpr (bAddChar) {
					r.push_back( MAS(0, uc::bitmask_6bit, 6) | MAS(1, uc::bitmask_6bit, 6) );
				}
				else {
					r++;
				}
				pos += 2;
			}
			else if ((pos[0] & 0b1111'0000) == 0b1110'0000) {	// ~0xffff
				if constexpr (bCheckError) {
					if ((pos + 2 >= end)
						|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
						|| ((pos[2] & 0b1100'0000) != 0b1000'0000)
						)
						throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				}
				if constexpr (bAddChar) {
					r.push_back( MAS(0, uc::bitmask_4bit, 12) | MAS(1, uc::bitmask_6bit, 6) | MAS(2, uc::bitmask_6bit, 0) );
				}
				else {
					r ++;
				}
				pos += 3;
			}
			else if ((pos[0] & 0b1111'1000) == 0b1111'0000) {	// ~0x10'ffff
				if constexpr (bCheckError) {
					if ((pos + 3 >= end)
						|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
						|| ((pos[2] & 0b1100'0000) != 0b1000'0000)
						|| ((pos[3] & 0b1100'0000) != 0b1000'0000)
						)
						throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				}
				if constexpr (gtlc::is_same_utf<tchar_to, char32_t>) {
					if constexpr (bAddChar) {
						r.push_back( MAS(0, uc::bitmask_3bit, 18) | MAS(1, uc::bitmask_6bit, 12) | MAS(2, uc::bitmask_6bit, 6) | MAS(3, uc::bitmask_6bit, 0) );
					}
					else {
						r += 1;
					}
				}
				else if constexpr (gtlc::is_same_utf<tchar_to, char16_t>) {
					if constexpr (bAddChar) {
						r += (uc::surrogate16h + (uint16_t)( ((pos[0] & uc::bitmask_3bit) << 8) | ((pos[1] & uc::bitmask_6bit) << 2) | ((pos[2]>> 4) & 0b0011) ));
						r += (uc::surrogate16l + (uint16_t)( ((pos[2] & uc::bitmask_4bit) << 6) | (pos[3] & uc::bitmask_6bit)) );
					}
					else {
						r += 2;
					}
				}
				else {
					static_assert(false);
				}
				pos += 4;
			}
			else {
				if constexpr (bCheckError) {
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				}
			}
		}


		//-------------------------------------------------------------------------------------------------------------------------
		/// @brief UTF16 -> UTF8, UTF32
		/// @tparam treturn 
		/// @param pos 
		/// @param end 
		/// @param r 
		template < gtlc::string_elem_utf tchar_to, gtlc::string_elem tchar_from = char16_t,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_to>, ptrdiff_t>
		>
		inline void UTFCharConverter_FromUTF16(tchar_from const*& pos, tchar_from const* const& end, treturn& r) {

			auto MAS = [&pos] (int i, tchar_to mask, int sh) -> tchar_to {	// MaskAndShift
				return (((tchar_to)pos[i]) & mask) << sh;
			};

			namespace uc = utf_const;

			char16_t c = pos[0];

			if constexpr (gtlc::is_same_utf<tchar_to, char8_t>) {
				if (c <= 0x7f) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(c));
					}
					else {
						r++;
					}
					pos += 1;
					return;
				}
				else if (c <= 0x07ff) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(0xc0 | ((c >> 6) & uc::bitmask_6bit)));
						r.push_back(static_cast<char8_t>(0x80 | (c& uc::bitmask_6bit)));
					}
					else {
						r += 2;
					}
					pos += 1;
					return;
				}
			}
			if ( (c < uc::fSurrogateW1.first) or (c > uc::fSurrogateW2.second) ) {
				if constexpr (gtlc::is_same_utf<tchar_to, char8_t>) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(0xe0 | ((c >> 12) & uc::bitmask_4bit)));		// 4 bits
						r.push_back(static_cast<char8_t>(0x80 | ((c >> 6) & uc::bitmask_6bit)));		// 6 bits
						r.push_back(static_cast<char8_t>(0x80 | (c & uc::bitmask_6bit)));				// 6 bits
					}
					else {
						r += 3;
					}
				}
				else if constexpr (gtlc::is_same_utf<tchar_to, char32_t>) {
					if constexpr (bAddChar) {
						r.push_back(c);
					}
					else {
						r += 1;
					}
				}
				else {
					static_assert(false);
				}
				pos += 1;
				return;
			}
			else if ( (c >= uc::fSurrogateW1.first) and (c <= uc::fSurrogateW1.second) ) {
				if constexpr (bCheckError) {
					if (((pos + 1) >= end)
						|| ((pos[1] < uc::fSurrogateW2.first) || (pos[1] > uc::fSurrogateW2.second))
						)
						throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid trailing surrogate" };
				}
				constexpr static uint32_t const pre = 0x1'0000 - ((uc::fSurrogateW1.first << uc::nBitSurrogate) + uc::fSurrogateW2.first);
				if constexpr (gtlc::is_same_utf<tchar_to, char8_t>) {
					if constexpr (bAddChar) {
						char32_t v = pre + (uint32_t)(pos[0] << 10) + pos[1];
						r.push_back(static_cast<char8_t>(0xf0 | ((v >> 18) & uc::bitmask_3bit)));		// 3 bits
						r.push_back(static_cast<char8_t>(0x80 | ((v >> 12) & uc::bitmask_6bit)));		// 6 bits
						r.push_back(static_cast<char8_t>(0x80 | ((v >> 6) & uc::bitmask_6bit)));		// 6 bits
						r.push_back(static_cast<char8_t>(0x80 | (v & uc::bitmask_6bit)));				// 6 bits
					}
					else {
						r += 4;
					}
				}
				else if constexpr (gtlc::is_same_utf<tchar_to, char32_t>) {
					if constexpr (bAddChar) {
						r.push_back(pre + (((uint32_t)(pos[0])) << 10) + pos[1]);
					}
					else {
						r += 1;
					}
				}
				else {
					static_assert(false);
				}
				pos += 2;
			}
			else {
				if constexpr (bCheckError) {
					throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid surrogate" };
				}
			}
		}


		//-------------------------------------------------------------------------------------------------------------------------
		/// @brief UTF32 -> UTF8, UTF16
		/// @tparam treturn 
		/// @param pos 
		/// @param end 
		/// @param r 
		template < gtlc::string_elem_utf tchar_to, gtlc::string_elem tchar_from = char32_t,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_to>, ptrdiff_t>
		>
		inline void UTFCharConverter_FromUTF32(tchar_from const*& pos, tchar_from const* const& end, treturn& r) {

			namespace uc = utf_const;
			auto c = pos[0];

			if constexpr (gtlc::is_same_utf<tchar_to, char8_t>) {
				if (c <= 0x7f) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(c));
					}
					else {
						r += 1;
					}
				}
				else if (c <= 0x07ff) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(0xc0 | ((c >> 6) & uc::bitmask_6bit)));
						r.push_back(static_cast<char8_t>(0x80 | (c & uc::bitmask_6bit)));
					}
					else {
						r += 2;
					}
				}
				else if (c <= 0xffff) /*((c < 0xd800) || ((c >= 0xe000) && (c <= 0xffff)))*/ {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(0xe0 | ((c >> 12) & uc::bitmask_4bit)));		// 4 bits
						r.push_back(static_cast<char8_t>(0x80 | ((c >> 6) & uc::bitmask_6bit)));		// 6 bits
						r.push_back(static_cast<char8_t>(0x80 | (c & uc::bitmask_6bit)));				// 6 bits
					}
					else {
						r += 3;
					}
				}
				else if (c <= 0x10'ffff) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(0xf0 | ((c >> 18) & uc::bitmask_3bit)));		// 3 bits
						r.push_back(static_cast<char8_t>(0x80 | ((c >> 12) & uc::bitmask_6bit)));		// 6 bits
						r.push_back(static_cast<char8_t>(0x80 | ((c >> 6) & uc::bitmask_6bit)));		// 6 bits
						r.push_back(static_cast<char8_t>(0x80 | (c & uc::bitmask_6bit)));				// 6 bits
					}
					else {
						r += 4;
					}
				}
				else {
					if constexpr (bCheckError) {
						throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf32 to utf16. 0x10FFFFU < c"};
					}
				}
			}
			else if constexpr (gtlc::is_same_utf<tchar_to, char16_t>) {
				if (c < uc::fSurrogateW1.first) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char16_t>(c));
					}
					else {
						r += 1;
					}
				}
				else if (c <= uc::fSurrogateW2.second) {
					if constexpr (bCheckError) {
						throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf32 to utf16. 0xD800u <= c <= 0xDFFFu"};
					}
				}
				else if (c <= 0xffffu) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char16_t>(c));
					}
					else {
						r += 1;
					}
				} else if (c <= 0x10'ffffu) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char16_t>(uc::surrogate16h + (c >> uc::nBitSurrogate)));
						r.push_back(static_cast<char16_t>(uc::surrogate16l + (c & 0b0011'1111'1111)));	// low 10 bits
					}
					else {
						r += 2;
					}
				} else {
					if constexpr (bCheckError) {
						throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf32 to utf16. 0x10FFFFU < c"};
					}
				}
			}

			pos += 1;
		}


		//-------------------------------------------------------------------------------------------------------------------------
		/// @brief UTF_x -> UTF_y
		/// @tparam treturn 
		/// @param pos 
		/// @param end 
		/// @param r 
		template < gtlc::string_elem_utf tchar_to, gtlc::string_elem tchar_from,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_to>, size_t>
		>
		inline void UTFCharConverter(tchar_from const*& pos, tchar_from const* const& end, treturn& r) {
			//if (pos >= end)
			//	return;
			namespace uc = utf_const;
			if constexpr (gtlc::is_same_utf<tchar_to, tchar_from>) {
				if constexpr (bAddChar) {
					r.push_back(*pos);
				}
				else {
					r += 1;
				}
				pos += 1;
			}
			else if constexpr (gtlc::is_same_utf<tchar_from, char8_t>) {
				UTFCharConverter_FromUTF8<tchar_to, tchar_from, bAddChar, bCheckError, treturn>(pos, end, r);
			}
			else if constexpr (gtlc::is_same_utf<tchar_from, char16_t>) {
				UTFCharConverter_FromUTF16<tchar_to, tchar_from, bAddChar, bCheckError, treturn>(pos, end, r);
			}
			else if constexpr (gtlc::is_same_utf<tchar_from, char32_t>) {
				UTFCharConverter_FromUTF32<tchar_to, tchar_from, bAddChar, bCheckError, treturn>(pos, end, r);
			}
			else {
				static_assert(false);
			}
		}

	}



	/// @brief UTFn -> UTFn coroutine. (experimental)
	/// @param svFrom source text
	/// @return converted text
	template < gtlc::string_elem_utf tchar_to, gtlc::string_elem_utf tchar_from >
	std::experimental::generator<tchar_to> SeqUTF(std::basic_string_view<tchar_from> svFrom) {
		namespace uc = gtl::utf_const;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		auto const* pos = svFrom.data();
		auto const* end = pos + svFrom.size();
		while (pos < end) {
			std::basic_string<tchar_to> str;
			internal::UTFCharConverter<tchar_to, tchar_from, true, true>(pos, end, str);
			for (auto c : str) {
				co_yield c;
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
