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

	/// @brief arithmetic type
	template < typename T >
	concept arithmetic = std::is_arithmetic_v<T>;


	/// @brief variadic template check version of 'std::is_same_v<t, t2>'
	template < typename tchar, typename ... ttypes >
	concept is_one_of_v = (std::is_same_v<tchar, ttypes> || ...);


	/// @brief type for string (uint16_t for KSSM (Korean Johab)
	template < typename tchar >
	concept string_elem = is_one_of_v<std::remove_cvref_t<tchar>, char, char8_t, char16_t, char32_t, wchar_t, uint16_t>;	// uint16_t for KSSM (Johab)


	/// @brief type for unicode string.
	template < typename tchar >
	concept string_elem_utf = is_one_of_v<std::remove_cvref_t<tchar>, char8_t, char16_t, char32_t>;


	/// @brief type for string buffer. ex) char buf[12]; std::array<char, 12> buf; std::vector<char> buf;...
	template < typename tcontainer >
	concept string_buffer_fixed =
		string_elem<std::remove_cvref_t<decltype(tcontainer{}[0])>>
		and
		(
			(	// bounded array of tchar
				std::is_bounded_array_v<tcontainer>
				and requires (tcontainer v) {
					{ v[0] }		-> std::convertible_to<std::remove_reference_t<decltype(v[0])>>;
				}
			)
			or requires ( tcontainer v ) {
				// general container of tchar
				{ v[0] }		-> std::convertible_to<std::remove_reference_t<decltype(v[0])>>;
				{ v.data() }	-> std::convertible_to<std::remove_reference_t<decltype(v[0])>*>;
				{ v.size() }	-> std::convertible_to<size_t>;
			}
		);

	/// @brief type for string buffer with type constraints
	template < typename tcontainer, typename tchar >
	concept string_buffer_fixed_c = 
		(std::is_same_v<std::remove_cvref_t<typename tcontainer::value_type>, tchar> or std::is_same_v<std::remove_cvref_t<decltype(tcontainer()[0])>, tchar>)
		and string_buffer_fixed<tcontainer>;


}

namespace gtlc = gtl::concepts;
