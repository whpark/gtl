//////////////////////////////////////////////////////////////////////
//
// gtl.h
//
// PWH
//    2021.05.20.
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

#include "_config.h"
#include "_macro.h"




// todo : streamable_unique_ptr / streamable_shared_ptr - derived from std::unique_ptr / std::shared_ptr  respectively. for dynamic creatable object


//-----------------------------------------------------------------------------
// Naming Convention :
/*

* basic : maximize the utilities of IDE
	+ class Name
		- camal case
		- class name starts with 'x'
			ex) xCoordTrans
		- template class name starts with 'T'
		- interface class name starts with 'I'
		- Abbrevs. are all in UPPER CASE.
		- Function Starts with Upper Case.

	+ class member variables :
		i) public member variables - starts with 'm_'
		ii) private members ....... ???

	+ class member functions : ... which to take?
		i) Categorizing - Category comes first ?
			ex) functions with 'Path'
				Path + Get -> PathGet, PathSet, PathInit ...
		ii) Verb + Object.
			ex) functions with 'Path'
				GetPath, SetPath, InitPath ...

		ex)
			PutSomethingSpecial() ('P', 'S', 'S' �빮�� ���)
			GetDNS_Name : Get, DNS, Name �� 'G'et, 'DNS', 'N'ame �� �빮�� ���. DNS �� Name �� �빮�ڷ� ���� �ǹǷ�, '_' �� ����Ͽ� �����.

ex)
	(1) (Set/Get), (Read/Write), (Send/Receive)
		- Set/Get : �ܼ��� ������ ���� ���� �� ���.
		- Read/Write : File/Stream ���� �����. IO ���̰� ������ ���.
		- Send/Receive : Ư�� Protocol �� ����Ͽ� �����͸� ��/���� �ؾ� �ϴ� ���.

	(2) Init, Clear
		- Init() : �ʱ�ȭ. Class �� ����ϱ� ��, � �۾�(���� ������̵�, ...)�� �ؾ� �� ���.
		- Clear() : �ʱ�ȭ. �ܼ� ������ 0���� ������ �۾�.

	(3) FromString, ToString



*/
