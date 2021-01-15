//////////////////////////////////////////////////////////////////////
//
// json_proxy.h: json adapter for (boost::json, nlohmann::json)
//
// PWH
//    2021.01.11~16.
// key : string / u8string.
// !!! importance !!! std::string contains ansi(MBCS) string (codepage). std::u8string will contain utf-8 string.
//
//////////////////////////////////////////////////////////////////////

#pragma once

//#include "gtl/concepts.h"
#include "gtl/string/convert_codepage.h"

// https://www.boost.org/doc/libs/1_75_0/libs/json/doc/html/json/quick_look.html
#define BOOST_JSON_STANDALONE
#define BOOST_NO_EXCEPTIONS
#include "boost/json.hpp"       // decl


// https://github.com/nlohmann/json
#include "nlohmann/json.hpp"


// from/to json
template < typename tjson, typename T >
void from_json(tjson const& j, T& object);
template < typename tjson, typename T >
void to_json(tjson& j, T const& object);

namespace gtl {
#pragma pack(push, 8)

    /// @brief json proxy for boost:json
    class bjson {
		using json_t = boost::json::value;
		std::optional<json_t> j__;
		json_t& j_;
	public:
		bjson() : j__(json_t{}), j_(j__.value()) {};
		bjson(bjson const&) = default;
		bjson(bjson &&) = default;
		bjson& operator = (bjson const&) = default;
		bjson& operator = (bjson &&) = default;

		bjson(json_t& b) : j_(b) {}

		json_t json() {
			return j_;
		}
		json_t const json() const {
			return j_;
		}

		template < typename T > requires (requires (json_t j, T v) { j = v; })
		bjson& operator = (T const& b) {
			j_ = b;
			return *this;
		}
		template < typename T > requires (!requires (json_t j, T v) { j = v; })
		bjson& operator = (T const& b) {
			to_json(*this, b);
			return *this;
		}
		bjson& operator = (std::string const& str) {
			j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str));
			return *this;
		}
		bjson& operator = (std::string&& str) {
			j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str));
			return *this;
		}
		bjson& operator = (std::string_view sv) {
			j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv));
			return *this;
		}
		bjson& operator = (std::u8string const& str) {
			j_ = reinterpret_cast<std::string const&>(str);
			return *this;
		}
		bjson& operator = (std::u8string && str) {
			j_ = reinterpret_cast<std::string &&>(str);
			return *this;
		}
		bjson& operator = (std::u8string_view sv) {
			j_ = reinterpret_cast<std::string_view&>(sv);
			return *this;
		}


		bjson operator [] (std::string_view svKey) {
			boost::json::object* pObject = j_.is_null() ? &j_.emplace_object() : &j_.as_object();
			return (*pObject)[svKey];
		}
		bjson operator [] (std::u8string_view svKey) {
			boost::json::object* pObject = j_.is_null() ? &j_.emplace_object() : &j_.as_object();
			return (*pObject)[reinterpret_cast<std::string_view&>(svKey)];
		}
		bjson operator [] (std::size_t index) {
			boost::json::array* pArray = j_.is_null() ? &j_.emplace_array() : &j_.as_array();
			if (pArray->size() <= index)
				pArray->resize(index + 1);

			return (*pArray)[index];
		}

		bjson operator [] (std::string_view svKey) const {
			if (j_.is_null())
				throw std::invalid_argument{"empty"};
			boost::json::object const* pObject = &j_.as_object();
			return (const_cast<boost::json::object&>(*pObject))[svKey];
		}
		bjson const operator [] (std::u8string_view svKey) const {
			if (j_.is_null())
				throw std::invalid_argument{"empty"};
			boost::json::object const* pObject = &j_.as_object();
			return (const_cast<boost::json::object&>(*pObject))[reinterpret_cast<std::string_view&>(svKey)];
		}

		bjson const operator [] (std::size_t index) const {
			if (j_.is_null())
				throw std::invalid_argument{"empty"};
			boost::json::array const* pArray = &j_.as_array();
			if (pArray->size() <= index)
				throw std::invalid_argument{"size"};

			return (const_cast<boost::json::array&>(*pArray))[index];
		}

		operator bool() const { return j_.as_bool(); }
		operator int() const { return (int)j_.as_int64(); }
		operator int64_t() const { return j_.as_int64(); }
		operator double() const { return j_.as_double(); }
		operator std::u8string() const {
			auto& jstrU8 = j_.as_string();
			return std::u8string((char8_t const*)jstrU8.data(), jstrU8.size());
		}
		operator std::string() const { 
			// todo : u8 -> ansi
			auto& jstrU8 = j_.as_string();
			return std::string{jstrU8.data(), jstrU8.size()};
		}
		template < typename T >
			requires (
				std::is_class_v<T>
				&& !std::is_convertible_v<T, std::u8string> && !std::is_convertible_v<T, std::string>
				&& !std::is_convertible_v<T, std::string_view> && !std::is_convertible_v<T, std::u8string_view>
				)
		operator T() const {
			T a;
			from_json(*this, a);
			return a;
		}

	};


	class njson {
		using json_t = nlohmann::json;
		std::optional<json_t> j__;
		json_t& j_;
	public:
		njson() : j__(json_t{}), j_(j__.value()) {};
		njson(njson const&) = default;
		njson(njson &&) = default;
		njson& operator = (njson const&) = default;
		njson& operator = (njson &&) = default;

		njson(json_t& b) : j_(b) {}

		json_t json() {
			return j_;
		}
		json_t const json() const {
			return j_;
		}

		//template < typename T > requires (requires (json_t j, T v) { j = v; })
		//njson& operator = (T const& b) {
		//	j_ = b;
		//	return *this;
		//}
		template < typename T >//requires (!requires (json_t j, T v) { j = v; })
		njson& operator = (T const& b) {
			using value_t = std::remove_cvref_t<T>;
			if constexpr (std::is_integral_v<value_t> or std::is_floating_point_v<value_t>) {
				j_ = b;
			} else {
				to_json(*this, b);
			}
			return *this;
		}
		njson& operator = (std::string const& str) {
			j_ = str;	// todo : covnert to u8string
			return *this;
		}
		njson& operator = (std::string&& str) {
			j_ = str;	// todo : covnert to u8string
			return *this;
		}
		njson& operator = (std::string_view sv) {
			j_ = sv;	// todo : covnert to u8string
			return *this;
		}
		njson& operator = (std::u8string const& str) {
			j_ = reinterpret_cast<std::string const&>(str);
			return *this;
		}
		njson& operator = (std::u8string && str) {
			j_ = reinterpret_cast<std::string &&>(str);
			return *this;
		}
		njson& operator = (std::u8string_view sv) {
			j_ = reinterpret_cast<std::string_view&>(sv);
			return *this;
		}


		njson operator [] (std::string_view svKey) {
			// todo : convert to u8string
			return j_[svKey.data()];
		}
		njson operator [] (std::u8string_view svKey) {
			return j_[(char const*)svKey.data()];
		}
		njson operator [] (std::size_t index) {
			return j_[index];
		}
		njson const operator [] (std::size_t index) const {
			return j_[index];
		}
		njson const operator [] (std::string_view svKey) const {
			// todo : convert to u8string
			return j_[svKey.data()];
		}
		njson const operator [] (std::u8string_view svKey) const {
			return j_[(char const*)svKey.data()];
		}

		operator bool() const { return (bool)j_; }
		operator int() const { return (int)j_; }
		operator int64_t() const { return (int64_t)j_; }
		operator double() const { return (double)j_; }
		operator std::u8string() const {
			std::u8string str = reinterpret_cast<std::u8string&>((std::string)j_);
			return std::move(str);
		}
		operator std::string() const { 
			// todo : u8 -> ansi
			std::string str = j_;
			return str;
		}
		template < typename T >
			requires (
				std::is_class_v<T>
				&& !std::is_convertible_v<T, std::u8string> && !std::is_convertible_v<T, std::string>
				&& !std::is_convertible_v<T, std::string_view> && !std::is_convertible_v<T, std::u8string_view>
				)
		operator T() const {
			T a;
			from_json(*this, a);
			return a;
		}

	};

#pragma pack(pop)
}
