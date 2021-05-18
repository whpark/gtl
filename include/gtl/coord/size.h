#pragma once

//////////////////////////////////////////////////////////////////////
//
// size.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 새로 작성 시작
//
//////////////////////////////////////////////////////////////////////

#include "gtl/_default.h"
#include "gtl/misc.h"
#include "gtl/concepts.h"

//#ifdef min
//#	undef min	// undefine WinAPI - min. ( use std::min only.)
//#endif

namespace gtl {

#pragma pack(push, 1)

	//=============================================================================

	template < typename T > struct TSize3;
	template < typename T > struct TSize2;

	//--------------------------------------------------------------------------------------------------------------------------------
	// Size 3 (cx, cy, cz)
	//
	template < typename T >
	struct __declspec(novtable) TSize3 {
	public:
		T cx{}, cy{}, cz{};
		using coord_t = std::array<T, 3>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr const coord_t& data() const { return std::bit_cast<coord_t const&>(*this); }

	public:
	// Constructors
		TSize3() = default;
		TSize3(const TSize3&) = default;
		TSize3(T cx, T cy, T cz) : cx(cx), cy(cy), cz(cz) { }
		template < typename T2 >
		TSize3(T2 cx, T2 cy, T2 cz) : cx(RoundOrForward<T,T2>(cx)), cy(RoundOrForward<T,T2>(cy)), cz(RoundOrForward<T,T2>(cz)) { }
		TSize3& operator = (const TSize3&) = default;

	// Copy Constructors and Copy Assign operators
		template < gtl::is__coord T_COORD > explicit TSize3(const T_COORD& B) { *this = B; };
		template < gtl::is__coord T_COORD > TSize3& operator = (const T_COORD& B) {
			if constexpr (has__cxy<T_COORD>) {
				cx = RoundOrForward<T>(B.cx);
				cy = RoundOrForward<T>(B.cy);
				if constexpr (has__cz<T_COORD>)
					cz = RoundOrForward<T>(B.cz);
			} else if constexpr (has__size2<T_COORD>) {
				cx = RoundOrForward<T>(B.width);
				cy = RoundOrForward<T>(B.height);
				if constexpr (has__depth<T_COORD>)
					cz = RoundOrForward<T>(B.depth);
			} else if constexpr (has__xy<T_COORD>) {
				cx = RoundOrForward<T>(B.x);
				cy = RoundOrForward<T>(B.y);
				if constexpr (has__z<T_COORD>)
					cz = RoundOrForward<T>(B.z);
			} else static_assert(false);
			return *this;
		};

		// Rect
		template < gtl::is__rect T_COORD > explicit TSize3(const T_COORD& B) { *this = B; }
		template < gtl::is__rect T_COORD > TSize3& operator = (const T_COORD& B) {
			*this = B.pt1 - B.pt0;
			return *this;
		}

	// MFC
		explicit TSize3(const RECT& B) { *this = B; }
		TSize3& operator = (const RECT& B) {
			cx = B.right-B.left;
			cy = B.bottom-B.top;
			return *this;
		}

	// Type Casting to cv::Size_, SIZE, ...
		template < typename T2, template <typename> typename T_COORD > requires gtl::is__coord2<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(cx), RoundOrForward<T2, T>(cy) }; };
		template < typename T2, template <typename> typename T_COORD > requires gtl::is__coord3<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(cx), RoundOrForward<T2, T>(cy), RoundOrForward<T2, T>(cz) }; };
		template < gtl::is__coord2 T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		template < gtl::is__coord3 T_COORD > operator T_COORD() const { return T_COORD{cx, cy, cz}; }

	// Compare
		auto operator <=> (const TSize3&) const = default;
		auto operator <=> (const T& b) const { return *this <=> TSize3(b, b, b); }

		auto CountNonZero() const														{ return std::count_if(data().begin(), data().end(), gtl::non_zero{}); }
		auto CountIf(std::function<bool(const T&)> pred) const							{ return std::count_if(data().begin(), data().end(), pred); }

	// Numerical Operators
		TSize3& operator +=  (const TSize3<T>& B)										{ cx += B.cx; cy += B.cy; cz += B.cz; return *this; }
		TSize3& operator -=  (const TSize3<T>& B)										{ cx -= B.cx; cy -= B.cy; cz -= B.cz; return *this; }
		TSize3 operator + (const TSize3<T>& B) const									{ return TSize3(cx+B.cx, cy+B.cy, cz+B.cz); }
		TSize3 operator - (const TSize3<T>& B) const									{ return TSize3(cx-B.cx, cy-B.cy, cz-B.cz); }

		TSize3 operator - () const														{ return TSize3(-cx, -cy, -cz); }

		template < is__coord T_COORD > TSize3& operator += (const T_COORD& B) {
			if constexpr (has__cxy<T_COORD>) {
				gtl::internal::DoArithmaticAdd(cx, B.cx);
				gtl::internal::DoArithmaticAdd(cy, B.cy);
				if constexpr (has__cz<T_COORD>)
					gtl::internal::DoArithmaticAdd(cz, B.cz);
			} else if constexpr (has__size2<T_COORD>) {
				gtl::internal::DoArithmaticAdd(cx, B.width);
				gtl::internal::DoArithmaticAdd(cy, B.height);
				if constexpr (has__depth<T_COORD>)
					gtl::internal::DoArithmaticAdd(cz, B.depth);
			} else if constexpr (has__xy<T_COORD>) {
				gtl::internal::DoArithmaticAdd(cx, B.x);
				gtl::internal::DoArithmaticAdd(cy, B.y);
				if constexpr (has__z<T_COORD>)
					gtl::internal::DoArithmaticAdd(cz, B.z);
			} else static_assert(false);
			return *this;
		};
		template < is__coord T_COORD > TSize3& operator -= (const T_COORD& B) {
			if constexpr (has__cxy<T_COORD>) {
				gtl::internal::DoArithmaticSub(cx, B.cx);
				gtl::internal::DoArithmaticSub(cy, B.cy);
				if constexpr (has__cz<T_COORD>)
					gtl::internal::DoArithmaticSub(cz, B.cz);
			} else if constexpr (has__size2<T_COORD>) {
				gtl::internal::DoArithmaticSub(cx, B.width);
				gtl::internal::DoArithmaticSub(cy, B.height);
				if constexpr (has__depth<T_COORD>)
					gtl::internal::DoArithmaticSub(cz, B.depth);
			} else if constexpr (has__xy<T_COORD>) {
				gtl::internal::DoArithmaticSub(cx, B.c);
				gtl::internal::DoArithmaticSub(cy, B.c);
				if constexpr (has__z<T_COORD>)
					gtl::internal::DoArithmaticSub(cz, B.z);
			} else static_assert(false);
			return *this;
		};

		template < is__coord T_COORD > TSize3& operator + (const T_COORD& B)			{ return TSize3(*this) += B; }
		template < is__coord T_COORD > TSize3& operator - (const T_COORD& B)			{ return TSize3(*this) -= B; }

	public:
		template < class Operation >
		inline TSize3& ForAll(Operation&& op) {
			op(cx); op(cy); op(cz);
			return *this;
		}

	public:
		template < gtl::is__arithmatic T2 > TSize3& operator *= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticMul(v, d); }); };
		template < gtl::is__arithmatic T2 > TSize3& operator /= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticDiv(v, d); }); };
		template < gtl::is__arithmatic T2 > TSize3& operator += (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticAdd(v, d); }); };
		template < gtl::is__arithmatic T2 > TSize3& operator -= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticSub(v, d); }); };

		template < gtl::is__arithmatic T2 > TSize3 operator * (T2 d) const				{ return TSize3(cx*d, cy*d, cz*d); };
		template < gtl::is__arithmatic T2 > TSize3 operator / (T2 d) const				{ return TSize3(cx/d, cy/d, cz/d); };
		template < gtl::is__arithmatic T2 > TSize3 operator + (T2 d) const				{ return TSize3(cx+d, cy+d, cz+d); };
		template < gtl::is__arithmatic T2 > TSize3 operator - (T2 d) const				{ return TSize3(cx-d, cy-d, cz-d); };

		template < gtl::is__arithmatic T2 > friend TSize3 operator * (T2 A, const TSize3& B) { return TSize3(A*B.cx, A*B.cy, A*B.cz); }
		template < gtl::is__arithmatic T2 > friend TSize3 operator / (T2 A, const TSize3& B) { return TSize3(A/B.cx, A/B.cy, A/B.cz); }
		template < gtl::is__arithmatic T2 > friend TSize3 operator + (T2 A, const TSize3& B) { return TSize3(A+B.cx, A+B.cy, A+B.cz); }
		template < gtl::is__arithmatic T2 > friend TSize3 operator - (T2 A, const TSize3& B) { return TSize3(A-B.cx, A-B.cy, A-B.cz); }

		//
		template < gtl::is__arithmatic T2 > 
		TSize3& Set(T2 cx, T2 cy, T2 cz) {
			this->cx = RoundOrForward<T, T2>(cx);
			this->cy = RoundOrForward<T, T2>(cy);
			this->cz = RoundOrForward<T, T2>(cz);
			return *this;
		}
		TSize3& Set(T cx = {}, T cy = {}, T cz = {})									{ this->cx = cx; this->cy = cy; this->cz = cz; return *this; }
		TSize3& SetAll(T value = {})													{ cx = cy = cz = value; return *this; }
		TSize3& SetZero()																{ return SetAll(); }
		TSize3& SetAsNAN() requires (std::is_floating_point_v<T>)						{ memset(&cx, 0xff, sizeof(*this)); return *this; };

		bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (int i = 0; i < data().size(); i++)
				if ( std::isnan(data()[i]) || std::isfinite(data()[i]) )
					return false;
			return true;
		};

	// Archiving
		template < class Archive > friend Archive& operator & (Archive& ar, const TSize3& B) {
			return ar & B.cx & B.cy & B.cz;
		}

	};

	//--------------------------------------------------------------------------------------------------------------------------------
	// Size 2 (cx, cy)
	//
	template < typename T >
	struct __declspec(novtable) TSize2 {
	public:
		T cx{}, cy{};
		using coord_t = std::array<T, 2>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr const coord_t& data() const { return std::bit_cast<coord_t const&>(*this); }

	public:
	// Constructors
		TSize2() = default;
		TSize2(const TSize2&) = default;
		TSize2(T cx, T cy) : cx(cx), cy(cy) { }
		template < typename T2 >
		TSize2(T2 cx, T2 cy) : cx(RoundOrForward<T,T2>(cx)), cy(RoundOrForward<T,T2>(cy)) { }
		TSize2& operator = (const TSize2&) = default;

		template < gtl::is__coord T_COORD > explicit TSize2(const T_COORD& B) { *this = B; };
		template < gtl::is__coord T_COORD > TSize2& operator = (const T_COORD& B) {
			if constexpr (has__cxy<T_COORD>) {
				cx = RoundOrForward<T>(B.cx);
				cy = RoundOrForward<T>(B.cy);
			} else if constexpr (has__size2<T_COORD>) {
				cx = RoundOrForward<T>(B.width);
				cy = RoundOrForward<T>(B.height);
			} else if constexpr (has__xy<T_COORD>) {
				cx = RoundOrForward<T>(B.x);
				cy = RoundOrForward<T>(B.y);
			} else static_assert(false);
			return *this;
		};

		// Rect
		template < gtl::is__rect T_COORD > explicit TSize2(const T_COORD& B) { *this = B; }
		template < gtl::is__rect T_COORD > TSize2& operator = (const T_COORD& B) {
			for (int i = 0; i < std::min(data().size(), B.pt0.size()); i++)
				data()[i] = RoundOrForward<T>(B.pt1[i]-B.pt0[i]);
			return *this;
		}

		// MFC
		explicit TSize2(const RECT& B) { *this = B; }
		TSize2& operator = (const RECT& B) {
			cx = B.right-B.left;
			cy = B.bottom-B.top;
			return *this;
		}

	// Type Casting to cv::Size_, SIZE, ...
		template < typename T2, template <typename> typename T_COORD > requires gtl::is__coord2<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(cx), RoundOrForward<T2, T>(cy) }; };
		template < typename T2, template <typename> typename T_COORD > requires gtl::is__coord3<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(cx), RoundOrForward<T2, T>(cy), {} }; };
		template < gtl::is__coord2 T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		template < gtl::is__coord3 T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }
		//template < gtl::is__xy	T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		//template < gtl::is__xyz	T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }
		//template < gtl::is__cxy	T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		//template < gtl::is__cxyz	T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }
		//template < gtl::is__size2	T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		//template < gtl::is__size3	T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }


	// Compare
		auto operator <=> (const TSize2&) const = default;
		auto operator <=> (const T& b) const { return *this <=> TSize2(b, b, b); }

		auto CountNonZero() const														{ return std::count_if(data().begin(), data().end(), gtl::non_zero{}); }
		auto CountIf(std::function<bool(const T&)> pred) const							{ return std::count_if(data().begin(), data().end(), pred); }

	// Numerical Operators
		TSize2& operator +=  (const TSize2<T>& B)										{ cx += B.cx; cy += B.cy; return *this; }
		TSize2& operator -=  (const TSize2<T>& B)										{ cx -= B.cx; cy -= B.cy; return *this; }
		TSize2 operator + (const TSize2<T>& B) const									{ return TSize2(*this) += B; }
		TSize2 operator - (const TSize2<T>& B) const									{ return TSize2(*this) -= B; }

		TSize2 operator - () const														{ return TSize2(-cx, -cy); }

		template < is__coord T_COORD > TSize2& operator += (const T_COORD& B) {
			if constexpr (has__cxy<T_COORD>) {
				cx += RoundOrForward<T>(B.cx);
				cy += RoundOrForward<T>(B.cy);
			} else if constexpr (has__size2<T_COORD>) {
				cx += RoundOrForward<T>(B.width);
				cy += RoundOrForward<T>(B.height);
			} else if constexpr (has__xy<T_COORD>) {
				cx += RoundOrForward<T>(B.x);
				cy += RoundOrForward<T>(B.y);
			} else static_assert(false);
			return *this;
		};
		template < is__coord T_COORD > TSize2& operator -= (const T_COORD& B) {
			if constexpr (has__cxy<T_COORD>) {
				cx -= RoundOrForward<T>(B.cx);
				cy -= RoundOrForward<T>(B.cy);
			} else if constexpr (has__size2<T_COORD>) {
				cx -= RoundOrForward<T>(B.width);
				cy -= RoundOrForward<T>(B.height);
			} else if constexpr (has__xy<T_COORD>) {
				cx -= RoundOrForward<T>(B.x);
				cy -= RoundOrForward<T>(B.y);
			} else static_assert(false);
			return *this;
		};

		template < is__coord T_COORD > TSize2& operator + (const T_COORD& B)			{ return TSize2(*this) += B; }
		template < is__coord T_COORD > TSize2& operator - (const T_COORD& B)			{ return TSize2(*this) -= B; }

	public:
		template < class Operation >
		inline TSize2& ForAll(Operation&& op) {
			op(cx); op(cy);
			return *this;
		}

	public:
		template < gtl::is__arithmatic T2 > TSize2& operator *= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticMul(v, d); }); };
		template < gtl::is__arithmatic T2 > TSize2& operator /= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticDiv(v, d); }); };
		template < gtl::is__arithmatic T2 > TSize2& operator += (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticAdd(v, d); }); };
		template < gtl::is__arithmatic T2 > TSize2& operator -= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticSub(v, d); }); };

		template < gtl::is__arithmatic T2 > TSize2 operator * (T2 d) const				{ return TSize2(cx*d, cy*d); };
		template < gtl::is__arithmatic T2 > TSize2 operator / (T2 d) const				{ return TSize2(cx/d, cy/d); };
		template < gtl::is__arithmatic T2 > TSize2 operator + (T2 d) const				{ return TSize2(cx+d, cy+d); };
		template < gtl::is__arithmatic T2 > TSize2 operator - (T2 d) const				{ return TSize2(cx-d, cy-d); };

		template < gtl::is__arithmatic T2 > friend TSize2 operator * (T2 A, const TSize2& B) { return TSize2(A*B.cx, A*B.cy); }
		template < gtl::is__arithmatic T2 > friend TSize2 operator / (T2 A, const TSize2& B) { return TSize2(A/B.cx, A/B.cy); }
		template < gtl::is__arithmatic T2 > friend TSize2 operator + (T2 A, const TSize2& B) { return TSize2(A+B.cx, A+B.cy); }
		template < gtl::is__arithmatic T2 > friend TSize2 operator - (T2 A, const TSize2& B) { return TSize2(A-B.cx, A-B.cy); }

		//
		void Set(T cx = {}, T cy = {})													{ this->cx = cx; this->cy = cy; }
		void SetAll(T value = {})														{ cx = cy = value; }
		void SetZero()																	{ SetAll(); }
		void SetAsNAN() requires (std::is_floating_point_v<T>) {
			memset(&cx, 0xff, sizeof(*this));
		};

		bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (int i = 0; i < data().size(); i++)
				if ( std::isnan(data()[i]) || std::isfinite(data()[i]) )
					return false;
			return true;
		};

	// Archiving
		template < class Archive > friend Archive& operator & (Archive& ar, const TSize2& B) {
			return ar & B.cx & B.cy;
		}

	};


#pragma pack(pop)

}	// namespace gtl
