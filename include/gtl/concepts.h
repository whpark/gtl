//////////////////////////////////////////////////////////////////////
//
// concepts.h:
//
// PWH
//    2020.11.12
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/_default.h"
#include "gtl/_lib_gtl.h"

#if !defined(__cpp_lib_concepts)
#	error ERROR! Supports C++v20 only.
#endif

namespace gtl::concepts {

	/// <summary>
	/// arithmetic type
	/// </summary>
	template < typename T >
	concept arithmetic = std::is_arithmetic_v<T>;


	/// <summary>
	///	variadic template check version of 'std::is_same_v<t, t2>'
	///	is_one_of_v<>
	/// </summary>
	/// <typeparam name="tchar_t"></typeparam>
	/// <typeparam name="...types_t"></typeparam>
	template < typename tchar_t, typename ... types_t >
	concept is_one_of_v = (std::is_same_v<tchar_t, types_t> || ...);


	template < typename tchar_t >
	concept string_elem = is_one_of_v<tchar_t, char, wchar_t, char8_t, char16_t, char32_t, uint16_t>;
		//|| std::is_same_v<tchar_t, wchar_t>
		//|| std::is_same_v<tchar_t, char8_t>
		//|| std::is_same_v<tchar_t, char16_t>
		//|| std::is_same_v<tchar_t, char32_t>
		//|| std::is_same_v<tchar_t, uint16_t>	// for KSSM
		//;

	template < typename tchar_t >
	concept utf_string_elem = is_one_of_v<tchar_t, char8_t, char16_t, char32_t>;
}

namespace gtlc = gtl::concepts;
