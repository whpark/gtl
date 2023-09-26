﻿#pragma once

//////////////////////////////////////////////////////////////////////
//
// misc.h: 잡다한것들...
//
// PWH
// 2019.01.03.
// define macro 들을 template class / function 으로 변경
// 2019.07.24. QL -> GTL
// 2021.01.04.
//
//////////////////////////////////////////////////////////////////////

//#include "gtl/_default.h"
#include <cmath>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <format>
#include <chrono>
#include <functional>
#include <filesystem>
#include <source_location>
#include "gtl/concepts.h"

namespace gtl::internal {
	template < typename T1, typename T2 > void op1mul(T1& v1, T2 v2) { v1 *= v2; }
	template < typename T1, typename T2 > void op1div(T1& v1, T2 v2) { v1 /= v2; }
	template < typename T1, typename T2 > void op1add(T1& v1, T2 v2) { v1 += v2; }
	template < typename T1, typename T2 > void op1sub(T1& v1, T2 v2) { v1 -= v2; }
	template < typename T1, typename T2 > void op2mul(T1& v1, T2 v2) { v1 = RoundOrForward<T1>(v1 * v2); }
	template < typename T1, typename T2 > void op2div(T1& v1, T2 v2) { v1 = RoundOrForward<T1>(v1 / v2); }
	template < typename T1, typename T2 > void op2add(T1& v1, T2 v2) { v1 = RoundOrForward<T1>(v1 + v2); }
	template < typename T1, typename T2 > void op2sub(T1& v1, T2 v2) { v1 = RoundOrForward<T1>(v1 - v2); }

	template < typename T1, typename T2 >
	void DoArithmaticMul(T1& v1, T2 v2) { if constexpr (std::is_same_v<T1, decltype(T1{}*T2{})> ) { op1mul(v1, v2); } else { op2mul(v1, v2); } };
	template < typename T1, typename T2 >
	void DoArithmaticDiv(T1& v1, T2 v2) { if constexpr (std::is_same_v<T1, decltype(T1{}/T2{})> ) { op1div(v1, v2); } else { op2div(v1, v2); } };
	template < typename T1, typename T2 >
	void DoArithmaticAdd(T1& v1, T2 v2) { if constexpr (std::is_same_v<T1, decltype(T1{}+T2{})> ) { op1add(v1, v2); } else { op2add(v1, v2); } };
	template < typename T1, typename T2 >
	void DoArithmaticSub(T1& v1, T2 v2) { if constexpr (std::is_same_v<T1, decltype(T1{}-T2{})> ) { op1sub(v1, v2); } else { op2sub(v1, v2); } };
}

namespace gtl {
#pragma pack(push, 8)

	template < typename ttime_point_t = std::chrono::seconds >
	auto ToLocalTime(auto t) {
		auto lt = std::chrono::current_zone()->to_local(t);
		return std::chrono::time_point_cast<ttime_point_t>(lt);
	}

	template < gtlc::string_elem tchar_t, size_t N >
	struct xStringLiteral {
		tchar_t str[N];

		constexpr xStringLiteral(tchar_t const (&sz)[N]) {
			std::copy_n(sz, N, str);
		}
	};

	//! @brief call back function for progress dialog or something like...
	using callback_progress_t = std::function<bool (int iPercent, bool bDone, bool bError)>;

	////-------------------------------------------------------------------------
	//// countof
	//template < typename T, size_t n > constexpr auto countof(const T (&arr)[n]) { return n; }
	//template < typename T, size_t n > constexpr auto countof(const std::array<T, n>& arr) { return n; }

	//-----------------------------------------------------------------------------
	// defaulted Copy Constructor/Assignment 에서 제외할 멤버 변수용
	template < typename T >
	class TCopyTransparent : public T {
	public:
		using base_t = T;

		using T::T;
		//TCopyTransparent() : T() {}
		TCopyTransparent(const TCopyTransparent& B) { }
		TCopyTransparent(TCopyTransparent&& B) = default;
		TCopyTransparent& operator = (const TCopyTransparent& B) { return *this; }
		TCopyTransparent& operator = (TCopyTransparent&& B) = default;
	};


	////-----------------------------------------------------------------------------
	//// Auto Copy Constructor 에서 제외, Movable 포함 멤버 변수용
	//template < typename T >
	//class TCopyTransparent : public T {
	//public:
	//	using T::T;
	//	TCopyTransparent(const TCopyTransparent& B) {};
	//	//TCopyTransparent(TCopyTransparent&& B) = default;
	//	//TCopyTransparent& operator = (TCopyTransparent&& B) = default;
	//};


	//-----------------------------------------------------------------------------
	//
	template <typename T>
	struct equal_to_value {
		T value;
		constexpr bool operator()(T const& v) const { return value == v; }
	};
	struct equal_to_zero {
		template <typename T>
		constexpr bool operator()(T const& v) const { return v == T{}; }
	};
	struct non_zero {
		template <typename T>
		constexpr bool operator()(T const& v) const { return v != T{}; }
	};

	/// @brief string to int/double ...
	/// @tparam value_t arithmetic type
	/// @param str 
	/// @param base 
	/// @return 
	template < gtlc::arithmetic value_t >
	value_t ToArithmeticValue(std::string_view sv, int base = 0, std::from_chars_result* result = {}) {
		value_t value{};
		auto* b = sv.data();
		auto* e = sv.data() + sv.size();
		// skip leading spaces
		while (b < e and std::isspace(*b)) b++;
		if constexpr (std::is_integral_v<value_t>) {
			if (base == 0) {
				std::string_view sv(b, e);
				if (sv.starts_with("0x")) {
					base = 16;
					b += 2;
				}
				else if (sv.starts_with("0b")) {
					b += 2;
					base = 2;
				}
				else if (sv.starts_with("0")) {
					b += 1;
					base = 8;
				}
				else {
					base = 10;
				}
			}
			auto r = std::from_chars(b, e, value, base);
			if (result)
				*result = r;
		}
		else {
			auto r = std::from_chars(b, e, value);
			if (result)
				*result = r;
		}
		return value;
	}

	template < gtlc::arithmetic value_t, gtlc::string_elem tchar_t >
	value_t ToArithmeticValue(std::basic_string<tchar_t> const& str, int base = 0, std::from_chars_result* result = {}) {
		return ToArithmeticValue<value_t>(std::string_view(ToString<char, tchar_t>(str)), base, result);
	}

	/// @brief RAI helper
	// xTrigger -> xFinalAction (naming from gsl::final_action)
	struct xFinalAction {
		std::function<void()> m_action;
		[[nodiscard]] xFinalAction(std::function<void()> action) noexcept : m_action(action) {}

		~xFinalAction() {
			DoAction();
		}
		void DoAction() {
			if (auto action = std::exchange(m_action, nullptr)) {
				action();
			}
		}
		void Release() {
			m_action = nullptr;
		}
	};




	//-------------------------------------------------------------------------
	// Variable Init
	template < gtlc::trivially_copyable T >
	constexpr inline void MakeZeroVar(T& var) { var = T{}; }


	//-------------------------------------------------------------------------
	// simple math
	// Square
	template < typename T > constexpr T Square(T x) {
		return x * x;
	}
	template < typename T > constexpr T Sqr(T x) {
		return x * x;
	}

	/// @brief Rounding to nearest integer
	/// @tparam T_DEST 
	/// @tparam T_SOURCE 
	/// @param v 
	/// @return 
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
		requires (std::is_integral_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST Round(T_SOURCE v) {
		//return T_DEST(std::round(v));
		return T_DEST(v+0.5*(v<T_SOURCE{}?-1:1));
	}
	/// @brief Rounding to nearest integer.
	/// @tparam T_DEST 
	/// @tparam T_SOURCE 
	/// @param v 
	/// @param place rounding place (ex, 1, 10, 100, 0.01 ...)
	/// @return 
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST Round(T_SOURCE v, T_SOURCE place) {
		//return T_DEST(T_DEST(std::round(v/place/10))*place*10);
		return T_DEST(T_DEST(v/place/10+0.5*(v<T_SOURCE{}?-1:1))*place*10);
	}
	
	/// @brief Round for gtlc::generic_coord
	template < gtlc::generic_coord T_COORD >
	constexpr [[nodiscard]] T_COORD Round(T_COORD const& coord) {
		T_COORD v;
		if constexpr (gtlc::has__x<T_COORD>) { v.x = std::round(coord.x); }
		if constexpr (gtlc::has__y<T_COORD>) { v.y = std::round(coord.y); }
		if constexpr (gtlc::has__z<T_COORD>) { v.z = std::round(coord.z); }
		if constexpr (gtlc::has__w<T_COORD>) { v.w = std::round(coord.w); }
		if constexpr (gtlc::has__cx<T_COORD>) { v.cx = std::round(coord.cx); }
		if constexpr (gtlc::has__cy<T_COORD>) { v.cy = std::round(coord.cy); }
		if constexpr (gtlc::has__cz<T_COORD>) { v.cz = std::round(coord.cz); }
		if constexpr (gtlc::has__width<T_COORD>) { v.width = std::round(coord.width); }
		if constexpr (gtlc::has__height<T_COORD>) { v.height = std::round(coord.height); }
		if constexpr (gtlc::has__depth<T_COORD>) { v.depth = std::round(coord.depth); }
		return v;
	}

	/// @brief Round or just forward values. (실수 -> 정수 대입시 반올림. (실수 -> 실수) 또는 (정수 -> 정수) 일 경우에는 값의 변화 없이 그대로 대입.)
	/// @tparam T_DEST 
	/// @tparam T_SOURCE 
	/// @param v2 
	/// @return 
	template < typename T_DEST, typename T_SOURCE >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST RoundOrForward(T_SOURCE v2) {
		if constexpr (std::is_integral_v<T_DEST> && std::is_floating_point_v<T_SOURCE>) {
			return Round<T_DEST, T_SOURCE>(v2);
		} else {
			return (T_DEST)v2;
		}
	}


	// Ceil
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST Ceil(T_SOURCE v) {
		return T_DEST(std::ceil(v));	// std::ceil is not constexpr yet.
	}
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST Ceil(T_SOURCE v, T_SOURCE place) {
		return T_DEST(std::ceil(v/place/10)*place*10);
	}
	// 실수 -> 정수 대입시 반올림. (실수 -> 실수) 또는 (정수 -> 정수) 일 경우에는 값의 변화 없이 그대로 대입.
	template < typename T_DEST, typename T_SOURCE >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST CeilOrForward(T_SOURCE v2) {
		if constexpr (std::is_integral_v<T_DEST> && std::is_floating_point_v<T_SOURCE>) {
			return Ceil<T_DEST, T_SOURCE>(v2);
		} else {
			return (T_DEST)v2;
		}
	}

	/// @brief Ceil for gtlc::generic_coord
	template < gtlc::generic_coord T_COORD >
	constexpr [[nodiscard]] T_COORD Ceil(T_COORD const& coord) {
		T_COORD v;
		if constexpr (gtlc::has__x<T_COORD>) { v.x = std::ceil(coord.x); }
		if constexpr (gtlc::has__y<T_COORD>) { v.y = std::ceil(coord.y); }
		if constexpr (gtlc::has__z<T_COORD>) { v.z = std::ceil(coord.z); }
		if constexpr (gtlc::has__w<T_COORD>) { v.w = std::ceil(coord.w); }
		if constexpr (gtlc::has__cx<T_COORD>) { v.cx = std::ceil(coord.cx); }
		if constexpr (gtlc::has__cy<T_COORD>) { v.cy = std::ceil(coord.cy); }
		if constexpr (gtlc::has__cz<T_COORD>) { v.cz = std::ceil(coord.cz); }
		if constexpr (gtlc::has__width<T_COORD>) { v.width = std::ceil(coord.width); }
		if constexpr (gtlc::has__height<T_COORD>) { v.height = std::ceil(coord.height); }
		if constexpr (gtlc::has__depth<T_COORD>) { v.depth = std::ceil(coord.depth); }
		return v;
	}

	// Floor
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST Floor(T_SOURCE v) {
		return T_DEST(std::floor(v));	// std::floor is not constexpr yet.
	}
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST Floor(T_SOURCE v, T_SOURCE place) {
		return T_DEST(std::floor(v/place/10)*place*10);
	}
	// 실수 -> 정수 대입시 반올림. (실수 -> 실수) 또는 (정수 -> 정수) 일 경우에는 값의 변화 없이 그대로 대입.
	template < typename T_DEST, typename T_SOURCE >
		requires (std::is_arithmetic_v<std::remove_cvref_t<T_DEST>> and std::is_arithmetic_v<std::remove_cvref_t<T_SOURCE>>)
	constexpr [[nodiscard]] T_DEST FloorOrForward(T_SOURCE v2) {
		if constexpr (std::is_integral_v<T_DEST> && std::is_floating_point_v<T_SOURCE>) {
			return Floor<T_DEST, T_SOURCE>(v2);
		} else {
			return (T_DEST)v2;
		}
	}

	/// @brief Floor for gtlc::generic_coord
	template < gtlc::generic_coord T_COORD >
	constexpr [[nodiscard]] T_COORD Floor(T_COORD const& coord) {
		T_COORD v{};
		if constexpr (gtlc::has__x<T_COORD>) { v.x = std::floor(coord.x); }
		if constexpr (gtlc::has__y<T_COORD>) { v.y = std::floor(coord.y); }
		if constexpr (gtlc::has__z<T_COORD>) { v.z = std::floor(coord.z); }
		if constexpr (gtlc::has__w<T_COORD>) { v.w = std::floor(coord.w); }
		if constexpr (gtlc::has__cx<T_COORD>) { v.cx = std::floor(coord.cx); }
		if constexpr (gtlc::has__cy<T_COORD>) { v.cy = std::floor(coord.cy); }
		if constexpr (gtlc::has__cz<T_COORD>) { v.cz = std::floor(coord.cz); }
		if constexpr (gtlc::has__width<T_COORD>) { v.width = std::floor(coord.width); }
		if constexpr (gtlc::has__height<T_COORD>) { v.height = std::floor(coord.height); }
		if constexpr (gtlc::has__depth<T_COORD>) { v.depth = std::floor(coord.depth); }
		return v;
	}

	// Boolean
	template < typename ... Args > constexpr bool IsAllTrue(Args&& ... args)						{ return (args && ...); }
	template < typename ... Args > constexpr bool IsNoneTrue(Args&& ... args)						{ return (!args && ...); }
	template < typename ... Args > constexpr bool IsAllFalse(Args&& ... args)						{ return ((false == args) && ...); }
	template < typename ... Args > constexpr bool IsAnyTrue(Args&& ... args)						{ return (args ||...); }
	template < typename ... Args > constexpr bool IsAllSame(Args&& ... args)						{ return IsAllTrue(args...) || IsNoneTrue(args...); }

	template < typename T, typename ... Args > constexpr bool IsValueOneOf(T v, Args&& ... args)	{ return ((v == args) || ...); }
	template < typename T, typename ... Args > constexpr bool IsValueNoneOf(T v, Args&& ... args)	{ return ((v != args) && ...); }

	template < typename T >
	constexpr bool IsInside(T const& v, T const& left, T const& right)						{ return (left <= v) && (v < right); }

	// Bit Set
	namespace internal {
		template < typename T >						constexpr T bit_single(int b)					{ return ((T)1 << b); }
		template < typename T, typename ... Bits >	constexpr T bit_multi(Bits ... bits)			{ return (bit_single<T>(bits) | ...); }
	}
	template < typename ... Bits >				constexpr std::uint32_t Bit32(Bits ... bits)		{ return internal::bit_multi<std::uint32_t>(bits...); }
	template < typename ... Bits >				constexpr std::uint64_t Bit64(Bits ... bits)		{ return internal::bit_multi<std::uint64_t>(bits...); }

	template < typename ... Bits >				constexpr std::bitset<32> BitSet32(Bits ... bits)	{ return std::bitset<32>(bit32(bits...)); }
	template < typename ... Bits >				constexpr std::bitset<64> BitSet64(Bits ... bits)	{ return std::bitset<64>(bit64(bits...)); }


	// Word Align Position
	template < std::integral T > constexpr inline T AdjustAlign128(T w) { return ((w+15)/16*16); }	//	((w+15)>>4)<<4
	template < std::integral T > constexpr inline T AdjustAlign64(T w) { return ((w+7)/8*8); }		//	((w+ 7)>>3)<<3
	template < std::integral T > constexpr inline T AdjustAlign32(T w) { return ((w+3)/4*4); }		//	((w+ 3)>>2)<<1
	template < std::integral T > constexpr inline T AdjustAlign16(T w) { return ((w+1)/2*2); }		//	((w+ 1)>>1)<<1


	// byte swap
#if (GTL__STRING_PRIMITIVES__WINDOWS_FRIENDLY)
	template < std::integral type >
	[[nodiscard]] inline auto GetByteSwap(type v) {
		if constexpr (sizeof(v) == sizeof(std::uint8_t)) {
			return v;
		} else if constexpr (sizeof(v) == sizeof(std::uint16_t)) {
			return _byteswap_ushort(v);
		} else if constexpr (sizeof(v) == sizeof(std::uint32_t)) {
			return _byteswap_ulong(v);
		} else if constexpr (sizeof(v) == sizeof(std::uint64_t)) {
			return _byteswap_uint64(v);
		} else {
			static_assert(gtlc::dependent_false_v, "not supported data type.");
		}
	}
	template < std::integral type >
	inline void ByteSwap(type& v) {
		v = GetByteSwap(v);
	}
#else
	template < std::integral type >
	[[nodiscard]] constexpr inline type GetByteSwap(type const v) {
		if constexpr (sizeof(v) == 1) {
			return v;
		}
		else {
			type r{};
			std::reverse_copy((uint8_t const*)&v, (uint8_t const*)&v+sizeof(v), (uint8_t*)&r);
			return r;
		}
	}
	template < std::integral type >
	constexpr inline void ByteSwap(type& v) {
		if constexpr (sizeof(v) == 1) {
			return ;
		}
		else {
			std::reverse((uint8_t*)&v, (uint8_t*)&v+sizeof(v));
		}
	}
#endif

	template < std::integral type >
	inline type GetNetworkByteOrder(type const v) {
		if constexpr (std::endian::native == std::endian::little) {
			return GetByteSwap(v);
		}
		else
			return v;
	}



	namespace internal {

		// tuple has special constructor for std::pair. converting pair -> T1, T2...
		template < typename T1, typename T2 >
		class pair : public std::pair<T1, T2> {
		public:
			using base_t = std::pair<T1,T2>;
			using base_t::base_t;

		};


		//// non-const function call in const function. -----> std::remove_cvref_t
		//template < typename T >
		//using remove_ref_const_t = std::remove_const_t<std::remove_reference_t<T>>;


	}

	/// color

	/// @brief color_rgba_t
	union color_rgba_t {
		struct { uint8_t r, g, b, a; };
		uint32_t cr{};

		using this_t = color_rgba_t;

		this_t& operator = (this_t const& b) { cr = b.cr; return *this; }
		bool operator == (this_t const& B) const { return cr == B.cr; }
		auto operator <=> (this_t const& B) const { return cr <=> B.cr; }
		template < typename tjson >
		friend void from_json(tjson const& j, this_t& color) {
			color.cr = (uint32_t)(int64_t)j;
		}
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& color) {
			j = (int64_t)color.cr;
		}

		template < typename archive >
		friend void serialize(archive& ar, this_t& cr, unsigned int const file_version) {
			ar & cr.cr;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, this_t& cr) {
			ar & cr.cr;
			return ar;
		}
	};
	constexpr static inline color_rgba_t ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = {}) {
		return color_rgba_t{.r = r, .g = g, .b = b, .a = a};
	}

	/// @brief color_bgra_t
	union color_bgra_t {
		struct { uint8_t b, g, r, a; };
		uint32_t cr{};

		using this_t = color_bgra_t;

		this_t& operator = (this_t const& b) { cr = b.cr; return *this; }
		bool operator == (this_t const& B) const { return cr == B.cr; }
		auto operator <=> (this_t const& B) const { return cr <=> B.cr; }
		template < typename tjson >
		friend void from_json(tjson const& j, this_t& color) {
			color.cr = (uint32_t)(int64_t)j;
		}
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& color) {
			j = (int64_t)color.cr;
		}

		template < typename archive >
		friend void serialize(archive& ar, this_t& cr, unsigned int const file_version) {
			ar & cr.cr;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, this_t& cr) {
			ar & cr.cr;
			return ar;
		}
	};
	constexpr static inline color_bgra_t ColorBGRA(uint8_t b, uint8_t g, uint8_t r, uint8_t a = {}) {
		return color_bgra_t{ .b = b, .g = g, .r = r, .a = a };
	}

	/// @brief color_abgr_t
	union color_abgr_t {
		struct { uint8_t a, b, g, r; };
		uint32_t cr{};

		using this_t = color_abgr_t;

		this_t& operator = (this_t const& b) { cr = b.cr; return *this; }
		bool operator == (this_t const& B) const { return cr == B.cr; }
		auto operator <=> (this_t const& B) const { return cr <=> B.cr; }
		template < typename tjson >
		friend void from_json(tjson const& j, this_t& color) {
			color.cr = (uint32_t)(int64_t)j;
		}
		template < typename tjson >
		friend void to_json(tjson& j, this_t const& color) {
			j = (int64_t)color.cr;
		}

		template < typename archive >
		friend void serialize(archive& ar, this_t& cr, unsigned int const file_version) {
			ar & cr.cr;
		}
		template < typename archive >
		friend archive& operator & (archive& ar, this_t& cr) {
			ar & cr.cr;
			return ar;
		}
	};
	constexpr static inline color_abgr_t ColorABGR(uint8_t b, uint8_t g, uint8_t r, uint8_t a = {}) {
		return color_abgr_t{ .a = a, .b = b, .g = g, .r = r };
	}

	//-----------------------------------------------------------------------------
	// enum helper
	template < typename T, typename eT >		constexpr T& enum_as(eT& e)		{ static_assert(sizeof(eT) == sizeof(T)); return (T&)e; }
	template < typename eT, typename T = int >	constexpr eT enum_inc(eT& e)	{ static_assert(sizeof(eT) == sizeof(T)); return (eT)++((T&)e); }
	template < typename eT, typename T = int >	constexpr eT enum_dec(eT& e)	{ static_assert(sizeof(eT) == sizeof(T)); return (eT)--((T&)e); }
	template < typename eT, typename T = int >	constexpr eT enum_next(eT e)	{ static_assert(sizeof(eT) == sizeof(T)); return (eT)((T&)e+1); }
	template < typename eT, typename T = int >	constexpr eT enum_prev(eT e)	{ static_assert(sizeof(eT) == sizeof(T)); return (eT)((T&)e-1); }


	//-------------------------------------------------------------------------
	// axis
	enum class eAXIS : int {
		//eAXIS::NW = -4, eAXIS::NZ, eAXIS::NY, eAXIS::NX, eAXIS::NONE = 0, eAXIS::X, eAXIS::Y, eAXIS::Z, eAXIS::W,

		NONE = 0,
		X =   1,	NX = -X,
		Y = X+1,	NY = -Y,
		Z = Y+1,	NZ = -Z,
		W = Z+1,	NW = -W,

		PX = X,
		PY = Y,
		PZ = Z,
		PW = W,
	};

	inline bool IsValidCoordSystem(eAXIS eDX, eAXIS eDY) {
		int iDX = std::abs(static_cast<int>(eDX)), iDY = std::abs(static_cast<int>(eDY));
		return static_cast<int>(eDX) && static_cast<int>(eDY) && (static_cast<int>(iDX) != static_cast<int>(iDY));
	}
	inline bool IsValidCoordSystem(eAXIS eDX, eAXIS eDY, eAXIS eDZ) {
		int iDX = std::abs(static_cast<int>(eDX)), iDY = std::abs(static_cast<int>(eDY)), iDZ = std::abs(static_cast<int>(eDZ));
		return static_cast<int>(eDX) && static_cast<int>(eDY) && static_cast<int>(eDZ)
			&& (static_cast<int>(iDX) != static_cast<int>(iDY))
			&& (static_cast<int>(iDY) != static_cast<int>(iDZ))
			&& (static_cast<int>(iDZ) != static_cast<int>(iDX));
	}

	inline eAXIS GetOppositeDirection(eAXIS eDirection) { return (eAXIS)(-static_cast<int>(eDirection)); }

	template < gtlc::string_elem tchar >
	constexpr inline eAXIS GetDirection(std::basic_string_view<tchar> svName) {
		signed char eNegative = tszsearch(svName, '-') ? -1 : 1;
		if (svName.find('x') != svName.npos or svName.find('X') != svName.npos) return (eAXIS)(static_cast<int>(eAXIS::X)*eNegative);
		if (svName.find('y') != svName.npos or svName.find('Y') != svName.npos) return (eAXIS)(static_cast<int>(eAXIS::Y)*eNegative);
		if (svName.find('z') != svName.npos or svName.find('Z') != svName.npos) return (eAXIS)(static_cast<int>(eAXIS::Z)*eNegative);
		return eAXIS::NONE;
	}
	constexpr static inline std::basic_string_view<char> GetDirectionName(eAXIS eDirection) {
		using namespace std::literals;
		switch (eDirection) {
		case eAXIS::NONE : return ""sv;
		case eAXIS::X  : return  "X"sv; case  eAXIS::Y : return  "Y"sv; case eAXIS::Z  : return  "Z"sv;
		case eAXIS::NX : return "-X"sv; case eAXIS::NY : return "-Y"sv; case eAXIS::NZ : return "-Z"sv;
		}
		return ""sv;
	}

	inline bool HasParentPath(std::filesystem::path const& path) {
		return path.has_parent_path() and path != path.parent_path();
	}
	/// @brief Get Project Name from source file path
	/// @param l : don't touch.
	/// @return 
	inline /*constexpr*/ std::wstring GetGTLProjectName(std::source_location const& l = std::source_location::current()) {
		std::filesystem::path path = l.file_name();
		if (path.extension() == ".h")	// CANNOT get project name from header file
			return {};
		while (HasParentPath(path)) {
			auto filename = path.filename();
			path = path.parent_path();
			if (path.filename() == L"src")
				return filename;
		}
		return {};
	}

	inline std::filesystem::path GetProjectRootFolder(std::wstring const& strProjectNameToBeRemoved, std::filesystem::path path = std::filesystem::current_path()) {
		// Init Temp Folder Names (ex, "Debug", "Release", ...)
		static auto const strsTempFolder = []{
			std::vector<std::wstring> folders;
			#ifdef _DEBUG
				folders.push_back(L"Debug");
			#else
				folders.push_back(L"Release");
			#endif
				if constexpr (sizeof(size_t) == sizeof(uint64_t)) {
					folders.push_back(L"x64");
				} else if constexpr (sizeof(size_t) == sizeof(uint32_t)) {
					folders.push_back(L"x86");
					folders.push_back(L"Win32");
				}
				folders.push_back(L"Temp");
			return folders;
		}();

		// Remove Project Folder ( ex, [src]/[ProjectName] )
		if ( HasParentPath(path) and (path.filename() == strProjectNameToBeRemoved) ) {
			path = path.parent_path();
			if (HasParentPath(path) and path.filename() == L"src") {
				path = path.parent_path();
			}
		}

		// Remove Output Dir Folder Name ( ex, Temp/x64/Debug/ )
		for (auto const& strTempFolder : strsTempFolder) {
			if (!HasParentPath(path))
				break;
			if (path.filename() == strTempFolder) {
				path = path.parent_path();
			}
		}
		return path;
	}

	inline std::optional<std::filesystem::path> SetCurrentPath_ProjectFolder(std::filesystem::path const& pathRelToProjectRoot, std::source_location const& l = std::source_location::current()) {
		// first, Get Root Folder, and then attach [pathRel]
		auto projectName = gtl::GetGTLProjectName(l);
		auto path = gtl::GetProjectRootFolder(projectName);
		if (!pathRelToProjectRoot.empty()) {
			if (pathRelToProjectRoot.filename() != path.filename())
				path /= pathRelToProjectRoot;
		}
		std::error_code ec{};
		std::filesystem::create_directories(path, ec);
		ec = {};
		std::filesystem::current_path(path, ec);
		if (ec)
			return {};
		return path;
	}
	inline auto SetCurrentPath_ProjectFolder(std::source_location const& l = std::source_location::current()) {
		auto projectName = gtl::GetGTLProjectName(l);
		return SetCurrentPath_ProjectFolder(std::filesystem::path(L"src") / projectName, l);
	}
	inline auto SetCurrentPath_BinFolder(std::source_location const& l = std::source_location::current()) {
		return SetCurrentPath_ProjectFolder(L"bin", l);
	}

#pragma pack(pop)
}	// namespac gtl
