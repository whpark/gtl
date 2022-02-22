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

//#include "boost/ptr_container/ptr_container.hpp"
//#include "boost/serialization/serialization.hpp"
//#include "boost/serialization/export.hpp"

export module gtl:coord_size;

import :concepts;
import :misc;

//#ifdef min
//#	undef min	// undefine WinAPI - min. ( use std::min only.)
//#endif

export namespace gtl {

	template < typename T, int dim > struct TSizeT;

	template < typename T > struct TSIZE3 { T cx{}, cy{}, cz{};		auto operator <=> (TSIZE3 const&) const = default; };
	template < typename T > struct TSIZE2 { T cx{}, cy{};			auto operator <=> (TSIZE2 const&) const = default; };

	//--------------------------------------------------------------------------------------------------------------------------------
	// General Size (cx,cy[,cz])
	//

	//--------------------------------------------------------------------------------------------------------------------------------
	// Size 3 (cx, cy, cz)
	//
	template < typename T, int dim >
	struct TSizeT : std::conditional_t< dim == 2, TSIZE2<T>, TSIZE3<T> > {
	public:

		//T cx{}, cy{}, cz{};
		using base_t = std::conditional_t< dim == 2, TSIZE2<T>, TSIZE3<T> >;
		using this_t = TSizeT;
		using coord_t = std::array<T, dim>;
		using value_type = T;

		constexpr auto& arr() { return reinterpret_cast<coord_t&>(*this); }
		constexpr auto const& arr() const { return reinterpret_cast<coord_t const&>(*this); }

		constexpr auto* data() { return (reinterpret_cast<coord_t&>(*this)).data(); }
		constexpr auto const* data() const { return (reinterpret_cast<coord_t const&>(*this)).data(); }
		constexpr auto const size() const { return (reinterpret_cast<coord_t const&>(*this)).size(); }
		constexpr value_type& member(size_t i) { return data()[i]; }
		constexpr value_type const& member(size_t i) const { return data()[i]; }

		static_assert(2 <= dim and dim <= 3);

	public:
		// Constructors
		constexpr TSizeT() = default;
		constexpr TSizeT(TSizeT const&) = default;
		constexpr TSizeT(T cx, T cy) requires (dim == 2) : base_t{cx, cy} { }
		constexpr TSizeT(T cx, T cy, T cz) requires (dim == 3) : base_t{cx, cy, cz} { }
		template < typename T2 >
		constexpr TSizeT(T2 cx, T2 cy) requires (dim == 2): base_t{ RoundOrForward<T,T2>(cx), RoundOrForward<T,T2>(cy) } { }
		template < typename T2 >
		constexpr TSizeT(T2 cx, T2 cy, T2 cz) requires (dim == 3): base_t{ RoundOrForward<T,T2>(cx), RoundOrForward<T,T2>(cy), RoundOrForward<T,T2>(cz) } { }
		TSizeT& operator = (TSizeT const&) = default;
		TSizeT& operator = (TSizeT &&) = default;

		// from vector
		explicit TSizeT(std::vector<T> const& B) {
			*this = B;
		}
		TSizeT& operator = (std::vector<T> const& B) {
			size_t n = std::min(size(), B.size());
			for (size_t i = 0; i < n; i++) {
				member(i) = B[i];
			}
			return *this;
		}

		// Copy Constructors and Copy Assign operators
		template < gtlc::generic_coord T_COORD > explicit TSizeT(T_COORD const& B) { *this = B; };
		template < gtlc::generic_coord T_COORD > TSizeT& operator = (T_COORD const& B) {
			if constexpr (gtlc::has__cxy<T_COORD>) {
				this->cx = RoundOrForward<T>(B.cx);
				this->cy = RoundOrForward<T>(B.cy);
				if constexpr ((dim >= 3) and gtlc::has__cz<T_COORD>)
					this->cz = RoundOrForward<T>(B.cz);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				this->cx = RoundOrForward<T>(B.width);
				this->cy = RoundOrForward<T>(B.height);
				if constexpr ((dim >= 3) and gtlc::has__depth<T_COORD>)
					this->cz = RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__xy<T_COORD>) {
				this->cx = RoundOrForward<T>(B.x);
				this->cy = RoundOrForward<T>(B.y);
				if constexpr ((dim >= 3) and has__z<T_COORD>)
					this->cz = RoundOrForward<T>(B.z);
			} else if constexpr (gtlc::rect<T_COORD>) {
				*this = (B.pt1 - B.pt0);
			} else if constexpr (gtlc::wnd_rect<T_COORD>) {
				this->cx = B.right - B.left;
				this->cy = B.bottom - B.top;
			} else static_assert(gtlc::dependenct_false_v);
			return *this;
		};


		constexpr static inline this_t All(T v) {
			if constexpr (dim == 2)
				return this_t(v, v);
			else if constexpr (dim == 3)
				return this_t(v, v, v);
			//else if (dim == 4)
			//	return this_t(v, v, v, v);
		}

		// Type Casting to cv::Size_, SIZE, ...
		template < gtlc::generic_coord T_COORD >
		operator T_COORD const () const {
		#define GTL__TYPE_CAST_COORD_VAL3(tx, ty, tz)\
			T_COORD{RoundOrForward<decltype(T_COORD::tx)>(this->cx), RoundOrForward<decltype(T_COORD::ty)>(this->cy), RoundOrForward<decltype(T_COORD::tz)>(this->cz)};
		#define GTL__TYPE_CAST_COORD_VAL2(tx, ty)\
			T_COORD{RoundOrForward<decltype(T_COORD::tx)>(this->cx), RoundOrForward<decltype(T_COORD::ty)>(this->cy)};

			if constexpr (false) {
			}
			else if constexpr ((dim >= 3) and gtlc::has__cxyz<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL3(cx, cy, cz);
			}
			else if constexpr ((dim >= 3) and gtlc::has__size3<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL3(width, height, depth);
			}
			else if constexpr ((dim >= 3) and gtlc::has__xyz<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL3(x, y, z);
			}
			else if constexpr ((dim >= 2) and gtlc::has__cxy<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL2(cx, cy);
			}
			else if constexpr ((dim >= 2) and gtlc::has__size2<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL2(width, height);
			}
			else if constexpr ((dim >= 2) and gtlc::has__xy<T_COORD>) {
				return GTL__TYPE_CAST_COORD_VAL2(x, y);
			}
			else {
				static_assert(gtlc::dependent_false_v);
			}

		#undef GTL__TYPE_CAST_COORD_VAL2
		#undef GTL__TYPE_CAST_COORD_VAL3
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

		[[nodiscard]] this_t operator - () const										{ auto C(*this); for (auto& v : C.arr()) v = -v; return C; }

		template < gtlc::generic_coord T_COORD > this_t& operator += (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				this->cx += RoundOrForward<T>(B.x);
				this->cy += RoundOrForward<T>(B.y);
				if constexpr ((dim >= 3) and gtlc::has__z<T_COORD>)
					this->cz += RoundOrForward<T>(B.z);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				this->cx += RoundOrForward<T>(B.width);
				this->cy += RoundOrForward<T>(B.height);
				if constexpr ((dim >= 3) and gtlc::has__depth<T_COORD>)
					this->cz += RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				this->cx += RoundOrForward<T>(B.cx);
				this->cy += RoundOrForward<T>(B.cy);
				if constexpr ((dim >= 3) and gtlc::has__cz<T_COORD>)
					this->cz += RoundOrForward<T>(B.cz);
			} else static_assert(gtlc::dependent_false_v);
			return *this;
		};
		template < gtlc::generic_coord T_COORD > this_t& operator -= (T_COORD const& B) {
			if constexpr (gtlc::has__xy<T_COORD>) {
				this->cx -= RoundOrForward<T>(B.x);
				this->cy -= RoundOrForward<T>(B.y);
				if constexpr ((dim >= 3) and gtlc::has__z<T_COORD>)
					this->cz -= RoundOrForward<T>(B.z);
				if constexpr ((dim >= 4) and gtlc::has__w<T_COORD>)
					this->cw -= RoundOrForward<T>(B.w);
			} else if constexpr (gtlc::has__size2<T_COORD>) {
				this->cx -= RoundOrForward<T>(B.width);
				this->cy -= RoundOrForward<T>(B.height);
				if constexpr ((dim >= 3) and gtlc::has__depth<T_COORD>)
					this->cz -= RoundOrForward<T>(B.depth);
			} else if constexpr (gtlc::has__cxy<T_COORD>) {
				this->cx -= RoundOrForward<T>(B.cx);
				this->cy -= RoundOrForward<T>(B.cy);
				if constexpr ((dim >= 3) and gtlc::has__cz<T_COORD>)
					this->cz -= RoundOrForward<T>(B.cz);
			} else static_assert(gtlc::dependent_false_v);
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
		void Set(T cx = {}, T cy = {})						requires (dim == 2) { this->cx = cx; this->cy = cy; }
		void Set(T cx = {}, T cy = {}, T cz = {})			requires (dim == 3) { this->cx = cx; this->cy = cy; this->cz = cz; }
		void SetAll(T value = {})							requires (dim == 2) { this->cx = this->cy = value; }
		void SetAll(T value = {})							requires (dim == 3) { this->cx = this->cy = this->cz = value; }
		void SetZero()															{ SetAll(); }
		[[nodiscard]] void SetAsNAN() requires (std::is_floating_point_v<T>) {
			if constexpr (std::is_same_v<T, float>) {
				SetAll(std::nanf(""));
			} else if constexpr (std::is_same_v<T, double>) {
				SetAll(std::nan(""));
			} else {
				static_assert(gtlc::dependent_false_v);
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
		template < typename tBoostArchive >
		friend void serialize(tBoostArchive &ar, TSizeT& size, unsigned int const version) {
			ar & size;
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

	};

	template < typename T > using TSize3 = TSizeT<T, 3>;
	template < typename T > using TSize2 = TSizeT<T, 2>;


}	// namespace gtl
