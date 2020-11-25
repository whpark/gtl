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
	/// <typeparam name="tchar"></typeparam>
	/// <typeparam name="...types_t"></typeparam>
	template < typename tchar, typename ... ttypes >
	concept is_one_of_v = (std::is_same_v<tchar, ttypes> || ...);


	template < typename tchar >
	concept string_elem = is_one_of_v<tchar, char, char8_t, char16_t, char32_t, wchar_t, uint16_t>;
		//|| std::is_same_v<tchar, wchar_t>
		//|| std::is_same_v<tchar, char8_t>
		//|| std::is_same_v<tchar, char16_t>
		//|| std::is_same_v<tchar, char32_t>
		//|| std::is_same_v<tchar, uint16_t>	// for KSSM (Korean Johab)
		//;

	template < typename tchar >
	concept utf_string_elem = is_one_of_v<tchar, char8_t, char16_t, char32_t>;

	//template < typename tcontainer >
	//concept string_container = 
	//requires ( tcontainer v ) {
	//	{ v[0] }			-> std::convertible_to<std::remove_cvref_t<decltype(v[0])>>;
	//	{ gtl::data(v) }	-> std::convertible_to<std::remove_cvref_t<decltype(v[0])>*>;
	//	{ std::size(v) }	-> std::convertible_to<size_t>;
	//};

	template < typename tcontainer >
	concept string_container_fixed =
		std::is_bounded_array_v<tcontainer>
		or requires ( tcontainer v ) {
			{ v[0] }			-> std::convertible_to<std::remove_reference_t<decltype(v[0])>>;
			{ v.data() }	-> std::convertible_to<std::remove_reference_t<decltype(v[0])>*>;
			{ v.size() }	-> std::convertible_to<size_t>;
	};

	template < typename tcontainer, typename tchar >
	concept string_container_fixed_c = 
		(std::is_same_v<typename tcontainer::value_type, tchar> or std::is_same_v<std::remove_reference_t<decltype(tcontainer()[0])>, tchar>)
		and string_container_fixed<tcontainer>;

	//template < template <typename tchar> typename Container, typename tchar >
	//concept string_container = requires ( Container<tchar> T ) {
	//	T.size();
	//	T.data();
	//};

}

namespace gtlc = gtl::concepts;
