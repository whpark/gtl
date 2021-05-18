#pragma once

//////////////////////////////////////////////////////////////////////
//
// rect.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 전체 수정
//
//////////////////////////////////////////////////////////////////////

#include "gtl/misc.h"
#include "gtl/concepts.h"

#include "gtl/coord/size.h"
#include "gtl/coord/point.h"

//#ifdef min
//#	undef min	// undefine WinAPI - min. ( use std::min only.)
//#endif

namespace gtl {

#pragma pack(push, 1)

	template < typename T > struct TRect3;
	template < typename T > struct TRect2;

	//--------------------------------------------------------------------------------------------------------------------------------
	// Rect 3 (left, top, front, right, bottom, back)
	//

	template < typename T >
	struct TRect3 {
	public:
		union {
			struct {
				T left, top, front;
				T right, bottom, back;
			};
			struct {
				TPoint3<T> pt0, pt1;
			};
		};

		using coord_t = std::array<T, 6>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr const coord_t& data() const { return std::bit_cast<coord_t const&>(*this); }

	public:
	// Constructors
		constexpr T default_front() const { return -1; }
		constexpr T default_back() const { return 1; }

		TRect3() : left{}, top{}, right{}, bottom{}, front(default_front()), back(default_back()) {}
		TRect3(const TRect3&) = default;

		TRect3(T l, T t, T r, T b) :
			left(l), top(t), right(r), bottom(b), front(default_front()), back(default_back()) {}
		TRect3(T l, T t, T f, T r, T b, T bk) :
			left(l), top(t), front(f),
			right(r), bottom(b), back(bk) {}
		template < typename T2 >
		TRect3(T2 l, T2 t, T2 r, T2 b) :
			left(RoundOrForward<T, T2>(l)), top(RoundOrForward<T, T2>(t)),
			right(RoundOrForward<T, T2>(r)), bottom(RoundOrForward<T, T2>(b)),
			front(default_front()), back(default_back()) {}
		template < typename T2 >
		TRect3(T2 l, T2 t, T2 f, T2 r, T2 b, T2 bk) :
			left(RoundOrForward<T, T2>(l)), top(RoundOrForward<T, T2>(t)), front(RoundOrForward<T, T2>(f)),
			right(RoundOrForward<T, T2>(r)), bottom(RoundOrForward<T, T2>(b)), back(RoundOrForward<T, T2>(bk)) {}

		explicit TRect3(const RECT& B) { *this = B; }

	// 3d
		template < gtl::is__xyz T_POINT >
		TRect3(const T_POINT& pt, const T_POINT& pt2) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, front{RoundOrForward<T>(pt.z)},
			right{RoundOrForward<T>(pt2.x)}, bottom{RoundOrForward<T>(pt2.y)}, back{RoundOrForward<T>(pt2.z)} {};

		template < gtl::is__xyz T_POINT, gtl::is__cxyz T_SIZE>
		TRect3(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, front{RoundOrForward<T>(pt.z)},
			right{RoundOrForward<T>(pt.x+size.cx)}, bottom{RoundOrForward<T>(pt.y+size.cy)}, back{RoundOrForward<T>(pt.z+size.cz)} {};

		template < gtl::is__xyz T_POINT, gtl::is__size3 T_SIZE>
		TRect3(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)}, front{RoundOrForward<T>(pt.z)},
			right{RoundOrForward<T>(pt.x+size.width)}, bottom{RoundOrForward<T>(pt.y+size.height)}, back{RoundOrForward<T>(pt.z+size.depth)} {};

	// 2d
		template < gtl::is__xy T_POINT >
		TRect3(const T_POINT& pt, const T_POINT& pt2) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt2.x)}, bottom{RoundOrForward<T>(pt2.y)},
			front(default_front()), back(default_back()) {};

		template < gtl::is__xy T_POINT, gtl::is__cxy T_SIZE>
		TRect3(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.cx)}, bottom{RoundOrForward<T>(pt.y+size.cy)},
			front(default_front()), back(default_back()) {};

		template < gtl::is__xy T_POINT, gtl::is__size2 T_SIZE>
		TRect3(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.width)}, bottom{RoundOrForward<T>(pt.y+size.height)},
			front(default_front()), back(default_back()) {};

	// assign operator
		TRect3& operator = (const TRect3& B) = default;

		template < typename T_RECT > requires requires (T_RECT rect) { rect.pt0; rect.pt1; }
		TRect3& operator = (const T_RECT& rect) { pt0 = rect.pt0; pt1 = rect.pt1; return *this; };

		template < typename T_CV_RECT > requires requires (T_CV_RECT rc) { rc.x; rc.y; rc.widht; rc.height; }
		TRect3& operator = (const T_CV_RECT& rc) {
			left	= RoundOrForward<T>(rc.x);
			top		= RoundOrForward<T>(rc.y);
			right	= RoundOrForward<T>(rc.x + rc.width);
			bottom	= RoundOrForward<T>(rc.y + rc.height);
			return *this;
		}

		TRect3& operator = (const RECT& srcRect) {
			left = srcRect.left; top = srcRect.top; /*front = 0;*/
			right = srcRect.right; bottom = srcRect.bottom; /*back = 0;*/
			return *this;
		}

		T Width() const		{ return right - left; }
		T Height() const	{ return bottom - top; }
		T Depth() const		{ return back - front; }

	// Type Casting to cv::Size_, SIZE, ...

		template < typename T2 > operator TRect3<T2> () const { return TRect3<T2>(left, top, front, right, bottom, back); }
		template < typename T2 > operator TRect2<T2> () const { return TRect2<T2>(left, top, right, bottom); }
		operator RECT () const { return RECT{RoundOrForward<LONG, T>(left), RoundOrForward<LONG, T>(top), RoundOrForward<LONG, T>(right), RoundOrForward<LONG, T>(bottom)}; }

		template < gtl::is__xy	T_COORD > operator T_COORD() const { return T_COORD{left, top}; }
		template < gtl::is__xyz	T_COORD > operator T_COORD() const { return T_COORD{left, top, front}; }
		template < gtl::is__cxy	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top}; }
		template < gtl::is__cxyz	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top, back-front}; }
		template < gtl::is__size2	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top}; }
		template < gtl::is__size3	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top, back-front}; }

		// cv::Rect_
		template < typename T2, template <typename> typename T_CV_RECT > requires requires (T_CV_RECT<T2> rc) { rc.x; rc.y; rc.widht; rc.height; }
		operator T_CV_RECT<T2>& () const {
			return T_CV_RECT<T2>(
				RoundOrForward<T2, T>(left),
				RoundOrForward<T2, T>(top),
				RoundOrForward<T2, T>(right-left),
				RoundOrForward<T2, T>(bottom-top));
		};

	// Compare
		auto operator <=> (const TRect3&) const = default;

		// B includes *this
		bool operator <= (const TRect3& B) const {
			return (left >= B.left) && (top >= B.top) && (front >= B.front)
				&& (right <= B.right) && (bottom <= B.bottom) && (back <= B.back);
		}
		// *this includes B
		bool operator >= (const TRect3& B) const {	// B includes *this
			return (left <= B.left) && (top <= B.top) && (front <= B.front)
				&& (right >= B.right) && (bottom >= B.bottom) && (back >= B.back);
		}
		// B includes *this
		bool operator < (const TRect3& B) const {
			return (left > B.left) && (top > B.top) && (front > B.front)
				&& (right < B.right) && (bottom < B.bottom) && (back < B.back);
		}
		// *this includes B
		bool operator > (const TRect3& B) const {	// B includes *this
			return (left < B.left) && (top < B.top) && (front < B.front)
				&& (right > B.right) && (bottom > B.bottom) && (back > B.back);
		}
		//auto operator <=> (const T& b) const { return *this <=> TRect3(b, b, b); }

				TPoint2<T>& TopLeft()				{ return (TPoint2<T>&)pt0; }
		const	TPoint2<T>& TopLeft() const			{ return (TPoint2<T>&)pt0; }
				TPoint3<T>& TopLeftFront()			{ return pt0; }
		const	TPoint3<T>& TopLeftFront() const	{ return pt0; }

				TPoint2<T>& BottomRight()			{ return (TPoint2<T>&)pt1; }
		const	TPoint2<T>& BottomRight() const		{ return (TPoint2<T>&)pt1; }
				TPoint3<T>& BottomRightEnd()		{ return pt1; }
		const	TPoint3<T>& BottomRightEnd() const	{ return pt1; }

		TPoint3<T> CenterPoint() const { return {(right+left)/(T)2, (bottom+top)/(T)2, (back+front)/(T)2}; }

		TSize3<T> GetSize() const { return {right-left, bottom-top, back-front}; }

		// returns true if rectangle has no area
		bool IsRectEmpty() const {
			return (left >= right) || (top >= bottom) || (front >= back);
		}
		// returns true if rectangle is at (0,0,0) and has no area
		bool IsRectNull() const {
			return !left && !top && !front && !right && !bottom && !back;
		}
		bool PtInRect(const TPoint3<T>& pt) const {
			return (pt.x >= left) && (pt.y >= top) && (pt.z >= front) && (pt.x < right) && (pt.y < bottom) && (pt.z < back);
		}
		bool PtInRect(const TPoint2<T>& pt) const {
			return (pt.x >= left) && (pt.y >= top) && (pt.x < right) && (pt.y < bottom);
		}
		// returns true if rect is within rectangle
		bool RectInRect(const TRect3<T>& B) const {
			return B <= *this;
		}

		// Operations

		// set rectangle from left, top, right, and bottom
		void SetRect(T x1 = 0, T y1 = 0, T z1 = default_front(), T x2 = 0, T y2 = 0, T z2 = default_back()) {
			left = x1; top = y1; front = z1; right = x2; bottom = y2; back = z2;
		}

		void SetRect(const TPoint3<T>& pt, const TSize3<T>& size)		{ pt0 = pt; pt1 = pt+size; }
		void SetRect(const TPoint3<T>& pt0, const TPoint3<T>& pt1)		{ this->pt0 = pt0; this->pt1 = pt1; }
		void SetRectEmpty() { pt0.SetZero(); pt1.SetZero(); }

		// infflate the rectangles's width, height and depth
		TRect3& InflateRect(T x, T y, T z) { left -= x; top -= y; front -= z; right += x; bottom += y; back += z; return *this; }
		TRect3& InflateRect(const TSize3<T>& size) { pt0 -= size; pt1 += size; return *this; }
		TRect3& InflateRect(const TRect3<T>& rect) { pt0 -= rect.pt0; pt1 += rect.pt1; return *this; }
		TRect3& InflateRect(T l, T t, T r, T b) { left -= l; top -= t; right += r; bottom += b; return *this; }
		TRect3& InflateRect(T l, T t, T f, T r, T b, T bk) { left -= l; top -= t; front -= f; right += r; bottom += b; back += bk; return *this; }

		// deflate the rectangle's width, height and depth
		TRect3& DeflateRect(T x, T y, T z) { left += x; top += y; front += z; right -= x; bottom -= y; back -= z; return *this; }
		TRect3& DeflateRect(const TSize3<T>& size) { pt0 += size; pt1 -= size; return *this; }
		TRect3& DeflateRect(const TRect3<T>& rect) { pt0 += rect.pt0; pt1 -= rect.pt1; return *this; }
		TRect3& DeflateRect(T l, T t, T r, T b)  { left += l; top += t; right -= r; bottom -= b; return *this; }
		TRect3& DeflateRect(T l, T t, T f, T r, T b, T bk) { left += l; top += t; front += f; right -= r; bottom -= b; back -= bk; return *this; }

		// translate the rectangle by moving its top and left
		TRect3& OffsetRect(T x, T y, T z) { left += x; top += y; front += z; right += x; bottom += y; back += z; return *this; }
		TRect3& OffsetRect(const TPoint3<T>& pt) { pt0 += pt; pt1 += pt; return *this; }			// for Point : move whole rect, for Size : move pt1 only.
		TRect3& OffsetSize(const TSize3<T>& size) { /*pt0 += size;*/ pt1 += size; return *this; }	// for Point : move whole rect, for Size : move pt1 only.

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

		void MoveTo(const TPoint3<T>& pt) {
			MoveToX(pt.x);
			MoveToY(pt.y);
			MoveToZ(pt.z);
		}

		// set this rectangle to intersection of two others
		TRect3& IntersectRect(TRect3 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pt0.x = std::max(pt0.x, rect2.pt0.x);
			pt0.y = std::max(pt0.y, rect2.pt0.y);
			pt0.z = std::max(pt0.z, rect2.pt0.z);

			pt1.x = std::min(pt1.x, rect2.pt1.x);
			pt1.y = std::min(pt1.y, rect2.pt1.y);
			pt1.z = std::min(pt1.z, rect2.pt1.z);

			return *this;
		}

		// set this rectangle to bounding union of two others
		TRect3& UnionRect(TRect3 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pt0.x = std::min(pt0.x, rect2.pt0.x);
			pt0.y = std::min(pt0.y, rect2.pt0.y);
			pt0.z = std::min(pt0.z, rect2.pt0.z);

			pt1.x = std::max(pt1.x, rect2.pt1.x);
			pt1.y = std::max(pt1.y, rect2.pt1.y);
			pt1.z = std::max(pt1.z, rect2.pt1.z);

			return *this;
		}

		// Operators
		TRect3& operator += (const TPoint3<T>& pt)		{ return OffsetRect(pt); }
		TRect3& operator -= (const TPoint3<T>& pt)		{ return OffsetRect(-pt); }
		TRect3& operator += (const TSize3<T>& size)		{ return OffsetSize(size); }
		TRect3& operator -= (const TSize3<T>& size)		{ return OffsetSize(-size); }
		TRect3& operator += (const TRect3& rect)		{ return InflateRect(rect); }
		TRect3& operator -= (const TRect3& rect)		{ return DeflateRect(rect); }
		TRect3& operator &= (const TRect3& rect)		{ return IntersectRect(rect); }
		TRect3& operator |= (const TRect3& rect)		{ return UnionRect(rect); }

		// Operators returning TRect3 data()
		TRect3 operator + (const TPoint3<T>& pt) const	{ return TRect3(*this) += pt; }
		TRect3 operator - (const TPoint3<T>& pt) const	{ return TRect3(*this) -= pt; }
		TRect3 operator + (const TSize3<T>& size) const	{ return TRect3(*this) += size; }
		TRect3 operator - (const TSize3<T>& size) const	{ return TRect3(*this) -= size; }
		TRect3 operator + (const TRect3& rect) const	{ return TRect3(*this) += rect; }
		TRect3 operator - (const TRect3& rect) const	{ return TRect3(*this) -= rect; }
		TRect3 operator & (const TRect3& rect) const	{ return TRect3(*this).IntersectRect(rect); }
		TRect3 operator | (const TRect3& rect) const	{ return TRect3(*this).UnionRect(rect); }

		template < class Archive > friend Archive& operator & (Archive& ar, const TRect3& B) {
			return ar & B.pt0 & B.pt1;
		}
	};


	//--------------------------------------------------------------------------------------------------------------------------------
	// Rect 2 (left, top, right, bottom)
	//
	template < typename T >
	struct TRect2 {
	public:
		union {
			struct {
				T left, top;
				T right, bottom;
			};
			struct {
				TPoint2<T> pt0, pt1;
			};
		};

		using coord_t = std::array<T, 4>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr const coord_t& data() const { return std::bit_cast<coord_t const&>(*this); }

	public:
		// Constructors

		TRect2() = default;
		TRect2(const TRect2&) = default;

		TRect2(T l, T t, T r, T b) :
			left(l), top(t), right(r), bottom(b) {}
		template < typename T2 >
		TRect2(T2 l, T2 t, T2 r, T2 b) :
			left(RoundOrForward<T, T2>(l)), top(RoundOrForward<T, T2>(t)),
			right(RoundOrForward<T, T2>(r)), bottom(RoundOrForward<T, T2>(b)) {}

		explicit TRect2(const RECT& B) { *this = B; }

		// 3d
		template < gtl::is__xyz T_POINT >
		TRect2(const T_POINT& pt, const T_POINT& pt2) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt2.x)}, bottom{RoundOrForward<T>(pt2.y)} {};

		template < gtl::is__xyz T_POINT, gtl::is__cxyz T_SIZE>
		TRect2(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.cx)}, bottom{RoundOrForward<T>(pt.y+size.cy)} {};

		template < gtl::is__xyz T_POINT, gtl::is__size3 T_SIZE>
		TRect2(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.width)}, bottom{RoundOrForward<T>(pt.y+size.height)} {};

		// 2d
		template < gtl::is__xy T_POINT >
		TRect2(const T_POINT& pt, const T_POINT& pt2) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt2.x)}, bottom{RoundOrForward<T>(pt2.y)} {};

		template < gtl::is__xy T_POINT, gtl::is__cxy T_SIZE>
		TRect2(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.cx)}, bottom{RoundOrForward<T>(pt.y+size.cy)} {};

		template < gtl::is__xy T_POINT, gtl::is__size2 T_SIZE>
		TRect2(const T_POINT& pt, const T_SIZE& size) :
			left{RoundOrForward<T>(pt.x)}, top{RoundOrForward<T>(pt.y)},
			right{RoundOrForward<T>(pt.x+size.width)}, bottom{RoundOrForward<T>(pt.y+size.height)} {};

		// assign operator
		TRect2& operator = (const TRect2& B) = default;

		template < typename T_RECT > requires requires (T_RECT rect) { rect.pt0; rect.pt1; }
		TRect2& operator = (const T_RECT& rect) { pt0 = rect.pt0; pt1 = rect.pt1; return *this; }

		template < typename T_CV_RECT > requires requires (T_CV_RECT rc) { rc.x; rc.y; rc.widht; rc.height; }
		TRect2& operator = (const T_CV_RECT& rc) {
			left	= RoundOrForward<T>(rc.x);
			top		= RoundOrForward<T>(rc.y);
			right	= RoundOrForward<T>(rc.x + rc.width);
			bottom	= RoundOrForward<T>(rc.y + rc.height);
			return *this;
		}

		TRect2& operator = (const RECT& srcRect) {
			left = srcRect.left; top = srcRect.top; /*front = 0;*/
			right = srcRect.right; bottom = srcRect.bottom; /*back = 0;*/
			return *this;
		}

		T Width() const		{ return right - left; }
		T Height() const	{ return bottom - top; }
		//T Depth() const		{ return back - front; }

		// Type Casting to cv::Size_, SIZE, ...

		//template < typename T2 > operator TRect3<T2> () const { return TRect3<T2>(left, top, front, right, bottom, back); }
		template < typename T2 > operator TRect2<T2> () const { return TRect2<T2>(left, top, right, bottom); };
		operator RECT () const { return RECT{RoundOrForward<LONG, T>(left), RoundOrForward<LONG, T>(top), RoundOrForward<LONG, T>(right), RoundOrForward<LONG, T>(bottom)}; };

		template < gtl::is__xy	T_COORD > operator T_COORD() const { return T_COORD{left, top}; };
		//template < gtl::is__xyz	T_COORD > operator T_COORD() const { return T_COORD{left, top, front}; };
		template < gtl::is__cxy	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top}; };
		//template < gtl::is__cxyz	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top, back-front}; };
		template < gtl::is__size2	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top}; };
		//template < gtl::is__size3	T_COORD > operator T_COORD() const { return T_COORD{right-left, bottom-top, back-front}; };

		// cv::Rect_
		template < typename T2, template <typename> typename T_CV_RECT > requires requires (T_CV_RECT<T2> rc) { rc.x; rc.y; rc.widht; rc.height; }
		operator T_CV_RECT<T2>& () const {
			return T_CV_RECT<T2>(
				RoundOrForward<T2, T>(left),
				RoundOrForward<T2, T>(top),
				RoundOrForward<T2, T>(right-left),
				RoundOrForward<T2, T>(bottom-top));
		}

		// Compare
		auto operator <=> (const TRect2&) const = default;

		// B includes *this
		bool operator <= (const TRect2& B) const {
			return (left >= B.left) && (top >= B.top)
				&& (right <= B.right) && (bottom <= B.bottom);
		}
		// *this includes B
		bool operator >= (const TRect2& B) const {	// B includes *this
			return (left <= B.left) && (top <= B.top)
				&& (right >= B.right) && (bottom >= B.bottom);
		}
		// B includes *this
		bool operator < (const TRect2& B) const {
			return (left > B.left) && (top > B.top)
				&& (right < B.right) && (bottom < B.bottom);
		}
		// *this includes B
		bool operator > (const TRect2& B) const {	// B includes *this
			return (left < B.left) && (top < B.top)
				&& (right > B.right) && (bottom > B.bottom);
		}

				TPoint2<T>& TopLeft()				{ return pt0; }
		const	TPoint2<T>& TopLeft() const			{ return pt0; }
		//		TPoint3<T>& TopLeftFront()			{ return pt0; }
		//const	TPoint3<T>& TopLeftFront() const	{ return pt0; }

				TPoint2<T>& BottomRight()			{ return pt1; }
		const	TPoint2<T>& BottomRight() const		{ return pt1; }
		//		TPoint3<T>& BottomRightEnd()		{ return pt1; }
		//const	TPoint3<T>& BottomRightEnd() const	{ return pt1; }

		TPoint2<T> CenterPoint() const { return {(right+left)/(T)2, (bottom+top)/(T)2}; }

		TSize2<T> GetSize() const { return {right-left, bottom-top}; }

		// returns true if rectangle has no area
		bool IsRectEmpty() const {
			return (left >= right) || (top >= bottom);
		}
		// returns true if rectangle is at (0,0,0) and has no area
		bool IsRectNull() const {
			return !left && !top && !right && !bottom;
		}
		//bool PtInRect(const TPoint3<T>& pt) const {
		//	return (pt.x >= left) && (pt.y >= top) && (pt.z >= front) && (pt.x < right) && (pt.y < bottom) && (pt.z < back);
		//}
		bool PtInRect(const TPoint2<T>& pt) const {
			return (pt.x >= left) && (pt.y >= top) && (pt.x < right) && (pt.y < bottom);
		}
		// returns true if rect is within rectangle
		bool RectInRect(const TRect2& B) const {
			return B <= *this;
		}

		// Operations

		// set rectangle from left, top, right, and bottom
		void SetRect(T x1 = 0, T y1 = 0, T x2 = 0, T y2 = 0) {
			left = x1; top = y1; right = x2; bottom = y2;
		}

		void SetRect(const TPoint2<T>& pt, const TSize2<T>& size)		{ pt0 = pt; pt1 = pt+size; }
		void SetRect(const TPoint2<T>& pt0, const TPoint2<T>& pt1)		{ this->pt0 = pt0; this->pt1 = pt1; }
		void SetRectEmpty() { pt0.SetZero(); pt1.SetZero(); }

		// infflate the rectangles's width, height and depth
		TRect2& InflateRect(T x, T y) { left -= x; top -= y; right += x; bottom += y; return *this; }
		TRect2& InflateRect(const TSize2<T>& size) { pt0 -= size; pt1 += size; return *this; }
		TRect2& InflateRect(const TRect2& rect) { pt0 -= rect.pt0; pt1 += rect.pt1; return *this; }
		TRect2& InflateRect(T l, T t, T r, T b) { left -= l; top -= t; right += r; bottom += b; return *this; }
		//TRect2& InflateRect(T l, T t, T f, T r, T b, T bk) { left -= l; top -= t; front -= f; right += r; bottom += b; back += bk; return *this; }

		// deflate the rectangle's width, height and depth
		TRect2& DeflateRect(T x, T y, T z) { left += x; top += y; right -= x; bottom -= y; return *this; }
		TRect2& DeflateRect(const TSize2<T>& size) { pt0 += size; pt1 -= size; return *this; }
		TRect2& DeflateRect(const TRect2& rect) { pt0 += rect.pt0; pt1 -= rect.pt1; return *this; }
		TRect2& DeflateRect(T l, T t, T r, T b)  { left += l; top += t; right -= r; bottom -= b; return *this; }
		//TRect2& DeflateRect(T l, T t, T f, T r, T b, T bk) { left += l; top += t; front += f; right -= r; bottom -= b; back -= bk; return *this; }

		// translate the rectangle by moving its top and left
		TRect2& OffsetRect(T x, T y) { left += x; top += y; right += x; bottom += y; return *this; }
		TRect2& OffsetRect(const TPoint3<T>& pt) { pt0 += pt; pt1 += pt; return *this; }			// for Point : move whole rect, for Size : move pt1 only.
		TRect2& OffsetSize(const TSize3<T>& size) { /*pt0 += size;*/ pt1 += size; return *this; }	// for Point : move whole rect, for Size : move pt1 only.

	public:
		void NormalizeRect() {
			if (left > right) std::swap(left, right);
			if (top > bottom) std::swap(top, bottom);
		}

		// absolute position of rectangle
		void MoveToX(T x) { right += (x-left); left = x; }
		void MoveToY(T y) { bottom += (y-top); top = y; }
		//void MoveToZ(T z) { back += (z-front); front = z; }
		//void MoveToN(int i, T v) { assert(pt0.data().size() > i); pt1[i] += (v-pt0[i]); pt0[i] = v; }

		void MoveToXY(T x, T y) { MoveToX(x); MoveToY(y); }
		//void MoveToYZ(T y, T z) { MoveToY(y); MoveToZ(z); }
		//void MoveToZX(T z, T x) { MoveToZ(z); MoveToX(x); }
		//void MoveToXYZ(T x, T y, T z) { MoveToX(x); MoveToY(y); MoveToZ(z); }

		void MoveTo(const TPoint2<T>& pt) {
			MoveToX(pt.x);
			MoveToY(pt.y);
			//MoveToZ(pt.z);
		}

		// set this rectangle to intersection of two others
		TRect2& IntersectRect(TRect2 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pt0.x = std::max(pt0.x, rect2.pt0.x);
			pt0.y = std::max(pt0.y, rect2.pt0.y);
			//pt0.z = std::max(pt0.z, rect2.pt0.z);

			pt1.x = std::min(pt1.x, rect2.pt1.x);
			pt1.y = std::min(pt1.y, rect2.pt1.y);
			//pt1.z = std::min(pt1.z, rect2.pt1.z);

			return *this;
		}

		// set this rectangle to bounding union of two others
		TRect2& UnionRect(TRect2 rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pt0.x = std::min(pt0.x, rect2.pt0.x);
			pt0.y = std::min(pt0.y, rect2.pt0.y);
			//pt0.z = std::min(pt0.z, rect2.pt0.z);

			pt1.x = std::max(pt1.x, rect2.pt1.x);
			pt1.y = std::max(pt1.y, rect2.pt1.y);
			//pt1.z = std::max(pt1.z, rect2.pt1.z);

			return *this;
		}

		// Operators
		TRect2& operator += (const TPoint2<T>& pt)		{ return OffsetRect(pt); }
		TRect2& operator -= (const TPoint2<T>& pt)		{ return OffsetRect(-pt); }
		TRect2& operator += (const TSize2<T>& size)		{ return OffsetSize(size); }
		TRect2& operator -= (const TSize2<T>& size)		{ return OffsetSize(-size); }
		TRect2& operator += (const TRect2& rect)		{ return InflateRect(rect); }
		TRect2& operator -= (const TRect2& rect)		{ return DeflateRect(rect); }
		TRect2& operator &= (const TRect2& rect)		{ return IntersectRect(rect); }
		TRect2& operator |= (const TRect2& rect)		{ return UnionRect(rect); }

		// Operators returning TRect2 data()
		TRect2 operator + (const TPoint2<T>& pt) const	{ return TRect2(*this) += pt; }
		TRect2 operator - (const TPoint2<T>& pt) const	{ return TRect2(*this) -= pt; }
		TRect2 operator + (const TSize2<T>& size) const	{ return TRect2(*this) += size; }
		TRect2 operator - (const TSize2<T>& size) const	{ return TRect2(*this) -= size; }
		TRect2 operator + (const TRect2& rect) const	{ return TRect2(*this) += rect; }
		TRect2 operator - (const TRect2& rect) const	{ return TRect2(*this) -= rect; }
		TRect2 operator & (const TRect2& rect) const	{ return TRect2(*this).IntersectRect(rect); }
		TRect2 operator | (const TRect2& rect) const	{ return TRect2(*this).UnionRect(rect); }

		template < class Archive > friend Archive& operator & (Archive& ar, const TRect2& B) {
			return ar & B.pt0 & B.pt1;
		}

		//-----------------------------------------------------------------------------
		// ROI
		//
		template <typename T2 = std::int32_t>
		[[nodiscard]] bool IsROI_Valid(const TSize2<T2>& sizeImage) const {
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

		bool AdjustROI(const TSize2<T>& sizeImage) {
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
		[[nodiscard]] TRect2<T2> GetSafeROI(const TSize2<T2>& sizeImage) const {
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

#pragma pack(pop)

}	// namespace gtl
