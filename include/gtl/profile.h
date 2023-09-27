///////////////////////////////////////////////////////////////////////////////
//
// profile.h
//
//			GTL : (G)reen (T)ea (L)atte
//
// 2023.09.27. from Mocha
//
// PWH
//
///////////////////////////////////////////////////////////////////////////////


#pragma once

#include "gtl/_config.h"
#include "gtl/_default.h"
#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"
#include "gtl/string.h"
#include "gtl/vector_map.h"

namespace gtl {

	// utf-8 string

	template < gtlc::string_elem tchar >
	class TProfileSection {
	public:
		using this_t = TProfileSection<tchar>;
		using char_type = tchar;
		using string_t = std::basic_string<tchar>;

		// nested item class
		class sItem {
		public:
			using this_t = sItem;
			using char_type = tchar;
			using string_t = std::basic_string<tchar>;

		public:
			string_t name;
			string_t value;
			string_t comment;
			int iCommentPos{ -1 };

			auto operator <=> (this_t const&) const noexcept = default;

			void Clear() { name.clear(); value.clear(); comment.clear(); iCommentPos = -1; }

		public:
			//void SetFromString(std::basic_string_view<tchar> str, tchar cDelimeter = '=', tchar cComment = ';') {
			//	Clear();
			//	auto i = str.begin();
			//	// skip white spaces
			//	for (; i != str.end() and gtl::IsSpace(*i); i++)
			//		;
			//	// name
			//	for (; i != str.end() and *i != cDelimeter and *i != cComment; i++)
			//		name += *i;
			//	// skip white spaces
			//	for (; i != str.end() and gtl::IsSpace(*i); i++)
			//		;
			//	for (; i != str.end() and *i != cComment; i++)
			//		value += *i;
			//}
		};

	protected:
		string_t m_name;
		std::unordered_map<string_t, this_t> m_sections;
		std::unordered_map<string_t, sItem> m_items;
	};

	class GTL__CLASS xProfile {
	public:
		using this_t = xProfile;
	};


}	// namespace gtl
