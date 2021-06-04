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


#if 1

	template < typename T, int dim, int DEFAULT_FRONT = -1, int DEFAULT_BACK = 1 > struct TRectT;

	template < typename T, int DEFAULT_FRONT = -1, int DEFAULT_BACK = 1 >
	struct TRECT3 {
		T left{}, top{}, front{DEFAULT_FRONT}, right{}, bottom{}, back{DEFAULT_BACK};
		auto operator <=> (TRECT3 const&) const = default;
	};
	template < typename T > struct TRECT2 {
		T left{}, top{}, right{}, bottom{};
		auto operator <=> (TRECT2 const&) const = default;
	};

	//--------------------------------------------------------------------------------------------------------------------------------
	// Rect 3 (left, top, front, right, bottom, back)
	//
	template < typename T, int dim, int DEFAULT_FRONT, int DEFAULT_BACK >
	struct TRectT : std::conditional_t<dim == 2, TRECT2<T>, TRECT3<T, DEFAULT_FRONT, DEFAULT_BACK> > {
	public:
		//T left{}, top{}, front{DEFAULT_FRONT};
		//T right{}, bottom{}, back{DEFAULT_BACK};

		using base_t = std::conditional_t< dim == 2, TRECT2<T>, TRECT3<T, DEFAULT_FRONT, DEFAULT_BACK> >;
		using this_t = TRectT;
		using coord_t = std::array<T, 2*dim>;
		using point_t = TPointT<T, dim>;
		using size_t = TSizeT<T, dim>;
		using value_type = T;

		constexpr auto& arr() { return reinterpret_cast<coord_t&>(*this); }
		constexpr auto const& arr() const { return reinterpret_cast<coord_t&>(*this); }

		constexpr auto* data() { return (reinterpret_cast<coord_t&>(*this)).data(); }
		constexpr auto const* data() const { return (reinterpret_cast<coord_t const&>(*this)).data(); }
		constexpr auto const size() const { return (reinterpret_cast<coord_t const&>(*this)).size(); }
		constexpr value_type& member(size_t i) { return data()[i]; }
		constexpr value_type const& member(size_t i) const { return data()[i]; }

		constexpr point_t& pts(int i = 0) { return (i == 0) ? *(point_t*)(&this->left) : *(point_t*)(&this->right); }
		constexpr point_t const& pts(int i = 0) const { return (i == 0) ? *(point_t*)(&this->left) : *(point_t*)(&this->right); }
		constexpr point_t& pt0() { return *(point_t*)(&this->left); }
		constexpr point_t& pt1() { return *(point_t*)(&this->right); }
		constexpr point_t const& pt0() const { return *(point_t*)(&this->left); }
		constexpr point_t const& pt1() const { return *(point_t*)(&this->right); }

		static_assert(2 <= dim and dim <= 3);

	public:
		// Constructors
		constexpr T default_front() const requires (dim == 3) { return DEFAULT_FRONT; }
		constexpr T default_back()  const requires (dim == 3) { return DEFAULT_BACK; }

		TRectT() = default;
		TRectT(TRectT const&) = default;

		TRectT(T l, T t, T r, T b) requires (dim == 2) : base_t{l, t, r, b} {}
		TRectT(T l, T t, T r, T b) requires (dim == 3) : base_t{l, t, default_front(), r, b, default_back()} {}
		TRectT(T l, T t, T f, T r, T b, T bk) requires (dim == 3) : base_t{l, t, f, r, b, bk} {}

		template < typename T2 >
		TRectT(T2 l, T2 t, T2 r, T2 b) requires (dim == 2) : base_t{RoundOrForward<T>(l), RoundOrForward<T>(t), RoundOrForward<T>(r), RoundOrForward<T>(b)} {}
		template < typename T2 >
		TRectT(T2 l, T2 t, T2 r, T2 b) requires (dim == 3) : base_t{RoundOrForward<T>(l), RoundOrForward<T>(t), default_front(), RoundOrForward<T>(r), RoundOrForward<T>(b), default_back()} {}
		template < typename T2 >
		TRectT(T2 l, T2 t, T2 f, T2 r, T2 b, T2 bk) requires (dim == 3) : base_t{RoundOrForward<T>(l), RoundOrForward<T>(t), RoundOrForward<T>(f), RoundOrForward<T>(r), RoundOrForward<T>(b), RoundOrForward<T>(bk)} {}

		template < gtlc::generic_coord T_COORD >
		explicit TRectT(T_COORD const& B) { *this = B; }
		template < gtlc::generic_coord T_COORD >
		TRectT& operator = (T_COORD const& B) {
			if constexpr ( ((dim == 3) and gtlc::rect3<T_COORD>) or ( (dim == 2) and gtlc::rect2<T_COORD>) ) {
				pts(0) = B.pts(0);
				pts(1) = B.pts(1);
			}
			else {
				if constexpr (gtlc::has__xy<T_COORD>) {
					this->left  = RoundOrForward<T>(B.x);
					this->top   = RoundOrForward<T>(B.y);
					if constexpr ((dim >= 3) and gtlc::has__z<T_COORD>) {
						this->front = RoundOrForward<T>(B.z);
					}
				}

				if constexpr (gtlc::has__size2<T_COORD>) {
					this->right = this->left + RoundOrForward<T>(B.width);
					this->bottom = this->top + RoundOrForward<T>(B.height);
					if constexpr ((dim >= 3) and gtlc::has__depth<T_COORD>) {
						this->back = this->front + RoundOrForward<T>(B.depth);
					}
				} else if constexpr (gtlc::has__cxy<T_COORD>) {
					this->right = this->left + RoundOrForward<T>(B.cx);
					this->bottom = this->top + RoundOrForward<T>(B.cy);
					if constexpr ((dim >= 3) and gtlc::has__cz<T_COORD>) {
						this->back = this->front + RoundOrForward<T>(B.cz);
					}
				} else if constexpr (gtlc::wnd_rect<T_COORD>) {
					this->left = B.left;
					this->top = B.top;
					this->right = B.right;
					this->bottom = B.bottom;
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
		TRectT(T_POINT const& pt0, T_POINT const& pt1) requires (dim == 3)
			: base_t{ RoundOrForward<T>(pt0.x), RoundOrForward<T>(pt0.y), RoundOrForward<T>(pt0.z), RoundOrForward<T>(pt1.x), RoundOrForward<T>(pt1.y), RoundOrForward<T>(pt1.z)} { };

		template < gtlc::has__xyz T_POINT, gtlc::has__cxyz T_SIZE>
		TRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(pt.z), RoundOrForward<T>(pt.x+size.cx), RoundOrForward<T>(pt.y+size.cy), RoundOrForward<T>(pt.z+size.cz)} {};

		template < gtlc::has__xyz T_POINT, gtlc::has__size3 T_SIZE>
		TRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(pt.z), RoundOrForward<T>(pt.x+size.width), RoundOrForward<T>(pt.y+size.height), RoundOrForward<T>(pt.z+size.depth)} {};

		template < typename T_POINT > requires (gtlc::has__xy<T_POINT> and !gtlc::has__z<T_POINT>)
		TRectT(T_POINT const& pt0, T_POINT const& pt1) requires (dim == 3)
			: base_t{ RoundOrForward<T>(pt0.x), RoundOrForward<T>(pt0.y), RoundOrForward<T>(pt0.z), RoundOrForward<T>(pt1.x), RoundOrForward<T>(pt1.y), RoundOrForward<T>(pt1.z)} { };

		template < typename T_POINT, typename T_SIZE> requires (gtlc::has__xy<T_POINT> and gtlc::has__cxy<T_SIZE> and !(gtlc::has__z<T_POINT> and gtlc::has__cz<T_SIZE>))
		TRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), default_front(), RoundOrForward<T>(pt.x+size.cx), RoundOrForward<T>(pt.y+size.cy), default_back()} {};

		template < typename T_POINT, typename T_SIZE> requires (gtlc::has__xy<T_POINT> and gtlc::has__size2<T_SIZE> and !(gtlc::has__z<T_POINT> and gtlc::has__depth<T_SIZE>))
		TRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), default_front(), RoundOrForward<T>(pt.x+size.width), RoundOrForward<T>(pt.y+size.height), default_back()} {};

		// 2d
		template < gtlc::has__xy T_POINT >
		TRectT(T_POINT const& pt0, T_POINT const& pt1) requires (dim == 2)
			: base_t{ RoundOrForward<T>(pt0.x), RoundOrForward<T>(pt0.y), RoundOrForward<T>(pt1.x), RoundOrForward<T>(pt1.y)} { };

		template < gtlc::has__xy T_POINT, gtlc::has__cxy T_SIZE>
		TRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 2)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(pt.x+size.cx), RoundOrForward<T>(pt.y+size.cy)} {};

		template < gtlc::has__xy T_POINT, gtlc::has__size2 T_SIZE>
		TRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 2)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(pt.x+size.width), RoundOrForward<T>(pt.y+size.height)} {};

		// assign operator
		this_t& operator = (this_t const& B) = default;
		this_t& operator = (this_t && B) = default;

		constexpr static inline this_t All(T v) {
			if constexpr (dim == 2)
				return this_t(v, v, v, v);
			else if constexpr (dim == 3)
				return this_t(v, v, v, v, v, v);
		}

		// Type Casting to cv::Size_, SIZE, ...
		template < gtlc::wnd_rect T_COORD >
		operator T_COORD () const { using T2 = decltype(T_COORD::left); return T_COORD{RoundOrForward<T2>(this->left), RoundOrForward<T2>(this->top), RoundOrForward<T2>(this->right), RoundOrForward<T2>(this->bottom) }; }
		template < gtlc::cv_rect T_COORD >
		operator T_COORD () const { using T2 = decltype(T_COORD::x); return T_COORD{RoundOrForward<T2>(this->left), RoundOrForward<T2>(this->top), RoundOrForward<T2>(this->right - this->left), RoundOrForward<T2>(this->bottom - this->top) }; }

		template < typename T2 > operator TRectT<T2, 2> () const requires (dim == 3) { return TRectT<T2, 2>(this->left, this->top, this->right, this->bottom); }

		// Compare
		auto operator <=> (TRectT const&) const = default;
		auto operator <=> (T const& v) const { return *this <=> All(v); }

		T Width() const						{ return this->right - this->left; }
		T Height() const					{ return this->bottom - this->top; }
		T Depth() const requires (dim == 3)	{ return this->back - this->front; }
		T CX() const						{ return this->right - this->left; }
		T CY() const						{ return this->bottom - this->top; }
		T CZ() const requires (dim == 3)	{ return this->back - this->front; }

		//		TPoint2<T>& TopLeft()									{ return (TPoint2<T>&)pts(0); }
		//const	TPoint2<T>& TopLeft() const								{ return (TPoint2<T>&)pts(0); }
		//		TPoint3<T>& TopLeftFront()			requires (dim == 3) { return pts(0); }
		//const	TPoint3<T>& TopLeftFront() const	requires (dim == 3) { return pts(0); }

		//		TPoint2<T>& BottomRight()								{ return (TPoint2<T>&)pts(1); }
		//const	TPoint2<T>& BottomRight() const							{ return (TPoint2<T>&)pts(1); }
		//		TPoint3<T>& BottomRightBack()		requires (dim == 3) { return pts(1); }
		//const	TPoint3<T>& BottomRightBack() const	requires (dim == 3) { return pts(1); }

		point_t CenterPoint() const { return (pts(0)+pts(1))/2; }

		size_t GetSize() const { return pts(1)-pts(0); }

		// returns true if rectangle has no area
		bool IsRectEmpty() const {
			if constexpr (dim == 3) {
				return (this->left >= this->right) || (this->top >= this->bottom) || (this->front >= this->back);
			} else {
				return (this->left >= this->right) || (this->top >= this->bottom);
			}
		}
		// returns true if rectangle is at (0,0,0) and has no area
		bool IsRectNull() const {
			return (pts(0) == point_t{}) and (pts(1) == point_t{});
		}
		bool PtInRect(point_t const& pt) const {
			if constexpr (dim == 3) {
				return (pt.x >= this->left) && (pt.y >= this->top) && (pt.z >= this->front) && (pt.x < this->right) && (pt.y < this->bottom) && (pt.z < this->back);
			} else {
				return (pt.x >= this->left) && (pt.y >= this->top) && (pt.x < this->right) && (pt.y < this->bottom);
			}
		}
		bool PtInRect(TPoint2<T> const& pt) const requires (dim == 3) {
			return (pt.x >= this->left) && (pt.y >= this->top) && (pt.x < this->right) && (pt.y < this->bottom);
		}
		// returns true if rect is within rectangle
		/// @brief *this including B
		/// @return 
		bool RectInRect(this_t const& B) const {
			return (pts(0) <= B.pts(0)) and (B.pts(1) <= pts(1));
		}
		/// @brief *this including B (No boundary)
		/// @return 
		bool RectInRectNE(this_t const& B) const {
			return (pts(0) < B.pts(0)) and (B.pts(1) < pts(1));
		}

		// Operations

		// set rectangle from left, top, right, and bottom
		void SetRect(T x0 = 0, T y0 = 0, T x1 = 0, T y1 = 0) requires (dim == 2) {
			this->left = x0; this->top = y0; this->right = x1; this->bottom = y1;
		}
		void SetRect(T x0 = 0, T y0 = 0, T z0 = default_front(), T x1 = 0, T y1 = 0, T z1 = default_back()) requires (dim == 3) {
			this->left = x0; this->top = y0; this->front = z0; this->right = x1; this->bottom = y1; this->back = z1;
		}

		void SetRect(point_t const& pt, size_t const& size)			{ pts(0) = pt; pts(1) = pt+size; }
		void SetRect(point_t const& pt0, point_t const& pt1)		{ pts(0) = pt0; pts(1) = pt1; }
		void SetRectEmpty()											{ SetRect(); }

		// infflate the rectangles's width, height and depth
		this_t& InflateRect(T x, T y)						requires (dim == 2) { this->left -= x; this->top -= y; this->right += x; this->bottom += y; return *this; }
		this_t& InflateRect(T x, T y, T z)					requires (dim == 3) { this->left -= x; this->top -= y; this->front -= z; this->right += x; this->bottom += y; this->back += z; return *this; }
		this_t& InflateRect(size_t const& size)									{ pts(0) -= size; pts(1) += size; return *this; }
		this_t& InflateRect(this_t const& rect)									{ pts(0) -= rect.pts(0); pts(1) += rect.pts(1); return *this; }
		this_t& InflateRect(T l, T t, T r, T b)				requires (dim == 2) { this->left -= l; this->top -= t; this->right += r; this->bottom += b; return *this; }
		this_t& InflateRect(T l, T t, T f, T r, T b, T bk)	requires (dim == 3) { this->left -= l; this->top -= t; this->front -= f; this->right += r; this->bottom += b; this->back += bk; return *this; }

		// deflate the rectangle's width, height and depth
		this_t& DeflateRect(T x, T y)						requires (dim == 2) { this->left += x; this->top += y; this->right -= x; this->bottom -= y; return *this; }
		this_t& DeflateRect(T x, T y, T z)					requires (dim == 3) { this->left += x; this->top += y; this->front += z; this->right -= x; this->bottom -= y; this->back -= z; return *this; }
		this_t& DeflateRect(size_t const& size)									{ pts(0) += size; pts(1) -= size; return *this; }
		this_t& DeflateRect(this_t const& rect)									{ pts(0) += rect.pts(0); pts(1) -= rect.pts(1); return *this; }
		this_t& DeflateRect(T l, T t, T r, T b)				requires (dim == 2) { this->left += l; this->top += t; this->right -= r; this->bottom -= b; return *this; }
		this_t& DeflateRect(T l, T t, T f, T r, T b, T bk)	requires (dim == 3) { this->left += l; this->top += t; this->front += f; this->right -= r; this->bottom -= b; this->back -= bk; return *this; }

		// translate the rectangle by moving its top and left
		this_t& OffsetRect(T x, T y)						requires (dim == 2) { this->left += x; this->top += y; this->right += x; this->bottom += y; return *this; }
		this_t& OffsetRect(T x, T y, T z)					requires (dim == 3) { this->left += x; this->top += y; this->front += z; this->right += x; this->bottom += y; this->back += z; return *this; }
		this_t& OffsetRect(point_t const& pt)									{ pts(0) += pt; pts(1) += pt; return *this; }		// for Point : move whole rect, for Size : move pts(1) only.

		this_t& OffsetSize(size_t const& size)									{ /*pt0 += size;*/ pts(1) += size; return *this; }	// for Point : move whole rect, for Size : move pts(1) only.

	public:
		void NormalizeRect() {
			if (this->left > this->right) std::swap(this->left, this->right);
			if (this->top > this->bottom) std::swap(this->top, this->bottom);
			if constexpr (dim >= 3) {
				if (this->front > this->back) std::swap(this->front, this->back);
			}
		}

		// absolute position of rectangle
		void MoveToX(T x)									{ this->right += (x-this->left); this->left = x; }
		void MoveToY(T y)									{ this->bottom += (y-this->top); this->top = y; }
		void MoveToZ(T z)				requires (dim >= 3)	{ this->back += (z-this->front); this->front = z; }
		//void MoveToN(int i, T v) { assert(pt0.data().size() > i); pt1[i] += (v-pt0[i]); pt0[i] = v; }

		void MoveToXY(T x, T y)								{ MoveToX(x); MoveToY(y); }
		void MoveToYZ(T y, T z)			requires (dim >= 3) { MoveToY(y); MoveToZ(z); }
		void MoveToZX(T z, T x)			requires (dim >= 3) { MoveToZ(z); MoveToX(x); }
		void MoveToXYZ(T x, T y, T z)	requires (dim >= 3) { MoveToX(x); MoveToY(y); MoveToZ(z); }

		void MoveTo(point_t const& pt) {
			MoveToX(pt.x);
			MoveToY(pt.y);
			if constexpr (dim >= 3)
				MoveToZ(pt.z);
		}

		// set this rectangle to intersection of two others
		this_t& IntersectRect(this_t rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pts(0).x = std::max(pts(0).x, rect2.pts(0).x);
			pts(0).y = std::max(pts(0).y, rect2.pts(0).y);
			if constexpr (dim >= 3)
				pts(0).z = std::max(pts(0).z, rect2.pts(0).z);

			pts(1).x = std::min(pts(1).x, rect2.pts(1).x);
			pts(1).y = std::min(pts(1).y, rect2.pts(1).y);
			if constexpr (dim >= 3)
				pts(1).z = std::min(pts(1).z, rect2.pts(1).z);

			return *this;
		}

		// set this rectangle to bounding union of two others
		this_t& UnionRect(this_t rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pts(0).x = std::min(pts(0).x, rect2.pts(0).x);
			pts(0).y = std::min(pts(0).y, rect2.pts(0).y);
			if constexpr (dim >= 3)
				pts(0).z = std::min(pts(0).z, rect2.pts(0).z);

			pts(1).x = std::max(pts(1).x, rect2.pts(1).x);
			pts(1).y = std::max(pts(1).y, rect2.pts(1).y);
			if constexpr (dim >= 3)
				pts(1).z = std::max(pts(1).z, rect2.pts(1).z);

			return *this;
		}

		// Operators
		this_t& operator += (point_t const& pt)			{ return OffsetRect(pt); }
		this_t& operator -= (point_t const& pt)			{ return OffsetRect(-pt); }
		this_t& operator += (size_t const& size)		{ return OffsetSize(size); }
		this_t& operator -= (size_t const& size)		{ return OffsetSize(-size); }
		this_t& operator += (this_t const& rect)		{ return InflateRect(rect); }
		this_t& operator -= (this_t const& rect)		{ return DeflateRect(rect); }
		this_t& operator &= (this_t const& rect)		{ return IntersectRect(rect); }
		this_t& operator |= (this_t const& rect)		{ return UnionRect(rect); }

		// Operators returning TRectT data()
		this_t operator + (point_t const& pt) const		{ return this_t(*this) += pt; }
		this_t operator - (point_t const& pt) const		{ return this_t(*this) -= pt; }
		this_t operator + (size_t const& size) const	{ return this_t(*this) += size; }
		this_t operator - (size_t const& size) const	{ return this_t(*this) -= size; }
		this_t operator + (this_t const& rect) const	{ return this_t(*this) += rect; }
		this_t operator - (this_t const& rect) const	{ return this_t(*this) -= rect; }
		this_t operator & (this_t const& rect) const	{ return this_t(*this).IntersectRect(rect); }
		this_t operator | (this_t const& rect) const	{ return this_t(*this).UnionRect(rect); }

		//friend class boost::serialization::access;
		template < typename tBoostArchive >
		friend void serialize(tBoostArchive &ar, TRectT& rect, unsigned int const version) {
			ar & rect;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, this_t& B) {
			return ar & B.pts(0) & B.pts(1);
		}
		template < typename JSON > friend void from_json(JSON const& j, this_t& B) { B.pts(0) = j["pt0"]; B.pts(1) = j["pt1"]; }
		template < typename JSON > friend void to_json(JSON&& j, this_t const& B) { j["pt0"] = B.pts(0); j["pt1"] = B.pts(1); }


		bool UpdateBoundary(point_t const& pt) {
			bool bModified{};
			for (int i = 0; i < pt.size(); i++) {
				if (pts(0).member(i) > pt.member(i)) {
					bModified = true;
					pts(0).member(i) = pt.member(i);
				}
			}
			for (int i = 0; i < pt.size(); i++) {
				if (pts(1).member(i) < pt.member(i)) {
					bModified = true;
					pts(1).member(i) = pt.member(i);
				}
			}
			return bModified;
		}

		//-----------------------------------------------------------------------------
		// ROI
		//
		template < std::integral T_INT = std::int32_t >
		[[nodiscard]] bool IsROI_Valid(TSize2<T_INT> const& sizeImage) const {
			if constexpr (!std::is_integral_v(T)) {
				TRectT<T_INT, 2> rect(*this);
				return rect.IsROI_Valid(sizeImage);
			}

			return 1
				&& (this->left >= 0)
				&& (this->top >= 0)
				&& ( (sizeImage.cx < 0) || ( (this->left < sizeImage.cx) && (this->right < sizeImage.cx) && (this->left < this->right) ) )
				&& ( (sizeImage.cy < 0) || ( (this->top < sizeImage.cy) && (this->bottom < sizeImage.cy) && (this->top < this->bottom) ) )
				;
		}

		template < std::integral T_INT = std::int32_t >
		bool AdjustROI(TSize2<T> const& sizeImage) {
			NormalizeRect();

			if (this->left < 0)
				this->left = 0;
			if (this->top < 0)
				this->top = 0;
			if ( (sizeImage.cx > 0) && (this->right > sizeImage.cx) )
				this->right = sizeImage.cx;
			if ( (sizeImage.cy > 0) && (this->bottom > sizeImage.cy) )
				this->bottom = sizeImage.cy;

			return !IsRectEmpty();
		}

		template < std::integral T_INT = std::int32_t>
		[[nodiscard]] TRectT<T_INT, 2> GetSafeROI(TSize2<T_INT> const& sizeImage) const {
			TRectT<T_INT, 2> rect(*this);

			rect.NormalizeRect();

			if (rect.left < 0)
				rect.left = 0;
			if (rect.top < 0)
				rect.top = 0;
			if ( (sizeImage.cx > 0) && (rect.right > sizeImage.cx) )
				rect.right = sizeImage.cx;
			if ( (sizeImage.cy > 0) && (rect.bottom > sizeImage.cy) )
				rect.bottom = sizeImage.cy;

			return rect;
		}

	};

	template < typename T > using TRect2 = TRectT<T, 2>;
	template < typename T > using TRect3 = TRectT<T, 3>;


#else

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
		using value_type = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_type& member(int i) { return data()[i]; }
		constexpr value_type const& member(int i) const { return data()[i]; }
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
		void SetRect(T x1 = 0, T y1 = 0, T z1 = default_front(), T x2 = 0, T y1 = 0, T z2 = default_back()) {
			left = x1; top = y1; front = z1; right = x2; bottom = y1; back = z2;
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

		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & *this;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, TRect3& B) {
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
		using value_type = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_type& member(int i) { return data()[i]; }
		constexpr value_type const& member(int i) const { return data()[i]; }
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
		void SetRect(T x1 = 0, T y1 = 0, T x2 = 0, T y1 = 0) {
			left = x1; top = y1; right = x2; bottom = y1;
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

		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & *this;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, TRect2& B) {
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

#endif


#pragma pack(pop)
}	// namespace gtl
