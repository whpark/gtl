#pragma once

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

	////-------------------------------------------------------------------------
	//// countof
	//template < typename T, size_t n > constexpr auto countof(const T (&arr)[n]) { return n; }
	//template < typename T, size_t n > constexpr auto countof(const std::array<T, n>& arr) { return n; }

	//-----------------------------------------------------------------------------
	// defaulted Copy Constructor/Assignment 에서 제외할 멤버 변수용
	template < typename T >
	class non_copyable_member : public T {
	public:
		using base_t = T;

		using T::T;
		//non_copyable_member() : T() {}
		non_copyable_member(const non_copyable_member& B) { }
		non_copyable_member(non_copyable_member&& B) = default;
		non_copyable_member& operator = (const non_copyable_member& B) { return *this; }
		non_copyable_member& operator = (non_copyable_member&& B) = default;
	};


	////-----------------------------------------------------------------------------
	//// Auto Copy Constructor 에서 제외, Movable 포함 멤버 변수용
	//template < typename T >
	//class non_copyable_movable_member : public T {
	//public:
	//	using T::T;
	//	non_copyable_movable_member(const non_copyable_movable_member& B) {};
	//	//non_copyable_movable_member(non_copyable_movable_member&& B) = default;
	//	//non_copyable_movable_member& operator = (non_copyable_movable_member&& B) = default;
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


	//-----------------------------------------------------------------------------
	// Clean Up
	//
	// ex 
	//
	//		int* pValue = new int(3);
	//		{
	//			XCleaner cleanup([&]() { delete pValue; pValue = nullptr; });
	//			...
	//		}// 블럭 나가면서 pValue 삭제.
	//
	template < typename tfcleaner >
	class TCleaner {
	public:
		tfcleaner m_cleaner;
	public:
		~TCleaner() { if (m_cleaner) m_cleaner(); }

	public:
		void CleanUp() { if (m_cleaner) m_cleaner(); m_cleaner = nullptr; }
	};


	/// @brief RAI helper
	struct CTrigger {
		std::function<void()> m_cleaner;
		CTrigger(std::function<void()> cleaner) : m_cleaner(cleaner) {}
		~CTrigger() {
			if (m_cleaner)
				m_cleaner();
		}
		void Trigger() {
			if (m_cleaner)
				m_cleaner();
			m_cleaner = nullptr;
		}
	};




	//-------------------------------------------------------------------------
	// Variable Init
	template < gtlc::trivially_copyable T >
	constexpr inline void ZeroVar(T& var) { var = T{}; }


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
	constexpr [[nodiscard]] T_DEST Round(T_SOURCE v, T_SOURCE place) {
		//return T_DEST(T_DEST(std::round(v/place/10))*place*10);
		return T_DEST(T_DEST(v/place/10+0.5*(v<T_SOURCE{}?-1:1))*place*10);
	}

	/// @brief Round or just forward values. (실수 -> 정수 대입시 반올림. (실수 -> 실수) 또는 (정수 -> 정수) 일 경우에는 값의 변화 없이 그대로 대입.)
	/// @tparam T_DEST 
	/// @tparam T_SOURCE 
	/// @param v2 
	/// @return 
	template < typename T_DEST, typename T_SOURCE >
	constexpr [[nodiscard]] T_DEST RoundOrForward(T_SOURCE v2) {
		if constexpr (std::is_integral_v<T_DEST> && std::is_floating_point_v<T_SOURCE>) {
			return Round<T_DEST, T_SOURCE>(v2);
		} else {
			return (T_DEST)v2;
		}
	}

	// Ceil
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
	constexpr [[nodiscard]] T_DEST Ceil(T_SOURCE v) {
		return T_DEST(std::ceil(v));	// std::ceil is not constexpr yet.
	}
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
	constexpr [[nodiscard]] T_DEST Ceil(T_SOURCE v, T_SOURCE place) {
		return T_DEST(std::ceil(v/place/10)*place*10);
	}
	// 실수 -> 정수 대입시 반올림. (실수 -> 실수) 또는 (정수 -> 정수) 일 경우에는 값의 변화 없이 그대로 대입.
	template < typename T_DEST, typename T_SOURCE >
	constexpr [[nodiscard]] T_DEST CeilOrForward(T_SOURCE v2) {
		if constexpr (std::is_integral_v<T_DEST> && std::is_floating_point_v<T_SOURCE>) {
			return Ceil<T_DEST, T_SOURCE>(v2);
		} else {
			return (T_DEST)v2;
		}
	}

	// Floor
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
	constexpr [[nodiscard]] T_DEST Floor(T_SOURCE v) {
		return T_DEST(std::floor(v));	// std::floor is not constexpr yet.
	}
	template < typename T_DEST = std::int32_t, typename T_SOURCE = double >
	constexpr [[nodiscard]] T_DEST Floor(T_SOURCE v, T_SOURCE place) {
		return T_DEST(std::floor(v/place/10)*place*10);
	}
	// 실수 -> 정수 대입시 반올림. (실수 -> 실수) 또는 (정수 -> 정수) 일 경우에는 값의 변화 없이 그대로 대입.
	template < typename T_DEST, typename T_SOURCE >
	constexpr [[nodiscard]] T_DEST FloorOrForward(T_SOURCE v2) {
		if constexpr (std::is_integral_v<T_DEST> && std::is_floating_point_v<T_SOURCE>) {
			return Floor<T_DEST, T_SOURCE>(v2);
		} else {
			return (T_DEST)v2;
		}
	}

	// Boolean
	template < typename ... Args > constexpr bool IsAllTrue(Args&& ... args)						{ return (args && ...); }
	template < typename ... Args > constexpr bool IsNoneTrue(Args&& ... args)						{ return (!args && ...); }
	template < typename ... Args > constexpr bool IsAllFalse(Args&& ... args)						{ return ((false == args) && ...); }
	template < typename ... Args > constexpr bool IsAnyTrue(Args&& ... args)						{ return (args ||...); }
	template < typename ... Args > constexpr bool IsAllSame(Args&& ... args)						{ return IsAllTrue(args...) || IsNoneTrue(args...); }

	template < typename T, typename ... Args > constexpr bool IsValueOneOf(T v, Args&& ... args)	{ return ((v == args) || ...); }
	template < typename T, typename ... Args > constexpr bool IsValueNoneOf(T v, Args&& ... args)	{ return ((v != args) && ...); }

	// Bit Set
	namespace internal {
		template < typename T >						constexpr T bit_single(int b)					{ return ((T)1 << b); }
		template < typename T, typename ... Bits >	constexpr T bit_multi(Bits ... bits)			{ return (bit_single<T>(bits) | ...); }
	}
	template < typename ... Bits >				constexpr std::uint32_t BIT32(Bits ... bits)		{ return internal::bit_multi<std::uint32_t>(bits...); }
	template < typename ... Bits >				constexpr std::uint64_t BIT64(Bits ... bits)		{ return internal::bit_multi<std::uint64_t>(bits...); }

	template < typename ... Bits >				constexpr std::bitset<32> BITSET32(Bits ... bits)	{ return std::bitset<32>(bit32(bits...)); }
	template < typename ... Bits >				constexpr std::bitset<64> BITSET64(Bits ... bits)	{ return std::bitset<64>(bit64(bits...)); }


	// Word Align Position
	template < std::integral T > constexpr inline T AdjustAlign128(T w) { return ((w+15)/16*16); }	//	((w+15)>>4)<<4
	template < std::integral T > constexpr inline T AdjustAlign64(T w) { return ((w+7)/8*8); }		//	((w+ 7)>>3)<<3
	template < std::integral T > constexpr inline T AdjustAlign32(T w) { return ((w+3)/4*4); }		//	((w+ 3)>>2)<<1
	template < std::integral T > constexpr inline T AdjustAlign16(T w) { return ((w+1)/2*2); }		//	((w+ 1)>>1)<<1


	// byte swap
#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY)
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
			static_assert(false, "not supported data type.");
		}
	}
	template < std::integral type >
	inline void ByteSwap(type& v) {
		v = GetByteSwap(v);
	}
#else
	template < std::integral type >
	[[nodiscard]] inline type GetByteSwap(type const v) {
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
	inline void ByteSwap(type& v) {
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
	/// @brief StopWatch
	/// @tparam tclock 
	template < typename tchar, class ttraits = std::char_traits<tchar>, typename tclock = std::chrono::steady_clock >
	class TStopWatch {
	protected:
		tclock::time_point t0 { tclock::now() };
		std::basic_ostream<tchar>& os;
		thread_local inline static int depth {-1};

		//struct item {
		//	tclock::time_point t0;
		//	tclock::time_point t1;
		//	std::string str;
		//};
		//std::deque<item> laps;
		constexpr static auto& GetDefaultOutStream() {
			if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				return std::cout;
			} else if constexpr (gtlc::is_one_of<tchar, wchar_t, char16_t>) {
				return std::wcout;
			}
		}

	public:
		TStopWatch(std::basic_ostream<tchar>& os = GetDefaultOutStream()) : os(os) {
			depth++;
		}
		~TStopWatch() {
			depth--;
		}

		tclock::time_point GetLastTick() const { return t0; };

	public:
		void Start() {
			t0 = tclock::now();
		}
		void Stop() {}

	public:
		template < typename ... Args >
		void Lap(Args&& ... args) {
			auto t = tclock::now();
			if constexpr (gtlc::is_one_of<tchar, char>) {
				os << std::format("STOP_WATCH - {0:{1}}{2} ", ' ', depth*4, std::chrono::duration_cast<std::chrono::milliseconds>(t-t0));
			} else if constexpr (gtlc::is_one_of<tchar, char8_t>) {
				os << std::format(u8"STOP_WATCH - {0:{1}}{2} ", ' ', depth*4, std::chrono::duration_cast<std::chrono::milliseconds>(t-t0));
			} else if constexpr (gtlc::is_one_of<tchar, char16_t>) {
				os << std::format(u"STOP_WATCH - {0:{1}}{2} ", ' ', depth*4, std::chrono::duration_cast<std::chrono::milliseconds>(t-t0));
			} else if constexpr (gtlc::is_one_of<tchar, char32_t>) {
				os << std::format(U"STOP_WATCH - {0:{1}}{2} ", ' ', depth*4, std::chrono::duration_cast<std::chrono::milliseconds>(t-t0));
			} else if constexpr (gtlc::is_one_of<tchar, wchar_t>) {
				os << std::format(L"STOP_WATCH - {0:{1}}{2} ", ' ', depth*4, std::chrono::duration_cast<std::chrono::milliseconds>(t-t0));
			} else {
				static_assert(false);
			}
			os << std::format(std::forward<Args>(args)...);
			os << (tchar)'\n';
			t0 = tclock::now();
		}
	};


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



#pragma pack(pop)
}	// namespac gtl
