//////////////////////////////////////////////////////////////////////
//
// rect.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 전체 수정
//
//////////////////////////////////////////////////////////////////////

module;

#include <compare>
#include <functional>
#include <algorithm>
#include <bit>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:coord_rect;

import :concepts;
import :misc;
import :coord_size;
import :coord_point;

//#ifdef min
//#	undef min	// undefine WinAPI - min. ( use std::min only.)
//#endif

export namespace gtl {

	template < typename T, int DEFAULT_FRONT = -1, int DEFAULT_BACK = 1 > struct TRect3;
	template < typename T > struct TRect2;

	//--------------------------------------------------------------------------------------------------------------------------------
	// Rect 3 (left, top, front, right, bottom, back)
	//
	template < typename T, int DEFAULT_FRONT, int DEFAULT_BACK >
	struct TRect3 {
	public:
		T left{}, top{}, front{DEFAULT_FRONT};
		T right{}, bottom{}, back{DEFAULT_BACK};

		using coord_t = std::array<T, 6>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_t& member(int i) { return data()[i]; }
		constexpr value_t const& member(int i) const { return data()[i]; }
		constexpr TPoint3<T>& pts(int i = 0) { return (i == 0) ? *std::bit_cast<TPoint3<T>*>(&left) : *std::bit_cast<TPoint3<T>*>(&right); }
		constexpr TPoint3<T> const& pts(int i = 0) const { return (i == 0) ? *std::bit_cast<TPoint3<T>*>(&left) : *std::bit_cast<TPoint3<T>*>(&right); }

	public:
	// Constructors
		constexpr T default_front() const { return DEFAULT_FRONT; }
		constexpr T default_back() const { return DEFAULT_BACK; }

		TRect3() = default;
		TRect3(TRect3 const&) = default;

		TRect3(T l, T t, T r, T b) :
			left(l), top(t), right(r), bottom(b), front(default_front()), back(default_back()) {}
		TRect3(T l, T t, T f, T r, T b, T bk) :
			left(l), top(t), front(f),
			right(r), bottom(b), back(bk) {}
		template < typename T2 >
		TRect3(T2 l, T2 t, T2 r, T2 b) :
			left(RoundOrForward<T>(l)), top(RoundOrForward<T>(t)),
			right(RoundOrForward<T>(r)), bottom(RoundOrForward<T>(b)),
			front(default_front()), back(default_back()) {}
		template < typename T2 >
		TRect3(T2 l, T2 t, T2 f, T2 r, T2 b, T2 bk) :
			left(RoundOrForward<T>(l)), top(RoundOrForward<T>(t)), front(RoundOrForward<T>(f)),
			right(RoundOrForward<T>(r)), bottom(RoundOrForward<T>(b)), back(RoundOrForward<T>(bk)) {}

		template < gtlc::generic_coord T_COORD >
		explicit TRect3(T_COORD const& B) { *this = B; }
		template < gtlc::generic_coord T_COORD >
		TRect3& operator = (T_COORD const& B) {
			if constexpr (gtlc::rect3<T_COORD>) {
				pts(0) = B.pts(0);
				pts(1) = B.pts(1);
			}
			else {
				if constexpr (gtlc::has__xy<T_COORD>) {
					left  = RoundOrForward<T>(B.x);
					top   = RoundOrForward<T>(B.y);
					if constexpr (gtlc::has__z<T_COORD>) {
						front = RoundOrForward<T>(B.z);
					}
				}

				if constexpr (gtlc::has__size2<T_COORD>) {
					right = left + RoundOrForward<T>(B.width);
					bottom = top + RoundOrForward<T>(B.height);
					if constexpr (gtlc::has__depth<T_COORD>) {
						back = front + RoundOrForward<T>(B.depth);
					}
				} else if constexpr (gtlc::has__cxy<T_COORD>) {
					right = left + RoundOrForward<T>(B.cx);
					bottom = top + RoundOrForward<T>(B.cy);
					if constexpr (gtlc::has__cz<T_COORD>) {
						back = front + RoundOrForward<T>(B.cz);
					}
				} else if constexpr (gtlc::wnd_rect<T_COORD>) {
					left = B.left;
					top = B.top;
					right = B.right;
					bottom = B.bottom;
					//z = {};
				} else if constexpr (gtlc::has__xy<T_COORD>) { // 삭제금지!. static_assert 걸러내기 위해서 첫 번째 if 절 다시 추가.
				} else {
					static_assert(false);
				}
			}
			return *this;
		}

	// 3d
		template < gtlc::has__xyz T_POINT >
		TRect3(T_POINT const& pt, T_POINT const& pt2) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, front{RoundOrForward<T>(pt.z)},
			right{RoundOrForward<T>(pt2.x)}, bottom{RoundOrForward<T>(pt2.y)}, back{RoundOrForward<T>(pt2.z)} {};

		template < gtlc::has__xyz T_POINT, gtlc::has__cxyz T_SIZE>
		TRect3(T_POINT const& pt, T_SIZE const& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, front{RoundOrForward<T>(pt.z)},
			right{RoundOrForward<T>(pt.x+size.cx)}, bottom{RoundOrForward<T>(pt.y+size.cy)}, back{RoundOrForward<T>(pt.z+size.cz)} {};

		template < gtlc::has__xyz T_POINT, gtlc::has__size3 T_SIZE>
		TRect3(T_POINT const& pt, T_SIZE const& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, front{RoundOrForward<T>(pt.z)},
			right{RoundOrForward<T>(pt.x+size.width)}, bottom{RoundOrForward<T>(pt.y+size.height)}, back{RoundOrForward<T>(pt.z+size.depth)} {};

	// 2d
		template < gtlc::has__xy T_POINT >
		TRect3(T_POINT const& pt, T_POINT const& pt2) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt2.x)}, bottom{RoundOrForward<T>(pt2.y)},
			front(default_front()), back(default_back()) {};

		template < gtlc::has__xy T_POINT, gtlc::has__cxy T_SIZE>
		TRect3(T_POINT const& pt, T_SIZE const& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.cx)}, bottom{RoundOrForward<T>(pt.y+size.cy)},
			front(default_front()), back(default_back()) {};

		template < gtlc::has__xy T_POINT, gtlc::has__size2 T_SIZE>
		TRect3(T_POINT const& pt, T_SIZE const& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.width)}, bottom{RoundOrForward<T>(pt.y+size.height)},
			front(default_front()), back(default_back()) {};

	// assign operator
		TRect3& operator = (TRect3 const& B) = default;

		T Width() const		{ return right - left; }
		T Height() const	{ return bottom - top; }
		T Depth() const		{ return back - front; }

	// Type Casting to cv::Size_, SIZE, ...
		template < gtlc::wnd_rect T_COORD >
		operator T_COORD () const { using T2 = decltype(T_COORD::left); return T_COORD{RoundOrForward<T2>(left), RoundOrForward<T2>(top), RoundOrForward<T2>(right), RoundOrForward<T2>(bottom) }; }
		template < gtlc::cv_rect T_COORD >
		operator T_COORD () const { using T2 = decltype(T_COORD::x); return T_COORD{RoundOrForward<T2>(left), RoundOrForward<T2>(top), RoundOrForward<T2>(right-left), RoundOrForward<T2>(bottom-top) }; }
		template < typename T2 > operator TRect3<T2> () const { return TRect3<T2>(left, top, front, right, bottom, back); }
		template < typename T2 > operator TRect2<T2> () const { return TRect2<T2>(left, top, right, bottom); }

	// Compare
		auto operator <=> (TRect3 const&) const = default;

				TPoint2<T>& TopLeft()				{ return (TPoint2<T>&)pts(0); }
		const	TPoint2<T>& TopLeft() const			{ return (TPoint2<T>&)pts(0); }
				TPoint3<T>& TopLeftFront()			{ return pts(0); }
		const	TPoint3<T>& TopLeftFront() const	{ return pts(0); }

				TPoint2<T>& BottomRight()			{ return (TPoint2<T>&)pts(1); }
		const	TPoint2<T>& BottomRight() const		{ return (TPoint2<T>&)pts(1); }
				TPoint3<T>& BottomRightEnd()		{ return pts(1); }
		const	TPoint3<T>& BottomRightEnd() const	{ return pts(1); }

		TPoint3<T> CenterPoint() const { return (pts(0)+pts(1))/2; }

		TSize3<T> GetSize() const { return pts(1)-pts(0); }

		// returns true if rectangle has no area
		bool IsRectEmpty() const {
			return (left >= right) || (top >= bottom) || (front >= back);
		}
		// returns true if rectangle is at (0,0,0) and has no area
		bool IsRectNull() const {
			return (pts(0) == TPoint3<T>{}) and (pts(1) == TPoint3<T>{});
		}
		bool PtInRect(TPoint3<T> const& pt) const {
			return (pt.x >= left) && (pt.y >= top) && (pt.z >= front) && (pt.x < right) && (pt.y < bottom) && (pt.z < back);
		}
		bool PtInRect(TPoint2<T> const& pt) const {
			return (pt.x >= left) && (pt.y >= top) && (pt.x < right) && (pt.y < bottom);
		}
		// returns true if rect is within rectangle
		/// @brief *this including B
		/// @return 
		bool RectInRect(TRect3 const& B) const {
			return (pts(0) <= B.pts(0)) and (B.pts(1) <= pts(1));
		}
		/// @brief *this including B (No boundary)
		/// @return 
		bool RectInRectNE(TRect3 const& B) const {
			return (pts(0) < B.pts(0)) and (B.pts(1) < pts(1));
		}

		// Operations

		// set rectangle from left, top, right, and bottom
		void SetRect(T x1 = 0, T y1 = 0, T z1 = default_front(), T x2 = 0, T y2 = 0, T z2 = default_back()) {
			left = x1; top = y1; front = z1; right = x2; bottom = y2; back = z2;
		}

		void SetRect(TPoint3<T> const& pt, TSize3<T> const& size)		{ pts(0) = pt; pts(1) = pt+size; }
		void SetRect(TPoint3<T> const& pt0, TPoint3<T> const& pt1)		{ pts(0) = pt0; pts(1) = pt1; }
		void SetRectEmpty() { pts(0).SetZero(); pts(1).SetZero(); }

		// infflate the rectangles's width, height and depth
		TRect3& InflateRect(T x, T y, T z) { left -= x; top -= y; front -= z; right += x; bottom += y; back += z; return *this; }
		TRect3& InflateRect(TSize3<T> const& size) { pts(0) -= size; pts(1) += size; return *this; }
		TRect3& InflateRect(TRect3<T> const& rect) { pts(0) -= rect.pts(0); pts(1) += rect.pts(1); return *this; }
		TRect3& InflateRect(T l, T t, T r, T b) { left -= l; top -= t; right += r; bottom += b; return *this; }
		TRect3& InflateRect(T l, T t, T f, T r, T b, T bk) { left -= l; top -= t; front -= f; right += r; bottom += b; back += bk; return *this; }

		// deflate the rectangle's width, height and depth
		TRect3& DeflateRect(T x, T y, T z) { left += x; top += y; front += z; right -= x; bottom -= y; back -= z; return *this; }
		TRect3& DeflateRect(TSize3<T> const& size) { pts(0) += size; pts(1) -= size; return *this; }
		TRect3& DeflateRect(TRect3<T> const& rect) { pts(0) += rect.pts(0); pts(1) -= rect.pts(1); return *this; }
		TRect3& DeflateRect(T l, T t, T r, T b)  { left += l; top += t; right -= r; bottom -= b; return *this; }
		TRect3& DeflateRect(T l, T t, T f, T r, T b, T bk) { left += l; top += t; front += f; right -= r; bottom -= b; back -= bk; return *this; }

		// translate the rectangle by moving its top and left
		TRect3& OffsetRect(T x, T y, T z) { left += x; top += y; front += z; right += x; bottom += y; back += z; return *this; }
		TRect3& OffsetRect(TPoint3<T> const& pt) { pts(0) += pt; pts(1) += pt; return *this; }			// for Point : move whole rect, for Size : move pts(1) only.
		TRect3& OffsetSize(TSize3<T> const& size) { /*pt0 += size;*/ pts(1) += size; return *this; }	// for Point : move whole rect, for Size : move pts(1) only.

	public:
		void NormalizeRect() {
			if (left > right) std::swap(left, right);
			if (top > bottom) std::swap(top, bottom);
			if (front > back) std::swap(front, back);
		}

		// absolute position of rectangle
		void MoveToX(T x) { right += (x-left); left = x; }
		void MoveToY(T y) { bottom += (y-top); top = y; }
		void MoveToZ(T z) { back += (z-front); front = z; }
		//void MoveToN(int i, T v) { assert(pt0.data().size() > i); pt1[i] += (v-pt0[i]); pt0[i] = v; }

		void MoveToXY(T x, T y) { MoveToX(x); MoveToY(y); }
		void MoveToYZ(T y, T z) { MoveToY(y); MoveToZ(z); }
		void MoveToZX(T z, T x) { MoveToZ(z); MoveToX(x); }
		void MoveToXYZ(T x, T y, T z) { MoveToX(x); MoveToY(y); MoveToZ(z); }

		void MoveTo(TPoint3<T> const& pt) {
			MoveToX(pt.x);
			MoveToY(pt.y);
			MoveToZ(pt.z);
		}

		// set this rectangle to intersection of two others
		TRect3& IntersectRect(TRect3 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pts(0).x = std::max(pts(0).x, rect2.pts(0).x);
			pts(0).y = std::max(pts(0).y, rect2.pts(0).y);
			pts(0).z = std::max(pts(0).z, rect2.pts(0).z);

			pts(1).x = std::min(pts(1).x, rect2.pts(1).x);
			pts(1).y = std::min(pts(1).y, rect2.pts(1).y);
			pts(1).z = std::min(pts(1).z, rect2.pts(1).z);

			return *this;
		}

		// set this rectangle to bounding union of two others
		TRect3& UnionRect(TRect3 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pts(0).x = std::min(pts(0).x, rect2.pts(0).x);
			pts(0).y = std::min(pts(0).y, rect2.pts(0).y);
			pts(0).z = std::min(pts(0).z, rect2.pts(0).z);

			pts(1).x = std::max(pts(1).x, rect2.pts(1).x);
			pts(1).y = std::max(pts(1).y, rect2.pts(1).y);
			pts(1).z = std::max(pts(1).z, rect2.pts(1).z);

			return *this;
		}

		// Operators
		TRect3& operator += (TPoint3<T> const& pt)		{ return OffsetRect(pt); }
		TRect3& operator -= (TPoint3<T> const& pt)		{ return OffsetRect(-pt); }
		TRect3& operator += (TSize3<T> const& size)		{ return OffsetSize(size); }
		TRect3& operator -= (TSize3<T> const& size)		{ return OffsetSize(-size); }
		TRect3& operator += (TRect3 const& rect)		{ return InflateRect(rect); }
		TRect3& operator -= (TRect3 const& rect)		{ return DeflateRect(rect); }
		TRect3& operator &= (TRect3 const& rect)		{ return IntersectRect(rect); }
		TRect3& operator |= (TRect3 const& rect)		{ return UnionRect(rect); }

		// Operators returning TRect3 data()
		TRect3 operator + (TPoint3<T> const& pt) const	{ return TRect3(*this) += pt; }
		TRect3 operator - (TPoint3<T> const& pt) const	{ return TRect3(*this) -= pt; }
		TRect3 operator + (TSize3<T> const& size) const	{ return TRect3(*this) += size; }
		TRect3 operator - (TSize3<T> const& size) const	{ return TRect3(*this) -= size; }
		TRect3 operator + (TRect3 const& rect) const	{ return TRect3(*this) += rect; }
		TRect3 operator - (TRect3 const& rect) const	{ return TRect3(*this) -= rect; }
		TRect3 operator & (TRect3 const& rect) const	{ return TRect3(*this).IntersectRect(rect); }
		TRect3 operator | (TRect3 const& rect) const	{ return TRect3(*this).UnionRect(rect); }

		template < typename Archive > friend Archive& operator & (Archive& ar, TRect3 const& B) {
			return ar & B.pts(0) & B.pts(1);
		}
		template < typename JSON > friend void from_json(JSON const& j, TRect3& B) { B.pts(0) = j["pt0"]; B.pts(1) = j["pt1"]; }
		template < typename JSON > friend void to_json(JSON& j, TRect3 const& B) { j["pt0"] = B.pts(0); j["pt1"] = B.pts(1); }
	};


	//--------------------------------------------------------------------------------------------------------------------------------
	// Rect 2 (left, top, right, bottom)
	//
	template < typename T >
	struct TRect2 {
	public:
		T left{}, top{};
		T right{}, bottom{};

		using coord_t = std::array<T, 4>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_t& member(int i) { return data()[i]; }
		constexpr value_t const& member(int i) const { return data()[i]; }
		constexpr TPoint2<T>& pts(int i = 0) { return (i == 0) ? *std::bit_cast<TPoint2<T>*>(&left) : *std::bit_cast<TPoint2<T>*>(&right); }
		constexpr TPoint2<T> const& pts(int i = 0) const { return (i == 0) ? *std::bit_cast<TPoint2<T>*>(&left) : *std::bit_cast<TPoint2<T>*>(&right); }

	public:
		// Constructors

		TRect2() = default;
		TRect2(TRect2 const&) = default;

		TRect2(T l, T t, T r, T b) :
			left(l), top(t), right(r), bottom(b) {}
		template < typename T2 >
		TRect2(T2 l, T2 t, T2 r, T2 b) :
			left(RoundOrForward<T>(l)), top(RoundOrForward<T>(t)),
			right(RoundOrForward<T>(r)), bottom(RoundOrForward<T>(b)) {}

		template < gtlc::generic_coord T_COORD >
		explicit TRect2(T_COORD const& B) { *this = B; }
		template < gtlc::generic_coord T_COORD >
		TRect2& operator = (T_COORD const& B) {
			if constexpr (gtlc::rect2<T_COORD>) {
				pts(0) = B.pts(0);
				pts(1) = B.pts(1);
			}
			else {
				if constexpr (gtlc::has__xy<T_COORD>) {
					left  = RoundOrForward<T>(B.x);
					top   = RoundOrForward<T>(B.y);
				}

				if constexpr (gtlc::has__size2<T_COORD>) {
					right = left + RoundOrForward<T>(B.width);
					bottom = top + RoundOrForward<T>(B.height);
				} else if constexpr (gtlc::has__cxy<T_COORD>) {
					right = left + RoundOrForward<T>(B.cx);
					bottom = top + RoundOrForward<T>(B.cy);
				} else if constexpr (gtlc::wnd_rect<T_COORD>) {
					left = B.left;
					top = B.top;
					right = B.right;
					bottom = B.bottom;
					//z = {};
				} else if constexpr (gtlc::has__xy<T_COORD>) { // 삭제금지!. static_assert 걸러내기 위해서 첫 번째 if 절 다시 추가.
				} else {
					static_assert(false);
				}
			}
			return *this;
		}

		// 3d
		template < gtlc::has__xy T_POINT >
		TRect2(T_POINT const& pt, T_POINT const& pt2) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, right{RoundOrForward<T>(pt2.x)}, bottom{RoundOrForward<T>(pt2.y)} {};

		template < gtlc::has__xy T_POINT, gtlc::has__cxy T_SIZE>
		TRect2(T_POINT const& pt, T_SIZE const& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, right{RoundOrForward<T>(pt.x+size.cx)}, bottom{RoundOrForward<T>(pt.y+size.cy)} {};

		template < gtlc::has__xy T_POINT, gtlc::has__size2 T_SIZE>
		TRect2(T_POINT const& pt, T_SIZE const& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, right{RoundOrForward<T>(pt.x+size.width)}, bottom{RoundOrForward<T>(pt.y+size.height)} {};

		// assign operator
		TRect2& operator = (TRect2 const& B) = default;

		T Width() const		{ return right - left; }
		T Height() const	{ return bottom - top; }

		// Type Casting to cv::Size_, SIZE, ...
		template < gtlc::wnd_rect T_COORD >
		operator T_COORD () const { using T2 = decltype(T_COORD::left); return T_COORD{RoundOrForward<T2>(left), RoundOrForward<T2>(top), RoundOrForward<T2>(right), RoundOrForward<T2>(bottom) }; }
		template < gtlc::cv_rect T_COORD >
		operator T_COORD () const { using T2 = decltype(T_COORD::x); return T_COORD{RoundOrForward<T2>(left), RoundOrForward<T2>(top), RoundOrForward<T2>(right-left), RoundOrForward<T2>(bottom-top) }; }
		//template < typename T2 > operator TRect3<T2> () const { return TRect3<T2>(left, top, -1, right, bottom, 1); }
		template < typename T2 > operator TRect2<T2> () const { return TRect2<T2>(left, top, right, bottom); }

		// Compare
		auto operator <=> (TRect2 const&) const = default;

				TPoint2<T>& TopLeft()				{ return pts(0); }
		const	TPoint2<T>& TopLeft() const			{ return pts(0); }

				TPoint2<T>& BottomRight()			{ return pts(1); }
		const	TPoint2<T>& BottomRight() const		{ return pts(1); }

		TPoint2<T> CenterPoint() const { return (pts(0)+pts(1))/2; }

		TSize2<T> GetSize() const { return pts(1)-pts(0); }

		// returns true if rectangle has no area
		bool IsRectEmpty() const {
			return (left >= right) || (top >= bottom);
		}
		// returns true if rectangle is at (0,0,0) and has no area
		bool IsRectNull() const {
			return (pts(0) == TPoint2<T>{}) and (pts(1) == TPoint2<T>{});
		}
		bool PtInRect(TPoint2<T> const& pt) const {
			return (pt.x >= left) && (pt.y >= top) && (pt.x < right) && (pt.y < bottom);
		}
		// returns true if rect is within rectangle
		/// @brief *this including B
		/// @return 
		bool RectInRect(TRect2 const& B) const {
			return (pts(0) <= B.pts(0)) and (B.pts(1) <= pts(1));
		}
		/// @brief *this including B (No boundary)
		/// @return 
		bool RectInRectNE(TRect2 const& B) const {
			return (pts(0) < B.pts(0)) and (B.pts(1) < pts(1));
		}

		// Operations

		// set rectangle from left, top, right, and bottom
		void SetRect(T x1 = 0, T y1 = 0, T x2 = 0, T y2 = 0) {
			left = x1; top = y1; right = x2; bottom = y2;
		}

		void SetRect(TPoint2<T> const& pt, TSize2<T> const& size)		{ pts(0) = pt; pts(1) = pt+size; }
		void SetRect(TPoint2<T> const& pt0, TPoint2<T> const& pt1)		{ pts(0) = pt0; pts(1) = pt1; }
		void SetRectEmpty() { pts(0).SetZero(); pts(1).SetZero(); }

		// infflate the rectangles's width, height and depth
		TRect2& InflateRect(T x, T y, T z) { left -= x; top -= y; right += x; bottom += y; return *this; }
		TRect2& InflateRect(TSize2<T> const& size) { pts(0) -= size; pts(1) += size; return *this; }
		TRect2& InflateRect(TRect2<T> const& rect) { pts(0) -= rect.pts(0); pts(1) += rect.pts(1); return *this; }
		TRect2& InflateRect(T l, T t, T r, T b) { left -= l; top -= t; right += r; bottom += b; return *this; }

		// deflate the rectangle's width, height and depth
		TRect2& DeflateRect(T x, T y, T z) { left += x; top += y; right -= x; bottom -= y; return *this; }
		TRect2& DeflateRect(TSize2<T> const& size) { pts(0) += size; pts(1) -= size; return *this; }
		TRect2& DeflateRect(TRect2<T> const& rect) { pts(0) += rect.pts(0); pts(1) -= rect.pts(1); return *this; }
		TRect2& DeflateRect(T l, T t, T r, T b)  { left += l; top += t; right -= r; bottom -= b; return *this; }

		// translate the rectangle by moving its top and left
		TRect2& OffsetRect(T x, T y) { left += x; top += y; right += x; bottom += y; return *this; }
		TRect2& OffsetRect(TPoint2<T> const& pt) { pts(0) += pt; pts(1) += pt; return *this; }			// for Point : move whole rect, for Size : move pts(1) only.
		TRect2& OffsetSize(TSize2<T> const& size) { /*pt0 += size;*/ pts(1) += size; return *this; }	// for Point : move whole rect, for Size : move pts(1) only.

	public:
		void NormalizeRect() {
			if (left > right) std::swap(left, right);
			if (top > bottom) std::swap(top, bottom);
		}

		// absolute position of rectangle
		void MoveToX(T x) { right += (x-left); left = x; }
		void MoveToY(T y) { bottom += (y-top); top = y; }
		//void MoveToN(int i, T v) { assert(pt0.data().size() > i); pt1[i] += (v-pt0[i]); pt0[i] = v; }

		void MoveToXY(T x, T y) { MoveToX(x); MoveToY(y); }

		void MoveTo(TPoint2<T> const& pt) {
			MoveToX(pt.x);
			MoveToY(pt.y);
		}

		// set this rectangle to intersection of two others
		TRect2& IntersectRect(TRect2 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pts(0).x = std::max(pts(0).x, rect2.pts(0).x);
			pts(0).y = std::max(pts(0).y, rect2.pts(0).y);

			pts(1).x = std::min(pts(1).x, rect2.pts(1).x);
			pts(1).y = std::min(pts(1).y, rect2.pts(1).y);

			return *this;
		}

		// set this rectangle to bounding union of two others
		TRect2& UnionRect(TRect2 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pts(0).x = std::min(pts(0).x, rect2.pts(0).x);
			pts(0).y = std::min(pts(0).y, rect2.pts(0).y);

			pts(1).x = std::max(pts(1).x, rect2.pts(1).x);
			pts(1).y = std::max(pts(1).y, rect2.pts(1).y);

			return *this;
		}

		// Operators
		TRect2& operator += (TPoint2<T> const& pt)		{ return OffsetRect(pt); }
		TRect2& operator -= (TPoint2<T> const& pt)		{ return OffsetRect(-pt); }
		TRect2& operator += (TSize2<T> const& size)		{ return OffsetSize(size); }
		TRect2& operator -= (TSize2<T> const& size)		{ return OffsetSize(-size); }
		TRect2& operator += (TRect2 const& rect)		{ return InflateRect(rect); }
		TRect2& operator -= (TRect2 const& rect)		{ return DeflateRect(rect); }
		TRect2& operator &= (TRect2 const& rect)		{ return IntersectRect(rect); }
		TRect2& operator |= (TRect2 const& rect)		{ return UnionRect(rect); }

		// Operators returning TRect3 data()
		TRect2 operator + (TPoint2<T> const& pt) const	{ return TRect2(*this) += pt; }
		TRect2 operator - (TPoint2<T> const& pt) const	{ return TRect2(*this) -= pt; }
		TRect2 operator + (TSize2<T> const& size) const	{ return TRect2(*this) += size; }
		TRect2 operator - (TSize2<T> const& size) const	{ return TRect2(*this) -= size; }
		TRect2 operator + (TRect2 const& rect) const	{ return TRect2(*this) += rect; }
		TRect2 operator - (TRect2 const& rect) const	{ return TRect2(*this) -= rect; }
		TRect2 operator & (TRect2 const& rect) const	{ return TRect2(*this).IntersectRect(rect); }
		TRect2 operator | (TRect2 const& rect) const	{ return TRect2(*this).UnionRect(rect); }

		template < typename Archive > friend Archive& operator & (Archive& ar, TRect2 const& B) {
			return ar & B.pts(0) & B.pts(1);
		}
		template < typename JSON > friend void from_json(JSON const& j, TRect2& B) { B.pts(0) = j["pt0"]; B.pts(1) = j["pt1"]; }
		template < typename JSON > friend void to_json(JSON& j, TRect2 const& B) { j["pt0"] = B.pts(0); j["pt1"] = B.pts(1); }

		//-----------------------------------------------------------------------------
		// ROI
		//
		template <typename T2 = std::int32_t>
		[[nodiscard]] bool IsROI_Valid(TSize2<T2> const& sizeImage) const {
			if constexpr (std::is_integral_v(T2)) {
				return 1
					&& (RoundOrForward(left) >= 0)
					&& (RoundOrForward(top) >= 0)
					&& ( (sizeImage.cx < 0) || ( (RoundOrForward(left) < sizeImage.cx) && (RoundOrForward(right) < sizeImage.cx) ) )
					&& ( (sizeImage.cy < 0) || ( (RoundOrForward(top) < sizeImage.cy) && (RoundOrForward(bottom) < sizeImage.cy) ) )
					;
			} else {
				return 1
					&& (left >= 0)
					&& (top >= 0)
					&& ( (sizeImage.cx < 0) || ( (left < sizeImage.cx) && (right < sizeImage.cx) ) )
					&& ( (sizeImage.cy < 0) || ( (top < sizeImage.cy) && (bottom < sizeImage.cy) ) )
					;
			}
		}

		bool AdjustROI(TSize2<T> const& sizeImage) {
			NormalizeRect();

			if (left < 0)
				left = 0;
			if (top < 0)
				top = 0;
			if ( (sizeImage.cx > 0) && (right > sizeImage.cx) )
				right = sizeImage.cx;
			if ( (sizeImage.cy > 0) && (bottom > sizeImage.cy) )
				bottom = sizeImage.cy;

			return !IsRectEmpty();
		}

		template <typename T2 = std::int32_t>
		[[nodiscard]] TRect2<T2> GetSafeROI(TSize2<T2> const& sizeImage) const {
			TRect2<T2> rectAdjusted(*this);

			rectAdjusted.NormalizeRect();

			if (rectAdjusted.left < 0)
				rectAdjusted.left = 0;
			if (rectAdjusted.top < 0)
				rectAdjusted.top = 0;
			if ( (sizeImage.cx > 0) && (rectAdjusted.right > sizeImage.cx) )
				rectAdjusted.right = sizeImage.cx;
			if ( (sizeImage.cy > 0) && (rectAdjusted.bottom > sizeImage.cy) )
				rectAdjusted.bottom = sizeImage.cy;

			return rectAdjusted;
		}

	};

}	// namespace gtl
