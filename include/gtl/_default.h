//////////////////////////////////////////////////////////////////////
//
// _default.h:
//
// PWH
//    2020.11.12
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <version>	// version information

#if !defined(_HAS_CXX20)
#	error ERROR! C++v20
#endif

//=================================================================================================================================
// include files. (until modules......)
//#define _USE_MATH_DEFINES
//#include <math.h>	======> <numbers>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cstdint>
#include <cstddef>
#include <cstdarg>
#include <clocale>
#include <cstring>
#include <cwchar>
#include <cwctype>
//#include <ciso646>	// -> c++20 <version>
//#include <tchar.h>
//#include <cstdbool>

#include <concepts>
#include <exception>
#include <limits>
#include <numeric>
#include <numbers>
#include <complex>
#include <initializer_list>
#include <typeinfo>
#include <type_traits>
#include <bit>
#include <bitset>
#include <compare>
#include <charconv>

#include <iostream>
#include <fstream>
#include <codecvt>
#include <string>
#include <string_view>
#if defined(__cpp_lib_format)
#	include <format>
#else
#endif
#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/args.h"
#include "fmt/chrono.h"
#include "fmt/color.h"
#include "fmt/compile.h"
#include "fmt/os.h"
#include "fmt/ostream.h"
#include "fmt/printf.h"
#include "fmt/ranges.h"
#include "fmt/std.h"
#include "fmt/xchar.h"

#include <regex>
//#if defined (__cpp_lib_ctre)
//#	include <ctre>
//#else
//#	include <ctre.hpp>
//#endif

#include <algorithm>
#include <iterator>
#include <any>
#include <tuple>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <queue>
#include <memory>
#include <optional>
#include <variant>
#include <ranges>

#include <chrono>
#include <filesystem>
#include <system_error>
#include <functional>
#include <utility>
#include <random>

#include <atomic>
#include <barrier>
#include <mutex>
#include <semaphore>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <coroutine>
#include <latch>

#if !defined (__cpp_lib_jthread)
#	include <jthread.hpp>
#endif
#if defined(__cpp_lib_source_location)
#	include <source_location>
#endif

#define SPDLOG_USE_STD_FORMAT
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define SPDLOG_WCHAR_FILENAMES
#include "spdlog/spdlog.h"

#define GTL__DEPR_SEC [[deprecated("NOT Secure. use 'sv' instead.")]]

#include "gtl/_config.h"

#if (GTL__USE_WINDOWS_API)
#	ifndef NOMINMAX
#		define NOMINMAX	// disable Windows::min/max
#	endif
#	pragma warning(push)
#	pragma warning(disable: 5104 5105)
#	include <windows.h>
#	pragma warning(pop)
#endif
#ifndef NOMINMAX
#	define NOMINMAX	// disable Windows::min/max
#endif
#if defined(min) || defined(max)
//#	error min/max must not be defined. turn it off using '#define NOMINMAX' before '#include <Windows.h>''
#	undef min
#	undef max
#endif

// until c++20 source_location
#ifdef __cpp_lib_source_location
#	define GTL__FUNCSIG std::string(std::source_location::current().function_name()) + " : "
#else
	#ifdef _MSC_VER
		#define GTL__FUNCSIG __FUNCSIG__ " : "
	#else
		#error ..... ????...
	#endif
#endif


#ifdef __cpp_lib_to_underlying
#else
namespace std {
	template <typename T>
	constexpr underlying_type_t<T> to_underlying(T value) noexcept {
		return static_cast<underlying_type_t<T>>(value);
	}
}
#endif


#ifdef __cpp_lib_ranges_enumerate
#else
// https://www.reedbeta.com/blog/python-like-enumerate-in-cpp17/
namespace std::ranges::views {
	template <typename T,
		typename TIter = decltype(std::begin(std::declval<T>())),
		typename = decltype(std::end(std::declval<T>()))>
	constexpr auto enumerate(T && iterable)
	{
		struct iterator
		{
			size_t i;
			TIter iter;
			bool operator != (const iterator & other) const { return iter != other.iter; }
			void operator ++ () { ++i; ++iter; }
			auto operator * () const { return std::tie(i, *iter); }
		};
		struct iterable_wrapper
		{
			T iterable;
			auto begin() { return iterator{ 0, std::begin(iterable) }; }
			auto end() { return iterator{ 0, std::end(iterable) }; }
		};
		return iterable_wrapper{ std::forward<T>(iterable) };
	}
}
#endif
namespace gtl {
	template < std::integral size_type, typename T, typename TIter = decltype(std::begin(std::declval<T>())), typename = decltype(std::end(std::declval<T>())) >
	constexpr auto enumerate_as(T&& iterable) {
		struct iterator {
			size_type i;
			TIter iter;
			bool operator != (iterator const& other) const { return iter != other.iter; }
			void operator ++ () { ++i; ++iter; }
			auto operator * () const { return std::tie(i, *iter); }
		};
		struct iterable_wrapper {
			T iterable;
			auto begin() { return iterator{ 0, std::begin(iterable) }; }
			auto end() { return iterator{ 0, std::end(iterable) }; }
		};
		return iterable_wrapper{ std::forward<T>(iterable) };
	}
}

// NUM_ARGS(...) macro arg 갯수
#define NUM_ARGS_64(                            _64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1, N, ...) N
#define NUM_ARGS(...) NUM_ARGS_64( __VA_ARGS__ , 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
static_assert(NUM_ARGS(1, 2, 3) == 3, "for MSVC, add compiler option /Zc:preprocessor");

#define I_GTL__PARENS ()

#define I_GTL__EXPAND(...)								I_GTL__EXPAND4(I_GTL__EXPAND4(I_GTL__EXPAND4(I_GTL__EXPAND4(__VA_ARGS__))))
#define I_GTL__EXPAND4(...)								I_GTL__EXPAND3(I_GTL__EXPAND3(I_GTL__EXPAND3(I_GTL__EXPAND3(__VA_ARGS__))))
#define I_GTL__EXPAND3(...)								I_GTL__EXPAND2(I_GTL__EXPAND2(I_GTL__EXPAND2(I_GTL__EXPAND2(__VA_ARGS__))))
#define I_GTL__EXPAND2(...)								I_GTL__EXPAND1(I_GTL__EXPAND1(I_GTL__EXPAND1(I_GTL__EXPAND1(__VA_ARGS__))))
#define I_GTL__EXPAND1(...)								__VA_ARGS__

#define GTL__RECURSIVE_MACRO_COMMA(macro, ...)				__VA_OPT__(I_GTL__EXPAND(I_GTL__RECURSIVE_MACRO_HELPER_COMMA(macro, __VA_ARGS__)))
#define GTL__RECURSIVE_MACRO_SPACE(macro, ...)				__VA_OPT__(I_GTL__EXPAND(I_GTL__RECURSIVE_MACRO_HELPER_SPACE(macro, __VA_ARGS__)))
#define I_GTL__RECURSIVE_MACRO_HELPER_COMMA(macro, a1, ...)	macro(a1) __VA_OPT__(,) __VA_OPT__(I_GTL__RECURSIVE_MACRO_AGAIN_COMMA I_GTL__PARENS (macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_HELPER_SPACE(macro, a1, ...)	macro(a1)               __VA_OPT__(I_GTL__RECURSIVE_MACRO_AGAIN_SPACE I_GTL__PARENS (macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_AGAIN_COMMA()				I_GTL__RECURSIVE_MACRO_HELPER_COMMA
#define I_GTL__RECURSIVE_MACRO_AGAIN_SPACE()				I_GTL__RECURSIVE_MACRO_HELPER_SPACE

namespace gtl {

	inline namespace literals {
	};

	// aliases
	using int8  = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;
	using uint8  = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using uchar = unsigned char;
	using uint = uint32_t;	// conflicts with cv::

	using ssize_t = std::common_type_t<std::ptrdiff_t, std::make_signed_t<size_t>>;

#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM)
	using charKSSM_t = uint16_t;
#endif
}
using gtl::ssize_t;

//#	pragma warning(disable: 4455)
#ifdef __cpp_size_t_suffix
#else
namespace std::inline literals {
	constexpr gtl::ssize_t operator "" z ( unsigned long long n ) { return n; }
	constexpr std::size_t operator "" uz ( unsigned long long n ) { return n; }
}
#endif
