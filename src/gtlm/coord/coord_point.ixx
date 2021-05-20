//////////////////////////////////////////////////////////////////////
//
// point.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 전체 수정
//
//////////////////////////////////////////////////////////////////////

module;

#include <functional>
#include <compare>
#include <algorithm>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:coord_point;

import :concepts;
import :misc;
import :unit;

export namespace gtl {

#pragma pack(push, 1)

	//=============================================================================

	template < typename T > struct TPoint3;
	template < typename T > struct TPoint2;

	//--------------------------------------------------------------------------------------------------------------------------------
	// Point 3 (x, y, z)
	//
	template < typename T >
	struct TPoint3 {
	public:
		T x{}, y{}, z{};
		using coord_t = std::array<T, 3>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_t& member(int i) { return data()[i]; }
		constexpr value_t const& member(int i) const { return data()[i]; }

	public:
		// Constructors
		TPoint3() = default;
		TPoint3(TPoint3 const&) = default;
		TPoint3(T x, T y, T z) : x(x), y(y), z(z) { }
		template < typename T2 >
		TPoint3(T2 x, T2 y, T2 z) : x(RoundOrForward<T,T2>(x)), y(RoundOrForward<T,T2>(y)), z(RoundOrForward<T,T2>(z)) { }
		TPoint3& operator = (TPoint3 const&) = default;

		// Copy Constructors and Assign Operators
		template < gtlc::generic_coord T_COORD > explicit TPoint3(T_COORD const& B) { *this = B; };
		template < gtlc::generic_coord T_COORD > TPoint3& operator = (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				x = RoundOrForward<T>(B.x);
				y = RoundOrForward<T>(B.y);
				if constexpr (gtlc::has__z<T_COORD>)
					z = RoundOrForward<T>(B.z);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				x = RoundOrForward<T>(B.width);
				y = RoundOrForward<T>(B.height);
				if constexpr (gtlc::has__depth<T_COORD>)
					z = RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				x = RoundOrForward<T>(B.cx);
				y = RoundOrForward<T>(B.cy);
				if constexpr (gtlc::has__cz<T_COORD>)
					z = RoundOrForward<T>(B.cz);
			} else if constexpr (gtlc::wnd_rect<T_COORD>) {
				x = B.left;
				y = B.top;
				//z = {};
			} else 
				static_assert(false);
			return *this;
		};

		// Type Casting to cv::Size_, SIZE, ...
		template < typename T_COORD > requires gtlc::wnd_point<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(x), RoundOrForward<decltype(T_COORD::y)>(y)}; }
		template < typename T_COORD > requires gtlc::wnd_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::cx)>(x), RoundOrForward<decltype(T_COORD::cy)>(y)}; }
		template < typename T_COORD > requires gtlc::cv_point2<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(x), RoundOrForward<decltype(T_COORD::y)>(y)}; }
		template < typename T_COORD > requires gtlc::cv_point3<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(x), RoundOrForward<decltype(T_COORD::y)>(y), RoundOrForward<decltype(T_COORD::z)>(z)}; }
		template < typename T_COORD > requires gtlc::cv_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::width)>(x), RoundOrForward<decltype(T_COORD::height)>(y)}; }


		// Compare
		[[nodiscard]] auto operator <=> (TPoint3 const&) const = default;
		[[nodiscard]] auto operator <=> (T const& b) const { return *this <=> TPoint3(b, b, b); }

		[[nodiscard]] auto CountNonZero() const											{ return std::count_if(data().begin(), data().end(), gtl::non_zero{}); }
		[[nodiscard]] auto CountIf(std::function<bool(T const&)> pred) const			{ return std::count_if(data().begin(), data().end(), pred); }

		// Numerical Operators
		TPoint3& operator +=  (TPoint3<T> const& B)										{ x += B.x; y += B.y; z += B.z; return *this; }
		TPoint3& operator -=  (TPoint3<T> const& B)										{ x -= B.x; y -= B.y; z -= B.z; return *this; }
		[[nodiscard]] TPoint3 operator + (TPoint3<T> const& B) const					{ return TPoint3(x+B.x, y+B.y, z+B.z); }
		[[nodiscard]] TPoint3 operator - (TPoint3<T> const& B) const					{ return TPoint3(x-B.x, y-B.y, z-B.z); }

		[[nodiscard]] TPoint3 operator - () const										{ return TPoint3(-x, -y, -z); }

		template < gtlc::coord T_COORD > TPoint3& operator += (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				x += RoundOrForward<T>(B.x);
				y += RoundOrForward<T>(B.y);
				if constexpr (gtlc::has__z<T_COORD>)
					z += RoundOrForward<T>(B.z);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				x += RoundOrForward<T>(B.width);
				y += RoundOrForward<T>(B.height);
				if constexpr (gtlc::has__depth<T_COORD>)
					z += RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				x += RoundOrForward<T>(B.cx);
				y += RoundOrForward<T>(B.cy);
				if constexpr (gtlc::has__cz<T_COORD>)
					z += RoundOrForward<T>(B.cz);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::coord T_COORD > TPoint3& operator -= (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				x -= RoundOrForward<T>(B.x);
				y -= RoundOrForward<T>(B.y);
				if constexpr (gtlc::has__z<T_COORD>)
					z -= RoundOrForward<T>(B.z);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				x -= RoundOrForward<T>(B.width);
				y -= RoundOrForward<T>(B.height);
				if constexpr (gtlc::has__depth<T_COORD>)
					z -= RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				x -= RoundOrForward<T>(B.cx);
				y -= RoundOrForward<T>(B.cy);
				if constexpr (gtlc::has__cz<T_COORD>)
					z -= RoundOrForward<T>(B.cz);
			} else static_assert(false);
			return *this;
		};

		template < gtlc::coord T_COORD > [[nodiscard]] TPoint3& operator + (T_COORD const& B)			{ return TPoint3(*this) += B; }
		template < gtlc::coord T_COORD > [[nodiscard]] TPoint3& operator - (T_COORD const& B)			{ return TPoint3(*this) -= B; }

	public:
		template < typename Operation >
		inline TPoint3& ForAll(Operation&& op) {
			op(x); op(y); op(z);
			return *this;
		}

	public:
		template < gtlc::arithmetic T2 > TPoint3& operator *= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticMul(v, d); }); };
		template < gtlc::arithmetic T2 > TPoint3& operator /= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticDiv(v, d); }); };
		template < gtlc::arithmetic T2 > TPoint3& operator += (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticAdd(v, d); }); };
		template < gtlc::arithmetic T2 > TPoint3& operator -= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticSub(v, d); }); };

		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint3 operator * (T2 d) const				{ return TPoint3(x*d, y*d, z*d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint3 operator / (T2 d) const				{ return TPoint3(x/d, y/d, z/d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint3 operator + (T2 d) const				{ return TPoint3(x+d, y+d, z+d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint3 operator - (T2 d) const				{ return TPoint3(x-d, y-d, z-d); };

		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint3 operator * (T2 A, TPoint3 const& B) { return TPoint3(A*B.x, A*B.y, A*B.z); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint3 operator / (T2 A, TPoint3 const& B) { return TPoint3(A/B.x, A/B.y, A/B.z); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint3 operator + (T2 A, TPoint3 const& B) { return TPoint3(A+B.x, A+B.y, A+B.z); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint3 operator - (T2 A, TPoint3 const& B) { return TPoint3(A-B.x, A-B.y, A-B.z); }

		// Cross
		[[nodiscard]] TPoint3 operator * (TPoint3 const& B) const						{ return Cross(B); }

		//
		void Set(T x = {}, T y = {}, T z = {})											{ this->x = x; this->y = y; this->z = z; }
		void SetAll(T value = {})														{ x = y = z = value; }
		void SetZero()																	{ SetAll(); }
		[[nodiscard]] void SetAsNAN() requires (std::is_floating_point_v<T>) {
			memset(&x, 0xff, sizeof(*this));
		}

		[[nodiscard]] bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (int i = 0; i < data().size(); i++)
				if ( std::isnan(data()[i]) || std::isfinite(data()[i]) )
					return false;
			return true;
		}

		// Archiving
		template < typename Archive > friend Archive& operator & (Archive& ar, TPoint3 const& B) {
			return ar & B.x & B.y & B.z;
		}
		template < typename JSON > friend void from_json(JSON const& j, TPoint3& B) { B.x = j["x"]; B.y = j["y"]; B.z = j["z"]; }
		template < typename JSON > friend void to_json(JSON& j, TPoint3 const& B) { j["x"] = B.x; j["y"] = B.y; j["z"] = B.z; }

	public:
		// Math
		[[nodiscard]] double Distance(TPoint3 const& pt) const {
			double dSum = Square(x-pt.x)+Square(y-pt.y)+Square(z-pt.z);
			return std::sqrt(dSum);
		}
		[[nodiscard]] double GetLength() const {
			double dSum = Square(x)+Square(y)+Square(z);
			return std::sqrt(dSum);
		}
		[[nodiscard]] rad_t GetAngleXY() const { return rad_t::atan2(y, x); }
		[[nodiscard]] rad_t GetAngleYZ() const { return rad_t::atan2(z, y); }
		[[nodiscard]] rad_t GetAngleZX() const { return rad_t::atan2(x, z); }

		[[nodiscard]] TPoint3 GetNormalizedVector() const { return *this / GetLength(); }	// Length == 1.0
		bool Normalize() { *this /= GetLength(); return IsAllValid(); }

		[[nodiscard]] TPoint3 GetNormalVectorXY() const { return {y, -x, {}}; }//Cross(TPoint3(0, 0, 1)); }		// Perpendicular(Normal) Vector (XY-Plan)
		[[nodiscard]] TPoint3 GetNormalVectorYZ() const { return {{}, z, -y}; }//Cross(TPoint3(1, 0, 0)); }		// Perpendicular(Normal) Vector (YZ-Plan)
		[[nodiscard]] TPoint3 GetNormalVectorZX() const { return {-z, {}, x}; }//Cross(TPoint3(0, 1, 0)); }		// Perpendicular(Normal) Vector (ZX-Plan)

		[[nodiscard]] TPoint3 Cross(TPoint3 const& B) const {
			return TPoint3(y*B.z - z*B.y, z*B.x - x*B.z, x*B.y - y*B.x);
		}
		[[nodiscard]] T CrossZ(TPoint2<T> const& B) const {
			return x*B.y - y*B.x;
		}
		[[nodiscard]] T CrossZ(TPoint3 const& B) const {
			return x*B.y - y*B.x;
		}
		[[nodiscard]] T Dot(TPoint3 const& B) const {
			return x*B.x + y*B.y + z*B.z;
		}
		[[nodiscard]] TPoint3 Mul(TPoint3 const& B) const {
			return TPoint3(x*B.x, y*B.y, z*B.z);
		}

		[[nodiscard]] bool CheckMinMax(TPoint3& ptMin, TPoint3& ptMax) const {
			bool bModified = false;
			for (int i = 0; i < data().size(); i++) {
				if (ptMin.data()[i] > data()[i]) { ptMin.data()[i] = data()[i]; bModified = true; }
				if (ptMax.data()[i] < data()[i]) { ptMax.data()[i] = data()[i]; bModified = true; }
			}
			return bModified;
		}
	};


	//--------------------------------------------------------------------------------------------------------------------------------
	// Point 2 (x, y)
	//
	template < typename T >
	struct TPoint2 {
	public:
		T x{}, y{};
		using coord_t = std::array<T, 2>;
		using value_t = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_t& member(int i) { return data()[i]; }
		constexpr value_t const& member(int i) const { return data()[i]; }

	public:
		// Constructors
		TPoint2() = default;
		TPoint2(TPoint2 const&) = default;
		TPoint2(T x, T y) : x(x), y(y) { }
		template < typename T2 >
		TPoint2(T2 x, T2 y) : x(RoundOrForward<T,T2>(x)), y(RoundOrForward<T,T2>(y)) { }
		TPoint2& operator = (TPoint2 const&) = default;

		// Copy Constructors and Assign Operators
		template < gtlc::generic_coord T_COORD > explicit TPoint2(T_COORD const& B) { *this = B; };
		template < gtlc::generic_coord T_COORD > TPoint2& operator = (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				x = RoundOrForward<T>(B.x);
				y = RoundOrForward<T>(B.y);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				x = RoundOrForward<T>(B.width);
				y = RoundOrForward<T>(B.height);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				x = RoundOrForward<T>(B.cx);
				y = RoundOrForward<T>(B.cy);
			} else if constexpr (gtlc::wnd_rect<T_COORD>) {
				x = B.left;
				y = B.top;
			} else if constexpr (gtlc::wnd_rect<T_COORD>) {
				x = B.left;
				y = B.top;
			} else static_assert(false);
			return *this;
		};

		// Type Casting to cv::Size_, SIZE, ...
		template < typename T_COORD > requires gtlc::wnd_point<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(x), RoundOrForward<decltype(T_COORD::y)>(y)}; }
		template < typename T_COORD > requires gtlc::wnd_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::cx)>(x), RoundOrForward<decltype(T_COORD::cy)>(y)}; }
		template < typename T_COORD > requires gtlc::cv_point2<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::x)>(x), RoundOrForward<decltype(T_COORD::y)>(y)}; }
		template < typename T_COORD > requires gtlc::cv_size<T_COORD>
		operator T_COORD const () const { return T_COORD{RoundOrForward<decltype(T_COORD::width)>(x), RoundOrForward<decltype(T_COORD::height)>(y)}; }

		// Compare
		[[nodiscard]] auto operator <=> (TPoint2 const&) const = default;
		[[nodiscard]] auto operator <=> (T const& b) const { return *this <=> TPoint2(b, b); }

		[[nodiscard]] auto CountNonZero() const														{ return std::count_if(data().begin(), data().end(), gtl::non_zero{}); }
		[[nodiscard]] auto CountIf(std::function<bool(T const&)> pred) const							{ return std::count_if(data().begin(), data().end(), pred); }

		// Numerical Operators
		TPoint2& operator +=  (TPoint2<T> const& B)										{ x += B.x; y += B.y; return *this; }
		TPoint2& operator -=  (TPoint2<T> const& B)										{ x -= B.x; y -= B.y; return *this; }
		[[nodiscard]] TPoint2 operator + (TPoint2<T> const& B) const									{ return TPoint2(x+B.x, y+B.y); }
		[[nodiscard]] TPoint2 operator - (TPoint2<T> const& B) const									{ return TPoint2(x-B.x, y-B.y); }

		[[nodiscard]] TPoint2 operator - () const														{ return TPoint2(-x, -y); }

		template < gtlc::coord T_COORD > TPoint2& operator += (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				x += RoundOrForward<T>(B.x);
				y += RoundOrForward<T>(B.y);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				x += RoundOrForward<T>(B.width);
				y += RoundOrForward<T>(B.height);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				x += RoundOrForward<T>(B.cx);
				y += RoundOrForward<T>(B.cy);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::coord T_COORD > TPoint2& operator -= (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				x -= RoundOrForward<T>(B.x);
				y -= RoundOrForward<T>(B.y);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				x -= RoundOrForward<T>(B.width);
				y -= RoundOrForward<T>(B.height);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				x -= RoundOrForward<T>(B.cx);
				y -= RoundOrForward<T>(B.cy);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::coord T_COORD > TPoint2& operator + (T_COORD const& B)	{ return TPoint2(*this) += B; };
		template < gtlc::coord T_COORD > TPoint2& operator - (T_COORD const& B)	{ return TPoint2(*this) -= B; };

	public:
		template < typename Operation >
		inline TPoint2& ForAll(Operation&& op) {
			op(x); op(y);
			return *this;
		}

	public:
		template < gtlc::arithmetic T2 > TPoint2& operator *= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticMul(v, d); }); };
		template < gtlc::arithmetic T2 > TPoint2& operator /= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticDiv(v, d); }); };
		template < gtlc::arithmetic T2 > TPoint2& operator += (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticAdd(v, d); }); };
		template < gtlc::arithmetic T2 > TPoint2& operator -= (T2 d) { return ForAll([d](T& v) { gtl::internal::DoArithmaticSub(v, d); }); };

		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint2 operator * (T2 d) const				{ return TPoint2(x*d, y*d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint2 operator / (T2 d) const				{ return TPoint2(x/d, y/d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint2 operator + (T2 d) const				{ return TPoint2(x+d, y+d); };
		template < gtlc::arithmetic T2 > [[nodiscard]] TPoint2 operator - (T2 d) const				{ return TPoint2(x-d, y-d); };

		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint2 operator * (T2 A, TPoint2 const& B) { return TPoint2(A*B.x, A*B.y); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint2 operator / (T2 A, TPoint2 const& B) { return TPoint2(A/B.x, A/B.y); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint2 operator + (T2 A, TPoint2 const& B) { return TPoint2(A+B.x, A+B.y); }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] TPoint2 operator - (T2 A, TPoint2 const& B) { return TPoint2(A-B.x, A-B.y); }

		// Cross
		[[nodiscard]] TPoint2 operator * (TPoint2 const& B) const										{ return Cross(B); }

		//
		void Set(T x = {}, T y = {})													{ this->x = x; this->y = y; }
		void SetAll(T value = {})														{ x = y = value; }
		void SetZero()																	{ SetAll(); }
		void SetAsNAN() requires (std::is_floating_point_v<T>) {
			memset(&x, 0xff, sizeof(*this));
		};

		[[nodiscard]] bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (int i = 0; i < data().size(); i++)
				if ( std::isnan(data()[i]) || std::isfinite(data()[i]) )
					return false;
			return true;
		};

		// Archiving
		template < typename Archive > friend Archive& operator & (Archive& ar, TPoint2 const& B) {
			return ar & B.x & B.y;
		}
		template < typename JSON > friend void from_json(JSON const& j, TPoint2& B) { B.x = j["x"]; B.y = j["y"]; }
		template < typename JSON > friend void to_json(JSON& j, TPoint2 const& B) { j["x"] = B.x; j["y"] = B.y; }

	public:
		// Math
		[[nodiscard]] double Distance(TPoint2 const& pt) const {
			double dSum = Square(x-pt.x)+Square(y-pt.y);
			return std::sqrt(dSum);
		}
		[[nodiscard]] double GetLength() const {
			double dSum = Square(x)+Square(y);
			return std::sqrt(dSum);
		}
		[[nodiscard]] rad_t GetAngleXY() const { return rad_t::atan2(y, x); }
		//rad_t GetAngleYZ() const { return rad_t::atan2(z, y); }
		//rad_t GetAngleZX() const { return rad_t::atan2(x, z); }

		[[nodiscard]] TPoint2 GetNormalizedVector() const { return *this / GetLength(); }	// Length == 1.0
		bool Normalize() { *this /= GetLength(); return IsAllValid(); }

		[[nodiscard]] TPoint2 GetNormalVectorXY() const { return { y, -x }; }//Cross(TPoint2(0, 0, 1)); }		// Perpendicular(Normal) Vector (XY-Plan)
		//TPoint2 GetNormalVectorYZ() const { return {{}, z, -y}; }//Cross(TPoint2(1, 0, 0)); }		// Perpendicular(Normal) Vector (YZ-Plan)
		//TPoint2 GetNormalVectorZX() const { return {-z, {}, x}; }//Cross(TPoint2(0, 1, 0)); }		// Perpendicular(Normal) Vector (ZX-Plan)

		[[nodiscard]] TPoint3<T> Cross(TPoint2 const& B) const {
			return { {}, {}, x*B.y - y*B.x};
		}
		[[nodiscard]] TPoint3<T> Cross(TPoint3<T> const& B) const {
			return TPoint2(y*B.z, -x*B.z, x*B.y - y*B.x);
		}
		[[nodiscard]] T CrossZ(TPoint3<T> const& B) const {
			return x*B.y - y*B.x;
		}
		[[nodiscard]] T CrossZ(TPoint2 const& B) const {
			return x*B.y - y*B.x;
		}
		[[nodiscard]] T Dot(TPoint2 const& B) const {
			return x*B.x + y*B.y;
		}
		[[nodiscard]] TPoint2 Mul(TPoint2 const& B) const {
			return TPoint2(x*B.x, y*B.y);
		}

		bool CheckMinMax(TPoint2& ptMin, TPoint2& ptMax) const {
			bool bModified = false;
			for (int i = 0; i < data().size(); i++) {
				if (ptMin.data()[i] > data()[i]) { ptMin.data()[i] = data()[i]; bModified = true; }
				if (ptMax.data()[i] < data()[i]) { ptMax.data()[i] = data()[i]; bModified = true; }
			}
			return bModified;
		}
	};


#pragma pack(pop)

}	// namespace gtl
