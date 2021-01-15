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


	/// @brief trivially copyable
	template < typename T >
	concept trivially_copyable = std::is_trivially_copyable_v<T>;


	/// @brief array or std::array
	template < typename tarray >
	concept is_array = std::is_array_v<tarray> or std::convertible_to<tarray, std::array<typename tarray::value_type, tarray{}.size()>>;


	/// @brief variadic template check version of 'std::is_same_v<t, t2>'
	template < typename tchar, typename ... ttypes >
	concept is_one_of = (std::is_same_v<tchar, ttypes> || ...);


	/// @brief type for string buffer. ex) char buf[12]; std::array<char, 12> buf; std::vector<char> buf;...
	template < typename tcontainer >
	concept contiguous_container =
		requires (tcontainer v) {
			v[0];
			std::data(v);
			std::size(v);
		};


	/// @brief type for string buffer. ex) char buf[12]; std::array<char, 12> buf; std::vector<char> buf;...
	template < typename tcontainer, typename type >
	concept contiguous_type_container =
		requires (tcontainer v) {
			{ v[0] }			-> std::convertible_to<type>;
			{ std::data(v) }	-> std::convertible_to<std::remove_cvref_t<type> const *>;
			{ std::size(v) }	-> std::convertible_to<size_t>;
		};


	/// @brief type for string buffer. ex) char buf[12]; std::array<char, 12> buf; std::vector<char> buf;...
	template < typename tcontainer, typename type >
	concept container =
		requires (tcontainer v) {
			{ v[0] }			-> std::convertible_to<type>;
			{ std::size(v) }	-> std::convertible_to<size_t>;
			v.begin();
			v.end();
		};


	/// @brief type for string (uint16_t for KSSM (Korean Johab)
	template < typename tchar >
	concept string_elem = is_one_of<std::remove_cvref_t<tchar>, char, char8_t, char16_t, char32_t, wchar_t
#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM)
		, uint16_t	// uint16_t for KSSM (Johab)
#endif
	>;

	/// @brief type for utf (unicode transformation format) string.
	template < typename tchar >
	concept string_elem_utf = is_one_of<std::remove_cvref_t<tchar>, char8_t, char16_t, char32_t, wchar_t>;



	/// @brief wchar_t to charXX_t
	template < string_elem tchar >
	struct as_utf {
		using type = tchar;
	};
	template <>
	struct as_utf<wchar_t> {
		using type = std::conditional_t<
				sizeof(wchar_t) == sizeof(char16_t),
					char16_t,
					std::conditional_t<sizeof(wchar_t) == sizeof(char32_t), char32_t, char>
				>;
	};
	template < string_elem tchar >
	using as_utf_t = typename as_utf<tchar>::type;

	/// @brief charXX_t to wchar_t
	template < string_elem tchar >
	struct as_wide {
		using type = tchar;
	};
	template <>
	struct as_wide< as_utf_t<wchar_t> > {
		using type = wchar_t;
	};
	template < string_elem tchar >
	using as_wide_t = typename as_wide<tchar>::type;


	/// @brief check if the type is same. (ex, char16_t == wchar_t for windows)
	template < typename tchar1, typename tchar2 >
	concept is_same_utf = (
		std::is_same_v<tchar1, tchar2>
		or (std::is_same_v<as_utf_t<tchar1>, as_utf_t<tchar2>>)
		);

	///// @brief type for string buffer. ex) char buf[12]; std::array<char, 12> buf; std::vector<char> buf;...
	//template < typename tcontainer >
	//concept contiguous_string_container = 
	//	string_elem<std::remove_cvref_t<decltype(tcontainer{}[0])>>
	//	and
	//	contiguous_container<tcontainer>;
	template < typename tcontainer >
	concept contiguous_string_container = 
		string_elem<std::remove_cvref_t<decltype(tcontainer{}[0])>>
		and
		(
			requires (tcontainer v) {
				v[0];
				{ std::data(v) } -> std::convertible_to<std::remove_cvref_t<decltype(v[0])> const*>;
				{ std::size(v) } -> std::convertible_to<size_t>;
			}
			or
			std::is_array_v<tcontainer>

		);


	/// @brief type for string buffer with type constraints
	template < typename tcontainer, typename tchar >
	concept contiguous_type_string_container = 
		(std::is_same_v<std::remove_cvref_t<typename tcontainer::value_type>, tchar> or std::is_same_v<std::remove_cvref_t<decltype(tcontainer()[0])>, tchar>)
		and
		contiguous_string_container<tcontainer>;


	/// @brief json container. not completed.
	template < typename tjson >
	concept json_like_container = requires (tjson j, char const* psz, int index) {
		// todo : complete json_like_container
		j[psz];
		j[index];
	};


}

namespace gtlc = gtl::concepts;
