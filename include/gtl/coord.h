#pragma once

//////////////////////////////////////////////////////////////////////
//
// PWH
// 2021.05.12. from Mocha
//
//////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <compare>
#include "gtl/concepts.h"

namespace gtl {
#pragma pack(push, 8)

	template < typename T > class T_POINT2 {
	public:
		union { struct { T x, y; }; T val[2]; };
		using coord_t = typename decltype(val);

		T_POINT2(T _x = 0, T _y = 0) : x(_x), y(_y) {}
		T_POINT2(T_POINT2 const&) = default;

		auto operator <=> (T_POINT2 const&) const = default;
	};
	template < typename T > class T_POINT3 {
	public:
		union { struct { T x, y, z; }; T val[3]; };
		using coord_t = typename decltype(val);

		T_POINT3(T _x = 0, T _y = 0, T _z = 0) : x(_x), y(_y), z(_z) {}
		T_POINT3(T_POINT3 const&) = default;

		auto operator <=> (T_POINT3 const&) const = default;
	};

	template < typename T > class T_SIZE2 {
	public:
		union { struct { T cx, cy; }; T val[2]; };
		using coord_t = typename decltype(val);

		T_SIZE2(T _cx = 0, T _cy = 0) : cx(_cx), cy(_cy) {}
		T_SIZE2(T_SIZE2 const&) = default;

		auto operator <=> (T_SIZE2 const&) const = default;
	};
	template < typename T > class T_SIZE3 {
	public:
		union { struct { T cx, cy, cz; }; T val[3]; };
		using coord_t = typename decltype(val);

		T_SIZE3(T _cx = 0, T _cy = 0, T _cz = 0) : cx(_cx), cy(_cy), cz(_cz) {}
		T_SIZE3(T_SIZE3 const&) = default;

		auto operator <=> (T_SIZE3 const&) const = default;
	};

	template < typename T >
	class T_RECT2 {
	public:
		union {
			struct {
				union {
					struct { T_POINT2<T> pt0; };
					struct { T left, top; };
				};
				union {
					struct { T_POINT2<T> pt1; };
					struct { T right, bottom; };
				};
			};
			T val[4];
		};
		using coord_t = typename decltype(val);

		T_RECT2(T l = 0, T t = 0, T r = 0, T b = 0) : left(l), top(t), right(r), bottom(b) {}
		T_RECT2(T_RECT2 const&) = default;
		T_RECT2(const T_POINT2<T>& pt, const T_SIZE2<T>& size) : pt0(pt), pt1(pt0+size) {}
		T_RECT2(const T_POINT2<T>& _pt0, const T_POINT2<T>& _pt1) : pt0(_pt0), pt1(_pt1) {}

		auto operator <=> (T_RECT2 const&) const = default;
	};
	template < typename T >
	class T_RECT3 {
	public:
		union {
			struct {
				union {
					struct { T_POINT3<T> pt0; };
					struct { T left, top, front; };
				};
				union {
					struct { T_POINT3<T> pt1; };
					struct { T right, bottom, back; };
				};
			};
			T val[6];
		};
		using coord_t = typename decltype(val);

		T_RECT3(T l = 0, T t = 0, T f = 0, T r = 0, T b = 0, T bk = 0) : left(l), top(t), front(f), right(r), bottom(b), back(bk) {}
		T_RECT3(T_RECT3 const&) = default;
		T_RECT3(const T_POINT3<T>& pt, const T_SIZE3<T>& size) : pt0(pt), pt1(pt0+size) {}
		T_RECT3(const T_POINT3<T>& _pt0, const T_POINT3<T>& _pt1) : pt0(_pt0), pt1(_pt1) {}

		auto operator <=> (T_RECT3 const&) const = default;
	};

	using SIZE2I	= T_SIZE2<int32_t>;
	using SIZE3I	= T_SIZE3<int32_t>;
	using SIZE2D	= T_SIZE2<double>;
	using SIZE3D	= T_SIZE3<double>;
	using POINT2I	= T_POINT2<int32_t>;
	using POINT3I	= T_POINT3<int32_t>;
	using POINT2D	= T_POINT2<double>;
	using POINT3D	= T_POINT3<double>;
	using RECT2I	= T_RECT2<int32_t>;
	using RECT3I	= T_RECT3<int32_t>;
	using RECT2D	= T_RECT2<double>;
	using RECT3D	= T_RECT3<double>;

#pragma pack(pop)
}	// namespace gtl
