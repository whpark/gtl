#pragma once

//////////////////////////////////////////////////////////////////////
//
// point.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 전체 수정
//
//////////////////////////////////////////////////////////////////////

#include <bit>

#include "gtl/concepts.h"
#include "gtl/misc.h"
#include "gtl/unit.h"

#include "opencv2/opencv.hpp"

namespace gtl {
#pragma pack(push, 1)

	//=============================================================================

#if 1

	template < typename T, int dim > struct TPointT;

	template < typename T > struct TPOINT4 { T x{}, y{}, z{}, w{};	auto operator <=> (TPOINT4 const&) const = default; };
	template < typename T > struct TPOINT3 { T x{}, y{}, z{};		auto operator <=> (TPOINT3 const&) const = default; };
	template < typename T > struct TPOINT2 { T x{}, y{};			auto operator <=> (TPOINT2 const&) const = default; };

	template < typename T, int dim >
	using TPOINTT = std::conditional_t<dim == 2, TPOINT2<T>, std::conditional_t<dim == 3, TPOINT3<T>, TPOINT4<T> > >;

	//--------------------------------------------------------------------------------------------------------------------------------
	// General Point (x,y[,z[,w]])
	//
	template < typename T, int dim >
	struct TPointT : TPOINTT<T, dim> {
	public:

		//T x, y;
		//T z requires (dim >= 3);
		//T w requires (dim >= 4);
		using base_t = TPOINTT<T, dim>;
		using this_t = TPointT;
		using coord_t = std::array<T, dim>;
		using value_type = T;

		constexpr auto& arr() { return reinterpret_cast<coord_t&>(*this); }
		constexpr auto const& arr() const { return reinterpret_cast<coord_t const&>(*this); }

		constexpr auto* data() { return (reinterpret_cast<coord_t&>(*this)).data(); }
		constexpr auto const* data() const { return (reinterpret_cast<coord_t const&>(*this)).data(); }
		constexpr auto const size() const { return (reinterpret_cast<coord_t const&>(*this)).size(); }
		constexpr value_type& member(int i) { return data()[i]; }
		constexpr value_type const& member(int i) const { return data()[i]; }

		static_assert(2 <= dim and dim <= 4);
		//static_assert(sizeof(TPointT<T, dim>) == sizeof(coord_t));

	public:
		// Constructors
		constexpr TPointT() = default;
		constexpr TPointT(TPointT const&) = default;
		constexpr TPointT(TPointT &&) = default;
		constexpr TPointT(T x, T y) requires (dim == 2) : base_t{x, y} { }
		constexpr TPointT(T x, T y, T z={}) requires (dim == 3) : base_t{x, y, z} { }
		constexpr TPointT(T x, T y, T z={}, T w={}) requires (dim == 4) : base_t{x, y, z, w} { }
		template < typename T2 >
		constexpr TPointT(T2 x, T2 y) requires (dim == 2) : base_t{RoundOrForward<T,T2>(x), RoundOrForward<T,T2>(y)} { }
		template < typename T2 >
		constexpr TPointT(T2 x, T2 y, T2 z) requires (dim == 3) : base_t{RoundOrForward<T,T2>(x), RoundOrForward<T,T2>(y), RoundOrForward<T,T2>(z)} { }
		template < typename T2 >
		constexpr TPointT(T2 x, T2 y, T2 z, T w)  requires (dim == 4) : base_t{RoundOrForward<T,T2>(x), RoundOrForward<T,T2>(y), RoundOrForward<T,T2>(z), RoundOrForward<T,T2>(w) } { }
		TPointT& operator = (TPointT const&) = default;
		TPointT& operator = (TPointT &&) = default;

		// from vector
		explicit TPointT(std::vector<T> const& B) {
			*this = B;
		}
		TPointT& operator = (std::vector<T> const& B) {
			size_t n = std::min(size(), B.size());
			for (size_t i = 0; i < n; i++) {
				member((int)i) = B[i];
			}
			return *this;
		}

		// Copy Constructors and Assign Operators
		template < gtlc::generic_coord T_COORD > explicit TPointT(T_COORD const& B) { *this = B; };
		template < gtlc::generic_coord T_COORD > TPointT& operator = (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				this->x = RoundOrForward<T>(B.x);
				this->y = RoundOrForward<T>(B.y);
				if constexpr (dim >= 3 and gtlc::has__z<T_COORD>)
					this->z = RoundOrForward<T>(B.z);
				if constexpr (dim >= 4 and gtlc::has__w<T_COORD>)
					this->w = RoundOrForward<T>(B.w);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				this->x = RoundOrForward<T>(B.width);
				this->y = RoundOrForward<T>(B.height);
				if constexpr (dim >= 3 and gtlc::has__depth<T_COORD>)
					this->z = RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				this->x = RoundOrForward<T>(B.cx);
				this->y = RoundOrForward<T>(B.cy);
				if constexpr (dim >= 3 and gtlc::has__cz<T_COORD>)
					this->z = RoundOrForward<T>(B.cz);
			} else if constexpr (dim >= 3 and gtlc::wnd_rect<T_COORD>) {
				this->x = B.left;
				this->y = B.top;
				//z = {};
			} else 
				static_assert(false);
			return *this;
		};


		constexpr static inline this_t All(T v) {
			if constexpr (dim == 2)
				return this_t(v, v);
			else if constexpr (dim == 3)
				return this_t(v, v, v);
			else if constexpr (dim == 4)
				return this_t(v, v, v, v);
		}


		// Type Casting to cv::Size_, SIZE, ...
		template < gtlc::generic_coord T_COORD >
		operator T_COORD const () const {
		#define GTL__TYPE_CAST_COORD_VAL4(tx, ty, tz, tw)\
			T_COORD{RoundOrForward<decltype(T_COORD::tx)>(this->x), RoundOrForward<decltype(T_COORD::ty)>(this->y), RoundOrForward<decltype(T_COORD::tz)>(this->z), RoundOrForward<decltype(T_COORD::tw)>(this->w)};
		#define GTL__TYPE_CAST_COORD_VAL3(tx, ty, tz)\
			T_COORD{RoundOrForward<decltype(T_COORD::tx)>(this->x), RoundOrForward<decltype(T_COORD::ty)>(this->y), RoundOrForward<decltype(T_COORD::tz)>(this->z)};
		#define GTL__TYPE_CAST_COORD_VAL2(tx, ty)\
			T_COORD{RoundOrForward<decltype(T_COORD::tx)>(this->x), RoundOrForward<decltype(T_COORD::ty)>(this->y)};

			if constexpr (false) {
			}
			else if constexpr ((dim >= 4) and gtlc::has__xyzw<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL4(x, y, z, w);
			}
			else if constexpr ((dim >= 3) and gtlc::has__xyz<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL3(x, y, z);
			}
			else if constexpr ((dim >= 3) and gtlc::has__cxyz<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL3(cx, cy, cz);
			}
			else if constexpr ((dim >= 3) and gtlc::has__size3<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL3(width, height, depth);
			}
			else if constexpr ((dim >= 2) and gtlc::has__xy<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL2(x, y);
			}
			else if constexpr ((dim >= 2) and gtlc::has__cxy<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL2(cx, cy);
			}
			else if constexpr ((dim >= 2) and gtlc::has__size2<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL2(width, height);
			}
			else {
				static_assert(false);
			}

		#undef GTL__TYPE_CAST_COORD_VAL2
		#undef GTL__TYPE_CAST_COORD_VAL3
		#undef GTL__TYPE_CAST_COORD_VAL4
		}


		// Compare
		[[nodiscard]] auto operator <=> (this_t const&) const = default;
		[[nodiscard]] auto operator <=> (T const& v) const								{ return *this <=> this_t::All(v); }

		[[nodiscard]] auto CountNonZero() const											{ return std::count_if(arr().begin(), arr().end(), gtl::non_zero{}); }
		[[nodiscard]] auto CountIf(std::function<bool(T const&)> pred) const			{ return std::count_if(arr().begin(), arr().end(), pred); }

		// Numerical Operators
		this_t& operator +=  (this_t const& B)											{ for (int i = 0; i < size(); i++) data()[i] += B.data()[i]; return *this; }
		this_t& operator -=  (this_t const& B)											{ for (int i = 0; i < size(); i++) data()[i] -= B.data()[i]; return *this; }
		[[nodiscard]] this_t operator + (this_t const& B) const							{ auto C(*this); return C += B; }
		[[nodiscard]] this_t operator - (this_t const& B) const							{ auto C(*this); return C -= B; }

		[[nodiscard]] this_t operator - () const										{ this_t C(*this); for (auto& v : C.arr()) v = -v; return C; }

		template < gtlc::generic_coord T_COORD > this_t& operator += (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				this->x += RoundOrForward<T>(B.x);
				this->y += RoundOrForward<T>(B.y);
				if constexpr ((dim >= 3) and gtlc::has__z<T_COORD>)
					this->z += RoundOrForward<T>(B.z);
				if constexpr ((dim >= 4) and gtlc::has__w<T_COORD>)
					this->w += RoundOrForward<T>(B.w);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				this->x += RoundOrForward<T>(B.width);
				this->y += RoundOrForward<T>(B.height);
				if constexpr ((dim >= 3) and gtlc::has__depth<T_COORD>)
					this->z += RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				this->x += RoundOrForward<T>(B.cx);
				this->y += RoundOrForward<T>(B.cy);
				if constexpr ((dim >= 3) and gtlc::has__cz<T_COORD>)
					this->z += RoundOrForward<T>(B.cz);
			} else static_assert(false);
			return *this;
		};
		template < gtlc::generic_coord T_COORD > this_t& operator -= (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				this->x -= RoundOrForward<T>(B.x);
				this->y -= RoundOrForward<T>(B.y);
				if constexpr ((dim >= 3) and gtlc::has__z<T_COORD>)
					this->z -= RoundOrForward<T>(B.z);
				if constexpr ((dim >= 4) and gtlc::has__w<T_COORD>)
					this->w -= RoundOrForward<T>(B.w);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				this->x -= RoundOrForward<T>(B.width);
				this->y -= RoundOrForward<T>(B.height);
				if constexpr ((dim >= 3) and gtlc::has__depth<T_COORD>)
					this->z -= RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				this->x -= RoundOrForward<T>(B.cx);
				this->y -= RoundOrForward<T>(B.cy);
				if constexpr ((dim >= 3) and gtlc::has__cz<T_COORD>)
					this->z -= RoundOrForward<T>(B.cz);
			} else static_assert(false);
			return *this;
		};

		template < gtlc::generic_coord T_COORD > [[nodiscard]] this_t operator + (T_COORD const& B)			{ return this_t(*this) += B; }
		template < gtlc::generic_coord T_COORD > [[nodiscard]] this_t operator - (T_COORD const& B)			{ return this_t(*this) -= B; }

	public:
		template < gtlc::arithmetic T2 > this_t& operator *= (T2 d)									{ for (auto& v : arr()) v = RoundOrForward<T>(v *= d); return *this; };
		template < gtlc::arithmetic T2 > this_t& operator /= (T2 d)									{ for (auto& v : arr()) v = RoundOrForward<T>(v /= d); return *this; };
		template < gtlc::arithmetic T2 > this_t& operator += (T2 d)									{ for (auto& v : arr()) v = RoundOrForward<T>(v += d); return *this; };
		template < gtlc::arithmetic T2 > this_t& operator -= (T2 d)									{ for (auto& v : arr()) v = RoundOrForward<T>(v -= d); return *this; };

		template < gtlc::arithmetic T2 > [[nodiscard]] this_t operator * (T2 d) const				{ auto C = *this; return C *= d; };
		template < gtlc::arithmetic T2 > [[nodiscard]] this_t operator / (T2 d) const				{ auto C = *this; return C /= d; };
		template < gtlc::arithmetic T2 > [[nodiscard]] this_t operator + (T2 d) const				{ auto C = *this; return C += d; };
		template < gtlc::arithmetic T2 > [[nodiscard]] this_t operator - (T2 d) const				{ auto C = *this; return C -= d; };

		template < gtlc::arithmetic T2 > friend [[nodiscard]] this_t operator * (T2 A, this_t B)	{ for (auto& v: B.arr()) v = RoundOrForward<T>(A*v); return B; }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] this_t operator / (T2 A, this_t B)	{ for (auto& v: B.arr()) v = RoundOrForward<T>(A/v); return B; }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] this_t operator + (T2 A, this_t B)	{ for (auto& v: B.arr()) v = RoundOrForward<T>(A+v); return B; }
		template < gtlc::arithmetic T2 > friend [[nodiscard]] this_t operator - (T2 A, this_t B)	{ for (auto& v: B.arr()) v = RoundOrForward<T>(A-v); return B; }

		//
		void Set(T x = {}, T y = {})						requires (dim == 2) { this->x = x; this->y = y; }
		void Set(T x = {}, T y = {}, T z = {})				requires (dim == 3) { this->x = x; this->y = y; this->z = z; }
		void Set(T x = {}, T y = {}, T z = {}, T w = {})	requires (dim == 4) { this->x = x; this->y = y; this->z = z; this->w = w; }
		void SetAll(T value = {})							requires (dim == 2) { this->x = this->y = value; }
		void SetAll(T value = {})							requires (dim == 3) { this->x = this->y = this->z = value; }
		void SetAll(T value = {})							requires (dim == 4) { this->x = this->y = this->z = this->w = value; }
		void SetZero()															{ SetAll(); }
		[[nodiscard]] void SetAsNAN() requires (std::is_floating_point_v<T>) {
			if constexpr (std::is_same_v<T, float>) {
				SetAll(std::nanf(""));
			} else if constexpr (std::is_same_v<T, double>) {
				SetAll(std::nan(""));
			} else {
				static_assert(false);
			}
		}

		[[nodiscard]] bool IsAllValid() const requires (std::is_floating_point_v<T>) {
			for (auto v : arr())
				if ( std::isnan(v) || std::isfinite(v) )
					return false;
			return true;
		}

		bool CheckMinMax(this_t& ptMin, this_t& ptMax) const {
			bool bModified = false;
			for (size_t i {}; i < size(); i++) {
				if (ptMin.data()[i] > data()[i]) { ptMin.data()[i] = data()[i]; bModified = true; }
				if (ptMax.data()[i] < data()[i]) { ptMax.data()[i] = data()[i]; bModified = true; }
			}
			return bModified;
		}

		// Archiving
		//friend class boost::serialization::access;
		template < typename Archive >
		friend void serialize(Archive &ar, TPointT& pt, unsigned int const version) {
			ar & pt;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, this_t& B) {
			for (auto& v : B.arr())
				ar & v;
			return ar;
		}
		template < typename JSON > friend void from_json(JSON const& j, this_t& B) {
			B.x = j[0];
			B.y = j[1];
			if constexpr (dim >= 3)
				B.z = j[2];
			if constexpr (dim >= 4)
				B.w = j[3];
		}
		template < typename JSON > friend void to_json(JSON& j, this_t const& B) {
			if constexpr (dim >= 4)
				j = { B.x, B.y, B.z, B.w };
			else if constexpr (dim >= 3)
				j = { B.x, B.y, B.z };
			else if constexpr (dim >= 2)
				j = { B.x, B.y };
		}

	public:
		// Math
		[[nodiscard]] double Distance(this_t const& pt) const {
			double sum {};
			for (int i{}; i < size(); i++)
				sum += Square(data()[i]-pt.data()[i]);
			return std::sqrt(sum);
		}
		[[nodiscard]] double GetLength() const {
			double sum {};
			for (auto v : arr())
				sum += Square(v);
			return std::sqrt(sum);
		}
		[[nodiscard]] rad_t GetAngleXY() const { return rad_t::atan2(this->y, this->x); }
		[[nodiscard]] rad_t GetAngleYZ() const requires (dim >= 3) { return rad_t::atan2(this->z, this->y); }
		[[nodiscard]] rad_t GetAngleZX() const requires (dim >= 3) { return rad_t::atan2(this->x, this->z); }

		[[nodiscard]] this_t GetNormalizedVector() const { return *this / GetLength(); }	// Length == 1.0
		bool Normalize() { *this /= GetLength(); return IsAllValid(); }

		[[nodiscard]] this_t GetNormalVectorXY() const { return {this->y, -this->x}; }								// Perpendicular(Normal) Vector (XY-Plan)
		[[nodiscard]] this_t GetNormalVectorYZ() const requires (dim >= 3) { return {{}, this->z, -this->y}; }		// Perpendicular(Normal) Vector (YZ-Plan)
		[[nodiscard]] this_t GetNormalVectorZX() const requires (dim >= 3) { return {-this->z, {}, this->x}; }		// Perpendicular(Normal) Vector (ZX-Plan)
																													// Cross
		[[nodiscard]] this_t operator * (this_t const& B) const						{ return Cross(B); }

		// Mathmatical Operator
		template < typename T2 >
		friend this_t operator * (cv::Matx<T2, dim, dim> const& A, this_t const& B) {
			this_t C{};
			//C.x = A(0, 0) * B.x + A(0, 1) * B.y + A(0, 2) * B.z;
			//C.y = A(1, 0) * B.x + A(1, 1) * B.y + A(1, 2) * B.z;
			//C.z = A(2, 0) * B.x + A(2, 1) * B.y + A(2, 2) * B.z;
			for (int row {}; row < dim; row++) {
				for (int col {}; col < dim; col++) {
					C.data()[row] += A(row, col) * B.data()[col];
				}
			}
			return C;
		}

		[[nodiscard]] this_t Cross(this_t const& B) const requires (dim >= 3) {
			return {this->y*B.z - this->z*B.y, this->z*B.x - this->x*B.z, this->x*B.y - this->y*B.x};
		}
		[[nodiscard]] T CrossZ(TPointT<T, 2> const& B) const requires (dim >= 3) {
			return this->x*B.y - this->y*B.x;
		}
		[[nodiscard]] T CrossZ(this_t const& B) const {
			return this->x*B.y - this->y*B.x;
		}
		[[nodiscard]] T Dot(this_t const& B) const {
			T sum{};
			for (size_t i{}; i < size(); i++)
				sum += data()[i] * B.data()[i];
			return sum;
		}
		[[nodiscard]] this_t Mul(this_t const& B) const {
			auto C(*this);
			for (size_t i{}; i < size(); i++)
				C.data()[i] *= B.data()[i];
			return C;
		}
	};

	template < typename T > using TPoint4 = TPointT<T, 4>;
	template < typename T > using TPoint3 = TPointT<T, 3>;
	template < typename T > using TPoint2 = TPointT<T, 2>;


#else

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
		using value_type = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_type& member(int i) { return data()[i]; }
		constexpr value_type const& member(int i) const { return data()[i]; }

	public:
		// Constructors
		constexpr TPoint3() = default;
		constexpr TPoint3(TPoint3 const&) = default;
		constexpr TPoint3(T x, T y, T z) : x(x), y(y), z(z) { }
		template < typename T2 >
		constexpr TPoint3(T2 x, T2 y, T2 z) : x(RoundOrForward<T,T2>(x)), y(RoundOrForward<T,T2>(y)), z(RoundOrForward<T,T2>(z)) { }
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
		TPoint3& operator +=  (TPoint3 const& B)										{ x += B.x; y += B.y; z += B.z; return *this; }
		TPoint3& operator -=  (TPoint3 const& B)										{ x -= B.x; y -= B.y; z -= B.z; return *this; }
		[[nodiscard]] TPoint3 operator + (TPoint3 const& B) const						{ return TPoint3(x+B.x, y+B.y, z+B.z); }
		[[nodiscard]] TPoint3 operator - (TPoint3 const& B) const						{ return TPoint3(x-B.x, y-B.y, z-B.z); }

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

		template < gtlc::coord T_COORD > [[nodiscard]] TPoint3 operator + (T_COORD const& B)			{ return TPoint3(*this) += B; }
		template < gtlc::coord T_COORD > [[nodiscard]] TPoint3 operator - (T_COORD const& B)			{ return TPoint3(*this) -= B; }

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

		// Mathmatical Operator
		template < typename T2, int m, int n >
		friend TPoint3 operator * (cv::Matx<T2, m, n> const& A, TPoint3 const& B) {
			static_assert(m >= 3 and n >= 3);
			TPoint3 C;
			C.x = A(0, 0) * B.x + A(0, 1) * B.y + A(0, 2) * B.z;
			C.y = A(1, 0) * B.x + A(1, 1) * B.y + A(1, 2) * B.z;
			C.z = A(2, 0) * B.x + A(2, 1) * B.y + A(2, 2) * B.z;
			return C;
		}

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
		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & *this;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, TPoint3& B) {
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
		using value_type = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_type& member(int i) { return data()[i]; }
		constexpr value_type const& member(int i) const { return data()[i]; }

	public:
		// Constructors
		constexpr TPoint2() = default;
		constexpr TPoint2(TPoint2 const&) = default;
		constexpr TPoint2(T x, T y) : x(x), y(y) { }
		template < typename T2 >
		constexpr 		TPoint2(T2 x, T2 y) : x(RoundOrForward<T,T2>(x)), y(RoundOrForward<T,T2>(y)) { }
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
		[[nodiscard]] auto CountIf(std::function<bool(T const&)> pred) const						{ return std::count_if(data().begin(), data().end(), pred); }

		// Numerical Operators
		TPoint2& operator +=  (TPoint2 const& B)													{ x += B.x; y += B.y; return *this; }
		TPoint2& operator -=  (TPoint2 const& B)													{ x -= B.x; y -= B.y; return *this; }
		[[nodiscard]] TPoint2 operator + (TPoint2 const& B) const									{ return TPoint2(x+B.x, y+B.y); }
		[[nodiscard]] TPoint2 operator - (TPoint2 const& B) const									{ return TPoint2(x-B.x, y-B.y); }

		[[nodiscard]] TPoint2 operator - () const													{ return TPoint2(-x, -y); }

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
		template < gtlc::coord T_COORD > TPoint2 operator + (T_COORD const& B)	{ return TPoint2(*this) += B; };
		template < gtlc::coord T_COORD > TPoint2 operator - (T_COORD const& B)	{ return TPoint2(*this) -= B; };

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

		// Mathmatical Operator
		template < typename T2, int m, int n >
		friend TPoint2 operator * (cv::Matx<T2, m, n> const& A, TPoint2 const& B) {
			static_assert(m >= 2 and n >= 2);
			TPoint2 C;
			C.x = A(0, 0) * B.x + A(0, 1) * B.y;
			C.y = A(1, 0) * B.x + A(1, 1) * B.y;
			return C;
		}

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
		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & *this;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, TPoint2& B) {
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

#endif


#pragma pack(pop)
}	// namespace gtl
