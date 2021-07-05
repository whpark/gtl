
//////////////////////////////////////////////////////////////////////
//
// utf_char_view.h : UTF 8/16/32 Characters one by one. (from any utf string.)
//
// PWH
// 2020.11.17.
// 2020.12.25. tchar_to for char8_t, char16_t, and char32_t
// 2020.12.28. 1st refactoring.
//
//////////////////////////////////////////////////////////////////////

module;

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:string_utf_char_view;
import :concepts;
import :string_primitives;
import :string_convert_utf;

namespace gtl {

	/// @brief UTF 8/16/32 Characters one by one. (from any utf string.)
	template < gtlc::string_elem_utf tchar_to, gtlc::string_elem_utf tchar_from, bool bCheckError = true/*, bool bThrow = true*/ >
	class utf_char_view {
	protected:
		std::basic_string_view<tchar_from> const sv_;
	public:
		utf_char_view(std::basic_string_view<tchar_from> sv) : sv_{sv} { }
		utf_char_view(std::basic_string<tchar_from> const& str) : sv_{str} { }
		utf_char_view() = delete;
		utf_char_view(utf_char_view const&) = default;
		utf_char_view(utf_char_view&&) = default;

		utf_char_view& operator = (utf_char_view const&) = default;

	public:
		class iterator {
		private:
			tchar_from const* position_;
			tchar_from const* const end_;
			std::basic_string<tchar_to> buffer_;
			size_t buf_index_{};
		public:
			iterator(tchar_from const* position, tchar_from const* const end) : position_(position), end_(end) {
				if constexpr (sizeof(tchar_from) != sizeof(char32_t)) {
					if (position_ >= end_) {
						buf_index_ = -1;
					} else {
						gtl::internal::UTFCharConverter<tchar_to, tchar_from, true, bCheckError, true>(position_, end_, buffer_);
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
				if constexpr (std::is_same_v<tchar_to, tchar_from>) {
					position_++;
				}
				else {
					if (buffer_.size() <= ++buf_index_) {
						if (position_ >= end_) {
							buf_index_ = -1;
						} else {
							buffer_.clear();
							gtl::internal::UTFCharConverter<tchar_to, tchar_from, true, bCheckError, true>(position_, end_, buffer_);
							buf_index_ = 0;
						}
					}
				}
				return *this;
			}
			tchar_to const& operator* () const {
				if constexpr (std::is_same_v<tchar_to, tchar_from>) {
					return *position_;
				}
				else {
					return buffer_[buf_index_];
				}
			}
		};

	public:
		iterator begin() const {
			return iterator{ sv_.data(), sv_.data() + sv_.size() };
		}
		iterator end() const {
			return iterator{ sv_.data() + sv_.size(), sv_.data() + sv_.size() };
		}

		size_t CountOutputLength() const {
			size_t count{};
			auto const* const end = sv_.data() + sv_.end();
			for (auto const* pos = sv_.data(); pos < end; ) {
				internal::UTFCharConverter<tchar_to, tchar_from, false, false, false>(pos, end, count);
			}
			return count;
		}

	};

}
