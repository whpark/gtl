#pragma once

//////////////////////////////////////////////////////////////////////
//
// point.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 전체 수정
//
//////////////////////////////////////////////////////////////////////

#include "gtl/misc.h"
#include "gtl/concepts.h"

namespace gtl {

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
		constexpr const coord_t& data() const { return std::bit_cast<coord_t const&>(*this); }

	public:
		// Constructors
		TPoint3() = default;
		TPoint3(const TPoint3&) = default;
		TPoint3(T x, T y, T z) : x(x), y(y), z(z) { }
		template < typename T2 >
		TPoint3(T2 x, T2 y, T2 z) : x(RoundOrForward<T,T2>(x)), y(RoundOrForward<T,T2>(y)), z(RoundOrForward<T,T2>(z)) { }
		TPoint3& operator = (const TPoint3&) = default;

		// Copy Constructors and Assign Operators
		template < gtlc::is__coord T_COORD > explicit TPoint3(const T_COORD& B) { *this = B; };
		template < gtlc::is__coord T_COORD > TPoint3& operator = (const T_COORD& B) {
			if constexpr (has__xy<T_COORD>) {
				x = RoundOrForward<T>(B.x);
				y = RoundOrForward<T>(B.y);
				if constexpr (has__z<T_COORD>)
					z = RoundOrForward<T>(B.z);
			} else if constexpr (has__size2<T_COORD>) {
				x = RoundOrForward<T>(B.width);
				y = RoundOrForward<T>(B.height);
				if constexpr (has__depth<T_COORD>)
					z = RoundOrForward<T>(B.depth);
			} else if constexpr (has__cxy<T_COORD>) {
				x = RoundOrForward<T>(B.cx);
				y = RoundOrForward<T>(B.cy);
				if constexpr (has__cz<T_COORD>)
					z = RoundOrForward<T>(B.cz);
			} else static_assert(false);
			return *this;
		};

		// Rect
		template < gtlc::is__rect T_COORD > explicit TPoint3(const T_COORD& B) { *this = B; }
		template < gtlc::is__rect T_COORD > TPoint3& operator = (const T_COORD& B) {
			*this = B.pt0;
			return *this;
		};

		// MFC
		explicit TPoint3(const RECT& B) { *this = B; }
		TPoint3& operator = (const RECT& B) {
			x = B.left;
			y = B.top;
			return *this;
		}

		// Type Casting to cv::Size_, SIZE, ...
		template < typename T2, template <typename> typename T_COORD > requires gtlc::is__coord2<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(x), RoundOrForward<T2, T>(y) }; };
		template < typename T2, template <typename> typename T_COORD > requires gtlc::is__coord3<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(x), RoundOrForward<T2, T>(y), RoundOrForward<T2, T>(z) }; };
		template < gtlc::is__coord2 T_COORD > operator T_COORD() const { return T_COORD{x, y}; }
		template < gtlc::is__coord3 T_COORD > operator T_COORD() const { return T_COORD{x, y, z}; }


		// Compare
		auto operator <=> (const TPoint3&) const = default;
		auto operator <=> (const T& b) const { return *this <=> (TPoint3(b, b, b)); }

		auto CountNonZero() const														{ return std::count_if(data().begin(), data().end(), gtlc::non_zero{}); }
		auto CountIf(std::function<bool(const T&)> pred) const							{ return std::count_if(data().begin(), data().end(), pred); }

		// Numerical Operators
		TPoint3& operator +=  (const TPoint3<T>& B)										{ x += B.x; y += B.y; z += B.z; return *this; }
		TPoint3& operator -=  (const TPoint3<T>& B)										{ x -= B.x; y -= B.y; z -= B.z; return *this; }
		TPoint3 operator + (const TPoint3<T>& B) const									{ return TPoint3(x+B.x, y+B.y, z+B.z); }
		TPoint3 operator - (const TPoint3<T>& B) const									{ return TPoint3(x-B.x, y-B.y, z-B.z); }

		TPoint3 operator - () const														{ return TPoint3(-x, -y, -z); }

		template < is__coord T_COORD > TPoint3& operator += (const T_COORD& B) {
			if constexpr (has__xy<T_COORD>) {
				x += RoundOrForward<T>(B.x);
				y += RoundOrForward<T>(B.y);
				if constexpr (has__z<T_COORD>)
					z += RoundOrForward<T>(B.z);
			} else if constexpr (has__size2<T_COORD>) {
				x += RoundOrForward<T>(B.width);
				y += RoundOrForward<T>(B.height);
				if constexpr (has__depth<T_COORD>)
					z += RoundOrForward<T>(B.depth);
			} else if constexpr (has__cxy<T_COORD>) {
				x += RoundOrForward<T>(B.cx);
				y += RoundOrForward<T>(B.cy);
				if constexpr (has__cz<T_COORD>)
					z += RoundOrForward<T>(B.cz);
			} else static_assert(false);
			return *this;
		};
		template < is__coord T_COORD > TPoint3& operator -= (const T_COORD& B) {
			if constexpr (has__xy<T_COORD>) {
				x -= RoundOrForward<T>(B.x);
				y -= RoundOrForward<T>(B.y);
				if constexpr (has__z<T_COORD>)
					z -= RoundOrForward<T>(B.z);
			} else if constexpr (has__size2<T_COORD>) {
				x -= RoundOrForward<T>(B.width);
				y -= RoundOrForward<T>(B.height);
				if constexpr (has__depth<T_COORD>)
					z -= RoundOrForward<T>(B.depth);
			} else if constexpr (has__cxy<T_COORD>) {
				x -= RoundOrForward<T>(B.cx);
				y -= RoundOrForward<T>(B.cy);
				if constexpr (has__cz<T_COORD>)
					z -= RoundOrForward<T>(B.cz);
			} else static_assert(false);
			return *this;
		};

		template < is__coord T_COORD > TPoint3& operator + (const T_COORD& B)			{ return TPoint3(*this) += B; }
		template < is__coord T_COORD > TPoint3& operator - (const T_COORD& B)			{ return TPoint3(*this) -= B; }

	public:
		template < class Operation >
		inline TPoint3& ForAll(Operation&& op) {
			op(x); op(y); op(z);
			return *this;
		}

	public:
		template < gtlc::is__arithmatic T2 > TPoint3& operator *= (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticMul(v, d); }); };
		template < gtlc::is__arithmatic T2 > TPoint3& operator /= (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticDiv(v, d); }); };
		template < gtlc::is__arithmatic T2 > TPoint3& operator += (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticAdd(v, d); }); };
		template < gtlc::is__arithmatic T2 > TPoint3& operator -= (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticSub(v, d); }); };

		template < gtlc::is__arithmatic T2 > TPoint3 operator * (T2 d) const				{ return TPoint3(x*d, y*d, z*d); };
		template < gtlc::is__arithmatic T2 > TPoint3 operator / (T2 d) const				{ return TPoint3(x/d, y/d, z/d); };
		template < gtlc::is__arithmatic T2 > TPoint3 operator + (T2 d) const				{ return TPoint3(x+d, y+d, z+d); };
		template < gtlc::is__arithmatic T2 > TPoint3 operator - (T2 d) const				{ return TPoint3(x-d, y-d, z-d); };

		template < gtlc::is__arithmatic T2 > friend TPoint3 operator * (T2 A, const TPoint3& B) { return TPoint3(A*B.x, A*B.y, A*B.z); }
		template < gtlc::is__arithmatic T2 > friend TPoint3 operator / (T2 A, const TPoint3& B) { return TPoint3(A/B.x, A/B.y, A/B.z); }
		template < gtlc::is__arithmatic T2 > friend TPoint3 operator + (T2 A, const TPoint3& B) { return TPoint3(A+B.x, A+B.y, A+B.z); }
		template < gtlc::is__arithmatic T2 > friend TPoint3 operator - (T2 A, const TPoint3& B) { return TPoint3(A-B.x, A-B.y, A-B.z); }

		// Cross
		TPoint3 operator * (const TPoint3& B) const										{ return Cross(B); }

		//
		void Set(T x = {}, T y = {}, T z = {})											{ this->x = x; this->y = y; this->z = z; }
		void SetAll(T value = {})														{ x = y = z = value; }
		void SetZero()																	{ SetAll(); }
		void SetAsNAN() requires (std::is_floating_point_v<T>) {
			memset(&x, 0xff, sizeof(*this));
		}

		bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (int i = 0; i < data().size(); i++)
				if ( std::isnan(data()[i]) || std::isfinite(data()[i]) )
					return false;
			return true;
		}

		// Archiving
		template < class Archive > friend Archive& operator & (Archive& ar, const TPoint3& B) {
			return ar & B.x & B.y & B.z;
		}

	public:
		// Math
		double Distance(const TPoint3& pt) const {
			double dSum = Square(x-pt.x)+Square(y-pt.y)+Square(z-pt.z);
			return std::sqrt(dSum);
		}
		double GetLength() const {
			double dSum = Square(x)+Square(y)+Square(z);
			return std::sqrt(dSum);
		}
		angle_rad_t GetAngleXY() const { return rad_t::atan2(y, x); }
		angle_rad_t GetAngleYZ() const { return rad_t::atan2(z, y); }
		angle_rad_t GetAngleZX() const { return rad_t::atan2(x, z); }

		[[nodiscard]] TPoint3 GetNormalizedVector() const { return *this / GetLength(); }	// Length == 1.0
		bool Normalize() { *this /= GetLength(); return IsAllValid(); }

		TPoint3 GetNormalVectorXY() const { return {y, -x, {}}; }//Cross(TPoint3(0, 0, 1)); }		// Perpendicular(Normal) Vector (XY-Plan)
		TPoint3 GetNormalVectorYZ() const { return {{}, z, -y}; }//Cross(TPoint3(1, 0, 0)); }		// Perpendicular(Normal) Vector (YZ-Plan)
		TPoint3 GetNormalVectorZX() const { return {-z, {}, x}; }//Cross(TPoint3(0, 1, 0)); }		// Perpendicular(Normal) Vector (ZX-Plan)

		[[nodiscard]] TPoint3 Cross(const TPoint3& B) const {
			return TPoint3(y*B.z - z*B.y, z*B.x - x*B.z, x*B.y - y*B.x);
		}
		T CrossZ(const TPoint2<T>& B) const {
			return x*B.y - y*B.x;
		}
		T CrossZ(const TPoint3& B) const {
			return x*B.y - y*B.x;
		}
		T Dot(const TPoint3& B) const {
			return x*B.x + y*B.y + z*B.z;
		}
		[[nodiscard]] TPoint3 Mul(const TPoint3& B) const {
			return TPoint3(x*B.x, y*B.y, z*B.z);
		}

		bool CheckMinMax(TPoint3& ptMin, TPoint3& ptMax) const {
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
		constexpr const coord_t& data() const { return std::bit_cast<coord_t const&>(*this); }

	public:
		// Constructors
		TPoint2() = default;
		TPoint2(const TPoint2&) = default;
		TPoint2(T x, T y) : x(x), y(y) { }
		template < typename T2 >
		TPoint2(T2 x, T2 y) : x(RoundOrForward<T,T2>(x)), y(RoundOrForward<T,T2>(y)) { }
		TPoint2& operator = (const TPoint2&) = default;

		// Copy Constructors and Assign Operators
		template < gtlc::is__coord T_COORD > explicit TPoint2(const T_COORD& B) { *this = B; };
		template < gtlc::is__coord T_COORD > TPoint2& operator = (const T_COORD& B) {
			if constexpr (has__xy<T_COORD>) {
				x = RoundOrForward<T>(B.x);
				y = RoundOrForward<T>(B.y);
			} else if constexpr (has__size2<T_COORD>) {
				x = RoundOrForward<T>(B.width);
				y = RoundOrForward<T>(B.height);
			} else if constexpr (has__cxy<T_COORD>) {
				x = RoundOrForward<T>(B.cx);
				y = RoundOrForward<T>(B.cy);
			} else static_assert(false);
			return *this;
		};

		// Rect
		template < gtlc::is__rect T_COORD > explicit TPoint2(const T_COORD& B) { *this = B; };
		template < gtlc::is__rect T_COORD > TPoint2& operator = (const T_COORD& B) {
			*this = B.pt0;
			return *this;
		};

		// MFC
		explicit TPoint2(const RECT& B) { *this = B; }
		TPoint2& operator = (const RECT& B) {
			x = B.left;
			y = B.top;
			return *this;
		}

		// Type Casting to cv::Size_, SIZE, ...
		template < typename T2, template <typename> typename T_COORD > requires gtlc::is__coord2<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(x), RoundOrForward<T2, T>(y) }; };
		template < typename T2, template <typename> typename T_COORD > requires gtlc::is__coord3<T_COORD<T2>>
		operator T_COORD<T2>() const { return T_COORD<T2>{ RoundOrForward<T2, T>(x), RoundOrForward<T2, T>(y), {} }; };
		template < gtlc::is__coord2 T_COORD > operator T_COORD() const { return T_COORD{x, y}; };
		template < gtlc::is__coord3 T_COORD > operator T_COORD() const { return T_COORD{x, y, {}}; };


		// Compare
		auto operator <=> (const TPoint2&) const = default;
		auto operator <=> (const T& b) const { return *this <=> TPoint2(b, b); }

		auto CountNonZero() const														{ return std::count_if(data().begin(), data().end(), gtlc::non_zero{}); }
		auto CountIf(std::function<bool(const T&)> pred) const							{ return std::count_if(data().begin(), data().end(), pred); }

		// Numerical Operators
		TPoint2& operator +=  (const TPoint2<T>& B)										{ x += B.x; y += B.y; return *this; }
		TPoint2& operator -=  (const TPoint2<T>& B)										{ x -= B.x; y -= B.y; return *this; }
		TPoint2 operator + (const TPoint2<T>& B) const									{ return TPoint2(x+B.x, y+B.y); }
		TPoint2 operator - (const TPoint2<T>& B) const									{ return TPoint2(x-B.x, y-B.y); }

		TPoint2 operator - () const														{ return TPoint2(-x, -y); }

		template < gtlc::is__coord T_COORD > TPoint2& operator += (const T_COORD& B) {
			if constexpr (has__xy<T_COORD>) {
				x += RoundOrForward<T>(B.x);
				y += RoundOrForward<T>(B.y);
			} else if constexpr (has__size2<T_COORD>) {
				x += RoundOrForward<T>(B.width);
				y += RoundOrForward<T>(B.height);
			} else if constexpr (has__cxy<T_COORD>) {
				x += RoundOrForward<T>(B.cx);
				y += RoundOrForward<T>(B.cy);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::is__coord T_COORD > TPoint2& operator -= (const T_COORD& B) {
			if constexpr (has__xy<T_COORD>) {
				x -= RoundOrForward<T>(B.x);
				y -= RoundOrForward<T>(B.y);
			} else if constexpr (has__size2<T_COORD>) {
				x -= RoundOrForward<T>(B.width);
				y -= RoundOrForward<T>(B.height);
			} else if constexpr (has__cxy<T_COORD>) {
				x -= RoundOrForward<T>(B.cx);
				y -= RoundOrForward<T>(B.cy);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::is__coord T_COORD > TPoint2& operator + (const T_COORD& B)	{ return TPoint2(*this) += B; };
		template < gtlc::is__coord T_COORD > TPoint2& operator - (const T_COORD& B)	{ return TPoint2(*this) -= B; };

	public:
		template < class Operation >
		inline TPoint2& ForAll(Operation&& op) {
			op(x); op(y);
			return *this;
		}

	public:
		template < gtlc::is__arithmatic T2 > TPoint2& operator *= (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticMul(v, d); }); };
		template < gtlc::is__arithmatic T2 > TPoint2& operator /= (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticDiv(v, d); }); };
		template < gtlc::is__arithmatic T2 > TPoint2& operator += (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticAdd(v, d); }); };
		template < gtlc::is__arithmatic T2 > TPoint2& operator -= (T2 d) { return ForAll([d](T& v) { gtlc::internal::DoArithmaticSub(v, d); }); };

		template < gtlc::is__arithmatic T2 > TPoint2 operator * (T2 d) const				{ return TPoint2(x*d, y*d); };
		template < gtlc::is__arithmatic T2 > TPoint2 operator / (T2 d) const				{ return TPoint2(x/d, y/d); };
		template < gtlc::is__arithmatic T2 > TPoint2 operator + (T2 d) const				{ return TPoint2(x+d, y+d); };
		template < gtlc::is__arithmatic T2 > TPoint2 operator - (T2 d) const				{ return TPoint2(x-d, y-d); };

		template < gtlc::is__arithmatic T2 > friend TPoint2 operator * (T2 A, const TPoint2& B) { return TPoint2(A*B.x, A*B.y); }
		template < gtlc::is__arithmatic T2 > friend TPoint2 operator / (T2 A, const TPoint2& B) { return TPoint2(A/B.x, A/B.y); }
		template < gtlc::is__arithmatic T2 > friend TPoint2 operator + (T2 A, const TPoint2& B) { return TPoint2(A+B.x, A+B.y); }
		template < gtlc::is__arithmatic T2 > friend TPoint2 operator - (T2 A, const TPoint2& B) { return TPoint2(A-B.x, A-B.y); }

		// Cross
		TPoint2 operator * (const TPoint2& B) const										{ return Cross(B); }

		//
		void Set(T x = {}, T y = {}, T z = {})											{ this->x = x; this->y = y; }
		void SetAll(T value = {})														{ x = y = value; }
		void SetZero()																	{ SetAll(); }
		void SetAsNAN() requires (std::is_floating_point_v<T>) {
			memset(&x, 0xff, sizeof(*this));
		};

		bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (int i = 0; i < data().size(); i++)
				if ( std::isnan(data()[i]) || std::isfinite(data()[i]) )
					return false;
			return true;
		};

		// Archiving
		template < class Archive > friend Archive& operator & (Archive& ar, const TPoint2& B) {
			return ar & B.x & B.y;
		}

	public:
		// Math
		double Distance(const TPoint2& pt) const {
			double dSum = Square(x-pt.x)+Square(y-pt.y);
			return std::sqrt(dSum);
		}
		double GetLength() const {
			double dSum = Square(x)+Square(y);
			return std::sqrt(dSum);
		}
		angle_rad_t GetAngleXY() const { return rad_t::atan2(y, x); }
		//angle_rad_t GetAngleYZ() const { return rad_t::atan2(z, y); }
		//angle_rad_t GetAngleZX() const { return rad_t::atan2(x, z); }

		[[nodiscard]] TPoint2 GetNormalizedVector() const { return *this / GetLength(); }	// Length == 1.0
		bool Normalize() { *this /= GetLength(); return IsAllValid(); }

		TPoint2 GetNormalVectorXY() const { return { y, -x }; }//Cross(TPoint2(0, 0, 1)); }		// Perpendicular(Normal) Vector (XY-Plan)
		//TPoint2 GetNormalVectorYZ() const { return {{}, z, -y}; }//Cross(TPoint2(1, 0, 0)); }		// Perpendicular(Normal) Vector (YZ-Plan)
		//TPoint2 GetNormalVectorZX() const { return {-z, {}, x}; }//Cross(TPoint2(0, 1, 0)); }		// Perpendicular(Normal) Vector (ZX-Plan)

		[[nodiscard]] TPoint3<T> Cross(const TPoint2& B) const {
			return { {}, {}, x*B.y - y*B.x};
		}
		[[nodiscard]] TPoint3<T> Cross(const TPoint3<T>& B) const {
			return TPoint2(y*B.z, -x*B.z, x*B.y - y*B.x);
		}
		T CrossZ(const TPoint3<T>& B) const {
			return x*B.y - y*B.x;
		}
		T CrossZ(const TPoint2& B) const {
			return x*B.y - y*B.x;
		}
		T Dot(const TPoint2& B) const {
			return x*B.x + y*B.y;
		}
		[[nodiscard]] TPoint2 Mul(const TPoint2& B) const {
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
