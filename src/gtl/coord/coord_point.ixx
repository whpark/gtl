//////////////////////////////////////////////////////////////////////
//
// point.h: coord.h 에서 따로 뺌. (너무 길어..)
//
// PWH
// 2019.11.02. 전체 수정
//
//////////////////////////////////////////////////////////////////////

module;

#include <compare>
#include <functional>
#include <algorithm>

#include "gtl/_config.h"
#include "gtl/_macro.h"

#include "opencv2/opencv.hpp"

//#include "boost/ptr_container/ptr_deque.hpp"
//#include "boost/serialization/serialization.hpp"
//#include "boost/serialization/export.hpp"
//#include "boost/serialization/base_object.hpp"

export module gtl:coord_point;

import :concepts;
import :misc;
import :unit;

export namespace gtl {

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
		constexpr value_type& member(size_t i) { return data()[i]; }
		constexpr value_type const& member(size_t i) const { return data()[i]; }

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
				member(i) = B[i];
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
			} else if constexpr (gtlc::wnd_rect<T_COORD>) {
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
			for (size_t i{}; i < dim; i++)
				B.member(i) = j[i];
		}
		template < typename JSON > friend void to_json(JSON&& j, this_t const& B) {
			for (size_t i{}; i < dim; i++)
				j[i] = B.member(i);
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


}	// namespace gtl
