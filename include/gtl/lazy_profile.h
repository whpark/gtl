///////////////////////////////////////////////////////////////////////////////
//
// lazy_profile.h
//
//			GTL : (G)reen (T)ea (L)atte
//
// 2023.09.27. lazy ini file parser(also writer), with comments
//             to preserve original file contents, just read lines and parses later.
//             but, trailing spaces are trimmed for convenience..
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
#include "gtl/archive.h"

namespace gtl {

	// utf-8 string

	//=============================================================================================================================
	//
	template < gtlc::string_elem tchar >
	class TLazyProfileSection {
	public:
		using this_t = TLazyProfileSection<tchar>;
		using char_type = tchar;
		using string_t = std::basic_string<tchar>;
		using string_view_t = std::basic_string_view<tchar>;

	public:
		void Clear() {
			m_sections.clear();
			m_items.clear();
		}

		string_view_t GetItem_String(string_view_t name) const {
			for (auto& item : m_items) {
				auto c = item.begin();
				for (; c != item.end() and std::isspace(*c); c++)
					;
				string_view_t sv(c, item.end());
				if (!sv.starts_with(name))
					continue;
				if (auto c2 = sv[name.size()];
					std::isspace(c2) or c2 == '=') {
					sv.remove_prefix(name.size());
					if (auto pos = sv.find('='); pos != sv.npos) {
						sv.remove_prefix(pos+1);
						return sv;
					}
				}
				return {};
			}
			return {};
		}

		template < std::integral tvalue >
		tvalue GetItemValue(string_view_t name) const {
			auto sv = GetItem_String(name);
			if (sv.empty())
				return 0;
			return gtl::FromString<tvalue>(sv);
		}

		template < std::convertible_to<string_t> tstring >
		bool SetItem(string_view_t name, tstring&& str) {
			std::forward<tstring>(str);
			return false;
		}
		bool SetItem(string_view_t name, std::integral auto&& item) {
			return false;
		}
		bool SetItem(string_view_t name, std::floating_point auto&& item) {
			return false;
		}
		bool SetItem(string_view_t name, bool item) {
		}

	protected:
		//string_t m_name;
		TVectorMap<string_t, this_t> m_sections;
		std::vector<string_t> m_items;
	};

	//=============================================================================================================================
	//
	template < gtlc::string_elem tchar >
	class TLazyProfile : public TLazyProfileSection<tchar> {
	public:
		using this_t = TProfile;
		using base_t = TLazyProfileSection<tchar>;
		using string_t = typename base_t::string_t;

	public:
		bool Load(std::istream& stream) {
			base_t::Clear();
			TArchive<std::istream> ar(stream);
			ar.ReadCodepageBOM();

			TLazyProfileSection<tchar>* section = &m_sections[""];
			while (auto rstr = ar.ReadLine<tchar>()) {
				string_t& str = *rstr;
				auto c = str.begin();
				// skip space
				for (; c != str.end() and std::isspace(*c); c++)
					;
				if (c == str.end())
					continue;

				if (*c == '[') {
					// section
					string_t name;
					for (; c != str.end() and c != ']'; c++)
						name += *c;
					gtl::Trim(name);
					section = &m_sections[name];

					// comment
					for (; c != str.end() and *c != ';'; c++)
						;
					if (c != str.end() and *c == ';') {
						auto& item = section->m_items["[]"];
						item.comment = string_t(c + 1, str.end());
						item.iCommentPos = c - str.begin();
					}
					continue;
				}

				// item
				section->m_items.push_back(std::move(str));
			}
			return false;
		}
	};


}	// namespace gtl
