//////////////////////////////////////////////////////////////////////
//
// _default.h:
//
// PWH
//    2020.11.12
//
//////////////////////////////////////////////////////////////////////

#pragma once

//=================================================================================================================================
// include files. (until modules......)
#define _USE_MATH_DEFINES
#include <math.h>
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
#include <tchar.h>
//#include <cstdbool>

#include <limits>
#include <numeric>
#include <initializer_list>
#include <exception>
#include <typeinfo>
#include <type_traits>
#include <bit>
#include <algorithm>
#include <compare>
#include <tuple>
#include <any>
#include <iostream>
#include <istream>
#include <ostream>
#include <fstream>
#include <string>
#include <string_view>
#include <iterator>
#include <bitset>
#include <map>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <queue>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>
#include <system_error>
#include <thread>
#include <utility>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <future>
#include <optional>
#include <filesystem>
#include <random>
#include <condition_variable>
#include <charconv>
#include <future>
//#include <tchar.h>
#include <codecvt>


#if !defined(__cpp_lib_concepts)
#	error ERROR! Supports C++v20 only.
#endif


namespace gtl {

	namespace concepts {


	}

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
