//module;
//
//#include "gtl/gtl.h"
//#include "gtl.ui.h"
//
//#define BOOST_JSON_STANDALONE
//#include "boost/json.hpp"
//
//
//export module gtl.ui:predefine;
//
//using namespace std::literals;
//using namespace gtl::literals;


#pragma once

#include "gtl/_default.h"
#include "gtl/string.h"

#define GText TEXT_u8


namespace gtl::ui {

	using char_type = char8_t;
	using string_t = std::basic_string<char_type>;
	using string_view_t = std::basic_string_view<char_type>;
	using xstring_t = gtl::TString<char_type>;
	using item_t = std::variant<int, string_t, std::vector<uint8_t>>;

	inline constexpr string_t GetDefaultFormatString(size_t nArg) {
		using namespace std::literals;
		string_t str;
		str.reserve(nArg*3);
		for (size_t i = 0; i < nArg; i++)
			str += GText("{} "s);
		return str;
	}

	template < typename ... Args >
	auto ToString(Args&& ... args) {
		return fmt::format(GetDefaultFormatString(sizeof...(args)), std::forward<Args>(args)...);
	}

	//! @brief translate string to html safe string ( '<' ==> "&lt;" )
	//! @param str 
	//! @return 
	template <bool bToHTMLSafeEntities>
	string_t TranslateHTMLChar(string_t str) {
		using namespace std::literals;
		static std::vector<std::pair<string_t, string_t>> const tbl{
			{ GText(" "s),	GText("&nbsp;"s) },
			{ GText("<"s),	GText("&lt;"s) },
			{ GText(">"s),	GText("&gt;"s) },
			//{ GText("&"s),	GText("&amp;"s) },
			{ GText("\""s),	GText("&quot;"s) },
			{ GText("\'"s),	GText("&apos;"s) },
		};

		xstring_t& strR = (xstring_t&)str;
		for (auto const& item : tbl) {
			if constexpr (bToHTMLSafeEntities) {
				strR.Replace(item.first, item.second);
			}
			else {
				strR.Replace(item.second, item.first);
			}
		}
		return (string_t&)str;
	}

};

