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
#include <ciso646>
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
#include <fmt/format.h>

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

#define GTL__DEPR_SEC [[deprecated("NOT Secure")]]

#include "gtl/_config.h"

#if (GTL__USE_WINDOWS_API)
	#define NOMINMAX
	#pragma warning(push)
	#pragma warning(disable: 5104 5105)
	#include <windows.h>
	#pragma warning(pop)
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

#define NOMINMAX	// disable Windows::min/max
#if defined(min) || defined(max)
//#	error min/max must not be defined. turn it off using '#define NOMINMAX' before '#include <Windows.h>''
#	undef min
#	undef max
#endif


// NUM_ARGS(...) macro arg 갯수
#define NUM_ARGS_64(                            _64,_63,_62,_61,_60,_59,_58,_57,_56,_55,_54,_53,_52,_51,_50,_49,_48,_47,_46,_45,_44,_43,_42,_41,_40,_39,_38,_37,_36,_35,_34,_33,_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1, N, ...) N
#define NUM_ARGS(...) NUM_ARGS_64( __VA_ARGS__ , 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
static_assert(NUM_ARGS(1, 2, 3) == 3, "for MSVC, add compiler option /Zc:preprocessor");

#define I_GTL__RECURSIVE_MACRO(macro, var)								macro(var)
#define I_GTL__RECURSIVE_MACRO_EX_COMMA(macro, var, expr)				macro(var), expr
#define I_GTL__RECURSIVE_MACRO_EX_SPACE(macro, var, expr)				macro(var)  expr
#define I_GTL__RECURSIVE_MACRO_1(seperator, macro, var)					macro(var)
#define I_GTL__RECURSIVE_MACRO_2(seperator, macro, var, var2)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_1(seperator, macro, var2))
#define I_GTL__RECURSIVE_MACRO_3(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_2(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_4(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_3(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_5(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_4(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_6(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_5(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_7(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_6(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_8(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_7(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_9(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_8(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_10(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_9(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_11(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_10(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_12(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_11(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_13(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_12(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_14(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_13(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_15(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_14(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_16(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_15(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_17(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_16(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_18(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_17(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_19(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_18(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_20(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_19(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_21(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_20(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_22(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_21(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_23(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_22(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_24(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_23(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_25(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_24(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_26(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_25(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_27(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_26(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_28(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_27(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_29(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_28(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_30(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_29(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_31(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_30(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_32(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_31(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_33(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_32(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_34(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_33(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_35(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_34(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_36(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_35(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_37(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_36(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_38(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_37(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_39(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_38(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_40(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_39(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_41(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_40(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_42(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_41(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_43(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_42(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_44(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_43(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_45(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_44(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_46(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_45(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_47(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_46(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_48(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_47(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_49(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_48(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_50(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_49(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_51(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_50(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_52(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_51(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_53(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_52(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_54(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_53(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_55(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_54(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_56(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_55(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_57(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_56(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_58(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_57(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_59(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_58(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_60(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_59(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_61(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_60(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_62(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_61(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_63(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_62(seperator, macro, __VA_ARGS__))
#define I_GTL__RECURSIVE_MACRO_64(seperator, macro, var, ...)			I_GTL__RECURSIVE_MACRO_EX_##seperator(macro, var, I_GTL__RECURSIVE_MACRO_63(seperator, macro, __VA_ARGS__))

#define I_GTL__RECURSIVE_MACRO_C(seperator, macro, N, ...)				I_GTL__RECURSIVE_MACRO_##N(seperator, macro, __VA_ARGS__)
#define I_GTL__RECURSIVE_MACRO_F(seperator, macro, N, ...)				I_GTL__RECURSIVE_MACRO_C(seperator, macro, N, __VA_ARGS__)
#define GTL__RECURSIVE_MACRO_COMMA(macro, ...)							I_GTL__RECURSIVE_MACRO_F(COMMA, macro, NUM_ARGS(__VA_ARGS__), __VA_ARGS__)
#define GTL__RECURSIVE_MACRO_SPACE(macro, ...)							I_GTL__RECURSIVE_MACRO_F(SPACE, macro, NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

namespace gtl {

	namespace literals {
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

#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM)
	using charKSSM_t = uint16_t;
#endif

}
