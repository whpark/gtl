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

#include "gtl/_config.h"
#include "gtl/concepts.h"
#include "gtl/string/convert_codepage.h"

// from/to json
template < typename tjson, typename T >
void from_json(tjson const& j, T& object);
template < typename tjson, typename T >
void to_json(tjson&& j, T const& object);

template < typename tjson, gtlc::arithmetic T >
void from_json(tjson const& j, T& value) {
	value = j;
}
template < typename tjson, gtlc::arithmetic T >
void to_json(tjson&& j, T const& value) {
	j = value;
}
template < typename tjson, gtlc::string_elem tchar >
void from_json(tjson const& j, std::basic_string<tchar>& str) {
	str = j;
}
template < typename tjson, gtlc::string_elem tchar >
void to_json(tjson&& j, std::basic_string<tchar> const& str) {
	j = str;
}
template < typename tjson, gtlc::string_elem tchar >
void to_json(tjson&& j, std::basic_string_view<tchar> const& sv) {
	j = sv;
}


#if (GTL__USE_BOOST_JSON)


	#if (GTL__BOOST_JSON__AS_STANDALONE)
	#	define BOOST_JSON_STANDALONE
	//#	define BOOST_NO_EXCEPTIONS
	#elif (GTL__BOOST_JSON__AS_NESTED_LIB)
	#	define BOOST_JSON_STANDALONE
	//#	define BOOST_NO_EXCEPTIONS
	#	ifdef GTL__EXPORTS
	#		define BOOST_JSON_DECL       __declspec(dllexport)
	#		define BOOST_JSON_CLASS_DECL __declspec(dllexport)
	#	else
	#		define BOOST_JSON_DECL       __declspec(dllimport)
	#		define BOOST_JSON_CLASS_DECL __declspec(dllimport)
	#	endif
	#elif (GTL__BOOST_JSON__AS_SHARED_LIB)
	#	define BOOST_JSON_DECL       __declspec(dllimport)
	#	define BOOST_JSON_CLASS_DECL __declspec(dllimport)
	//#	pragma comment(lib, "boost.json.lib")
	#endif
	
	
	#pragma warning(push)
	#pragma warning(disable: 5104 5105)
	// https://www.boost.org/doc/libs/1_75_0/libs/json/doc/html/json/quick_look.html
	#include "boost/json.hpp"       // decl
	#pragma warning(pop)
	
	namespace gtl {
	#pragma pack(push, 8)
	
	    /// @brief json proxy for boost:json
		/// for string, converts to utf-8.
		template < typename json_t = boost::json::value >
	    class bjson {
			std::optional<json_t> j__;
			json_t& j_;
		public:
			bjson() : j__(json_t{}), j_(j__.value()) {};
			bjson(bjson const&) = default;
			bjson(bjson &&) = default;
			bjson& operator = (bjson const&) = default;
			bjson& operator = (bjson &&) = default;
	
			bjson(json_t& b) : j_(b) {}
	
			json_t& json() {
				return j_;
			}
			json_t const& json() const {
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
			bjson& operator = (std::string const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
			bjson& operator = (std::wstring const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
			bjson& operator = (std::u8string const& str) { j_ = reinterpret_cast<std::string const&>(str); return *this; }
			bjson& operator = (std::u16string const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
			bjson& operator = (std::u32string const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
	
			bjson& operator = (std::string_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
			bjson& operator = (std::wstring_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
			bjson& operator = (std::u8string_view sv) { j_ = reinterpret_cast<std::string_view&>(sv); return *this; }
			bjson& operator = (std::u16string_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
			bjson& operator = (std::u32string_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
	
	
			template < size_t n >
			bjson operator [] (char const (&sz)[n]) { return operator [](std::string_view{sz}); }
			template < size_t n >
			bjson operator [] (char8_t const (&sz)[n]) { return operator [](std::u8string_view{sz}); }
			bjson operator [] (std::string_view svKey) {
				boost::json::object* pObject = j_.is_null() ? &j_.emplace_object() : &j_.as_object();
				// todo :.... speed up for svKey...
				return (*pObject)[boost::json::string_view{svKey.data(), svKey.size()}];
			}
			bjson operator [] (std::u8string_view svKey) {
				boost::json::object* pObject = j_.is_null() ? &j_.emplace_object() : &j_.as_object();
				return (*pObject)[boost::json::string_view{(char const*)svKey.data(), svKey.size()}];
			}
			bjson operator [] (std::size_t index) {
				boost::json::array* pArray = j_.is_null() ? &j_.emplace_array() : &j_.as_array();
				if (pArray->size() <= index)
					pArray->resize(index + 1);
	
				return (*pArray)[index];
			}
	
			template < size_t n >
			bjson operator [] (char const (&sz)[n]) const { return operator [](std::string_view{sz}); }
			template < size_t n >
			bjson operator [] (char8_t const (&sz)[n]) const { return operator [](std::u8string_view{sz}); }
			bjson operator [] (std::string_view svKey) const {
				if (j_.is_null())
					throw std::invalid_argument{GTL__FUNCSIG "empty"};
				boost::json::object const* pObject = &j_.as_object();
				return (const_cast<boost::json::object&>(*pObject))[std::string(svKey)];
			}
			bjson const operator [] (std::u8string_view svKey) const {
				if (j_.is_null())
					throw std::invalid_argument{GTL__FUNCSIG "empty"};
				boost::json::object const* pObject = &j_.as_object();
				std::u8string strKey{svKey};
				return (const_cast<boost::json::object&>(*pObject))[reinterpret_cast<std::string&>(strKey)];
			}
			bjson const operator [] (std::size_t index) const {
				if (j_.is_null())
					throw std::invalid_argument{GTL__FUNCSIG "empty"};
				boost::json::array const* pArray = &j_.as_array();
				if (pArray->size() <= index)
					throw std::invalid_argument{GTL__FUNCSIG "size"};
	
				return (const_cast<boost::json::array&>(*pArray))[index];
			}
	
			operator bool() const { return j_.as_bool(); }
			operator int() const { return (int)j_.as_int64(); }
			operator int64_t() const { return j_.as_int64(); }
			operator double() const { return j_.is_double() ? j_.as_double() : (j_.is_int64() ? (double)j_.as_int64() : 0.0); }
	
			template < typename T >
			T value_or(T const& default_value) {
				if constexpr (std::is_same_v<T, bool>) {
					return j_.is_bool() ? j_.as_bool() : default_value;
				}
				else if constexpr (std::is_integral_v<T>) {
					return j_.is_int64() ? (T)j_.as_int64() : default_value;
				}
				else if constexpr (std::is_floating_point_v<T>) {
					return j_.is_double() ? j_.as_double() : j_.is_int64() ? (double)j_.as_int64() : default_value;
				}
				else {
					static_assert(gtlc::dependent_false_v);
				}
			}
	
			template < gtlc::string_elem tchar_t >
			operator std::basic_string<tchar_t> () const {
				if (!j_.is_string())
					return {};
				auto& jstrU8 = j_.as_string();
				if constexpr (std::is_same_v<tchar_t, char8_t>) {
					return std::u8string((char8_t const*)jstrU8.data(), jstrU8.size());
				} else {
					std::u8string_view svU8{(char8_t const*)jstrU8.data(), jstrU8.size()};
					return gtl::ToString<tchar_t, char8_t, false>(svU8);
				}
			}
			//template < gtlc::string_elem tchar_t >
			//std::basic_string<tchar_t> as_basic_string() const {
			//	auto& jstrU8 = j_.as_string();
			//	if constexpr (std::is_same_v<tchar_t, char8_t>) {
			//		return std::u8string((char8_t const*)jstrU8.data(), jstrU8.size());
			//	} else {
			//		std::u8string_view svU8{(char8_t const*)jstrU8.data(), jstrU8.size()};
			//		return gtl::ToString<tchar_t, char8_t, false>(svU8);
			//	}
			//}
			//std::string string() const { return as_basic_string<char>(); }
			//std::wstring wstring() const { return as_basic_string<wchar_t>(); }
			//std::u8string u8string() const { return as_basic_string<char8_t>(); }
			//std::u16string u16string() const { return as_basic_string<char16_t>(); }
			//std::u32string u32string() const { return as_basic_string<char32_t>(); }
	
			template < typename T >
				requires (
					std::is_class_v<T>
					&& !std::is_convertible_v<T, std::string> && !std::is_convertible_v<T, std::wstring>
					&& !std::is_convertible_v<T, std::u8string> && !std::is_convertible_v<T, std::u16string> && !std::is_convertible_v<T, std::u32string>
					&& !std::is_convertible_v<T, std::string_view> && !std::is_convertible_v<T, std::wstring_view>
					&& !std::is_convertible_v<T, std::u8string_view> && !std::is_convertible_v<T, std::u16string_view> && !std::is_convertible_v<T, std::u32string_view>
					)
			operator T() const {
				T a;
				from_json(*this, a);
				return a;
			}
	
			auto read(std::filesystem::path const& path) {
				std::ifstream stream(path, std::ios_base::binary);
				return read(stream);
			}
			bool read(std::istream& is) {
				// orinin : https://www.boost.org/doc/libs/1_75_0/libs/json/doc/html/json/input_output.html
				boost::json::error_code ec;
				boost::json::stream_parser p;
				std::string line;
				size_t nLine{};
				while (std::getline(is, line)) {
					nLine++;
					p.write(line, ec);
					if (ec) {
						throw std::ios_base::failure(GTL__FUNCSIG + std::format("line {}", nLine), ec);
						return false;
					}
				}
				p.finish(ec);
				if (ec)
					return false;
				//return p.release();
				j_ = p.release();
				return true;
			}
	
			auto write(std::filesystem::path const& path) {
				std::ofstream stream(path, std::ios_base::binary);
				return write(stream);
			}
			bool write(std::ostream& os) {
				try {
					os << j_;
					return true;
				}
				catch (...) {
					return false;
				}
			}
	
			////
			//template < typename T > 
			//friend void from_json(bjson const& j, std::vector<T>& container) {
			//	container = boost::json::value_to<std::vector<T>>(j.json());
			//}
			//template < typename T > 
			//friend void to_json(bjson&& j, std::vector<T> const& container) {
			//	j.json() = boost::json::value_from(container);
			//}
	
		};
	#pragma pack(pop)
	} // namespace gtl;
	
#endif	// GTL__USE_BOOST_JSON





#if (GTL__USE_NLOHMANN_JSON)

	  // https://github.com/nlohmann/json
	#include "nlohmann/json.hpp"

	namespace gtl {
	#pragma pack(push, 8)



		/// @brief json proxy for nlohmann:json
		template < typename json_t = nlohmann::json >
		class njson {
			std::optional<json_t> j__;
			json_t& j_;
		public:
			njson() : j__(json_t{}), j_(j__.value()) {};
			njson(njson const&) = default;
			njson(njson &&) = default;
			njson& operator = (njson const&) = default;
			njson& operator = (njson &&) = default;

			njson(json_t& b) : j_(b) {}

			json_t& json() {
				return j_;
			}
			json_t const& json() const {
				return j_;
			}

			//template < typename T > requires (requires (json_t j, T v) { j = v; })
			//njson& operator = (T const& b) {
			//	j_ = b;
			//	return *this;
			//}
			template < typename T >//requires (!requires (json_t j, T v) { j = v; })
			njson& operator = (T const& b) {
				using value_type = std::remove_cvref_t<T>;
				if constexpr (std::is_integral_v<value_type> or std::is_floating_point_v<value_type>) {
					j_ = b;
				} else {
					to_json(*this, b);
				}
				return *this;
			}
			njson& operator = (std::string const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
			njson& operator = (std::wstring const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
			njson& operator = (std::u8string const& str) { j_ = reinterpret_cast<std::string const&>(str); return *this; }
			njson& operator = (std::u16string const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }
			njson& operator = (std::u32string const& str) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(str)); return *this; }

			njson& operator = (std::string_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
			njson& operator = (std::wstring_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
			njson& operator = (std::u8string_view sv) { j_ = reinterpret_cast<std::string_view&>(sv); return *this; }
			njson& operator = (std::u16string_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }
			njson& operator = (std::u32string_view sv) { j_ = reinterpret_cast<std::string&&>(gtl::ToStringU8(sv)); return *this; }


			template < size_t n >
			njson operator [] (char const (&sz)[n]) { return operator [](std::string_view{sz}); }
			template < size_t n >
			njson operator [] (char8_t const (&sz)[n]) { return operator [](std::u8string_view{sz}); }
			njson operator [] (std::string_view svKey) {
				return j_[reinterpret_cast<std::string&>(gtl::ToString<char8_t>(svKey))];
			}
			njson operator [] (std::u8string_view svKey) {
				// todo : how to use string_view???
				return j_[std::string((char const*)svKey.data(), svKey.size())];
			}
			njson operator [] (std::size_t index) {
				return j_[index];
			}
			template < size_t n >
			njson operator [] (char const (&sz)[n]) const { return operator [](std::string_view{sz}); }
			template < size_t n >
			njson operator [] (char8_t const (&sz)[n]) const { return operator [](std::u8string_view{sz}); }
			njson const operator [] (std::string_view svKey) const {
				return j_[reinterpret_cast<std::string&>(gtl::ToString<char8_t>(svKey))];
			}
			njson const operator [] (std::u8string_view svKey) const {
				return j_[std::string((char const*)svKey.data(), svKey.size())];
			}
			njson const operator [] (std::size_t index) const {
				return j_[index];
			}

			operator bool() const { return j_.is_boolean() ? (bool)j_ : false; }
			operator int() const { return j_.is_number_integer() ? (int)j_ : 0; }
			operator int64_t() const { return j_.is_number_integer() ? (int64_t)j_ : 0ll; }
			operator double() const { return j_.is_number_float() ? (double)j_ : 0.0; }
			operator float() const { return j_.is_number_float() ? (float)(double)j_ : 0.0f; }
			template < gtlc::string_elem tchar_t >
			operator std::basic_string<tchar_t> () const {
				if (!j_.is_string())
					return {};
				auto jstrU8 = (std::string)j_;
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
				if (j_.is_object())
					from_json(*this, a);
				return a;
			}

		};

	#pragma pack(pop)
	}

#endif	// GTL__USE_NLOHMANN_JSON

