//////////////////////////////////////////////////////////////////////
//
// basic_string.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__BASIC_STRING
#define GTL_HEADER__BASIC_STRING

#include <string>
#include <concepts>


#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"

#if !defined(__cpp_lib_concepts)
#	error ERROR! Supports C++v20 only.
#endif

#ifndef _t
#	ifdef _UNICODE
#		define _t(x) _u(x)
#	else
#		define _t(x) x
#	endif
#endif

//-----------------------------------------------------------------------------
// Text Constant macros
#define _A(x) x
#define ___WIDE_TEXT(x) L##x
#define _W(x) ___WIDE_TEXT(x)
#define ___UTF8_TEXT(x) u8##x
#define _u8(x) ___UTF8_TEXT(x)
#define ___UTF16_TEXT(x) u##x
#define _u(x) ___UTF16_TEXT(x)
#define ___UTF32_TEXT(x) U##x
#define _U(x) ___UTF32_TEXT(x)

#define _EOL			"\r\n"
#define EOL				_t(_EOL)
#define EOLA			_A(_EOL)
#define EOLW			_W(_EOL)
#define EOLu8			_u8(_EOL)
#define SPACE_STRING	" \t\r\n"

namespace gtl {
#pragma pack(push, 8)

//#ifdef _UNICODE
//	using TCHAR = char16_t;
//#else
//	using TCHAR = char;
//#endif

	//-----------------------------------------------------------------------------
	/// @brief pre-defines : basic_string_t
	template < gtlc::string_elem tchar_t, class _Traits = std::char_traits<tchar_t>, class _Alloc = std::allocator<tchar_t> >
	using basic_string_t = std::basic_string<tchar_t, _Traits, _Alloc>;

	//-----------------------------------------------------------------------------
	/// @brief  misc. GetSpaceString()
	/// @return " \r\t\n"
	template < gtlc::string_elem tchar_t > [[nodiscard]] constexpr const tchar_t* GetSpaceString() {
		if constexpr (std::is_same_v<tchar_t, char>) { return _A(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar_t, wchar_t>) { return _W(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar_t, char8_t>) { return _u8(SPACE_STRING); }
		else { static_assert(false, "tchar_t must be one of (char, char8_t, wchar_t) !"); }
	}
	[[nodiscard]] GTL_API constexpr const char*		GetSpaceString(const char*)		{ return GetSpaceString<char>(); }
	[[nodiscard]] GTL_API constexpr const wchar_t*	GetSpaceString(const wchar_t*)	{ return GetSpaceString<wchar_t>(); }
	[[nodiscard]] GTL_API constexpr const char8_t*	GetSpaceString(const char8_t*)	{ return GetSpaceString<char8_t>(); }
	[[nodiscard]] GTL_API constexpr const char*		GetSpaceStringA()				{ return GetSpaceString<char>(); }
	[[nodiscard]] GTL_API constexpr const wchar_t*	GetSpaceStringW()				{ return GetSpaceString<wchar_t>(); }
	[[nodiscard]] GTL_API constexpr const char8_t*	GetSpaceStringU8()				{ return GetSpaceString<char8_t>(); }

	constexpr static inline bool is_space(int const c) { return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' '); }


	//-----------------------------------------------------------------------------
	/// @brief TrimLeft, TrimRight, Trim
	/// @tparam tchar_t 
	/// @param str : string
	/// @param pszTrim : chars to trim
	template < gtlc::string_elem tchar_t > void TrimRight(basic_string_t<tchar_t>& str, const tchar_t* pszTrim);
	template < gtlc::string_elem tchar_t > void TrimLeft(basic_string_t<tchar_t>& str, const tchar_t* pszTrim);
	template < gtlc::string_elem tchar_t > void Trim(basic_string_t<tchar_t>& str, const tchar_t* pszTrim);
	template < gtlc::string_elem tchar_t > void TrimRight(basic_string_t<tchar_t>& str, const tchar_t cTrim);
	template < gtlc::string_elem tchar_t > void TrimLeft(basic_string_t<tchar_t>& str, const tchar_t cTrim);
	template < gtlc::string_elem tchar_t > void Trim(basic_string_t<tchar_t>& str, const tchar_t cTrim);
	template < gtlc::string_elem tchar_t > void TrimLeft(basic_string_t<tchar_t>& str)	{ TrimLeft(str, GetSpaceString<tchar_t>()); }
	template < gtlc::string_elem tchar_t > void TrimRight(basic_string_t<tchar_t>& str)	{ TrimRight(str, GetSpaceString<tchar_t>()); }
	template < gtlc::string_elem tchar_t > void Trim(basic_string_t<tchar_t>& str)		{ Trim(str, GetSpaceString<tchar_t>()); }

	template < gtlc::string_elem tchar_t > void TrimRight(basic_string_t<tchar_t>& str, const tchar_t* pszTrim) {
		str.erase(str.begin() + (str.find_last_not_of(pszTrim) + 1), str.end());
	}
	template < gtlc::string_elem tchar_t > void TrimLeft(basic_string_t<tchar_t>& str, const tchar_t* pszTrim) {
		auto pos = str.find_first_not_of(pszTrim);
		if (pos == str.npos)
			str.clear();
		else
			str.erase(str.begin(), str.begin()+pos);
	}
	template < gtlc::string_elem tchar_t > void Trim(basic_string_t<tchar_t>& str, const tchar_t* pszTrim) {
		TrimRight(str, pszTrim);
		TrimLeft(str, pszTrim);
	}

	template < gtlc::string_elem tchar_t > void TrimRight(basic_string_t<tchar_t>& str, const tchar_t cTrim) {
		str.erase(str.begin() + (str.find_last_not_of(cTrim) + 1), str.end());
	}
	template < gtlc::string_elem tchar_t > void TrimLeft(basic_string_t<tchar_t>& str, const tchar_t cTrim) {
		auto pos = str.find_first_not_of(cTrim);
		if (pos == str.npos)
			str.clear();
		else
			str.erase(str.begin(), str.begin()+pos);
	}
	template < gtlc::string_elem tchar_t > void Trim(basic_string_t<tchar_t>& str, const tchar_t cTrim) {
		TrimRight(str, cTrim);
		TrimLeft(str, cTrim);
	}


	/// @brief ToLower, ToUpper, ToDigit, IsSpace ... (locale irrelavant)
	inline constexpr int ToLower(int c/* Locale Irrelavant */) { if ((c >= 'A') && (c <= 'Z')) return c - 'A' + 'a'; return c; }
	inline constexpr int ToUpper(int c/* Locale Irrelavant */) { if ((c >= 'a') && (c <= 'z')) return c - 'a' + 'A'; return c; }
	inline constexpr int IsDigit(int const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '9'); }
	inline constexpr int IsOdigit(int const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '7'); }
	inline constexpr int IsXdigit(int const c/* Locale Irrelavant */) { return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')); }
	inline constexpr int IsSpace(int const c/* Locale Irrelavant */) { return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' '); }
	inline constexpr int IsNotSpace(int const c/* Locale Irrelavant */) { return !IsSpace(c); }


#if 1
	/// @brief  tszlen (string length)
	/// @param psz : string
	/// @return string length
	template < gtlc::string_elem tchar_t >
	constexpr size_t tszlen(tchar_t const* psz) {
		if (!psz) return 0;
		tchar_t const pos = psz;
		while (*pos) { pos++; }
		return pos-psz;
	}


	// todo : documents...

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pszDest"></param>
	/// <param name="size"></param>
	/// <param name="pszSrc"></param>
	/// <returns></returns>
	template < gtlc::string_elem tchar_t >
	errno_t tszcpy(tchar_t* pszDest, size_t size, tchar_t const* pszSrc) {
		if (!pszDest)
			return EINVAL;
		if (size <= 0)
			return ERANGE;
		if (!pszSrc) {
			size = 0;
			return EINVAL;
		}
		for (int i = 0; i < size; i++) {
			pszDest[i] = pszSrc[i];
			if (!*pszSrc)
				return 0;
		}
		pszDest[0] = 0;
		return ERANGE;
	}

	template < gtlc::string_elem tchar_t, int size >
	errno_t tszcpy(std::array<tchar_t, size>& szDest, tchar_t const* pszSrc) {
		return tszcpy(szDest.data(), size, pszSrc);
	}

	template < gtlc::string_elem tchar_t, int size >
	errno_t tszcpy(tchar_t (&szDest)[size], tchar_t const* pszSrc) {
		return tszcpy(szDest, size, pszSrc);
	}

	template < gtlc::string_elem tchar_t >
	errno_t tszncpy(tchar_t* pszDest, size_t size, tchar_t const* pszSrc, size_t nLen = _TRUNCATE) {
		if (!pszDest)
			return EINVAL;
		if (size <= 0)
			return ERANGE;
		if (!pszSrc) {
			size = 0;
			return EINVAL;
		}
		auto n = (nLen == _TRUNCATE) ? (size-1) : (std::min(size-1, nLen));
		decltype(size) i {};
		for (; i < n; i++) {
			pszDest[i] = pszSrc[i];
			if (!*pszSrc)
				return 0;
		}
		if (i < size-1) {
			pszDest[i] = 0;
			return 0;
		}
		pszDest[0] = 0;
		return ERANGE;
		//return strncpy_s(pszDest, size, pszSrc, nLen);
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszncpy(tchar_t (&szDest)[size], tchar_t const* pszSrc, size_t nLen = _TRUNCATE) {
		return tszncpy(szDest, size, pszSrc, nLen);
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszncpy(std::array<tchar_t, size>& szDest, tchar_t const* pszSrc, size_t nLen = _TRUNCATE) {
		return tszncpy(szDest.data(), size, pszSrc, nLen);
	}

	template < gtlc::string_elem tchar_t >
	errno_t tszcat(tchar_t* pszDest, size_t size, tchar_t const* pszSrc) {
		if (!pszDest || (size <= 0) || (size > RSIZE_MAX) )
			return EINVAL;
		if (!pszSrc) {
			if (size && (size <= RSIZE_MAX))
				pszDest[0] = 0;
			return EINVAL;
		}
		auto const* const pszEnd = pszDest + size;
		for (; pszDest < pszEnd; pszDest++, pszSrc++) {
			*pszDest = *pszSrc;
			if (!*pszSrc)
				return 0;
		}
		// pszSrc is longer
		if (size && (size <= RSIZE_MAX))
			pszDest[0] = 0;
		return ERANGE;
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszcat(tchar_t (&szDest)[size], tchar_t const* pszSrc) {
		return tszcat(szDest, size, pszSrc);
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszcat(std::array<tchar_t, size>& szDest, tchar_t const* pszSrc) {
		return tszcat(szDest.data(), size, pszSrc);
	}
	template < gtlc::string_elem tchar_t >
	int tszcmp(tchar_t const* pszA, tchar_t const* pszB) {
		if (!pszA && !pszB)	// if both are nullptr, return 0;
			return 0;
		if (pszA && !pszB)	// if only one has value, its bigger.
			return *pszA;
		else if (!pszA && pszB)
			return *pszB;

		for (; !*pszA || !*pszB; pszA++, pszB++) {
			auto r = *pszA - *pszB;
			if (!r)
				return r;
		}
		if (!*pszA && !*pszB)
			return 0;
		auto r = *pszA - *pszB;
		return r;
	}

	template < gtlc::string_elem tchar_t >
	int tszncmp(tchar_t const* pszA, tchar_t const* pszB, size_t nLen) {
		if (!pszA && !pszB)
			return 0;
		if (pszA && !pszB)
			return *pszA;
		else if (!pszA && pszB)
			return *pszB;

		if (!nLen)
			return 0;

		for (; !*pszA || !*pszB; pszA++, pszB++) {
			auto r = *pszA - *pszB;
			if (!r)
				return r;
			if (--nLen == 0)
				return 0;
		}
		if (!*pszA && !*pszB)
			return 0;
		auto r = *pszA - *pszB;
		return r;
	}
	template < gtlc::string_elem tchar_t >
	int tszicmp(tchar_t const* pszA, tchar_t const* pszB) {
		if (!pszA && !pszB)	// if both are nullptr, return 0;
			return 0;
		if (pszA && !pszB)	// if only one has value, its bigger.
			return *pszA;
		else if (!pszA && pszB)
			return *pszB;

		for (; !*pszA || !*pszB; pszA++, pszB++) {
			auto r = ToLower(*pszA) - ToLower(*pszB);
			if (!r)
				return r;
		}
		if (!*pszA && !*pszB)
			return 0;
		auto r = *pszA - *pszB;
		return r;
	}
	template < gtlc::string_elem tchar_t >
	int tsznicmp(tchar_t const* pszA, tchar_t const* pszB, size_t nLen) {
		if (!pszA && !pszB)
			return 0;
		if (pszA && !pszB)
			return *pszA;
		else if (!pszA && pszB)
			return *pszB;

		if (!nLen)
			return 0;

		for (; !*pszA || !*pszB; pszA++, pszB++) {
			auto r = ToLower(*pszA) - ToLower(*pszB);
			if (!r)
				return r;
			if (--nLen == 0)
				return 0;
		}
		if (!*pszA && !*pszB)
			return 0;
		auto r = *pszA - *pszB;
		return r;
	}

	template < gtlc::string_elem tchar_t >
	errno_t tszupr(tchar_t* sz, size_t nLen) {
		if (!sz)
			return EINVAL;
		for (size_t i = 0; i < nLen; i++) {
			auto& c = *sz++;
			if (!c)
				break;
			c = ToUpper(c);
		}
		return 0;
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszupr(tchar_t (&sz)[size]) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToUpper(c);
		}
		return 0;
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszupr(std::array<tchar_t, size>& sz) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToUpper(c);
		}
		return 0;
	}

	template < gtlc::string_elem tchar_t >
	errno_t tszlwr(tchar_t* sz, size_t nLen) {
		if (!sz)
			return EINVAL;
		for (size_t i = 0; i < nLen; i++) {
			auto& c = *sz++;
			if (!c)
				break;
			c = ToLower(c);
		}
		return 0;
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszlwr(tchar_t(&sz)[size]) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToLower(c);
		}
		return 0;
	}
	template < int size, gtlc::string_elem tchar_t >
	errno_t tszlwr(std::array<tchar_t, size>& sz) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToLower(c);
		}
		return 0;
	}

	template < gtlc::string_elem tchar_t >
	tchar_t* tszrev(tchar_t* psz) {
		if (!psz)
			return nullptr;
		std::reverse(psz, psz + tszlen(psz));
	}
	template < gtlc::string_elem tchar_t >
	tchar_t const* tszsearch(tchar_t const* psz, tchar_t c) {
		if (!psz)
			return nullptr;
		auto const* const end = psz + tszlen(psz);
		auto const* p = std::search(psz, end, c);
		if (p == end)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar_t >
	tchar_t* tszsearch(tchar_t* psz, tchar_t c) {
		if (!psz)
			return nullptr;
		auto* const end = psz + tszlen(psz);
		auto* p = std::search(psz, end, c);
		if (p == end)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar_t >
	tchar_t const* tszsearch(tchar_t const* psz, tchar_t const* pszSub) {
		if (!psz || !pszSub)
			return nullptr;
		auto const* const end = psz + tszlen(psz);
		auto const* const endSub = pszSub + tszlen(pszSub);
		auto const* p = std::search(psz, end, pszSub, endSub);
		if (p == end)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar_t >
	tchar_t* tszsearch(tchar_t* psz, tchar_t* pszSub) {
		if (!psz || !pszSub)
			return nullptr;
		auto* const end = psz + tszlen(psz);
		auto* const endSub = pszSub + tszlen(pszSub);
		auto* p = std::search(psz, end, pszSub, endSub);
		if (p == end)
			return nullptr;
		return p;
	}

	namespace internal {
		template < typename tvalue, int tsize >
		struct digit_table {
			tvalue tbl[tsize];
			consteval digit_table() {
				for (int i = 0; i < tsize; i++) tbl[i] = -1;
				for (int i = 0; i < 10; i++) tbl[i + '0'] = i;
				for (int i = 0; i < 'Z' - 'A' + 1; i++) tbl[i + 'A'] = i + 10;
				for (int i = 0; i < 'z' - 'a' + 1; i++) tbl[i + 'a'] = i + 10;
			}
			consteval size_t size() const { return tsize; }
			constexpr tvalue operator[] (int i) const {
				return tbl[i];
			}
		};
	}

	/// <summary>
	/// digit contants to integral type value.
	///  - radix detecting (c++ notation)
	///	   ex "0b1001'1100", "0xABCD1234", "0b1234568"
	///  - digit seperator (such as ',' or '\'' for c++ notation.)
	///  - only throws when (radix > 36)
	/// </summary>
	/// <param name="psz"></param>
	/// <param name="pszEnd"></param>
	/// <param name="ppszStopped">where conversion stopped</param>
	/// <param name="radix">radix</param>
	/// <param name="cSplitter">digit splitter. such as ',' (thousand sepperator) or '\'' (like c++v14 notation)</param>
	/// <returns>number value. (no overflow checked)</returns>
	template < std::integral tvalue_t = int, gtlc::string_elem tchar_t >
	tvalue_t _tsztoi(tchar_t const* psz, tchar_t const* pszEnd, tchar_t** ppszStopped = nullptr, int radix = 0, tchar_t cSplitter = 0) {
		if (!psz)
			return {};

		// skip white space
		while ((psz < pszEnd) && is_space(*psz))
			psz++;

		if (psz >= pszEnd)
			return {};

		// Check sign (+/-)
		bool bMinus{};
		if (*psz == '-')
			psz++, bMinus = true;
		else if (*psz == '+')
			psz++;

		// skip white space
		while ((psz < pszEnd) && is_space(*psz))
			psz++;

		// skip white space
		if (radix == 0) {
			radix = 10;
			if ( (psz[0] == '0') && (psz+1 < pszEnd) ) {
				auto const s = psz[1];
				if ('b' == s || 'B' == s) {
					psz += 2;
					radix = 2;
				}
				else if (('x' == s) || ('X' == s)) {
					psz += 2;
					radix = 16;
				}
				else if (('0' <= s) && (s < '8')) {
					psz += 1;
					radix = 8;
				}
			}
		}
		else if (radix > 10+('z'-'a'+1))
			throw std::invalid_argument{ GTL__FUNCSIG "wrong radix" };

		tvalue_t value{};
		constexpr static auto const tbl = internal::digit_table<uint8_t, 256>{};

		for (; (psz < pszEnd) && *psz; psz++) {
			if (cSplitter == *psz)
				continue;
			auto c = *psz;
			if constexpr (sizeof(tchar_t) > sizeof(uint8_t)) {
				if (c > 'z')
					break;
			}
			auto v = tbl[c];
			if (v >= radix)
				break;
			value = value*radix + v;	// no overflow-check
		}
		if (bMinus)
			value = -value;

		if (ppszStopped)
			*ppszStopped = const_cast<tchar_t*>(psz);

		return value;
	}

	template < std::integral tvalue_t = int, gtlc::string_elem tchar_t = char16_t >
	inline tvalue_t tsztoi(std::basic_string_view<tchar_t> sv, tchar_t** ppszStopped = nullptr, int radix = 0, tchar_t cSplitter = 0) {
		return _tsztoi<tvalue_t, tchar_t>(sv.data(), sv.data() + sv.size(), ppszStopped, radix, cSplitter);
	}
	//template < std::integral tvalue_t = int, gtlc::string_elem tchar_t = char16_t >
	//inline tvalue_t tsztoi(std::basic_string<tchar_t> const& str, tchar_t** ppszStopped = nullptr, int radix = 0, tchar_t cSplitter = 0) {
	//	return _tsztoi<tvalue_t, tchar_t>(str.data(), str.data() + str.size(), ppszStopped, radix, cSplitter);
	//}

	template < std::floating_point tvalue_t = double, gtlc::string_elem tchar_t = char16_t >
	[[deprecated]] constexpr tvalue_t _tsztod(tchar_t const* psz, tchar_t const* pszEnd, tchar_t** ppszStopped = nullptr, tchar_t cSplitter = 0) {
		if (!psz)
			return {};

		using namespace ctre::literals;

		// skip white space
		while ((psz < pszEnd) && is_space(*psz))
			psz++;

		// Check sign (+/-)
		bool bMinus{};
		if (*psz == '-')
			psz++, bMinus = true;
		else if (*psz == '+')
			psz++;

		tvalue_t value{};
		constexpr static auto const tbl = internal::digit_table<uint8_t, 256>{};

		// mantissa
		for (; (psz < pszEnd) && *psz; psz++) {
			if (cSplitter == *psz)
				continue;
			auto c{ *psz };
			if constexpr (sizeof(tchar_t) > sizeof(uint8_t)) {
				if (c > 'z')
					break;
			}
			auto v = tbl[c];
			if (v >= 10)
				break;
			value = value * 10. + v;	// no overflow-check
		}
		// below .
		if ( (psz < pszEnd) && (*psz == '.') ) {
			psz++;
			for (int i = 1; (psz < pszEnd) && *psz; psz++, i++) {
				if (cSplitter == *psz)
					continue;
				auto c{ *psz };
				if constexpr (sizeof(tchar_t) > sizeof(uint8_t)) {
					if (c > 'z')
						break;
				}
				double v = tbl[c];
				if (v >= 10)
					break;
				value += v * std::pow(0.1, i);
			}
		}
		// exponent
		if ((psz+1 < pszEnd) && ((psz[0] == 'e') || (psz[0] == 'E'))
			&& ( IsDigit(psz[1]) || ( (psz+2 < pszEnd) && (psz[1] == '-') && IsDigit(psz[2]) ) )
			)
		{
			psz++;
			tchar_t* ppszStopped_local{};
			int e = tsztoi(psz, pszEnd, &ppszStopped_local, 10, cSplitter);
			psz = ppszStopped_local;
			if (e != 0)
				value *= std::pow(10, e);
		}

		if (bMinus)
			value = -value;

		if (ppszStopped)
			*ppszStopped = const_cast<tchar_t*>(psz);

		return value;
	}


	template < std::floating_point tvalue_t = double, gtlc::string_elem tchar_t = char16_t>
	inline tvalue_t tsztod(std::basic_string_view<tchar_t> sv, tchar_t** ppszStopped = nullptr) {
		if constexpr (sizeof(tchar_t) == sizeof(char)) {
			tvalue_t value;
			auto [ptr, ec] = std::from_chars(sv.data(), sv.data()+sv.size(), value, std::chars_format::general);
			if (ppszStopped)
				*ppszStopped = const_cast<tchar_t*>(ptr);
			return value;
		}
		else {
			std::string str;
			str.reserve(16);
			tchar_t const* pos = sv.data();
			tchar_t const* end = sv.data() + sv.size();
			while (pos < end && is_space(*pos))
				pos++;
			for (; pos < end; pos++) {
				static_assert(std::is_unsigned_v<tchar_t>);
				if (*pos > 127 || !std::strchr("+-.0123456789eE", *pos))
					break;
				str += *pos;
			}
			return tsztod<tvalue_t, char>(str, ppszStopped);
		}
	}

	//template < std::floating_point tvalue_t = double, gtlc::string_elem tchar_t = char16_t >
	//inline tvalue_t tsztod(std::basic_string_view<tchar_t> sv, tchar_t** ppszStopped = nullptr) {
	//	return _tsztod<tvalue_t, tchar_t>(sv.data(), sv.data() + sv.size(), ppszStopped, cSplitter);
	//}
	//template < gtlc::string_elem tchar_t, std::floating_point tvalue_t = double >
	//inline double tsztod(std::basic_string<tchar_t> const& str, tchar_t** ppszStopped = nullptr) {
	//	return _tsztod<tvalue_t, tchar_t>(str.data(), str.data() + str.size(), ppszStopped, cSplitter);
	//}


#else

	//-- char
	//template <>				size_t			tszlen<char>(const char* psz)													{ return std::strlen(psz); }
	template < int size >	errno_t			tszcpy(char (&szDest)[size], const char* pszSrc)								{ return strcpy_s(szDest, pszSrc); }
	inline					errno_t			tszcpy(char* pszDest, size_t size, const char* pszSrc)							{ return strcpy_s(pszDest, size, pszSrc); }
	template < int size >	errno_t			tszncpy(char (&szDest)[size], const char* pszSrc, size_t nLen)					{ return strncpy_s(szDest, pszSrc, nLen); }
	inline					errno_t			tszncpy(char* pszDest, size_t size, const char* pszSrc, size_t nLen)			{ return strncpy_s(pszDest, size, pszSrc, nLen); }
	template < int size >	errno_t			tszcat(char (&szDest)[size], const char* pszSrc)								{ return strcat_s(szDest, pszSrc); }
	inline					errno_t			tszcat(char* pszDest, size_t size, const char* pszSrc)							{ return strcat_s(pszDest, size, pszSrc); }
	inline					errno_t			tszcmp(const char* pszA, const char* pszB)										{ return strcmp(pszA, pszB); }
	inline					errno_t			tszncmp(const char* pszA, const char* pszB, size_t nLen)						{ return strncmp(pszA, pszB, nLen); }
	inline					errno_t			tszicmp(const char* pszA, const char* pszB)										{ return _stricmp(pszA, pszB); }
	inline					errno_t			tsznicmp(const char* pszA, const char* pszB, size_t nLen)						{ return _strnicmp(pszA, pszB, nLen); }
	template < int size >	errno_t			tszupr(char (&sz)[size])														{ return _strupr_s(sz); }
	inline					errno_t			tszupr(char* psz, size_t size)													{ return _strupr_s(psz, size); }
	template < int size >	errno_t			tszlwr(char (&sz)[size])														{ return _strlwr_s(sz); }
	inline					errno_t			tszlwr(char* psz, int size)														{ return _strlwr_s(psz, size); }
	inline					char*			tszrev(char* psz)																{ return _strrev(psz); }
	inline					char*			tszsearch(char* const psz, int c)												{ return strchr(psz, c); }
	inline					char*			tszsearch(char* const psz, const char* const pszSub)							{ return strstr(psz, pszSub); }
	inline					const char*		tszsearch(const char* const psz, int c)											{ return strchr(psz, c); }
	inline					const char*		tszsearch(const char* const psz, const char* const pszSub)						{ return strstr(psz, pszSub); }
	inline					int				tsztoi(const char* psz)															{ return atoi(psz); }
	inline					int32_t			tsztol(const char* psz,    char const** ppszEnd = nullptr,    int radix = 0)	{ return strtol(psz, (char**)ppszEnd, radix); }
	inline					uint32_t		tsztoul(const char* psz,    char const** ppszEnd = nullptr,    int radix = 0)	{ return strtoul(psz, (char**)ppszEnd, radix); }
	inline					int64_t			tsztoi64(const char* psz,    char const** ppszEnd = nullptr,    int radix = 0)	{ return _strtoi64(psz, (char**)ppszEnd, radix); }
	inline					uint64_t		tsztoui64(const char* psz,    char const** ppszEnd = nullptr,    int radix = 0)	{ return _strtoui64(psz, (char**)ppszEnd, radix); }
	inline					double			tsztod(const char* psz,    char const** ppszEnd = nullptr)						{ return strtod(psz, (char**)ppszEnd); }

	//-- wchar_t
	//template<>				size_t			tszlen<wchar_t>(const wchar_t* psz)												{ return wcslen(psz); }
	template < int size >	errno_t			tszcpy(wchar_t (&szDest)[size], const wchar_t* pszSrc)							{ return wcscpy_s(szDest, pszSrc); }
	inline					errno_t			tszcpy(wchar_t* pszDest, size_t size, const wchar_t* pszSrc)					{ return wcscpy_s(pszDest, size, pszSrc); }
	template < int size >	errno_t			tszncpy(wchar_t (&szDest)[size], const wchar_t* pszSrc, size_t nLen)			{ return wcsncpy_s(szDest, pszSrc, nLen); }
	inline					errno_t			tszncpy(wchar_t* pszDest, size_t size, const wchar_t* pszSrc, size_t nLen)		{ return wcsncpy_s(pszDest, size, pszSrc, nLen); }
	template < int size >	errno_t			tszcat(wchar_t (&szDest)[size], const wchar_t* pszSrc)							{ return wcscat_s(szDest, pszSrc); }
	inline					errno_t			tszcat(wchar_t* pszDest, size_t size, const wchar_t* pszSrc)					{ return wcscat_s(pszDest, size, pszSrc); }
	inline					errno_t			tszcmp(const wchar_t* pszA, const wchar_t* pszB)								{ return wcscmp(pszA, pszB); }
	inline					errno_t			tszncmp(const wchar_t* pszA, const wchar_t* pszB, size_t nLen)					{ return wcsncmp(pszA, pszB, nLen); }
	inline					errno_t			tszicmp(const wchar_t* pszA, const wchar_t* pszB)								{ return _wcsicmp(pszA, pszB); }
	inline					errno_t			tsznicmp(const wchar_t* pszA, const wchar_t* pszB, size_t nLen)					{ return _wcsnicmp(pszA, pszB, nLen); }
	template < int size >	errno_t			tszupr(wchar_t (&sz)[size])														{ return _wcsupr_s(sz); }
	inline					errno_t			tszupr(wchar_t* psz, size_t size)												{ return _wcsupr_s(psz, size); }
	template < int size >	errno_t			tszlwr(wchar_t (&sz)[size])														{ return _wcslwr_s(sz); }
	inline					errno_t			tszlwr(wchar_t* psz, int size)													{ return _wcslwr_s(psz, size); }
	inline					wchar_t*		tszrev(wchar_t* psz)															{ return _wcsrev(psz); }
	inline					wchar_t*		tszsearch(wchar_t* const psz, int c)											{ return wcschr(psz, c); }
	inline					wchar_t*		tszsearch(wchar_t* const psz, const wchar_t* const pszSub)						{ return wcsstr(psz, pszSub); }
	inline					const wchar_t*	tszsearch(const wchar_t* const psz, int c)										{ return wcschr(psz, c); }
	inline					const wchar_t*	tszsearch(const wchar_t* const psz, const wchar_t* const pszSub)				{ return wcsstr(psz, pszSub); }
	inline					int				tsztoi(const wchar_t* psz)														{ return wcstol(psz, nullptr, 10); }
	inline					int32_t			tsztol(const wchar_t* psz, wchar_t const ** ppszEnd = nullptr, int radix = 0)	{ return wcstol    (psz, (wchar_t**)ppszEnd, radix); }
	inline					uint32_t		tsztoul(const wchar_t* psz, wchar_t const ** ppszEnd = nullptr, int radix = 0)	{ return wcstoul   (psz, (wchar_t**)ppszEnd, radix); }
	inline					int64_t			tsztoi64(const wchar_t* psz, wchar_t const ** ppszEnd = nullptr, int radix = 0)	{ return _wcstoi64 (psz, (wchar_t**)ppszEnd, radix); }
	inline					uint64_t		tsztoui64(const wchar_t* psz, wchar_t const ** ppszEnd = nullptr, int radix = 0){ return _wcstoui64(psz, (wchar_t**)ppszEnd, radix); }
	inline					double			tsztod(const wchar_t* psz, wchar_t const ** ppszEnd = nullptr)					{ return wcstod    (psz, (wchar_t**)ppszEnd); }

	//-- utf8 (utf8 에 대해서는 동작 되는지 테스트 안해봄. 안되는 함수가 많을 것 같은데....)
	//template<>				size_t			tszlen<char8_t>(const char8_t* psz)												{ return strlen((const char*)psz); }
	template < int size >	errno_t			tszcpy(char8_t (&szDest)[size], const char8_t* pszSrc)							{ return strcpy_s(szDest, pszSrc); }
	inline					errno_t			tszcpy(char8_t* pszDest, size_t size, const char8_t* pszSrc)					{ return strcpy_s((char*)pszDest, size, (const char*)pszSrc); }
	template < int size >	errno_t			tszncpy(char8_t (&szDest)[size], const char8_t* pszSrc, size_t nLen)			{ return strncpy_s(szDest, pszSrc, nLen); }
	inline					errno_t			tszncpy(char8_t* pszDest, size_t size, const char8_t* pszSrc, size_t nLen)		{ return strncpy_s((char*)pszDest, size, (const char*)pszSrc, nLen); }
	template < int size >	errno_t			tszcat(char8_t (&szDest)[size], const char8_t* pszSrc)							{ return strcat_s(szDest, pszSrc); }
	inline					errno_t			tszcat(char8_t* pszDest, size_t size, const char8_t* pszSrc)					{ return strcat_s((char*)pszDest, size, (const char*)pszSrc); }
	inline					errno_t			tszcmp(const char8_t* pszA, const char8_t* pszB)								{ return strcmp((const char*)pszA, (const char*)pszB); }
	inline					errno_t			tszncmp(const char8_t* pszA, const char8_t* pszB, size_t nLen)					{ return strncmp((const char*)pszA, (const char*)pszB, nLen); }
	inline					errno_t			tszicmp(const char8_t* pszA, const char8_t* pszB)								{ return _stricmp((const char*)pszA, (const char*)pszB); }
	inline					errno_t			tsznicmp(const char8_t* pszA, const char8_t* pszB, size_t nLen)					{ return _strnicmp((const char*)pszA, (const char*)pszB, nLen); }
	template < int size >	errno_t			tszupr(char8_t (&sz)[size])														{ return _strupr_s(sz); }
	inline					errno_t			tszupr(char8_t* psz, size_t size)												{ return _strupr_s((char*)psz, size); }
	template < int size >	errno_t			tszlwr(char8_t (&sz)[size])														{ return _strlwr_s(sz); }
	inline					errno_t			tszlwr(char8_t* psz, int size)													{ return _strlwr_s((char*)psz, size); }
	inline					char8_t*		tszrev(char8_t* psz)															{ return (char8_t*)_strrev((char*)psz); }
	inline					char8_t*		tszsearch(char8_t* const psz, int c)											{ return (char8_t*)strchr((char*)psz, c); }
	inline					char8_t*		tszsearch(char8_t* const psz, const char8_t* const pszSub)						{ return (char8_t*)strstr((char*)psz, (const char*)pszSub); }
	inline					const char8_t*	tszsearch(const char8_t* const psz, int c)										{ return (const char8_t*)strchr((const char*)psz, c); }
	inline					const char8_t*	tszsearch(const char8_t* const psz, const char8_t* const pszSub)				{ return (const char8_t*)strstr((const char*)psz, (const char*)pszSub); }
	inline					int				tsztoi(const char8_t* psz)														{ return atoi((const char*)psz); }
	inline					int32_t			tsztol(const char8_t* psz, char8_t const** ppszEnd = nullptr, int radix = 0)	{ return strtol((const char*)psz, (char**)ppszEnd, radix); }
	inline					uint32_t		tsztoul(const char8_t* psz, char8_t const** ppszEnd = nullptr, int radix = 0)	{ return strtoul((const char*)psz, (char**)ppszEnd, radix); }
	inline					int64_t			tsztoi64(const char8_t* psz, char8_t const** ppszEnd = nullptr, int radix = 0)	{ return _strtoi64((const char*)psz, (char**)ppszEnd, radix); }
	inline					uint64_t		tsztoui64(const char8_t* psz, char8_t const** ppszEnd = nullptr, int radix = 0)	{ return _strtoui64((const char*)psz, (char**)ppszEnd, radix); }
	inline					double			tsztod(const char8_t* psz, char8_t const** ppszEnd = nullptr)					{ return strtod((const char*)psz, (char**)ppszEnd); }

	//-- char16_t
	//template<>				size_t			tszlen<char16_t>(const char16_t* psz)												{ return wcslen(psz); }
	template < int size >	errno_t			tszcpy(char16_t (&szDest)[size], const char16_t* pszSrc)						{ return wcscpy_s((wchar_t*)szDest, (wchar_t const*)pszSrc); }
	inline					errno_t			tszcpy(char16_t* pszDest, size_t size, const char16_t* pszSrc)					{ return wcscpy_s((wchar_t*)pszDest, size, (wchar_t const*)pszSrc); }
	template < int size >	errno_t			tszncpy(char16_t (&szDest)[size], const char16_t* pszSrc, size_t nLen)			{ return wcsncpy_s((wchar_t*)szDest, (wchar_t const*)pszSrc, nLen); }
	inline					errno_t			tszncpy(char16_t* pszDest, size_t size, const char16_t* pszSrc, size_t nLen)	{ return wcsncpy_s((wchar_t*)pszDest, size, (wchar_t const*)pszSrc, nLen); }
	template < int size >	errno_t			tszcat(char16_t (&szDest)[size], const char16_t* pszSrc)						{ using var_t = wchar_t[size]; return wcscat_s((var_t&)szDest, (wchar_t const*)pszSrc); }
	inline					errno_t			tszcat(char16_t* pszDest, size_t size, const char16_t* pszSrc)					{ return wcscat_s((wchar_t*)pszDest, size, (wchar_t const*)pszSrc); }
	inline					errno_t			tszcmp(const char16_t* pszA, const char16_t* pszB)								{ return wcscmp((wchar_t const*)pszA, (wchar_t const*)pszB); }
	inline					errno_t			tszncmp(const char16_t* pszA, const char16_t* pszB, size_t nLen)				{ return wcsncmp((wchar_t const*)pszA, (wchar_t const*)pszB, nLen); }
	inline					errno_t			tszicmp(const char16_t* pszA, const char16_t* pszB)								{ return _wcsicmp((wchar_t const*)pszA, (wchar_t const*)pszB); }
	inline					errno_t			tsznicmp(const char16_t* pszA, const char16_t* pszB, size_t nLen)				{ return _wcsnicmp((wchar_t const*)pszA, (wchar_t const*)pszB, nLen); }
	template < int size >	errno_t			tszupr(char16_t (&sz)[size])													{ using var_t = wchar_t[size]; return _wcsupr_s((var_t&)sz); }
	inline					errno_t			tszupr(char16_t* psz, size_t size)												{ return _wcsupr_s((wchar_t*)psz, size); }
	template < int size >	errno_t			tszlwr(char16_t (&sz)[size])													{ using var_t = wchar_t[size]; return _wcslwr_s((var_t&)sz); }
	inline					errno_t			tszlwr(char16_t* psz, int size)													{ return _wcslwr_s((wchar_t*)psz, size); }
	inline					char16_t*		tszrev(char16_t* psz)															{ return (char16_t*)_wcsrev((wchar_t*)psz); }
	inline					char16_t*		tszsearch(char16_t* const psz, int c)											{ return (char16_t*)wcschr((wchar_t*)psz, c); }
	inline					char16_t*		tszsearch(char16_t* const psz, const char16_t* const pszSub)					{ return (char16_t*)wcsstr((wchar_t const*)psz, (wchar_t const*)pszSub); }
	inline					const char16_t*	tszsearch(const char16_t* const psz, int c)										{ return (char16_t const*)wcschr((wchar_t const*)psz, c); }
	inline					const char16_t*	tszsearch(const char16_t* const psz, const char16_t* const pszSub)				{ return (char16_t const*)wcsstr((wchar_t const*)psz, (wchar_t const*)pszSub); }
	inline					int				tsztoi(const char16_t* psz)															{ return wcstol    ((wchar_t const*)psz, nullptr, 10); }
	inline					int32_t			tsztol(const char16_t* psz, char16_t const ** ppszEnd = nullptr, int radix = 0)		{ return wcstol    ((wchar_t const*)psz, (wchar_t**)ppszEnd, radix); }
	inline					uint32_t		tsztoul(const char16_t* psz, char16_t const ** ppszEnd = nullptr, int radix = 0)	{ return wcstoul   ((wchar_t const*)psz, (wchar_t**)ppszEnd, radix); }
	inline					int64_t			tsztoi64(const char16_t* psz, char16_t const ** ppszEnd = nullptr, int radix = 0)	{ return _wcstoi64 ((wchar_t const*)psz, (wchar_t**)ppszEnd, radix); }
	inline					uint64_t		tsztoui64(const char16_t* psz, char16_t const ** ppszEnd = nullptr, int radix = 0)	{ return _wcstoui64((wchar_t const*)psz, (wchar_t**)ppszEnd, radix); }
	inline					double			tsztod(const char16_t* psz, char16_t const ** ppszEnd = nullptr)					{ return wcstod    ((wchar_t const*)psz, (wchar_t**)ppszEnd); }
#endif

	template < gtlc::string_elem tchar_t >
	constexpr size_t tszrmchar(tchar_t* psz, int chRemove);	// Remove Charactor from str. returns str length

															/// @brief Compare strings (according to numbers)
	template < gtlc::string_elem tchar_t >
	constexpr int	CompareStringIncludingNumber(const tchar_t* pszA, const tchar_t* pszB, bool bIgnoreCase = false);
	inline					constexpr int	tdszcmp(const char* pszA, const char* pszB)										{ return CompareStringIncludingNumber<char>(pszA, pszB, false); }
	inline					constexpr int	tdszcmp(const wchar_t* pszA, const wchar_t* pszB)								{ return CompareStringIncludingNumber<wchar_t>(pszA, pszB, false); }
	inline					constexpr int	tdszcmp(const char8_t* pszA, const char8_t* pszB)								{ return CompareStringIncludingNumber<char8_t>(pszA, pszB, false); }
	inline					constexpr int	tdszicmp(const char* pszA, const char* pszB)									{ return CompareStringIncludingNumber<char>(pszA, pszB, true); }
	inline					constexpr int	tdszicmp(const wchar_t* pszA, const wchar_t* pszB)								{ return CompareStringIncludingNumber<wchar_t>(pszA, pszB, true); }
	inline					constexpr int	tdszicmp(const char8_t* pszA, const char8_t* pszB)								{ return CompareStringIncludingNumber<char8_t>(pszA, pszB, true); }


	/// @brief  tszsearch_oneof
	/// @param psz    : string
	/// @param pszSet : chars to find
	/// @return position of found char.
	template < gtlc::string_elem tchar_t >
	tchar_t* tszsearch_oneof(tchar_t* psz, tchar_t const * const pszSet) {
		for (; *psz; psz++) {
			if (tszsearch(pszSet, *psz))
				return psz;
		}
		return nullptr;
	}
	template < gtlc::string_elem tchar_t >
	tchar_t const* tszsearch_oneof(tchar_t const* psz, tchar_t const* const pszSet) {
		for (; *psz; psz++) {
			if (tszsearch(pszSet, *psz))
				return psz;
		}
		return nullptr;
	}


	/// @brief tszto_number
	/// @param psz 
	/// @param pszEnd 
	/// @param radix 
	/// @return number
	template < gtlc::string_elem tchar_t, gtlc::arithmetic tvalue_t >
	tvalue_t tszto(const tchar_t* psz, tchar_t const* pszEnd, tchar_t** pszStopped = nullptr, int radix = 0, int cSplitter = 0) {
		if constexpr (std::is_integral_v<tvalue_t>) {
			return tsztoi<tvalue_t>(psz, pszEnd, pszStopped, radix, cSplitter);
		} else {
			return tsztod<tvalue_t>(psz, pszEnd, pszStopped, cSplitter);
		}
	}

	/// @brief Remove Charactors from str.
	/// @param psz 
	/// @param chRemove 
	/// @return str length
	template < gtlc::string_elem tchar_t >
	constexpr size_t tszrmchar(tchar_t* psz, int chRemove) {
		if (!psz || !chRemove)
			return 0;
		tchar_t* pszHeader = psz;
		do {
			if (*psz == chRemove) {
				tchar_t* p = psz+1;
				do {
					if (*p == chRemove)
						p++;
					else
						if (!(*psz++ = *p++))
							break;
				} while (true);
				break;
			}
		} while (*psz++);
		return psz - pszHeader - 1;
	}

	/// @brief Compare two Strings. according to number
	/// @param pszA 
	/// @param pszB 
	/// @return 
	template < gtlc::string_elem tchar_t, bool bIgnoreCase >
	constexpr int CompareStringIncludingNumber(const tchar_t* pszA, const tchar_t* pszB) {
		const tchar_t* p0 = pszA;
		const tchar_t* p1 = pszB;
		for (; *p0 && *p1; p0++, p1++) {
			tchar_t c0 = *p0;
			tchar_t c1 = *p1;
			if (IsDigit(c0) && IsDigit(c1)) {	// for numbers
												// skip '0'
				while (c0 == '0')
					c0 = *(++p0);
				while (c1 == '0')
					c1 = *(++p1);

				// Count Digit Length
				int nDigit0, nDigit1;
				for (nDigit0 = 0; IsDigit(p0[nDigit0]); nDigit0++)
					;
				for (nDigit1 = 0; IsDigit(p1[nDigit1]); nDigit1++)
					;

				// Compare
				if (nDigit0 == nDigit1) {
					if (nDigit0 == 0) {
						continue;
					}
					for (; (nDigit0 > 0) && *p0 && *p1; p0++, p1++, nDigit0--) {
						if (*p0 == *p1)
							continue;
						return *p0 - *p1;
					}
					p0--;
					p1--;
				} else
					return nDigit0 - nDigit1;
			} else {
				if constexpr (bIgnoreCase) { c0 = (tchar_t)ToLower(c0); c1 = (tchar_t)ToLower(c1); }
				auto r = c0 - c1;
				if (r == 0)
					continue;
				return r;
			}
		}

		if constexpr (bIgnoreCase)
			return tszicmp(p0, p1);
		return tszcmp(p0, p1);
	}
	template < gtlc::string_elem tchar_t >
	constexpr int CompareStringIncludingNumber(const tchar_t* pszA, const tchar_t* pszB, bool bIgnoreCase) {
		return bIgnoreCase ? CompareStringIncludingNumber<tchar_t, true>(pszA, pszB) : CompareStringIncludingNumber<tchar_t, false>(pszA, pszB);
	}


#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__BASIC_STRING
