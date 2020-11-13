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

	template < typename T >
	concept arithmetic = std::is_arithmetic_v<T>;

	template < typename tchar_t >
	concept string_elem = std::is_same_v<tchar_t, char>
		|| std::is_same_v<tchar_t, wchar_t>
		|| std::is_same_v<tchar_t, char8_t>
		|| std::is_same_v<tchar_t, char16_t>
		|| std::is_same_v<tchar_t, char32_t>
		|| std::is_same_v<tchar_t, uint16_t>	// for KSSM
		;

}

namespace gtlc = gtl::concepts;
