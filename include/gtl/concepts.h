//////////////////////////////////////////////////////////////////////
//
// concepts.h:
//
// PWH
//    2020.11.12
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <concepts>
#include "_config.h"

#if !defined(__cpp_lib_concepts)
#	error ERROR! Supports C++v20 only.
#endif

#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM)
namespace gtl {
	using charKSSM_t = uint16_t;
}
#endif

namespace gtl::concepts {

	template<typename> inline constexpr bool dependent_false_v = false;

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


	/// @brief type for string (charKSSM_t for KSSM (Korean Johab)
	template < typename tchar >
	concept string_elem = is_one_of<std::remove_cvref_t<tchar>, char, char8_t, char16_t, char32_t, wchar_t
#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM)
		, charKSSM_t	// charKSSM_t for KSSM (Johab)
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


	/// @brief is__coord
	template < typename T_COORD > concept coord2 = std::is_convertible_v< typename T_COORD::coord_t, std::array<typename T_COORD::value_type, 2> >;
	template < typename T_COORD > concept coord3 = std::is_convertible_v< typename T_COORD::coord_t, std::array<typename T_COORD::value_type, 3> >;
	template < typename T_COORD > concept coord_rect2 = std::is_convertible_v< typename T_COORD::coord_t, std::array<typename T_COORD::value_type, 4> >;
	template < typename T_COORD > concept coord_rect3 = std::is_convertible_v< typename T_COORD::coord_t, std::array<typename T_COORD::value_type, 6> >;
	template < typename T_COORD > concept coord_srect2 = std::is_convertible_v< typename T_COORD::coord_t, std::array<typename T_COORD::value_type, 4> > and requires (T_COORD a) { a.width; a.height; };
	template < typename T_COORD > concept coord_srect3 = std::is_convertible_v< typename T_COORD::coord_t, std::array<typename T_COORD::value_type, 6> > and requires (T_COORD a) { a.width; a.height; a.depth; };
	template < typename T_COORD > concept coord = coord2<T_COORD> or coord3<T_COORD> or coord_rect2<T_COORD> or coord_rect3<T_COORD> or coord_srect2<T_COORD> or coord_srect3<T_COORD>;

	template < typename T_COORD, typename T > concept tcoord2 = std::is_same_v< typename T_COORD::coord_t, std::array<T, 2> >;
	template < typename T_COORD, typename T > concept tcoord3 = std::is_same_v< typename T_COORD::coord_t, std::array<T, 3> >;
	template < typename T_COORD, typename T > concept tcoord = coord2<T_COORD> or coord3<T_COORD>;

	template < typename T_COORD > concept point = requires (T_COORD a) { a.x; a.y; };
	template < typename T_COORD > concept point2 = ( requires (T_COORD a) { a.x; a.y; } and !requires (T_COORD a) { a.z; } );
	template < typename T_COORD > concept point3 = ( requires (T_COORD a) { a.x; a.y; a.z; } );

	template < typename T_COORD > concept size = requires (T_COORD a) { a.cx; a.cy; };
	template < typename T_COORD > concept size2 = ( requires (T_COORD a) { a.cx; a.cy; } and !requires (T_COORD a) { a.cz; } );
	template < typename T_COORD > concept size3 = ( requires (T_COORD a) { a.cx; a.cy; a.cz; } );

	template < typename T_COORD > concept rect = requires (T_COORD a) { a.pt0(); a.pt1(); a.left; a.top; a.right; a.bottom; };
	template < typename T_COORD > concept rect2 = ( gtl::concepts::rect<T_COORD> and requires (T_COORD a) { a.left; a.top; a.right; a.bottom; } and !requires (T_COORD a) { a.front; a.back; } );
	template < typename T_COORD > concept rect3 = ( gtl::concepts::rect<T_COORD> and requires (T_COORD a) { a.left; a.top; a.right; a.bottom; a.front; a.back;} );
	template < typename T_COORD > concept srect = requires (T_COORD a) { a.tl(); a.coord_size(); a.left; a.top; a.width; a.height; };
	template < typename T_COORD > concept srect2 = ( gtl::concepts::srect<T_COORD> and requires (T_COORD a) { a.x; a.y; a.width; a.height; } and !requires (T_COORD a) { a.z; a.depth; } );
	template < typename T_COORD > concept srect3 = ( gtl::concepts::srect<T_COORD> and requires (T_COORD a) { a.x; a.y; a.width; a.height; a.z; a.depth;} );

	template < typename T_COORD > concept wnd_point = requires (T_COORD a) { a.x; a.y; };
	template < typename T_COORD > concept wnd_size = requires (T_COORD a) { a.cx; a.cy; };
	template < typename T_COORD > concept wnd_rect = requires (T_COORD a) { a.left; a.top; a.right; a.bottom; };

	template < typename T_COORD > concept cv_point = requires (T_COORD a) { a.x; a.y; };
	template < typename T_COORD > concept cv_point2 = requires (T_COORD a) { a.x; a.y; } and !requires(T_COORD a) { a.z; };
	template < typename T_COORD > concept cv_point3 = requires (T_COORD a) { a.x; a.y; a.z; };
	template < typename T_COORD > concept cv_size = requires (T_COORD a) { a.width; a.height; };
	template < typename T_COORD > concept cv_rect = requires (T_COORD a) { a.x; a.y; a.width; a.height; };

	template < typename T_COORD > concept has__x = requires (T_COORD a) { a.x; };
	template < typename T_COORD > concept has__y = requires (T_COORD a) { a.y; };
	template < typename T_COORD > concept has__z = requires (T_COORD a) { a.z; };
	template < typename T_COORD > concept has__w = requires (T_COORD a) { a.w; };
	template < typename T_COORD > concept has__xy = requires (T_COORD a) { a.x; a.y; };
	template < typename T_COORD > concept has__xyz = requires (T_COORD a) { a.x; a.y; a.z; };
	template < typename T_COORD > concept has__xyzw = requires (T_COORD a) { a.x; a.y; a.z; a.w; };
	template < typename T_COORD > concept has__cx = requires (T_COORD a) { a.cx; };
	template < typename T_COORD > concept has__cy = requires (T_COORD a) { a.cy; };
	template < typename T_COORD > concept has__cz = requires (T_COORD a) { a.cz; };
	template < typename T_COORD > concept has__cxy = requires (T_COORD a) { a.cx; a.cy; };
	template < typename T_COORD > concept has__cxyz = requires (T_COORD a) { a.cx; a.cy; a.cz; };
	template < typename T_COORD > concept has__width = requires (T_COORD a) { a.width;};
	template < typename T_COORD > concept has__height = requires (T_COORD a) { a.height; };
	template < typename T_COORD > concept has__depth = requires (T_COORD a) { a.depth; };
	template < typename T_COORD > concept has__size2 = requires (T_COORD a) { a.width; a.height; };
	template < typename T_COORD > concept has__size3 = requires (T_COORD a) { a.width; a.height; a.depth; };

	template < typename T_COORD > concept generic_point = has__xy<T_COORD>;
	template < typename T_COORD > concept generic_coord = coord<T_COORD> or wnd_point<T_COORD> or wnd_size<T_COORD> or wnd_rect<T_COORD> or generic_point<T_COORD> or cv_size<T_COORD> or cv_rect<T_COORD>;


	//template < typename T_COORD > concept is__xy = has__xy<T_COORD> && !has__z<T_COORD> && !has__cxy<T_COORD> && !has__cz<T_COORD> && !has__size2<T_COORD>;
	//template < typename T_COORD > concept is__cxy = has__cxy<T_COORD> && !has__cz<T_COORD> && !has__xy<T_COORD> && !has__size2<T_COORD>;
	//template < typename T_COORD > concept is__size2 = has__size2<T_COORD> && !has__depth<T_COORD> && !has__xy<T_COORD> && !has__cxy<T_COORD>;
	//template < typename T_COORD > concept is__xyz = has__xyz<T_COORD> && !has__cxy<T_COORD> && !has__size2<T_COORD>;
	//template < typename T_COORD > concept is__cxyz = has__cxyz<T_COORD> && !has__xy<T_COORD> && !has__size2<T_COORD>;
	//template < typename T_COORD > concept is__size3 = has__size3<T_COORD> && !has__xy<T_COORD> && !has__cxy<T_COORD>;

	//template < typename T_COORD > concept is__coord2 = is__xy<T_COORD>  || is__cxy<T_COORD>  || is__size2<T_COORD>;
	//template < typename T_COORD > concept is__coord3 = is__xyz<T_COORD> || is__cxyz<T_COORD> || is__size3<T_COORD>;

	////template < typename T_COORD > concept is__rect = requires (T_COORD a) { a.pt0; a.pt1; };

	//template < typename T_COORD > concept is__cv_rect = has__xy<T_COORD> && !has__z<T_COORD> && has__size2<T_COORD> && !has__depth<T_COORD>;

}

namespace gtlc = gtl::concepts;
