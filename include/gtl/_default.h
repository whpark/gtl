﻿//////////////////////////////////////////////////////////////////////
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
#	define FMT_HEADER_ONLY
#	include <fmt/format.h>
#endif
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

#define GTL_DEPR_SEC [[deprecated("NOT Secure")]]

#include "gtl/config_gtl.h"

// until c++20 source_location
#ifdef __cpp_lib_source_location
#	define GTL__FUNCSIG std::u8string(std::source_location::current().function_name()) + " : "
#else
	#ifdef _MSC_VER
		#define GTL__FUNCSIG __FUNCSIG__ " : "
	#else
		#error ..... ????...
	#endif
#endif

#define NOMINMAX	// disable Windows::min/max
#if defined(min) || defined(max)
#	error min/max must not be defined. turn it off using '#define NOMINMAX' before '#include <Windows.h>''
#endif


namespace gtl {

	namespace literals {
	};

	// byte swap
#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY)
	template < std::integral tvar >
	[[nodiscard]] inline auto GetByteSwap(tvar v) {
		if constexpr (sizeof(v) == sizeof(std::uint16_t)) {
			return _byteswap_ushort(v);
		} else if constexpr (sizeof(v) == sizeof(std::uint32_t)) {
			return _byteswap_ulong(v);
		} else if constexpr (sizeof(v) == sizeof(std::uint64_t)) {
			return _byteswap_uint64(v);
		} else {
			static_assert(false, "not supported data type.");
		}
	}
	template < std::integral tvar >
	inline void ByteSwap(tvar& v) {
		v = GetByteSwap(v);
	}
#else
	template < std::integral tvar >
	[[nodiscard]] inline tvar GetByteSwap(tvar const v) {
		tvar r{};
		std::reverse_copy((uint8_t const*)&v, (uint8_t const*)&v+sizeof(v), (uint8_t*)&r);
		return r;
	}
	template < std::integral tvar >
	inline void ByteSwap(tvar& v) {
		std::reverse((uint8_t*)&v, (uint8_t*)&v+sizeof(v));
	}
#endif


	//template < typename tcontainer >
	//constexpr inline std::remove_cvref_t<decltype(tcontainer{}[0])> const* data(tcontainer const& v) {
	//	if constexpr (std::is_array_v<tcontainer>) {
	//		return &v[0];
	//	}
	//	else {
	//		return v.data();
	//	}
	//}
	//template < typename tcontainer >
	//constexpr inline std::remove_cvref_t<decltype(tcontainer{}[0])> * data(tcontainer& v) {
	//	if constexpr (std::is_array_v<tcontainer>) {
	//		return &v[0];
	//	}
	//	else {
	//		return v.data();
	//	}
	//}


}



//-----------------------------------------------------------------------------
// Naming Convention :
/*

(*) 기본 : 
	- Camal Case. 단어 시작할 때 대문자로 시작.
	- 단축어 줄임말 등은 전체 대문자 사용.
	-

	ex)
		PutSomethingSpecial() ('P', 'S', 'S' 대문자 사용)
		GetDNS_Name : Get, DNS, Name 의 'G'et, 'DNS', 'N'ame 은 대문자 사용. DNS 와 Name 이 대문자로 연결 되므로, '_' 를 사용하여 띄워줌.


1. 
	(1) Class Name : 'C' 로 시작.


	(2) 여러가지 방법이 있으므로.....

		* (동사 + 목적어) 또는 (목적어 + 동사) 둘 중 하나만 골라서 사용하기.

		* Member Functino Name : 그룹으로 묶어서 사용.
			ex) path 에 대한 것일 경우,
				Path + Get -> PathGet, PathSet, PathInit ...




2. 구체적인 이름 :
	(1) (Set/Get), (Read/Write), (Send/Receive)
		- Set/Get : 단순히 변수의 값을 설정 할 경우.
		- Read/Write : File/Stream 등의 입출력. IO 길이가 정해진 경우.
		- Send/Receive : 특정 Protocol 을 사용하여 데이터를 송/수신 해야 하는 경우.

	(2) Init, Clear
		- Init() : 초기화. Class 를 사용하기 전, 어떤 작업(파일 입출력이든, ...)을 해야 할 경우.
		- Clear() : 초기화. 단순 값들을 0으로 돌리는 작업.

	(3) FromString, ToString



*/
