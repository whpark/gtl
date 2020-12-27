
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
