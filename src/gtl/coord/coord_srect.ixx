//////////////////////////////////////////////////////////////////////
//
// rect_size.h: rect - top,left/width,height
//
// PWH
// 2021.08.13.
//
//////////////////////////////////////////////////////////////////////

module;

#include <compare>
#include <functional>
#include <algorithm>
#include <bit>

#include "gtl/_config.h"
#include "gtl/_macro.h"

//#include "boost/ptr_container/ptr_container.hpp"
//#include "boost/serialization/serialization.hpp"
//#include "boost/serialization/export.hpp"

export module gtl:coord_srect;

import :concepts;
import :misc;
import :coord_size;
import :coord_point;

//#ifdef min
//#	undef min	// undefine WinAPI - min. ( use std::min only.)
//#endif

export namespace gtl {


	template < typename T, int dim, int DEFAULT_FRONT = -1, int DEFAULT_DEPTH = 2 > struct TSRectT;

	template < typename T, int DEFAULT_FRONT = -1, int DEFAULT_DEPTH = 2 >
	struct TSRECT3 {
		T x{}, y{}, z{DEFAULT_FRONT}, width{}, height{}, depth{2};
		auto operator <=> (TSRECT3 const&) const = default;
	};
	template < typename T > struct TSRECT2 {
		T x{}, y{}, width{}, height{};
		auto operator <=> (TSRECT2 const&) const = default;
	};

	//--------------------------------------------------------------------------------------------------------------------------------
	// Rect 3 (left, top, front, right, bottom, back)
	//
	template < typename T, int dim, int DEFAULT_FRONT, int DEFAULT_DEPTH >
	struct TSRectT : std::conditional_t<dim == 2, TSRECT2<T>, TSRECT3<T, DEFAULT_FRONT, DEFAULT_DEPTH> > {
	public:
		//T left{}, top{}, front{DEFAULT_FRONT};
		//T right{}, bottom{}, back{DEFAULT_DEPTH};

		using base_t = std::conditional_t< dim == 2, TSRECT2<T>, TSRECT3<T, DEFAULT_FRONT, DEFAULT_DEPTH> >;
		using this_t = TSRectT;
		using coord_t = std::array<T, 2*dim>;
		using coord_point_t = TPointT<T, dim>;
		using coord_size_t = TSizeT<T, dim>;
		using value_type = T;

		constexpr auto& arr() { return reinterpret_cast<coord_t&>(*this); }
		constexpr auto const& arr() const { return reinterpret_cast<coord_t&>(*this); }

		constexpr auto* data() { return (reinterpret_cast<coord_t&>(*this)).data(); }
		constexpr auto const* data() const { return (reinterpret_cast<coord_t const&>(*this)).data(); }
		constexpr auto const size() const { return (reinterpret_cast<coord_t const&>(*this)).size(); }
		constexpr value_type& member(size_t i) { return data()[i]; }
		constexpr value_type const& member(size_t i) const { return data()[i]; }

		constexpr coord_point_t&		tl()				{ return *(coord_point_t*)(&this->x); }
		constexpr coord_point_t&		coord_size()		{ return *(coord_point_t*)(&this->width); }
		constexpr coord_point_t const&	tl() const			{ return *(coord_point_t*)(&this->x); }
		constexpr coord_point_t const&	coord_size() const	{ return *(coord_point_t*)(&this->width); }

		constexpr coord_point_t&		pt0()		{ return *(coord_point_t*)(&this->x); }
		//constexpr coord_point_t&		pt1()		{ return *(coord_point_t*)(&this->width); }
		constexpr coord_point_t const&	pt0() const	{ return *(coord_point_t*)(&this->x); }
		constexpr coord_point_t const&	pt1() const	{ return tl()+coord_size(); }

		static_assert(2 <= dim and dim <= 3);

	public:
		// Constructors
		constexpr T default_front() const requires (dim == 3) { return DEFAULT_FRONT; }
		constexpr T default_depth()  const requires (dim == 3) { return DEFAULT_DEPTH; }

		TSRectT() = default;
		TSRectT(TSRectT const&) = default;

		TSRectT(T x, T y, T w, T h) requires (dim == 2) : base_t{x, y, w, h} {}
		TSRectT(T x, T y, T w, T h) requires (dim == 3) : base_t{x, y, default_front(), w, h, default_depth()} {}
		TSRectT(T x, T y, T z, T w, T h, T d) requires (dim == 3) : base_t{x, y, z, w, h, d} {}

		template < typename T2 >
		TSRectT(T2 x, T2 y, T2 w, T2 h) requires (dim == 2) : base_t{RoundOrForward<T>(x), RoundOrForward<T>(y), RoundOrForward<T>(w), RoundOrForward<T>(h)} {}
		template < typename T2 >
		TSRectT(T2 x, T2 y, T2 w, T2 h) requires (dim == 3) : base_t{RoundOrForward<T>(x), RoundOrForward<T>(y), default_front(), RoundOrForward<T>(w), RoundOrForward<T>(h), default_depth()} {}
		template < typename T2 >
		TSRectT(T2 x, T2 y, T2 z, T2 w, T2 h, T2 d) requires (dim == 3) : base_t{RoundOrForward<T>(x), RoundOrForward<T>(y), RoundOrForward<T>(z), RoundOrForward<T>(w), RoundOrForward<T>(h), RoundOrForward<T>(d)} {}

		template < gtlc::generic_coord T_COORD >
		explicit TSRectT(T_COORD const& B) { *this = B; }
		template < gtlc::generic_coord T_COORD >
		TSRectT& operator = (T_COORD const& B) {
			if constexpr ( ((dim == 3) and gtlc::srect3<T_COORD>) or ( (dim == 2) and gtlc::srect2<T_COORD>) ) {
				pt0() = B.pt0();
				coord_size() = B.GetSize();
			}
			else if constexpr ( ((dim == 3) and gtlc::rect3<T_COORD>) or ( (dim == 2) and gtlc::rect2<T_COORD>) ) {
				pt0() = B.pt0();
				coord_size() = B.GetSize();
			}
			else {
				if constexpr (gtlc::has__xy<T_COORD>) {
					this->x = RoundOrForward<T>(B.x);
					this->y = RoundOrForward<T>(B.y);
					if constexpr ((dim >= 3) and gtlc::has__z<T_COORD>) {
						this->z = RoundOrForward<T>(B.z);
					}
				}

				if constexpr (gtlc::has__size2<T_COORD>) {
					this->width = RoundOrForward<T>(B.width);
					this->height = RoundOrForward<T>(B.height);
					if constexpr ((dim >= 3) and gtlc::has__depth<T_COORD>) {
						this->depth = RoundOrForward<T>(B.depth);
					}
				} else if constexpr (gtlc::has__cxy<T_COORD>) {
					this->width = RoundOrForward<T>(B.cx);
					this->height = RoundOrForward<T>(B.cy);
					if constexpr ((dim >= 3) and gtlc::has__cz<T_COORD>) {
						this->depth = RoundOrForward<T>(B.cz);
					}
				} else if constexpr (gtlc::wnd_rect<T_COORD>) {
					this->x = B.left;
					this->y = B.top;
					this->width = B.right - B.left;
					this->height = B.bottom - B.top;
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
		TSRectT(T_POINT const& pt0, T_POINT const& pt1) requires (dim == 3)
			: base_t{ RoundOrForward<T>(pt0.x), RoundOrForward<T>(pt0.y), RoundOrForward<T>(pt0.z), RoundOrForward<T>(pt1.x-pt0.x), RoundOrForward<T>(pt1.y-pt0.y), RoundOrForward<T>(pt1.z-pt0.z)} { };

		template < gtlc::has__xyz T_POINT, gtlc::has__cxyz T_SIZE>
		TSRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(pt.z), RoundOrForward<T>(size.cx), RoundOrForward<T>(size.cy), RoundOrForward<T>(size.cz)} {};

		template < gtlc::has__xyz T_POINT, gtlc::has__size3 T_SIZE>
		TSRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(pt.z), RoundOrForward<T>(size.width), RoundOrForward<T>(size.height), RoundOrForward<T>(size.depth)} {};

		template < typename T_POINT > requires (gtlc::has__xy<T_POINT> and !gtlc::has__z<T_POINT>)
		TSRectT(T_POINT const& pt0, T_POINT const& pt1) requires (dim == 3)
			: base_t{ RoundOrForward<T>(pt0.x), RoundOrForward<T>(pt0.y), RoundOrForward<T>(pt0.z), RoundOrForward<T>(pt1.x-pt0.x), RoundOrForward<T>(pt1.y-pt0.y), RoundOrForward<T>(pt1.z-pt0.z)} { };

		template < typename T_POINT, typename T_SIZE> requires (gtlc::has__xy<T_POINT> and gtlc::has__cxy<T_SIZE> and !(gtlc::has__z<T_POINT> and gtlc::has__cz<T_SIZE>))
		TSRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), default_front(), RoundOrForward<T>(size.cx), RoundOrForward<T>(size.cy), default_depth()} {};

		template < typename T_POINT, typename T_SIZE> requires (gtlc::has__xy<T_POINT> and gtlc::has__size2<T_SIZE> and !(gtlc::has__z<T_POINT> and gtlc::has__depth<T_SIZE>))
		TSRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 3)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), default_front(), RoundOrForward<T>(size.width), RoundOrForward<T>(size.height), default_depth()} {};

		// 2d
		template < gtlc::has__xy T_POINT >
		TSRectT(T_POINT const& pt0, T_POINT const& pt1) requires (dim == 2)
			: base_t{ RoundOrForward<T>(pt0.x), RoundOrForward<T>(pt0.y), RoundOrForward<T>(pt1.x-pt0.x), RoundOrForward<T>(pt1.y-pt0.y)} { };

		template < gtlc::has__xy T_POINT, gtlc::has__cxy T_SIZE>
		TSRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 2)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(size.cx), RoundOrForward<T>(size.cy)} {};

		template < gtlc::has__xy T_POINT, gtlc::has__size2 T_SIZE>
		TSRectT(T_POINT const& pt, T_SIZE const& size) requires (dim == 2)
			: base_t { RoundOrForward<T>(pt.x), RoundOrForward<T>(pt.y), RoundOrForward<T>(size.width), RoundOrForward<T>(size.height)} {};

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
		operator T_COORD () const { using T2 = decltype(T_COORD::left); return T_COORD{RoundOrForward<T2>(this->x), RoundOrForward<T2>(this->y), RoundOrForward<T2>(this->x+this->width), RoundOrForward<T2>(this->y+this->height) }; }
		template < gtlc::cv_rect T_COORD >
		operator T_COORD () const { using T2 = decltype(T_COORD::x); return T_COORD{RoundOrForward<T2>(this->x), RoundOrForward<T2>(this->y), RoundOrForward<T2>(this->width), RoundOrForward<T2>(this->height) }; }

		template < typename T2 > operator TSRectT<T2, 2> () const requires (dim == 3) { return TSRectT<T2, 2>(this->x, this->y, this->width, this->height); }

		// Compare
		auto operator <=> (TSRectT const&) const = default;
		auto operator <=> (T const& v) const { return *this <=> All(v); }

		T Width() const						{ return this->width; }
		T Height() const					{ return this->height; }
		T Depth() const requires (dim == 3)	{ return this->depth; }
		T& Width() 							{ return this->width; }
		T& Height() 						{ return this->height; }
		T& Depth() requires (dim == 3)		{ return this->depth; }
		T Right() const						{ return this->x + this->width; }
		T Bottom() const					{ return this->y + this->height;; }
		T Back() const requires (dim == 3)	{ return this->z + this->depth; }

		coord_point_t CenterPoint() const { return pt0()+coord_size()/2; }

		coord_size_t GetSize() const { return coord_size(); }

		// returns true if rectangle has no area
		bool IsRectEmpty() const {
			if constexpr (dim == 3) {
				return (this->width <= 0) || (this->height <= 0) || (this->height <= 0);
			} else {
				return (this->width <= 0) || (this->height <= 0);
			}
		}
		bool IsRectHavingLength2d() const {
			return (this->width > 0) or (this->height > 0);
		}
		// returns true if rectangle is at (0,0,0) and has no area
		bool IsRectNull() const {
			return (pt0() == coord_point_t{}) and (coord_size() == coord_point_t{});
		}
		bool PtInRect(coord_point_t const& pt) const {
			if constexpr (dim == 3) {
				return (pt.x >= this->x) && (pt.y >= this->y) && (pt.z >= this->z) && (pt.x < this->x+this->width) && (pt.y < this->y+this->height) && (pt.z < this->z+this->depth);
			} else {
				return (pt.x >= this->x) && (pt.y >= this->y) && (pt.x < this->x+this->width) && (pt.y < this->y+this->height);
			}
		}
		bool PtInRect(TPoint2<T> const& pt) const requires (dim == 3) {
			return (pt.x >= this->x) && (pt.y >= this->y) && (pt.x < this->x+this->width) && (pt.y < this->y+this->height);
		}
		// returns true if rect is within rectangle
		/// @brief *this including B
		/// @return 
		bool RectInRect(this_t const& B) const {
			return (pt0() <= B.pt0()) and (B.pt1() <= pt1());
		}
		/// @brief *this including B (No boundary)
		/// @return 
		bool RectInRectNE(this_t const& B) const {
			return (pt0() < B.pt0()) and (B.pt1() < pt1());
		}

		// Operations

		// set rectangle from left, top, right, and bottom
		void SetRect(T x0 = 0, T y0 = 0, T w = 0, T h = 0) requires (dim == 2) {
			this->x = x0; this->y = y0; this->width = w; this->height = h;
		}
		void SetRect(T x0 = 0, T y0 = 0, T z0 = default_front(), T w = 0, T h = 0, T d = default_depth()) requires (dim == 3) {
			this->x = x0; this->y = y0; this->z = z0; this->width = w; this->height = h; this->depth = d;
		}

		void SetRect(coord_point_t const& pt, coord_size_t const& size)			{ tl() = pt; coord_size() = size; }
		void SetRect(coord_point_t const& pt0, coord_point_t const& pt1)		{ tl() = pt0; coord_size() = pt1-pt0; }
		void SetRectEmpty()														{ SetRect(); }
		void SetRectEmptyForMinMax()											{ tl().SetAll(DBL_MAX); coord_size().SetAll(-DBL_MAX); }
		void SetRectEmptyForMinMax2d()											{ this->x = this->y = DBL_MAX; this->width = this->height = -DBL_MAX; }

		// infflate the rectangles's width, height and depth
		this_t& InflateRect(T x, T y)						requires (dim == 2) { this->x -= x; this->y -= y; this->width += 2*x; this->height += 2*y; return *this; }
		this_t& InflateRect(T x, T y, T z)					requires (dim == 3) { this->x -= x; this->y -= y; this->z -= z; this->widht += 2*x; this->height += 2*y; this->depth += 2*z; return *this; }
		this_t& InflateRect(coord_size_t const& size)							{ tl() -= size; coord_size() += 2*size; return *this; }
		//this_t& InflateRect(this_t const& rect)									{ pt0() -= rect.pt0(); coord_size() += 2*rect.coord_size(); return *this; }
		this_t& InflateRect(T l, T t, T r, T b)				requires (dim == 2) { this->x -= l; this->y -= t; this->width += l+r; this->y += t+b; return *this; }
		this_t& InflateRect(T l, T t, T f, T r, T b, T bk)	requires (dim == 3) { this->x -= l; this->y -= t; this->z -= f; this->width += l+r; this->y += t+b; this->z += f+bk; return *this; }

		// deflate the rectangle's width, height and depth
		this_t& DeflateRect(T x, T y)						requires (dim == 2) { this->x += x; this->y += y; this->widht -= 2*x; this->y -= 2*y; return *this; }
		this_t& DeflateRect(T x, T y, T z)					requires (dim == 3) { this->x += x; this->y += y; this->z += z; this->width -= 2*x; this->height -= 2*y; this->depth -= 2*z; return *this; }
		this_t& DeflateRect(coord_size_t const& size)							{ tl() += size; coord_size() -= 2*size; return *this; }
		//this_t& DeflateRect(this_t const& rect)									{ pt0() += rect.pt0(); pt1() -= rect.pt1(); return *this; }
		this_t& DeflateRect(T l, T t, T r, T b)				requires (dim == 2) { this->x += l; this->y += t; this->width -= l+r; this->height -= t+b; return *this; }
		this_t& DeflateRect(T l, T t, T f, T r, T b, T bk)	requires (dim == 3) { this->x += l; this->y += t; this->z += f; this->width -= l+r; this->height -= t+b; this->depth -= f+bk; return *this; }

		// translate the rectangle by moving its top and left
		this_t& OffsetRect(T x, T y)						requires (dim == 2) { this->x += x; this->y += y; return *this; }
		this_t& OffsetRect(T x, T y, T z)					requires (dim == 3) { this->x += x; this->y += y; this->z += z; return *this; }
		this_t& OffsetRect(coord_point_t const& pt)								{ tl() += pt; return *this; }		// for Point : move whole rect, for Size : move pt1() only.

		this_t& OffsetSize(coord_size_t const& size)							{ coord_size() += size; return *this; }	// for Point : move whole rect, for Size : move pt1() only.

	public:
		void NormalizeRect() {
			if (this->width < 0) {
				this->x = this->x + this->width;
				this->width = -this->width;
			}
			if (this->height < 0) {
				this->y = this->y + this->height;
				this->height = -this->height;
			}
			if constexpr (dim >= 3) {
				if (this->depth < 0) {
					this->z = this->z + this->depth;
					this->depth = -this->depth;
				}
			}
		}
		bool IsNormalized() const {
			if constexpr (dim >= 3) {
				return (this->width >= 0) and (this->height >= 0) and (this->depth >= 0);
			}
			else {
				return (this->width >= 0) and (this->height >= 0);
			}
		}

		// absolute position of rectangle
		void MoveToX(T x)									{ this->x = x; }
		void MoveToY(T y)									{ this->y = y; }
		void MoveToZ(T z)				requires (dim >= 3)	{ this->z = z; }
		//void MoveToN(int i, T v) { assert(pt0.data().size() > i); pt1[i] += (v-pt0[i]); pt0[i] = v; }

		void MoveToXY(T x, T y)								{ MoveToX(x); MoveToY(y); }
		void MoveToYZ(T y, T z)			requires (dim >= 3) { MoveToY(y); MoveToZ(z); }
		void MoveToZX(T z, T x)			requires (dim >= 3) { MoveToZ(z); MoveToX(x); }
		void MoveToXYZ(T x, T y, T z)	requires (dim >= 3) { MoveToX(x); MoveToY(y); MoveToZ(z); }

		void MoveTo(coord_point_t const& pt) {
			tl() = pt;
		}

		// set this rectangle to intersection of two others
		this_t& IntersectRect(this_t rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			this->x = std::max(this->x, rect2.x);
			this->y = std::max(this->y, rect2.y);
			if constexpr (dim >= 3)
				this->z = std::max(this->z, rect2.z);

			this->width = std::min(this->x + this->width, rect2.x + rect2.width) - this->x;
			this->height = std::min(this->y + this->height, rect2.y + rect2.height) - this->y;
			if constexpr (dim >= 3)
				this->depth = std::min(this->z + this->depth, rect2.z + this->depth) - this->z;

			return *this;
		}

		// set this rectangle to bounding union of two others
		this_t& UnionRect(this_t rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			this->x = std::min(this->x, rect2.x);
			this->y = std::min(this->y, rect2.y);
			if constexpr (dim >= 3)
				this->z = std::min(this->z, rect2.z);

			this->width = std::max(this->x + this->width, rect2.x + rect2.width) - this->x;
			this->height = std::max(this->y + this->height, rect2.x + rect2.height) - this->y;
			if constexpr (dim >= 3)
				this->depth = std::max(this->z + this->depth, rect2.z + rect2.depth) - this->z;

			return *this;
		}

		// Operators
		this_t& operator += (coord_point_t const& pt)		{ return OffsetRect(pt); }
		this_t& operator -= (coord_point_t const& pt)		{ return OffsetRect(-pt); }
		this_t& operator += (coord_size_t const& size)		{ return OffsetSize(size); }
		this_t& operator -= (coord_size_t const& size)		{ return OffsetSize(-size); }
		//this_t& operator += (this_t const& rect)			{ return InflateRect(rect); }
		//this_t& operator -= (this_t const& rect)			{ return DeflateRect(rect); }
		this_t& operator &= (this_t const& rect)			{ return IntersectRect(rect); }
		this_t& operator |= (this_t const& rect)			{ return UnionRect(rect); }

		// Operators returning TSRectT data()
		this_t operator + (coord_point_t const& pt) const	{ return this_t(*this) += pt; }
		this_t operator - (coord_point_t const& pt) const	{ return this_t(*this) -= pt; }
		this_t operator + (coord_size_t const& size) const	{ return this_t(*this) += size; }
		this_t operator - (coord_size_t const& size) const	{ return this_t(*this) -= size; }
		//this_t operator + (this_t const& rect) const		{ return this_t(*this) += rect; }
		//this_t operator - (this_t const& rect) const		{ return this_t(*this) -= rect; }
		this_t operator & (this_t const& rect) const		{ return this_t(*this).IntersectRect(rect); }
		this_t operator | (this_t const& rect) const		{ return this_t(*this).UnionRect(rect); }

		//friend class boost::serialization::access;
		template < typename tBoostArchive >
		friend void serialize(tBoostArchive &ar, TSRectT& rect, unsigned int const version) {
			ar & rect;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, this_t& B) {
			return ar & B.tl() & B.coord_size();
		}
		template < typename JSON > friend void from_json(JSON const& j, this_t& B) { B.tl() = j["tl"]; B.coord_size() = j["coord_size"]; }
		template < typename JSON > friend void to_json(JSON&& j, this_t const& B) { j["tl"] = B.tl(); j["coord_size"] = B.coord_size(); }


		bool UpdateBoundary(coord_point_t const& pt) {
			bool bModified{};
			for (size_t i {}; i < pt.size(); i++) {
				if (tl().member(i) > pt.member(i)) {
					bModified = true;
					tl().member(i) = pt.member(i);
				}
			}
			for (size_t i = 0; i < pt.size(); i++) {
				if (pt1().member(i) < pt.member(i)) {
					bModified = true;
					coord_size().member(i) = pt.member(i) - tl().member(i);
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
				TSRectT<T_INT, 2> rect(*this);
				return rect.IsROI_Valid(sizeImage);
			}

			return 1
				&& (this->x >= 0)
				&& (this->y >= 0)
				&& ( (sizeImage.cx < 0) || ( (this->x < sizeImage.cx) && (this->x + this->width < sizeImage.cx) && (this->width > 0) ) )
				&& ( (sizeImage.cy < 0) || ( (this->y < sizeImage.cy) && (this->y + this->height < sizeImage.cy) && (this->height > 0) ) )
				;
		}

		template < std::integral T_INT = std::int32_t >
		bool AdjustROI(TSize2<T> const& sizeImage) {
			NormalizeRect();

			if (this->x < 0)
				this->x = 0;
			if (this->y < 0)
				this->y = 0;
			if ( (sizeImage.cx > 0) && (this->x + this->width > sizeImage.cx) )
				this->width = std::max(0, sizeImage.cx - this->x);
			if ( (sizeImage.cy > 0) && (this->y + this->height > sizeImage.cy) )
				this->height = std::max(0, sizeImage.cy - this->y);

			return !IsRectEmpty();
		}

		template < std::integral T_INT = std::int32_t>
		[[nodiscard]] TSRectT<T_INT, 2> GetSafeROI(TSize2<T_INT> const& sizeImage) const {
			TSRectT<T_INT, 2> rect(*this);

			rect.NormalizeRect();

			if (rect.x < 0)
				rect.x = 0;
			if (rect.y < 0)
				rect.y = 0;
			if ( (sizeImage.cx > 0) && (rect.x + rect.width > sizeImage.cx) )
				rect.width = std::max(0, sizeImage.cx - rect.x);
			if ( (sizeImage.cy > 0) && (rect.y + rect.height > sizeImage.cy) )
				rect.height = std::max(0, sizeImage.cy - rect.y);

			return rect;
		}

	};

	template < typename T > using TSRect2 = TSRectT<T, 2>;
	template < typename T > using TSRect3 = TSRectT<T, 3>;


}	// namespace gtl
