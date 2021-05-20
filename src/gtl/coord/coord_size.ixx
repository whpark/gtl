//////////////////////////////////////////////////////////////////////
//
// size.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 새로 작성 시작
//
//////////////////////////////////////////////////////////////////////

module;

#include <compare>
#include <functional>
#include <algorithm>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:coord_size;

import :concepts;
import :misc;

//#ifdef min
//#	undef min	// undefine WinAPI - min. ( use std::min only.)
//#endif

export namespace gtl {

	//=============================================================================

	template < typename T > struct TSize3;
	template < typename T > struct TSize2;

	//--------------------------------------------------------------------------------------------------------------------------------
	// Size 3 (cx, cy, cz)
	//
	template < typename T >
	struct TSize3 {
	public:
		T cx{}, cy{}, cz{};
		using coord_t = std::array<T, 3>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_t& member(int i) { return data()[i]; }
		constexpr value_t const& member(int i) const { return data()[i]; }

	public:
	// Constructors
		TSize3() = default;
		TSize3(TSize3 const&) = default;
		TSize3(T cx, T cy, T cz) : cx(cx), cy(cy), cz(cz) { }
		template < typename T2 >
		TSize3(T2 cx, T2 cy, T2 cz) : cx(RoundOrForward<T,T2>(cx)), cy(RoundOrForward<T,T2>(cy)), cz(RoundOrForward<T,T2>(cz)) { }
		TSize3& operator = (TSize3 const&) = default;

	// Copy Constructors and Copy Assign operators
		template < gtlc::generic_coord T_COORD > explicit TSize3(T_COORD const& B) { *this = B; };
		template < gtlc::generic_coord T_COORD > TSize3& operator = (T_COORD const& B) {
			if constexpr (gtlc::has__cxy<T_COORD>) {
				cx = RoundOrForward<T>(B.cx);
				cy = RoundOrForward<T>(B.cy);
				if constexpr (gtlc::has__cz<T_COORD>)
					cz = RoundOrForward<T>(B.cz);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				cx = RoundOrForward<T>(B.width);
				cy = RoundOrForward<T>(B.height);
				if constexpr (gtlc::has__depth<T_COORD>)
					cz = RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__xy<T_COORD>) {
				cx = RoundOrForward<T>(B.x);
				cy = RoundOrForward<T>(B.y);
				if constexpr (has__z<T_COORD>)
					cz = RoundOrForward<T>(B.z);
			} else if constexpr (gtlc::rect<T_COORD>) {
				*this = (B.pt1 - B.pt0);
			} else static_assert(false);
			return *this;
		};

	// Type Casting to cv::Size_, SIZE, ...
		//template < typename T2, template <typename> typename T_COORD > requires gtlc::coord2<T_COORD<T2>>
		//operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(cx), RoundOrForward<T2, T>(cy) }; };
		//template < typename T2, template <typename> typename T_COORD > requires gtlc::coord3<T_COORD<T2>>
		//operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(cx), RoundOrForward<T2, T>(cy), RoundOrForward<T2, T>(cz) }; };
		//template < gtlc::coord2 T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		//template < gtlc::coord3 T_COORD > operator T_COORD() const { return T_COORD{cx, cy, cz}; }
		template < typename T_COORD > requires gtlc::wnd_point<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(cx), RoundOrForward<decltype(T_COORD::y)>(cy)}; }
		template < typename T_COORD > requires gtlc::wnd_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::cx)>(cx), RoundOrForward<decltype(T_COORD::cy)>(cy)}; }
		template < typename T_COORD > requires gtlc::cv_point2<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(cx), RoundOrForward<decltype(T_COORD::y)>(cy)}; }
		template < typename T_COORD > requires gtlc::cv_point3<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(cx), RoundOrForward<decltype(T_COORD::y)>(cy), RoundOrForward<decltype(T_COORD::z)>(cz)}; }
		template < typename T_COORD > requires gtlc::cv_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::width)>(cx), RoundOrForward<decltype(T_COORD::height)>(cy)}; }

	// Compare
		[[nodiscard]] auto operator <=> (TSize3 const&) const = default;
		[[nodiscard]] auto operator <=> (T const& b) const { return *this <=> TSize3(b, b, b); }

		[[nodiscard]] auto CountNonZero() const														{ return std::count_if(data().begin(), data().end(), gtl::non_zero{}); }
		[[nodiscard]] auto CountIf(std::function<bool(T const&)> pred) const							{ return std::count_if(data().begin(), data().end(), pred); }

	// Numerical Operators
		TSize3& operator +=  (TSize3<T> const& B)										{ cx += B.cx; cy += B.cy; cz += B.cz; return *this; }
		TSize3& operator -=  (TSize3<T> const& B)										{ cx -= B.cx; cy -= B.cy; cz -= B.cz; return *this; }
		[[nodiscard]] TSize3 operator + (TSize3<T> const& B) const									{ return TSize3(cx+B.cx, cy+B.cy, cz+B.cz); }
		[[nodiscard]] TSize3 operator - (TSize3<T> const& B) const									{ return TSize3(cx-B.cx, cy-B.cy, cz-B.cz); }

		[[nodiscard]] TSize3 operator - () const														{ return TSize3(-cx, -cy, -cz); }

		template < gtlc::coord T_COORD > TSize3& operator += (T_COORD const& B) {
			if constexpr (gtlc::has__cxy<T_COORD>) {
				gtl::internal::DoArithmaticAdd(cx, B.cx);
				gtl::internal::DoArithmaticAdd(cy, B.cy);
				if constexpr (gtlc::has__cz<T_COORD>)
					gtl::internal::DoArithmaticAdd(cz, B.cz);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				gtl::internal::DoArithmaticAdd(cx, B.width);
				gtl::internal::DoArithmaticAdd(cy, B.height);
				if constexpr (gtlc::has__depth<T_COORD>)
					gtl::internal::DoArithmaticAdd(cz, B.depth);
			} else if constexpr (gtlc::has__xy<T_COORD>) {
				gtl::internal::DoArithmaticAdd(cx, B.x);
				gtl::internal::DoArithmaticAdd(cy, B.y);
				if constexpr (gtlc::has__z<T_COORD>)
					gtl::internal::DoArithmaticAdd(cz, B.z);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::coord T_COORD > TSize3& operator -= (T_COORD const& B) {
			if constexpr (gtlc::has__cxy<T_COORD>) {
				gtl::internal::DoArithmaticSub(cx, B.cx);
				gtl::internal::DoArithmaticSub(cy, B.cy);
				if constexpr (gtlc::has__cz<T_COORD>)
					gtl::internal::DoArithmaticSub(cz, B.cz);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				gtl::internal::DoArithmaticSub(cx, B.width);
				gtl::internal::DoArithmaticSub(cy, B.height);
				if constexpr (gtlc::has__depth<T_COORD>)
					gtl::internal::DoArithmaticSub(cz, B.depth);
			} else if constexpr (gtlc::has__xy<T_COORD>) {
				gtl::internal::DoArithmaticSub(cx, B.c);
				gtl::internal::DoArithmaticSub(cy, B.c);
				if constexpr (gtlc::has__z<T_COORD>)
					gtl::internal::DoArithmaticSub(cz, B.z);
			} else static_assert(false);
			return *this;
		};

		template < gtlc::coord T_COORD > [[nodiscard]] TSize3& operator + (T_COORD const& B)			{ return TSize3(*this) += B; }
		template < gtlc::coord T_COORD > [[nodiscard]] TSize3& operator - (T_COORD const& B)			{ return TSize3(*this) -= B; }

	public:
		template < typename Operation >
		inline TSize3& ForAll(Operation&& op) {
			op(cx); op(cy); op(cz);
			return *this;
		}

	public:
		template < gtlc::arithmetic T2 > TSize3& operator *= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticMul(v, d); }); };
		template < gtlc::arithmetic T2 > TSize3& operator /= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticDiv(v, d); }); };
		template < gtlc::arithmetic T2 > TSize3& operator += (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticAdd(v, d); }); };
		template < gtlc::arithmetic T2 > TSize3& operator -= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticSub(v, d); }); };

		template < gtlc::arithmetic T2 > [[nodiscard]] TSize3 operator * (T2 d) const				{ return TSize3(cx*d, cy*d, cz*d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TSize3 operator / (T2 d) const				{ return TSize3(cx/d, cy/d, cz/d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TSize3 operator + (T2 d) const				{ return TSize3(cx+d, cy+d, cz+d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TSize3 operator - (T2 d) const				{ return TSize3(cx-d, cy-d, cz-d); };

		template < gtlc::arithmetic T2 > friend [[nodiscard]] TSize3 operator * (T2 A, TSize3 const& B) { return TSize3(A*B.cx, A*B.cy, A*B.cz); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TSize3 operator / (T2 A, TSize3 const& B) { return TSize3(A/B.cx, A/B.cy, A/B.cz); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TSize3 operator + (T2 A, TSize3 const& B) { return TSize3(A+B.cx, A+B.cy, A+B.cz); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TSize3 operator - (T2 A, TSize3 const& B) { return TSize3(A-B.cx, A-B.cy, A-B.cz); }

		//
		template < gtlc::arithmetic T2 > 
		[[nodiscard]] TSize3& Set(T2 cx, T2 cy, T2 cz) {
			this->cx = RoundOrForward<T, T2>(cx);
			this->cy = RoundOrForward<T, T2>(cy);
			this->cz = RoundOrForward<T, T2>(cz);
			return *this;
		}
		void Set(T cx = {}, T cy = {}, T cz = {})									{ this->cx = cx; this->cy = cy; this->cz = cz; }
		void SetAll(T value = {})													{ cx = cy = cz = value; }
		void SetZero()																{ SetAll(); }
		void SetAsNAN() requires (std::is_floating_point_v<T>)						{ memset(&cx, 0xff, sizeof(*this)); };

		[[nodiscard]] bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (int i = 0; i < data().size(); i++)
				if ( std::isnan(data()[i]) || std::isfinite(data()[i]) )
					return false;
			return true;
		};

	// Archiving
		template < typename Archive > friend Archive& operator & (Archive& ar, TSize3 const& B) {
			return ar & B.cx & B.cy & B.cz;
		}
		template < typename JSON > friend void from_json(JSON const& j, TSize3& B) { B.cx = j["cx"]; B.cy = j["cy"]; B.cz = j["cz"]; }
		template < typename JSON > friend void to_json(JSON& j, TSize3 const& B) { j["cx"] = B.cx; j["cy"] = B.cy; j["cz"] = B.cz; }

	};

	//--------------------------------------------------------------------------------------------------------------------------------
	// Size 2 (cx, cy)
	//
	template < typename T >
	struct TSize2 {
	public:
		T cx{}, cy{};
		using coord_t = std::array<T, 2>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_t& member(int i) { return data()[i]; }
		constexpr value_t const& member(int i) const { return data()[i]; }

	public:
	// Constructors
		TSize2() = default;
		TSize2(TSize2 const&) = default;
		TSize2(T cx, T cy) : cx(cx), cy(cy) { }
		template < typename T2 >
		TSize2(T2 cx, T2 cy) : cx(RoundOrForward<T,T2>(cx)), cy(RoundOrForward<T,T2>(cy)) { }
		TSize2& operator = (TSize2 const&) = default;

		template < gtlc::generic_coord T_COORD > explicit TSize2(T_COORD const& B) { *this = B; };
		template < gtlc::generic_coord T_COORD > TSize2& operator = (T_COORD const& B) {
			if constexpr (gtlc::has__cxy<T_COORD>) {
				cx = RoundOrForward<T>(B.cx);
				cy = RoundOrForward<T>(B.cy);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				cx = RoundOrForward<T>(B.width);
				cy = RoundOrForward<T>(B.height);
			} else if constexpr (gtlc::has__xy<T_COORD>) {
				cx = RoundOrForward<T>(B.x);
				cy = RoundOrForward<T>(B.y);
			} else if constexpr (gtlc::rect<T_COORD>) {
				*this = (B.pt1 - B.pt0);
			} else static_assert(false);
			return *this;
		};

	// Type Casting to cv::Size_, SIZE, ...
		//template < typename T2, template <typename> typename T_COORD > requires gtlc::coord2<T_COORD<T2>>
		//operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2>(cx), RoundOrForward<T2>(cy) }; };
		////template < typename T2, template <typename> typename T_COORD > requires gtlc::coord3<T_COORD<T2>>
		////operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2>(cx), RoundOrForward<T2>(cy), {} }; };
		//template < gtlc::coord2 T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		////template < gtl::is__coord3 T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }
		////template < gtl::is__xy	T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		////template < gtl::is__xyz	T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }
		////template < gtl::is__cxy	T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		////template < gtl::is__cxyz	T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }
		////template < gtl::is__size2	T_COORD > operator T_COORD() const { return T_COORD{cx, cy}; }
		////template < gtl::is__size3	T_COORD > operator T_COORD() const { return T_COORD{cx, cy, {}}; }
		template < typename T_COORD > requires gtlc::wnd_point<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(cx), RoundOrForward<decltype(T_COORD::y)>(cy)}; }
		template < typename T_COORD > requires gtlc::wnd_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::cx)>(cx), RoundOrForward<decltype(T_COORD::cy)>(cy)}; }
		template < typename T_COORD > requires gtlc::cv_point2<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(cx), RoundOrForward<decltype(T_COORD::y)>(cy)}; }
		template < typename T_COORD > requires gtlc::cv_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::width)>(cx), RoundOrForward<decltype(T_COORD::height)>(cy)}; }


	// Compare
		[[nodiscard]] auto operator <=> (TSize2 const&) const = default;
		[[nodiscard]] auto operator <=> (T const& b) const { return *this <=> TSize2(b, b, b); }

		[[nodiscard]] auto CountNonZero() const														{ return std::count_if(data().begin(), data().end(), gtl::non_zero{}); }
		[[nodiscard]] auto CountIf(std::function<bool(T const&)> pred) const							{ return std::count_if(data().begin(), data().end(), pred); }

	// Numerical Operators
		TSize2& operator +=  (TSize2<T> const& B)										{ cx += B.cx; cy += B.cy; return *this; }
		TSize2& operator -=  (TSize2<T> const& B)										{ cx -= B.cx; cy -= B.cy; return *this; }
		[[nodiscard]] TSize2 operator + (TSize2<T> const& B) const									{ return TSize2(*this) += B; }
		[[nodiscard]] TSize2 operator - (TSize2<T> const& B) const									{ return TSize2(*this) -= B; }

		TSize2 operator - () const														{ return TSize2(-cx, -cy); }

		template < gtlc::coord T_COORD > TSize2& operator += (T_COORD const& B) {
			if constexpr (gtlc::has__cxy<T_COORD>) {
				cx += RoundOrForward<T>(B.cx);
				cy += RoundOrForward<T>(B.cy);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				cx += RoundOrForward<T>(B.width);
				cy += RoundOrForward<T>(B.height);
			} else if constexpr (gtlc::has__xy<T_COORD>) {
				cx += RoundOrForward<T>(B.x);
				cy += RoundOrForward<T>(B.y);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::coord T_COORD > TSize2& operator -= (T_COORD const& B) {
			if constexpr (gtlc::has__cxy<T_COORD>) {
				cx -= RoundOrForward<T>(B.cx);
				cy -= RoundOrForward<T>(B.cy);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				cx -= RoundOrForward<T>(B.width);
				cy -= RoundOrForward<T>(B.height);
			} else if constexpr (gtlc::has__xy<T_COORD>) {
				cx -= RoundOrForward<T>(B.x);
				cy -= RoundOrForward<T>(B.y);
			} else static_assert(false);
			return *this;
		};

		template < gtlc::coord T_COORD > [[nodiscard]] TSize2& operator + (T_COORD const& B)			{ return TSize2(*this) += B; }
		template < gtlc::coord T_COORD > [[nodiscard]] TSize2& operator - (T_COORD const& B)			{ return TSize2(*this) -= B; }

	public:
		template < typename Operation >
		inline TSize2& ForAll(Operation&& op) {
			op(cx); op(cy);
			return *this;
		}

	public:
		template < gtlc::arithmetic T2 > TSize2& operator *= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticMul(v, d); }); };
		template < gtlc::arithmetic T2 > TSize2& operator /= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticDiv(v, d); }); };
		template < gtlc::arithmetic T2 > TSize2& operator += (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticAdd(v, d); }); };
		template < gtlc::arithmetic T2 > TSize2& operator -= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticSub(v, d); }); };

		template < gtlc::arithmetic T2 > TSize2 operator * (T2 d) const				{ return TSize2(cx*d, cy*d); };
		template < gtlc::arithmetic T2 > TSize2 operator / (T2 d) const				{ return TSize2(cx/d, cy/d); };
		template < gtlc::arithmetic T2 > TSize2 operator + (T2 d) const				{ return TSize2(cx+d, cy+d); };
		template < gtlc::arithmetic T2 > TSize2 operator - (T2 d) const				{ return TSize2(cx-d, cy-d); };

		template < gtlc::arithmetic T2 > friend TSize2 operator * (T2 A, TSize2 const& B) { return TSize2(A*B.cx, A*B.cy); }
		template < gtlc::arithmetic T2 > friend TSize2 operator / (T2 A, TSize2 const& B) { return TSize2(A/B.cx, A/B.cy); }
		template < gtlc::arithmetic T2 > friend TSize2 operator + (T2 A, TSize2 const& B) { return TSize2(A+B.cx, A+B.cy); }
		template < gtlc::arithmetic T2 > friend TSize2 operator - (T2 A, TSize2 const& B) { return TSize2(A-B.cx, A-B.cy); }

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
		template < typename Archive > friend Archive& operator & (Archive& ar, TSize2 const& B) {
			return ar & B.cx & B.cy;
		}
		template < typename JSON > friend void from_json(JSON const& j, TSize2& B) { B.cx = j["cx"]; B.cy = j["cy"]; }
		template < typename JSON > friend void to_json(JSON& j, TSize2 const& B) { j["cx"] = B.cx; j["cy"] = B.cy; }

	};


}	// namespace gtl
