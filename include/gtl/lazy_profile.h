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
#include "gtl/container_map.h"
#include "gtl/archive.h"

#include "ctre.hpp"
#include "ctre-unicode.hpp"

namespace gtl {

	// utf-8 string

	//=============================================================================================================================
	//
	template < gtlc::string_elem tchar,
		bool bCASE_SENSITIVE = false,
		bool bTREAT_BOOL_AS_INT = false,
		bool bSTRING_BE_QUOTED = false
		/*, bool bTRANSLATE_ESCAPE_SEQUENCE_STRING_VALUE = false*/
	>
	class TLazyProfile {
	public:
		using this_t = TLazyProfile;
		using char_t = tchar;
		using string_t = std::basic_string<tchar>;
		using string_view_t = std::basic_string_view<tchar>;
		struct sCompareString {
			bool operator()(std::basic_string_view<tchar> a, std::basic_string_view<tchar> b) const {
				if constexpr (bCASE_SENSITIVE) {
					return a == b;
				}
				else {
					return tszicmp(a, b) == 0;
				}
			}
		};
		using map_t = TContainerMap<std::vector, string_t, this_t, sCompareString>;
		using section_t = this_t;

		static constexpr inline bool bCaseSensitive = bCASE_SENSITIVE;
		static constexpr inline bool bTreatBoolAsInt = bTREAT_BOOL_AS_INT;
		static constexpr inline bool bStringBeQuoted = bSTRING_BE_QUOTED;
		//static constexpr inline bool bTranslateEscapeSequenceStringValue = bTRANSLATE_ESCAPE_SEQUENCE_STRING_VALUE;

		//regex for section name
		// key : any trimmed(whitespace) chars quoted by bracket "[]", ex) [ HeadDriver1:1 ]
		// trailing : any string after section name
		static constexpr inline auto s_reSection = ctre::match<R"xxx(\s*\[\s*([^\]]*[^\]\s]+)\s*\](.*))xxx">;

		// regex for item
		// key : any string except '=', including space
		// value : 1. any string except ';'.
		//         2. if value starts with '"', it can contain any character except '"'.
		// comment : any string after ';', including space.
		static constexpr inline auto s_reItem = ctre::match<R"xxx(\s*([\w\s]*\w+)\s*(=)\s*("(?:[^\\"]|\\.)*"|[^;\n]*)\s*[^;]*(;.*)?)xxx">;

		template < bool bDO_NOT_QUOTE_STRING = false >
		struct TLazyProfileValue {
			section_t& rThis;
			string_view_t key;

			template < typename tvalue >
			TLazyProfileValue& operator = (tvalue&& v) {
				rThis.SetItemValue<tvalue, bDO_NOT_QUOTE_STRING>(key, std::forward<tvalue>(v));
				return *this;
			}
		};
	protected:
		//string_t m_key;
		map_t m_sections;	// child sections
		std::vector<string_t> m_items;
		string_t m_line;	// anything after section name

	//protected:
	//	constexpr static inline int posEQDefault = 25;
	//	constexpr static inline int posCommentDefault = 80;
	//	mutable int m_posEQ {posEQDefault};
	//	mutable int m_posComment {posCommentDefault};

	public:
		TLazyProfile() = default;
		TLazyProfile(TLazyProfile const&) = default;
		TLazyProfile(TLazyProfile&&) = default;
		TLazyProfile& operator=(TLazyProfile const&) = default;
		TLazyProfile& operator=(TLazyProfile&&) = default;

		bool operator == (this_t const& ) const = default;
		bool operator != (this_t const& ) const = default;

		// section
		TLazyProfile& operator[](string_view_t key) {
			return m_sections[key];
		}
		// section
		TLazyProfile const& operator[](string_view_t key) const {
			return m_sections[key];
		}

		// getter
		template < typename tvalue >
		auto operator()(string_view_t key, tvalue&& vDefault = tvalue{}) const {
			return GetItemValue(key, std::forward<tvalue>(vDefault));
		}
		// getter
		string_view_t operator()(string_view_t key) const {
			return GetItemValueRaw(key);
		}
		// setter
		template < bool bDO_NOT_QUOTE_STRING = false >
		auto operator()(string_view_t key) {
			TLazyProfileValue<bDO_NOT_QUOTE_STRING> valueProxy{*this, key};
			return valueProxy;
		}

		void Clear() {
			m_sections.clear();
			m_items.clear();
			m_line.clear();
		}

		section_t& GetSection(string_view_t key) { return m_sections[key]; }
		section_t const& GetSection(string_view_t key) const {
			if (auto iter = m_sections.find(key); iter != m_sections.end())
				return iter->second;
			static section_t const dummy;
			return dummy;
		}

		auto& GetSections() { return m_sections; }
		auto const& GetSections() const { return m_sections; }

		auto& GetRawItems() { return m_items; }
		auto const& GetRawItems() const { return m_items; }

		auto GetItemsView() const {
			struct sItem { string_view_t key, value; };
			std::vector<sItem> items;
			items.reserve(m_items.size());
			for (auto const& item : m_items) {
				auto [whole, key1, eq1, value1, comment1] = s_reItem(item);
				if (!whole)
					continue;
				string_view_t key{key1.begin(), key1.end()};
				string_view_t value{value1.begin(), value1.end()};
				key = gtl::TrimView(key);
				value = gtl::TrimView(value);
				if constexpr (bStringBeQuoted) { DeQuote(value); }
				items.emplace_back(key, value);
			}
			return items;
		}

		string_view_t GetItemValueRaw(string_view_t key) const {
			for (auto const& item : m_items) {
				auto [whole, key1, eq1, value1, comment1] = s_reItem(item);
				if (!whole)
					continue;
				if (sCompareString{}(string_view_t{key1.begin(), key1.end()}, key) == 0)
					return string_view_t{value1.begin(), value1.end()};
				//return {};
			}
			return {};
		}

		void SetItemValueRaw(string_view_t key, string_view_t value, string_view_t comment={}/* comment starting with ';'*/) {
			// if comment does not start with ';', add one.
			string_t cmt;
			if (!comment.empty() and !comment.starts_with(';')) {
				cmt.reserve(comment.size() + 1);
				cmt = ';';
				cmt += comment;
				comment = cmt;
			}

			int posEQ{-1};
			int posComment{-1};
			for (auto& item : m_items) {
				auto [whole, key1, eq1, value1, comment1] = s_reItem(item);
				if (!whole)
					continue;
				posEQ = std::max(posEQ, (int)(eq1.begin() - whole.begin()));
				posComment = std::max(posComment, comment1 ? (int)(comment1.begin() - whole.begin()) : 0);
				if (sCompareString{}(string_view_t{key1}, key) != 0)
					continue;
				if (comment.empty() and comment1)
					comment = comment1;
				//int starting = key1.begin() - whole.begin();
				auto str = FormatToTString<tchar, "{}{:<{}}{}">(
					string_view_t(whole.begin(), value1.begin()), string_view_t(value),
					comment.empty() ? value.size()
						: ( comment1 ? (comment1.begin()-value1.begin()) : std::max(0, (int)(posComment - (value1.begin()-whole.begin()))) ),
					comment);
				item = std::move(str);
				return;
			}
			auto str = FormatToTString<tchar, "{:{}}= {}">(
				string_view_t(key), std::max((int)key.size(), (int)posEQ),
				string_view_t(value));
			if (!comment.empty()) {
				str += FormatToTString<tchar, "{:{}}{}">(
					tchar(';'), std::max(0, posComment - (int)str.size()),
					comment);
			}
			if (m_items.empty()) {
				m_items.push_back(std::move(str));
			}
			else {
				// let empty lines behinde.
				for (auto iter = m_items.rbegin(); iter != m_items.rend(); iter++) {
					auto const& cur = *iter;
					if (gtl::TrimView<tchar>(cur).empty())
						continue;

					m_items.insert(m_items.begin() + std::distance(iter, m_items.rend()), std::move(str));
					break;
				}
			}
		}

		void SetItemComment(string_view_t key, string_view_t comment) {
			string_t value = GetItemValueRaw(key);
			SetItemValueRaw(key, value, comment);
		}

		auto HasItem(string_view_t key) const {
			auto sv = GetItemValueRaw(key);
			sv = gtl::TrimView(sv);
			return !sv.empty();
		}

		/// @brief Get Item Value
		template < typename tvalue >
		auto GetItemValue(string_view_t key, tvalue&& vDefault = tvalue{}) const -> tvalue {
			auto sv = GetItemValueRaw(key);
			if (sv.empty())
				return vDefault;
			sv = gtl::TrimView(sv);
			if constexpr (std::is_same_v<tvalue, bool>) {
				if (std::isdigit(sv[0]))
					return (gtl::tsztod(sv) == 0.0) ? false : true;
				return gtl::tsznicmp(sv, string_view_t{gtl::TStringLiteral<char_t, "true">().value}, 4) == 0;
			}
			else if constexpr (std::is_integral_v<tvalue>) {
				return gtl::tsztoi<tvalue>(sv);
			}
			else if constexpr (std::is_floating_point_v<tvalue>) {
				return gtl::tsztod<tvalue>(sv);
			}
			else if constexpr (std::is_same_v<tvalue, string_view_t>) {
				if constexpr (bStringBeQuoted) { DeQuote(sv); }
				return sv;
			}
			else if constexpr (std::is_convertible_v<tvalue, string_view_t>) {
				if constexpr (bStringBeQuoted) { DeQuote(sv); }
				return tvalue{sv};
			}
			else {
				static_assert(gtlc::dependent_false_v<tvalue>);
			}
			return {};
		}

		/// @brief Set Item Value
		template < typename tvalue, bool bDO_NOT_QUOTE_STRING = false >
		void SetItemValue(string_view_t key, tvalue const& value, string_view_t comment = {}) {
			if constexpr (bTreatBoolAsInt and std::is_same_v<std::remove_cvref_t<tvalue>, bool>) {
				SetItemValue(key, value ? 1 : 0, comment);
			}
			else if constexpr (std::is_convertible_v<tvalue, string_view_t>) {
				if constexpr (bStringBeQuoted and !bDO_NOT_QUOTE_STRING) {
					SetItemValueRaw(key, Quote(value), comment);
				}
				else {
					SetItemValueRaw(key, value, comment);
				}
			}
			else {
				SetItemValueRaw(key, fmt::format(GetDefaultFormatString<tchar>(), value), comment);
			}
		}

		/// @brief Sync
		template < typename tvalue, bool bDO_NOT_QUOTE_STRING = false >
		void SyncItemValue(bool bToProfile, string_view_t key, tvalue& value) {
			if (bToProfile) {
				SetItemValue<tvalue, bDO_NOT_QUOTE_STRING>(key, value);
			} else {
				//if (HasItemValue(key))
				value = GetItemValue(key, value);
			}
		}
		template < bool bToProfile, typename tvalue, bool bDO_NOT_QUOTE_STRING = false >
		void SyncItemValue(string_view_t key, tvalue& value) {
			if constexpr (bToProfile) {
				SetItemValue<tvalue, bDO_NOT_QUOTE_STRING>(key, value);
			} else {
				//if (HasItemValue(key))
				value = GetItemValue(key, std::forward<tvalue>(value));
			}
		}

		void DeleteItemsIf(std::function<bool(string_view_t /*key*/, string_view_t /*value*/)> funcIsItemDeprecated) {
			auto Check = [&](auto const& item) {
				auto [whole, key1, eq1, value1, comment1] = s_reItem(item);
				if (!whole)
					return false;
				auto key = gtl::TrimView(string_view_t{key1.begin(), key1.end()});
				auto value = gtl::TrimView(string_view_t{value1.begin(), value1.end()});
				return funcIsItemDeprecated(key, value);
			};
			auto iter = std::remove_if(m_items.begin(), m_items.end(), Check);
			m_items.erase(iter, m_items.end());
		}
		void DeleteSectionsIf(std::function<bool(section_t& /*section*/)> funcIsSectionDeprecated) {
			auto iter = std::remove_if(m_sections.begin(), m_sections.end(), funcIsSectionDeprecated);
			m_items.erase(iter, m_items.end());
		}

	public:
		bool Load(std::filesystem::path const& path) {
			std::ifstream stream(path, std::ios::binary);
			return Load(stream);
		}
		bool Save(std::filesystem::path const& path, bool bWriteBOM = !std::is_same_v<tchar, char>) const {
			std::ofstream stream(path, std::ios::binary);
			return Save(stream, bWriteBOM);
		}

		bool Load(std::istream& stream) try {
			Clear();
			m_sections.base().emplace_back();
			TArchive<std::istream> ar(stream);
			ar.ReadCodepageBOM();

			section_t* section = &m_sections.front().second;
			while (auto rstr = ar.ReadLine<tchar>()) {
				string_t& str = *rstr;

				// section name
				if (auto [whole, key1, trailing1] = s_reSection.match(str); whole) {
					string_t key{key1.begin(), key1.end()};
					m_sections.base().emplace_back(std::pair{key, section_t{}});
					section = &m_sections.base().back().second;
					section->m_line = std::move(str);
					continue;
				}

				// item : just put it into current section
				//if (auto [whole, key1, eq1, value1, comment1] = s_reItem(str); whole) {
				//}
				section->m_items.push_back(std::move(str));
			}

			return true;
		}
		catch (...) {
			return false;
		}

		bool Save(std::ostream& stream, bool bWriteBOM = !std::is_same_v<tchar, char>) const try {
			TArchive ar(stream);
			if (bWriteBOM) {
				eCODEPAGE eCodepage = eCODEPAGE_DEFAULT<tchar>;
				if (eCodepage == eCODEPAGE::DEFAULT)
					eCodepage = eCODEPAGE::UTF8;
				ar.WriteCodepageBOM(eCodepage);
			}
			for (auto& [key, section] : m_sections) {
				if (!key.empty()) {
					if (section.m_line.empty()) {
						string_t str = FormatToTString<tchar, "[{}]">(key);
						ar.WriteLine(GetDefaultFormatString<tchar>(), str);
					}
					else {
						ar.WriteLine(GetDefaultFormatString<tchar>(), section.m_line);
					}
				}
				for (auto& item : section.m_items) {
					ar.WriteLine(GetDefaultFormatString<tchar>(), item);
				}
			}
			return true;
		}
		catch (...) {
			return false;
		}

	public:
		string_t Quote(string_view_t sv) const {
			std::string str;
			if (sv.empty())
				return str;
			//if (auto n = std::ranges::count(sv, '"')) {
			//	str.reserve(sv.size() + n + 2);
			//	str = '"';
			//	for (auto c : sv) {
			//		if (c == '"')
			//			str += '\\';
			//		str += c;
			//	}
			//	str += '"';
			//} else
			{
				str.reserve(sv.size()+2);
				str = '"';
				str += sv;
				str += '"';
			}
			return str;
		}
		template < typename tstring >
			requires std::is_convertible_v<tstring, string_view_t>
				or std::is_convertible_v<tstring, string_t>
		bool DeQuote(tstring& str) const {
			if (str.size() >= 2 and str.front() == '"' and str.back() == '"') {
				str = str.substr(1, str.size() - 2);
				return true;
			}
			return false;
		}

	};


}	// namespace gtl
