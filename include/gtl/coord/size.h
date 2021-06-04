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

#if 1
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
			} else static_assert(false);
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
				static_assert(false);
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
			} else static_assert(false);
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

#else

	//=============================================================================

	//template < typename T > struct TSize3;
	//template < typename T > struct TSize2;

	//--------------------------------------------------------------------------------------------------------------------------------
	// Size 3 (cx, cy, cz)
	//
	template < typename T >
	struct TSize3 {
	public:
		T cx{}, cy{}, cz{};
		using coord_t = std::array<T, 3>;
		using value_type = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_type& member(int i) { return data()[i]; }
		constexpr value_type const& member(int i) const { return data()[i]; }

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
		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & *this;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, TSize3& B) {
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
		using value_type = T;
		constexpr coord_t& data() { return std::bit_cast<coord_t&>(*this); }
		constexpr coord_t const& data() const { return std::bit_cast<coord_t const&>(*this); }
		constexpr value_type& member(int i) { return data()[i]; }
		constexpr value_type const& member(int i) const { return data()[i]; }

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
		friend class boost::serialization::access;
		template < typename tBoostArchive >
		void serialize(tBoostArchive &ar, unsigned int const version) {
			ar & *this;
		}
		template < typename Archive > friend Archive& operator & (Archive& ar, TSize2& B) {
			return ar & B.cx & B.cy;
		}
		template < typename JSON > friend void from_json(JSON const& j, TSize2& B) { B.cx = j["cx"]; B.cy = j["cy"]; }
		template < typename JSON > friend void to_json(JSON& j, TSize2 const& B) { j["cx"] = B.cx; j["cy"] = B.cy; }

	};

#endif


#pragma pack(pop)
}	// namespace gtl
