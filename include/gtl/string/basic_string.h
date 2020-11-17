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

#ifdef _UNICODE
	using TCHAR = char16_t;
#else
	using TCHAR = char;
#endif

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

	/// @brief  tszlen (string length)
	/// @param psz : string
	/// @return string length
	template < gtlc::string_elem tchar_t >
	constexpr size_t tszlen(tchar_t const* psz) {
		if (!psz) return 0;
		tchar_t const pos = psz;
		while (*psz) { psz++; }
		return size;
	}

	// todo : primitive functions
#if 1
	/// @brief tchar_t wrapper (string function)
	template < gtlc::string_elem tchar_t >
	constexpr size_t tszlen(tchar_t const* psz) {
		if (!psz) return 0;
		size_t size = 0;
		while (*psz) { psz++, size++; }
		return size;
	}
	template < gtlc::string_elem tchar_t >
	errno_t tszcpy(tchar_t* pszDest, size_t size, tchar_t const* pszSrc) {
		if (!szDest)
			return EINVAL;
		if (size <= 0)
			return ERANGE;
		if (!pszSrc) {
			size = 0;
			return EINVAL;
		}
		for (int i = 0; i < size; i++) {
			szDest[i] = pszSrc[i];
			if (!*pszSrc)
				return 0;
		}
		szDest[0] = 0;
		return ERANGE;
	}

	template < gtlc::string_elem tchar_t, int size >
	errno_t tszcpy(std::array<tchar_t, size>& szDest, tchar_t const* pszSrc) {
		return tszcpy(szDest.data(), size, pszSrfc);
	}

	template < gtlc::string_elem tchar_t, int size >
	errno_t tszcpy(tchar_t (&szDest)[size], tchar_t const* pszSrc) {
		return tszcpy(szDest, size, pszSrfc);
	}

	template < gtlc::string_elem tchar_t >
	errno_t tszncpy(tchar_t* pszDest, size_t size, tchar_t const* pszSrc, size_t nLen = _TRUNCATE) {
		if (!szDest)
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
			szDest[i] = pszSrc[i];
			if (!*pszSrc)
				return 0;
		}
		if (i < size-1) {
			szDest[i] = 0;
			return 0;
		}
		szDest[0] = 0;
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
		if (!szDest || (size <= 0) || (size > RSIZE_MAX) )
			return EINVAL;
		if (!pszSrc) {
			if (size && (size <= RSIZE_MAX))
				szDest[0] = 0;
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
			szDest[0] = 0;
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

	template < gtlc::string_elem tchar_t >
	int				tsztoi(tchar_t const* psz)															{ return atoi(psz); }
	template < gtlc::string_elem tchar_t >
	int32_t			tsztol(tchar_t const* psz,    tchar_t const** ppszEnd = nullptr,    int radix = 0)	{ return strtol(psz, (tchar_t**)ppszEnd, radix); }
	template < gtlc::string_elem tchar_t >
	uint32_t		tsztoul(tchar_t const* psz,    tchar_t const** ppszEnd = nullptr,    int radix = 0)	{ return strtoul(psz, (tchar_t**)ppszEnd, radix); }
	template < gtlc::string_elem tchar_t >
	int64_t			tsztoi64(tchar_t const* psz,    tchar_t const** ppszEnd = nullptr,    int radix = 0)	{ return _strtoi64(psz, (tchar_t**)ppszEnd, radix); }
	template < gtlc::string_elem tchar_t >
	uint64_t		tsztoui64(tchar_t const* psz,    tchar_t const** ppszEnd = nullptr,    int radix = 0)	{ return _strtoui64(psz, (tchar_t**)ppszEnd, radix); }
	template < gtlc::string_elem tchar_t >
	double			tsztod(tchar_t const* psz,    tchar_t const** ppszEnd = nullptr)						{ return strtod(psz, (tchar_t**)ppszEnd); }

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
	tchar_t* tszsearch_oneof(tchar_t* psz, tchar_t const * const pszSet);
	template < gtlc::string_elem tchar_t >
	tchar_t const* tszsearch_oneof(tchar_t const* psz, tchar_t const* const pszSet);
	template < gtlc::string_elem tchar_t >
	tchar_t const* tszsearch_oneof(tchar_t const* psz, tchar_t const* const pszSet);


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
	template < gtlc::string_elem tchar_t, gtlc::arithmetic T_NUMBER >
	T_NUMBER tszto_number(const tchar_t* psz, tchar_t const** pszEnd = nullptr, int radix = 0) {
		if constexpr (std::is_integral_v<T_NUMBER>) {
			if constexpr (std::is_signed_v<T_NUMBER>) {
				if constexpr (sizeof(T_NUMBER) == sizeof(std::int64_t)) {
					return tsztoi64(psz, pszEnd, radix);
				} else {
					return tsztoi(psz, pszEnd, radix);
				}
			} else {
				if constexpr (sizeof(T_NUMBER) == sizeof(std::uint64_t)) {
					return tsztoui64(psz, pszEnd, radix);
				} else {
					return tsztoul(psz, pszEnd, radix);
				}
			}
		} else {
			return tsztod(psz, pszEnd);
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
