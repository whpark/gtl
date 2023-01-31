//////////////////////////////////////////////////////////////////////
//
// json_sync.h: json adapter (Sync) for (boost::json, nlohmann::json)
//
// PWH
//    2023.01.20~
// key : string / u8string / wstring
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/_config.h"
#include "gtl/concepts.h"
#include "gtl/string/convert_codepage.h"

// sync data using json

namespace gtl {
#pragma pack(push, 8)

	template < typename tjson >
	class json_sync {
	public:
		using json_t = tjson;
	protected:
		std::optional<json_t> m_jObject;
		json_t& m_j;
	public:
		json_sync() : m_jObject(json_t{}), m_j(*m_jObject) {}
		json_sync(json_sync const&) = default;
		json_sync(json_sync &&) = default;
		json_sync& operator = (json_sync const&) = default;
		json_sync& operator = (json_sync &&) = default;

		json_sync(json_t& b) : m_j(b) {}

		json_t& json() {
			return m_j;
		}
		json_t const& json() const {
			return m_j;
		}

		json_sync& operator = (std::string const& str) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
		json_sync& operator = (std::wstring const& str) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
		json_sync& operator = (std::u8string const& str) { m_j = reinterpret_cast<std::string const&>(str); return *this; }
		json_sync& operator = (std::u16string const& str) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
		json_sync& operator = (std::u32string const& str) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }

		json_sync& operator = (std::string_view sv) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
		json_sync& operator = (std::wstring_view sv) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
		json_sync& operator = (std::u8string_view sv) { m_j = reinterpret_cast<std::string_view&>(sv); return *this; }
		json_sync& operator = (std::u16string_view sv) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
		json_sync& operator = (std::u32string_view sv) { m_j = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }


		template < size_t n >
		json_sync operator [] (char const (&sz)[n]) { return operator [](std::string_view{sz}); }
		template < size_t n >
		json_sync operator [] (char8_t const (&sz)[n]) { return operator [](std::u8string_view{sz}); }
		json_sync operator [] (std::string_view svKey) {
			return m_j[reinterpret_cast<std::string&>(gtl::ToString<char8_t>(svKey))];
		}
		json_sync operator [] (std::u8string_view svKey) {
			// todo : how to use string_view???
			return m_j[std::string((char const*)svKey.data(), svKey.size())];
		}
		json_sync operator [] (std::size_t index) {
			return m_j[index];
		}
		template < size_t n >
		json_sync operator [] (char const (&sz)[n]) const { return operator [](std::string_view{sz}); }
		template < size_t n >
		json_sync operator [] (char8_t const (&sz)[n]) const { return operator [](std::u8string_view{sz}); }
		json_sync const operator [] (std::string_view svKey) const {
			return m_j[reinterpret_cast<std::string&>(gtl::ToString<char8_t>(svKey))];
		}
		json_sync const operator [] (std::u8string_view svKey) const {
			return m_j[std::string((char const*)svKey.data(), svKey.size())];
		}
		json_sync const operator [] (std::size_t index) const {
			return m_j[index];
		}

		operator bool() const { return m_j.is_boolean() ? (bool)m_j : false; }
		operator int() const { return m_j.is_number_integer() ? (int)m_j : 0; }
		operator int64_t() const { return m_j.is_number_integer() ? (int64_t)m_j : 0ll; }
		operator double() const { return m_j.is_number_float() ? (double)m_j : 0.0; }
		operator float() const { return m_j.is_number_float() ? (float)(double)m_j : 0.0f; }
		template < gtlc::string_elem tchar_t >
		operator std::basic_string<tchar_t> () const {
			if (!m_j.is_string())
				return {};
			auto jstrU8 = (std::string)m_j;
			if constexpr (std::is_same_v<tchar_t, char8_t>) {
				return std::u8string((char8_t const*)jstrU8.data(), jstrU8.size());
			} else {
				std::u8string_view svU8{(char8_t const*)jstrU8.data(), jstrU8.size()};
				return gtl::ToString<tchar_t, char8_t, false>(svU8);
			}
		}
		template < typename T >
			requires (
		(std::is_class_v<T> || std::is_enum_v<T> )
			&& !std::is_convertible_v<T, std::string> && !std::is_convertible_v<T, std::wstring>
			&& !std::is_convertible_v<T, std::u8string> && !std::is_convertible_v<T, std::u16string> && !std::is_convertible_v<T, std::u32string>
			&& !std::is_convertible_v<T, std::string_view> && !std::is_convertible_v<T, std::wstring_view>
			&& !std::is_convertible_v<T, std::u8string_view> && !std::is_convertible_v<T, std::u16string_view> && !std::is_convertible_v<T, std::u32string_view>
			)
			operator T() const {
			T a{};
			if (m_j.is_object())
				from_json(*this, a);
			return a;
		}
};

#pragma pack(pop)
};
