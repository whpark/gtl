#pragma once

//////////////////////////////////////////////////////////////////////
//
// rect_size.h: rect - top,left/width,height
//
// PWH
// 2021.08.13.
//
//////////////////////////////////////////////////////////////////////

#include "gtl/misc.h"
#include "gtl/concepts.h"

#include "coord_size.h"
#include "coord_point.h"

//#ifdef min
//#	undef min	// undefine WinAPI - min. ( use std::min only.)
//#endif

namespace gtl {
#pragma pack(push, 1)


	template < typename T, int dim, int DEFAULT_FRONT = -1, int DEFAULT_DEPTH = 1 > struct TSRectT;

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
	template < typename T, int dim, int DEFAULT_FRONT = -1, int DEFAULT_DEPT = 2 >
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
				coord_size() = B.coord_size();
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
			: base_t{ RoundOrForward<T>(pt0.x), RoundOrForward<T>(pt0.y), RoundOrForward<T>(pt0.z), RoundOrForward<T>(pt1.x-pt0.x), RoundOrForward<T>(pt1.y-pt1.y), RoundOrForward<T>(pt1.z-pt0.z)} { };

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
			return (width > 0) or (height > 0);
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
		void SetRectEmptyForMinMax()											{ pt0().SetAll(DBL_MAX); pt1().SetAll(-DBL_MAX); }
		void SetRectEmptyForMinMax2d()											{ this->left = this->top = DBL_MAX; this->right = this->bottom = -DBL_MAX; }

		// infflate the rectangles's width, height and depth
		this_t& InflateRect(T x, T y)						requires (dim == 2) { this->left -= x; this->top -= y; this->right += x; this->bottom += y; return *this; }
		this_t& InflateRect(T x, T y, T z)					requires (dim == 3) { this->left -= x; this->top -= y; this->front -= z; this->right += x; this->bottom += y; this->back += z; return *this; }
		this_t& InflateRect(coord_size_t const& size)							{ pt0() -= size; pt1() += size; return *this; }
		this_t& InflateRect(this_t const& rect)									{ pt0() -= rect.pt0(); pt1() += rect.pt1(); return *this; }
		this_t& InflateRect(T l, T t, T r, T b)				requires (dim == 2) { this->left -= l; this->top -= t; this->right += r; this->bottom += b; return *this; }
		this_t& InflateRect(T l, T t, T f, T r, T b, T bk)	requires (dim == 3) { this->left -= l; this->top -= t; this->front -= f; this->right += r; this->bottom += b; this->back += bk; return *this; }

		// deflate the rectangle's width, height and depth
		this_t& DeflateRect(T x, T y)						requires (dim == 2) { this->left += x; this->top += y; this->right -= x; this->bottom -= y; return *this; }
		this_t& DeflateRect(T x, T y, T z)					requires (dim == 3) { this->left += x; this->top += y; this->front += z; this->right -= x; this->bottom -= y; this->back -= z; return *this; }
		this_t& DeflateRect(coord_size_t const& size)							{ pt0() += size; pt1() -= size; return *this; }
		this_t& DeflateRect(this_t const& rect)									{ pt0() += rect.pt0(); pt1() -= rect.pt1(); return *this; }
		this_t& DeflateRect(T l, T t, T r, T b)				requires (dim == 2) { this->left += l; this->top += t; this->right -= r; this->bottom -= b; return *this; }
		this_t& DeflateRect(T l, T t, T f, T r, T b, T bk)	requires (dim == 3) { this->left += l; this->top += t; this->front += f; this->right -= r; this->bottom -= b; this->back -= bk; return *this; }

		// translate the rectangle by moving its top and left
		this_t& OffsetRect(T x, T y)						requires (dim == 2) { this->left += x; this->top += y; this->right += x; this->bottom += y; return *this; }
		this_t& OffsetRect(T x, T y, T z)					requires (dim == 3) { this->left += x; this->top += y; this->front += z; this->right += x; this->bottom += y; this->back += z; return *this; }
		this_t& OffsetRect(coord_point_t const& pt)								{ pt0() += pt; pt1() += pt; return *this; }		// for Point : move whole rect, for Size : move pt1() only.

		this_t& OffsetSize(coord_size_t const& size)							{ /*pt0 += size;*/ pt1() += size; return *this; }	// for Point : move whole rect, for Size : move pt1() only.

	public:
		void NormalizeRect() {
			if (this->left > this->right) std::swap(this->left, this->right);
			if (this->top > this->bottom) std::swap(this->top, this->bottom);
			if constexpr (dim >= 3) {
				if (this->front > this->back) std::swap(this->front, this->back);
			}
		}
		bool IsNormalized() const {
			bool bResult = (this->left <= this->right) and (this->top <= this->bottom);
			if constexpr (dim >= 3) {
				bResult &= this->front <= this->back;
			}
			return bResult;
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

		void MoveTo(coord_point_t const& pt) {
			MoveToX(pt.x);
			MoveToY(pt.y);
			if constexpr (dim >= 3)
				MoveToZ(pt.z);
		}

		// set this rectangle to intersection of two others
		this_t& IntersectRect(this_t rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pt0().x = std::max(pt0().x, rect2.pt0().x);
			pt0().y = std::max(pt0().y, rect2.pt0().y);
			if constexpr (dim >= 3)
				pt0().z = std::max(pt0().z, rect2.pt0().z);

			pt1().x = std::min(pt1().x, rect2.pt1().x);
			pt1().y = std::min(pt1().y, rect2.pt1().y);
			if constexpr (dim >= 3)
				pt1().z = std::min(pt1().z, rect2.pt1().z);

			return *this;
		}

		// set this rectangle to bounding union of two others
		this_t& UnionRect(this_t rect2) {
			NormalizeRect();
			rect2.NormalizeRect();

			pt0().x = std::min(pt0().x, rect2.pt0().x);
			pt0().y = std::min(pt0().y, rect2.pt0().y);
			if constexpr (dim >= 3)
				pt0().z = std::min(pt0().z, rect2.pt0().z);

			pt1().x = std::max(pt1().x, rect2.pt1().x);
			pt1().y = std::max(pt1().y, rect2.pt1().y);
			if constexpr (dim >= 3)
				pt1().z = std::max(pt1().z, rect2.pt1().z);

			return *this;
		}

		// Operators
		this_t& operator += (coord_point_t const& pt)		{ return OffsetRect(pt); }
		this_t& operator -= (coord_point_t const& pt)		{ return OffsetRect(-pt); }
		this_t& operator += (coord_size_t const& size)		{ return OffsetSize(size); }
		this_t& operator -= (coord_size_t const& size)		{ return OffsetSize(-size); }
		this_t& operator += (this_t const& rect)			{ return InflateRect(rect); }
		this_t& operator -= (this_t const& rect)			{ return DeflateRect(rect); }
		this_t& operator &= (this_t const& rect)			{ return IntersectRect(rect); }
		this_t& operator |= (this_t const& rect)			{ return UnionRect(rect); }

		// Operators returning TSRectT data()
		this_t operator + (coord_point_t const& pt) const	{ return this_t(*this) += pt; }
		this_t operator - (coord_point_t const& pt) const	{ return this_t(*this) -= pt; }
		this_t operator + (coord_size_t const& size) const	{ return this_t(*this) += size; }
		this_t operator - (coord_size_t const& size) const	{ return this_t(*this) -= size; }
		this_t operator + (this_t const& rect) const		{ return this_t(*this) += rect; }
		this_t operator - (this_t const& rect) const		{ return this_t(*this) -= rect; }
		this_t operator & (this_t const& rect) const		{ return this_t(*this).IntersectRect(rect); }
		this_t operator | (this_t const& rect) const		{ return this_t(*this).UnionRect(rect); }

		//friend class boost::serialization::access;
		template < typename tBoostArchive >
		friend void serialize(tBoostArchive &ar, TSRectT& rect, unsigned int const version) {
			ar & rect;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, this_t& B) {
			return ar & B.pt0() & B.pt1();
		}
		template < typename JSON > friend void from_json(JSON const& j, this_t& B) { B.pt0() = j["pt0"]; B.pt1() = j["pt1"]; }
		template < typename JSON > friend void to_json(JSON&& j, this_t const& B) { j["pt0"] = B.pt0(); j["pt1"] = B.pt1(); }


		bool UpdateBoundary(coord_point_t const& pt) {
			bool bModified{};
			for (size_t i {}; i < pt.size(); i++) {
				if (pt0().member(i) > pt.member(i)) {
					bModified = true;
					pt0().member(i) = pt.member(i);
				}
			}
			for (size_t i = 0; i < pt.size(); i++) {
				if (pt1().member(i) < pt.member(i)) {
					bModified = true;
					pt1().member(i) = pt.member(i);
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
		[[nodiscard]] TSRectT<T_INT, 2> GetSafeROI(TSize2<T_INT> const& sizeImage) const {
			TSRectT<T_INT, 2> rect(*this);

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

	template < typename T > using TRect2 = TSRectT<T, 2>;
	template < typename T > using TRect3 = TSRectT<T, 3>;


#pragma pack(pop)
}	// namespace gtl
