
//////////////////////////////////////////////////////////////////////
//
// utf_string_view.h : utf8/16/32 string view. composing 'each' character (returning char32_t or char16_t)
//
// PWH
// 2020.11.17.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING_UTF_STRING_VIEW
#define GTL_HEADER__STRING_UTF_STRING_VIEW

#include "string_primitives.h"

#include <experimental/generator>

namespace gtl {
#pragma pack(push, 8)


	/// <summary>
	/// constants for utf conversion
	/// </summary>
	namespace utf_const {
		constexpr static inline uint8_t const mask_3bit = 0b0000'0111;
		constexpr static inline uint8_t const mask_4bit = 0b0000'1111;
		constexpr static inline uint8_t const mask_6bit = 0b0011'1111;
		constexpr static inline std::pair const fSurrogateW1{ (char16_t)0xd800u, (char16_t)0xdbffu };	// 0xd800u ~ 0xdbffu;
		constexpr static inline std::pair const fSurrogateW2{ (char16_t)0xdc00u, (char16_t)0xdfffu };	// 0xd800u ~ 0xdbffu;
		constexpr static inline int const nBitSurrogate{ 10 };
	}

	//--------------------------------------------------------------------------------------------------------------------------------
	/// <summary>
	/// char32_t character from utf8/16 string. (yet, NOT having std::generator...)
	///   someday, upgrate to std::generator std::generator<tchar_return_t> ...
	/// </summary>
	template < gtlc::utf_string_elem tchar_t >
	class utf_string_view {
	public:
		using utf_char_return_t = char32_t;

	protected:
		std::basic_string_view<tchar_t> sv_;

	public:
		utf_string_view(std::basic_string_view<tchar_t> sv) : sv_(sv) { }
		utf_string_view(std::basic_string<tchar_t> const& str) : sv_(str.data(), str.data() + str.size()) { }
		utf_string_view() = default;
		utf_string_view(utf_string_view const&) = default;
		utf_string_view(utf_string_view&&) = default;

		utf_string_view& operator = (utf_string_view const&) = default;

		size_t CountOutputLength() {
			if constexpr (sizeof(tchar_t) == sizeof(char8_t)) {
				// utf8
				size_t count{};
				auto const* const end = sv_.data() + sv_.end();
				for (auto const* pos = sv_.data(); pos < end; pos++) {
					auto c = *pos;
					if (c <= 0x7f) {
					}
					else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7ff
						pos += 1;
					}
					else if ((c & 0b1111'0000) == 0b1110'0000) {	// ~0xffff
						pos += 2;
					}
					else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0x10'ffff
						pos += 3;
					}
					count++;
				}
				return count;
			}
			else if constexpr (sizeof(tchar_t) == sizeof(char16_t)) {
				// utf16
				size_t count{};
				auto const* const end = sv_.data() + sv_.end();
				for (auto const* pos = sv_.data(); pos < end; pos++) {
					auto c = *pos;
					if ((utf_const::fSurrogateW1.first <= c) && (c <= utf_const::fSurrogateW1.second)) {	// ~0x10'ffff
						pos += 1;
					}
					count++;
				}
				return count;
			}
			else if constexpr (sizeof(tchar_t) == sizeof(char32_t)) {
				// utf32
				return sv_.size();
			}
		}

	public:
		class iterator {
		private:
			tchar_t const* position_;
			tchar_t const* const end_;
			int diff_to_next_{};		// 
			utf_char_return_t code_{};
		public:
			iterator(tchar_t const* position, tchar_t const* const end) : position_(position), end_(end) {
				if constexpr (sizeof(tchar_t) != sizeof(char32_t)) {
					update();
				}
			}
		protected:
			void update() {
				namespace uc = utf_const;
				if (position_ >= end_)
					return;
				if constexpr (sizeof(tchar_t) == sizeof(char8_t)) {
					// utf8
					if (position_[0] <= 0x7f) {
						code_ = position_[0];
						diff_to_next_ = 1;
					}
					else if ((position_[0] & 0b1110'0000) == 0b1100'0000) {	// 0~0x7ff
						if ((position_ + 1 >= end_)
							|| ((position_[1] & 0b1100'0000) != 0b1000'0000)
							)
							throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
						code_ = ((position_[0] & uc::mask_6bit) << 6) | (position_[1] & uc::mask_6bit);
						diff_to_next_ = 2;
					}
					else if ((position_[0] & 0b1111'0000) == 0b1110'000) {	// ~0xffff
						if ((position_ + 2 >= end_)
							|| ((position_[1] & 0b1100'0000) != 0b1000'0000)
							|| ((position_[2] & 0b1100'0000) != 0b1000'0000)
							)
							throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
						code_ = ((position_[0] & uc::mask_4bit) << 12) | ((position_[1] & uc::mask_6bit) << 6) | (position_[2] & uc::mask_6bit);
						diff_to_next_ = 3;
					}
					else if ((position_[0] & 0b1111'1000) == 0b1111'0000) {	// ~0x10'ffff
						if ((position_ + 3 >= end_)
							|| ((position_[1] & 0b1100'0000) != 0b1000'0000)
							|| ((position_[2] & 0b1100'0000) != 0b1000'0000)
							|| ((position_[3] & 0b1100'0000) != 0b1000'0000)
							)
							throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
						code_ = ((position_[0] & uc::mask_3bit) << 18) | ((position_[1] & uc::mask_6bit) << 12) | ((position_[2] & uc::mask_6bit) << 6) | (position_[3] & uc::mask_6bit);
						diff_to_next_ = 4;
					}
					else {
						throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
					}
				}
				else if constexpr (sizeof(tchar_t) == sizeof(char16_t)) {
					// utf16
					if ((position_[0] < uc::fSurrogateW1.first) || (position_[0] > uc::fSurrogateW2.second)) [[ likely ]] {
						code_ = position_[0];
						diff_to_next_ = 1;
					}
					else if (position_[0] <= uc::fSurrogateW2.first) [[unlikely]] { // leading surrogate
						if (((position_ + 1) >= end_)
							|| ((position_[1] < uc::fSurrogateW2.first) || (position_[1] > uc::fSurrogateW2.second))
							)
							throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid trailing surrogate" };

						constexpr static uint32_t const pre = 0x1'0000 - ((uc::fSurrogateW1.first << uc::nBitSurrogate) + uc::fSurrogateW2.first);
						code_ = (pre + (position_[0] << 10) + position_[1]);
						diff_to_next_ = 2;
					}
					else {
						throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid surrogate" };
					}
				}
				else if constexpr (sizeof(tchar_t) == sizeof(char32_t)) {
					code_ = *position_;
					diff_to_next_ = 1;
				}
			}

		public:
			//auto operator == (iterator const& b) const {
			//	return position_ == b.position_;
			//}
			auto operator != (iterator const& b) const {
				return position_ != b.position_;
			}
			iterator& operator++ () {
				if constexpr (std::is_same_v<tchar_t, char32_t>) {
					position_++;
				}
				else {
					auto len = std::exchange(diff_to_next_, 0);
					code_ = 0;
					position_ += len;

					update();
				}

				return *this;
			}
			char32_t const& operator* () const {
				if constexpr (std::is_same_v<tchar_t, char32_t>) {
					return *position_;
				}
				else {
					return code_;
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

	};

#pragma pack(pop)
}

#endif // GTL_HEADER__STRING_UTF_STRING_VIEW
