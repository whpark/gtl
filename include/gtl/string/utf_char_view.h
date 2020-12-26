
//////////////////////////////////////////////////////////////////////
//
// utf_char_view.h : utf8/16/32 string view. composing 'each' character (returning char32_t or char16_t)
//
// PWH
// 2020.11.17.
// 2020.12.25. tchar_return for char8_t, char16_t, and char32_t
//
//////////////////////////////////////////////////////////////////////

#pragma once

#if 1
#ifndef GTL_HEADER__STRING_UTF_STRING_VIEW
#define GTL_HEADER__STRING_UTF_STRING_VIEW

#include <experimental/generator>

#include "string_primitives.h"
#include "convert_utf.h"

namespace gtl {
#pragma pack(push, 8)

	namespace internal {

		//-------------------------------------------------------------------------------------------------------------------------
		/// @brief UTF8 -> UTF16, UTF32
		/// @tparam treturn 
		/// @param pos 
		/// @param end 
		/// @param r 
		template < gtlc::string_elem_utf tchar_return, gtlc::string_elem tchar = char8_t,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_return>, ptrdiff_t>
		>
		inline void UTFCharConverter_FromUTF8(tchar const*& pos, tchar const* const& end, treturn& r) {

			auto MAS = [&pos] (int i, tchar_return mask, int sh) -> tchar_return {	// MaskAndShift
				return (((tchar_return)pos[i]) & mask) << sh;
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
				if constexpr (std::is_same_v<tchar_return, char32_t>) {
					if constexpr (bAddChar) {
						r.push_back( MAS(0, uc::bitmask_3bit, 18) | MAS(1, uc::bitmask_6bit, 12) | MAS(2, uc::bitmask_6bit, 6) | MAS(3, uc::bitmask_6bit, 0) );
					}
					else {
						r += 1;
					}
				}
				else if constexpr (std::is_same_v<tchar_return, char16_t>) {
					if constexpr (bAddChar) {
						constexpr static uint16_t const preH = uc::fSurrogateW1.first - (0x1'0000u >> uc::nBitSurrogate);
						constexpr static uint16_t const preL = uc::fSurrogateW2.first;

						r += (preH + (uint16_t)( ((pos[0] & uc::bitmask_3bit) << 8) | ((pos[1] & uc::bitmask_6bit) << 2) | ((pos[2]>> 4) & 0b0011) ));
						r += (preL + (uint16_t)( ((pos[2] & uc::bitmask_4bit) << 6) | (pos[3] & uc::bitmask_6bit)) );
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
		template < gtlc::string_elem_utf tchar_return, gtlc::string_elem tchar = char16_t,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_return>, ptrdiff_t>
		>
		inline void UTFCharConverter_FromUTF16(tchar const*& pos, tchar const* const& end, treturn& r) {

			auto MAS = [&pos] (int i, tchar_return mask, int sh) -> tchar_return {	// MaskAndShift
				return (((tchar_return)pos[i]) & mask) << sh;
			};

			namespace uc = utf_const;

			char16_t c = pos[0];

			if constexpr (std::is_same_v<tchar_return, char8_t>) {
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
				if constexpr (std::is_same_v<tchar_return, char8_t>) {
					if constexpr (bAddChar) {
						r.push_back(static_cast<char8_t>(0xe0 | ((c >> 12) & uc::bitmask_4bit)));		// 4 bits
						r.push_back(static_cast<char8_t>(0x80 | ((c >> 6) & uc::bitmask_6bit)));		// 6 bits
						r.push_back(static_cast<char8_t>(0x80 | (c & uc::bitmask_6bit)));				// 6 bits
					}
					else {
						r += 3;
					}
				}
				else if constexpr (std::is_same_v<tchar_return, char32_t>) {
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
				if constexpr (std::is_same_v<tchar_return, char8_t>) {
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
				else if constexpr (std::is_same_v<tchar_return, char32_t>) {
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
		template < gtlc::string_elem_utf tchar_return, gtlc::string_elem tchar = char32_t,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_return>, ptrdiff_t>
		>
		inline void UTFCharConverter_FromUTF32(tchar const*& pos, tchar const* const& end, treturn& r) {

			namespace uc = utf_const;
			auto c = pos[0];

			if constexpr (std::is_same_v<tchar_return, char8_t>) {
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
			else if constexpr (std::is_same_v<tchar_return, char16_t>) {
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
						r.push_back(static_cast<char16_t>(0xd7c0u + (c >> 10)));
						r.push_back(static_cast<char16_t>(0xdc00u + (c & 0x3ffu)));
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
		template < gtlc::string_elem_utf tchar_return, gtlc::string_elem tchar,
			bool bAddChar = true, bool bCheckError = true,
			typename treturn = std::conditional_t<bAddChar, std::basic_string<tchar_return>, size_t>
		>
		inline void UTFCharConverter(tchar const*& pos, tchar const* const& end, treturn& r) {
			//if (pos >= end)
			//	return;
			namespace uc = utf_const;
			if constexpr (std::is_same_v<tchar_return, tchar>) {
				if constexpr (bAddChar) {
					r.push_back(*pos);
				}
				pos += 1;
			}
			else if constexpr (std::is_same_v<tchar, char8_t>) {
				UTFCharConverter_FromUTF8<tchar_return, char8_t, bAddChar, bCheckError, treturn>(pos, end, r);
			}
			else if constexpr (std::is_same_v<tchar, char16_t>) {
				UTFCharConverter_FromUTF16<tchar_return, char16_t, bAddChar, bCheckError, treturn>(pos, end, r);
			}
			else if constexpr (std::is_same_v<tchar, char32_t>) {
				UTFCharConverter_FromUTF32<tchar_return, char32_t, bAddChar, bCheckError, treturn>(pos, end, r);
			}
			else {
				static_assert(false);
			}
		}

	}



	/// @brief char32_t from utf8/16 string. (yet, NOT having std::generator...)
	template < gtlc::string_elem_utf tchar_return, gtlc::string_elem_utf tchar, bool bCheckError = true >
	class utf_char_view {
	protected:
		std::basic_string_view<tchar> sv_;
	public:
		utf_char_view(std::basic_string_view<tchar> sv) : sv_{sv} { }
		utf_char_view(std::basic_string<tchar> const& str) : sv_{str} { }
		utf_char_view() = default;
		utf_char_view(utf_char_view const&) = default;
		utf_char_view(utf_char_view&&) = default;

		utf_char_view& operator = (utf_char_view const&) = default;

	public:
		class iterator {
		private:
			tchar const* position_;
			tchar const* const end_;
			//int diff_to_next_{};		// 
			//tchar_return code_{};
			std::basic_string<tchar_return> buffer_;
			size_t buf_index_{};
		public:
			iterator(tchar const* position, tchar const* const end) : position_(position), end_(end) {
				if constexpr (sizeof(tchar) != sizeof(char32_t)) {
					if (position_ >= end_) {
						buf_index_ = -1;
					} else {
						gtl::internal::UTFCharConverter<tchar_return, tchar, true, bCheckError>(position_, end_, buffer_);
						buf_index_ = 0;
					}
				}
			}
		protected:
			void update() {
				//namespace uc = utf_const;
				//if (position_ >= end_)
				//	return;

				//if constexpr (sizeof(tchar) == sizeof(char8_t)) {
				//	// utf8
				//	if (position_[0] <= 0x7f) {
				//		code_ = position_[0];
				//		diff_to_next_ = 1;
				//	}
				//	else if ((position_[0] & 0b1110'0000) == 0b1100'0000) {	// 0~0x7ff
				//		if ((position_ + 1 >= end_)
				//			|| ((position_[1] & 0b1100'0000) != 0b1000'0000)
				//			)
				//			throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				//		code_ = ((position_[0] & uc::bitmask_6bit) << 6) | (position_[1] & uc::bitmask_6bit);
				//		diff_to_next_ = 2;
				//	}
				//	else if ((position_[0] & 0b1111'0000) == 0b1110'0000) {	// ~0xffff
				//		if ((position_ + 2 >= end_)
				//			|| ((position_[1] & 0b1100'0000) != 0b1000'0000)
				//			|| ((position_[2] & 0b1100'0000) != 0b1000'0000)
				//			)
				//			throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				//		code_ = ((position_[0] & uc::bitmask_4bit) << 12) | ((position_[1] & uc::bitmask_6bit) << 6) | (position_[2] & uc::bitmask_6bit);
				//		diff_to_next_ = 3;
				//	}
				//	else if ((position_[0] & 0b1111'1000) == 0b1111'0000) {	// ~0x10'ffff
				//		if ((position_ + 3 >= end_)
				//			|| ((position_[1] & 0b1100'0000) != 0b1000'0000)
				//			|| ((position_[2] & 0b1100'0000) != 0b1000'0000)
				//			|| ((position_[3] & 0b1100'0000) != 0b1000'0000)
				//			)
				//			throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				//		code_ = ((position_[0] & uc::bitmask_3bit) << 18) | ((position_[1] & uc::bitmask_6bit) << 12) | ((position_[2] & uc::bitmask_6bit) << 6) | (position_[3] & uc::bitmask_6bit);
				//		diff_to_next_ = 4;
				//	}
				//	else {
				//		throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				//	}
				//}
				//else if constexpr (sizeof(tchar) == sizeof(char16_t)) {
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
				//else if constexpr (sizeof(tchar) == sizeof(char32_t)) {
				//	code_ = *position_;
				//	diff_to_next_ = 1;
				//}
			}

		public:
			//auto operator == (iterator const& b) const {
			//	return position_ == b.position_;
			//}
			auto operator != (iterator const& b) const {
				return (position_ != b.position_) or (buf_index_ != b.buf_index_);
			}
			iterator& operator++ () {
				if constexpr (std::is_same_v<tchar_return, tchar>) {
					position_++;
				}
				else {
					if (buffer_.size() <= ++buf_index_) {
						if (position_ >= end_) {
							buf_index_ = -1;
						} else {
							buffer_.clear();
							gtl::internal::UTFCharConverter<tchar_return, tchar, true, bCheckError>(position_, end_, buffer_);
							buf_index_ = 0;
						}
					}
				}
				return *this;
			}
			tchar_return const& operator* () const {
				if constexpr (std::is_same_v<tchar_return, tchar>) {
					return *position_;
				}
				else {
					return buffer_[buf_index_];
				}
			}
		};

	public:
		iterator begin() {
			return iterator{ sv_.data(), sv_.data() + sv_.size() };
		}
		iterator end() {
			return iterator{ sv_.data() + sv_.size(), sv_.data() + sv_.size() };
		}

		size_t CountOutputLength() {
			size_t count{};
			auto const* const end = sv_.data() + sv_.end();
			for (auto const* pos = sv_.data(); pos < end; ) {
				internal::UTFCharConverter<tchar_return, tchar, false, false>(pos, end, count);
			}
			return count;
		}

	};

#pragma pack(pop)
}

#endif // GTL_HEADER__STRING_UTF_STRING_VIEW


#endif
