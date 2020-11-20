//////////////////////////////////////////////////////////////////////
//
// string_primitives.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__BASIC_STRING
#define GTL_HEADER__BASIC_STRING

#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"

#if !defined(__cpp_lib_concepts)
#	error ERROR! Supports C++v20 only.
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
	template < typename tchar_t > constexpr inline tchar_t ToLower(tchar_t c/* Locale Irrelavant */) { if ((c >= 'A') && (c <= 'Z')) return c - 'A' + 'a'; return c; }
	template < typename tchar_t > constexpr inline tchar_t ToUpper(tchar_t c/* Locale Irrelavant */) { if ((c >= 'a') && (c <= 'z')) return c - 'a' + 'A'; return c; }
	template < typename tchar_t > constexpr inline tchar_t IsDigit(tchar_t const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '9'); }
	template < typename tchar_t > constexpr inline tchar_t IsOdigit(tchar_t const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '7'); }
	template < typename tchar_t > constexpr inline tchar_t IsXdigit(tchar_t const c/* Locale Irrelavant */) { return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')); }
	template < typename tchar_t > constexpr inline tchar_t IsSpace(tchar_t const c/* Locale Irrelavant */) { return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' '); }
	template < typename tchar_t > constexpr inline tchar_t IsNotSpace(tchar_t const c/* Locale Irrelavant */) { return !IsSpace(c); }


	/// @brief  tszlen (string length)
	/// @param psz : string
	/// @return string length
	template < gtlc::string_elem tchar_t >
	constexpr size_t tszlen(tchar_t const* psz) {
		if (!psz) return 0;
		tchar_t const* pos = psz;
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
	template < std::integral tvalue_t = int, typename tchar_t = char>
	tvalue_t tsztoi(std::basic_string_view<tchar_t> svNumberString, tchar_t** ppszStopped = nullptr, int radix = 0, int cSplitter = 0) {
		if (svNumberString.empty())
			return {};

		tchar_t const* psz = svNumberString.data();
		tchar_t const* const pszEnd = svNumberString.data() + svNumberString.size();

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

	template < std::integral tvalue_t = int, gtlc::string_elem tchar_t >
	inline tvalue_t tsztoi(std::basic_string<tchar_t> const& str, tchar_t** ppszStopped = nullptr, int radix = 0, tchar_t cSplitter = 0) {
		return tsztoi<tvalue_t, tchar_t>((std::basic_string_view<tchar_t>)str, ppszStopped, radix, cSplitter);
	}
	template < std::integral tvalue_t = int, gtlc::string_elem tchar_t >
	inline tvalue_t tsztoi(tchar_t const* psz, tchar_t** ppszStopped = nullptr, int radix = 0, tchar_t cSplitter = 0) {
		return tsztoi<tvalue_t, tchar_t>(std::basic_string_view<tchar_t>{ psz, psz + tszlen(psz) }, ppszStopped, radix, cSplitter);
	}



	template < std::floating_point tvalue_t = double, gtlc::string_elem tchar_t = char16_t >
	[[deprecated("NOT STANDARD CONVERTING !")]] constexpr tvalue_t _tsztod(tchar_t const* psz, tchar_t const* pszEnd, tchar_t** ppszStopped = nullptr, tchar_t cSplitter = 0) {
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
			tchar_t* pszStopped_local{};
			int e = tsztoi(psz, pszEnd, &pszStopped_local, 10, cSplitter);
			psz = pszStopped_local;
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
		// ... not secure.
		//else if constexpr (sizeof(tchar_t) == sizeof(wchar_t)) {
		//	return wcstod((wchar_t const*)sv.data(), ppszStopped);
		//}
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

	template < std::floating_point tvalue_t = double, gtlc::string_elem tchar_t = char16_t>
	inline tvalue_t tsztod(std::basic_string<tchar_t> const& str, tchar_t** ppszStopped = nullptr) {
		return tsztod<tvalue_t, tchar_t>((std::basic_string_view<tchar_t>)str, ppszStopped);
	}
	template < std::floating_point tvalue_t = double, gtlc::string_elem tchar_t = char16_t>
	inline tvalue_t tsztod(tchar_t const* psz, tchar_t** ppszStopped = nullptr) {
		return tsztod<tvalue_t, tchar_t>(std::basic_string_view<tchar_t>{ psz, psz+tszlen(psz) }, ppszStopped);
	}

	template < gtlc::string_elem tchar_t >
	constexpr size_t tszrmchar(tchar_t* psz, int chRemove);	// Remove Charactor from str. returns str length

	/// @brief Compare strings (according to numbers)
	template < gtlc::string_elem tchar_t >
	constexpr int	CompareStringIncludingNumber(const tchar_t* pszA, const tchar_t* pszB, bool bIgnoreCase = false);
	constexpr inline int tdszcmp(const char* pszA, const char* pszB)		{ return CompareStringIncludingNumber<char>(pszA, pszB, false); }
	constexpr inline int tdszcmp(const wchar_t* pszA, const wchar_t* pszB)	{ return CompareStringIncludingNumber<wchar_t>(pszA, pszB, false); }
	constexpr inline int tdszcmp(const char8_t* pszA, const char8_t* pszB)	{ return CompareStringIncludingNumber<char8_t>(pszA, pszB, false); }

	constexpr inline int tdszicmp(const char* pszA, const char* pszB)		{ return CompareStringIncludingNumber<char>(pszA, pszB, true); }
	constexpr inline int tdszicmp(const wchar_t* pszA, const wchar_t* pszB)	{ return CompareStringIncludingNumber<wchar_t>(pszA, pszB, true); }
	constexpr inline int tdszicmp(const char8_t* pszA, const char8_t* pszB)	{ return CompareStringIncludingNumber<char8_t>(pszA, pszB, true); }


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
	constexpr int/*std::strong_ordering*/ CompareStringIncludingNumber(std::basic_string_view<tchar_t> svA, std::basic_string_view<tchar_t> svB) {
		tchar_t const* pszA				= svA.data();
		tchar_t const* const pszAend	= svA.data() + svA.size();
		tchar_t const* pszB				= svB.data();
		tchar_t const* const pszBend	= svB.data() + svB.size();

		for (; (pszA < pszAend) && (pszB < pszBend) && *pszA && *pszB; pszA++, pszB++) {
			if (IsDigit(*pszA) && IsDigit(*pszB)) {	// for numbers
				// skip '0'
				auto Skip0 = [](tchar_t const*& psz, tchar_t const* const pszEnd) {
					for ( ; (psz < pszEnd) and ('0' == *psz); psz++)
						;
				};
				Skip0(pszA, pszAend);
				Skip0(pszB, pszBend);

				// Count Digit Length
				auto CountDigitLength = [](tchar_t const*& psz, tchar_t const* const pszEnd) -> int {
					int nDigit{};
					for (; (psz + nDigit < pszEnd) and IsDigit(psz[nDigit]); nDigit++)
						;
					return nDigit;
				};
				int nDigitA = CountDigitLength(pszA, pszAend);
				int nDigitB = CountDigitLength(pszB, pszBend);

				// Compare digit length
				if (int diff = nDigitA - nDigitB; diff)
					return diff;

				if (nDigitA == 0) {
					continue;
				}
				for (; (nDigitA > 0); pszA++, pszB++, nDigitA--) {
					if (*pszA == *pszB)
						continue;
					return *pszA - *pszB;
				}
				pszA--;
				pszB--;
			} else {
				tchar_t cA = *pszA;
				tchar_t cB = *pszB;
				if constexpr (bIgnoreCase) { cA = (tchar_t)ToLower(cA); cB = (tchar_t)ToLower(cB); }
				auto r = cA - cB;
				if (r == 0)
					continue;
				return r;
			}
		}

		if constexpr (bIgnoreCase)
			return tszicmp(pszA, pszB);
		return tszcmp(pszA, pszB);
	}
	template < gtlc::string_elem tchar_t >
	constexpr int CompareStringIncludingNumber(const tchar_t* pszA, const tchar_t* pszB, bool bIgnoreCase) {
		return bIgnoreCase ? CompareStringIncludingNumber<tchar_t, true>(pszA, pszB) : CompareStringIncludingNumber<tchar_t, false>(pszA, pszB);
	}


#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__BASIC_STRING
