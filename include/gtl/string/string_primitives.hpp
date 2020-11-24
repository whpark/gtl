//////////////////////////////////////////////////////////////////////
//
// string_primitives.hpp : inline functions (Don't include this header file directly!)
//
// PWH
// 2020.11.24.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__BASIC_STRING_IMPL
#define GTL_HEADER__BASIC_STRING_IMPL


namespace gtl {
#pragma pack(push, 8)

	//-----------------------------------------------------------------------------
	/// @brief  misc. GetSpaceString()
	/// @return " \r\t\n"
	template < gtlc::string_elem tchar > [[nodiscard]] constexpr const tchar* GetSpaceString() {
		if constexpr (std::is_same_v<tchar, char>) { return _A(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, char8_t>) { return _u8(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, char16_t>) { return _u(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, char32_t>) { return _U(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, wchar_t>) { return _W(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, uint16_t>) { return (uint16_t*)_u(SPACE_STRING); }
		else { static_assert(false, "tchar must be one of (char, char8_t, wchar_t) !"); }
	}

	constexpr static inline bool is_space(int const c) { return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' '); }


	//-----------------------------------------------------------------------------
	/// @brief TrimLeft, TrimRight, Trim
	/// @tparam tchar 
	/// @param str : string
	/// @param pszTrim : chars to trim
	template < gtlc::string_elem tchar > void TrimLeft(basic_string_t<tchar>& str)	{
		TrimLeft(str, GetSpaceString<tchar>());
	}
	template < gtlc::string_elem tchar > void TrimRight(basic_string_t<tchar>& str)	{
		TrimRight(str, GetSpaceString<tchar>());
	}
	template < gtlc::string_elem tchar > void Trim(basic_string_t<tchar>& str) {
		Trim(str, GetSpaceString<tchar>());
	}

	template < gtlc::string_elem tchar > void TrimRight(basic_string_t<tchar>& str, const tchar* pszTrim) {
		str.erase(str.begin() + (str.find_last_not_of(pszTrim) + 1), str.end());
	}
	template < gtlc::string_elem tchar > void TrimLeft(basic_string_t<tchar>& str, const tchar* pszTrim) {
		auto pos = str.find_first_not_of(pszTrim);
		if (pos == str.npos)
			str.clear();
		else
			str.erase(str.begin(), str.begin()+pos);
	}
	template < gtlc::string_elem tchar > void Trim(basic_string_t<tchar>& str, const tchar* pszTrim) {
		TrimRight(str, pszTrim);
		TrimLeft(str, pszTrim);
	}

	template < gtlc::string_elem tchar > void TrimRight(basic_string_t<tchar>& str, const tchar cTrim) {
		str.erase(str.begin() + (str.find_last_not_of(cTrim) + 1), str.end());
	}
	template < gtlc::string_elem tchar > void TrimLeft(basic_string_t<tchar>& str, const tchar cTrim) {
		auto pos = str.find_first_not_of(cTrim);
		if (pos == str.npos)
			str.clear();
		else
			str.erase(str.begin(), str.begin()+pos);
	}
	template < gtlc::string_elem tchar > void Trim(basic_string_t<tchar>& str, const tchar cTrim) {
		TrimRight(str, cTrim);
		TrimLeft(str, cTrim);
	}


	/// @brief ToLower, ToUpper, ToDigit, IsSpace ... (locale irrelavant)
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToLower(tchar c/* Locale Irrelavant */) { if ((c >= 'A') && (c <= 'Z')) return c - 'A' + 'a'; return c; }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToUpper(tchar c/* Locale Irrelavant */) { if ((c >= 'a') && (c <= 'z')) return c - 'a' + 'A'; return c; }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsDigit(tchar const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '9'); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsOdigit(tchar const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '7'); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsXdigit(tchar const c/* Locale Irrelavant */) { return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsSpace(tchar const c/* Locale Irrelavant */) { return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' '); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsNotSpace(tchar const c/* Locale Irrelavant */) { return !IsSpace(c); }


	/// @brief  tszlen (string length). you cannot input string literal. you don't need tszlen("testlen");. ==> just call "testlen"sv.size();
	/// @param psz : null terminating char_t* var. (no string literals)
	/// @return string length
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] [[deprecated("NOT Secure")]] size_t tszlen(tchar const*const& psz) {
		if (!psz) return 0;
		tchar const* pos = psz;
		while (*pos) { pos++; }
		return pos-psz;
	}
	/// @brief  tszlen (string length)
	/// @param psz : string
	/// @param pszMax : (end+1) of the allocated buffer.
	/// @return string length. if not reached, SIZE_MAX.
	namespace internal {
		template < gtlc::string_elem tchar >
		constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, tchar const* const pszMax) {
			for (tchar const* pos = psz; pos < pszMax; pos++) {
				if (!*pos)
					return pos-psz;
			}
			return SIZE_MAX;
		}
	}
	template < gtlc::string_elem tchar >
	constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, tchar const* const pszMax) {
		if (!psz)
			return 0;
		return internal::tszlen(psz, pszMax);
	}
	template < gtlc::string_elem tchar >
	constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, size_t sizeOfBuf) {
		return tszlen(psz, psz+sizeOfBuf);
	}
	template < gtlc::string_elem tchar, int sizeOfBuf >
	constexpr inline [[nodiscard]] size_t tszlen(tchar (&sz)[sizeOfBuf]) {
		return internal::tszlen(sz, sz+sizeOfBuf);
	}
	template < gtlc::string_elem tchar, int sizeOfBuf >
	constexpr inline [[nodiscard]] size_t tszlen(tchar const (&sz)[sizeOfBuf]) {
		return internal::tszlen(sz, sz+sizeOfBuf);
	}
	template < gtlc::string_elem tchar, int sizeOfBuf >
	constexpr inline [[nodiscard]] size_t tszlen(std::array<tchar, sizeOfBuf> const& sz) {
		return internal::tszlen(sz.data(), sz.data()+sizeOfBuf);
	}
	template < gtlc::string_elem tchar, int sizeOfBuf >
	constexpr inline [[nodiscard]] size_t tszlen(std::array<tchar const, sizeOfBuf> const& sz) {
		return internal::tszlen(sz.data(), sz.data()+sizeOfBuf);
	}


	// todo : documents...

	/// <summary>
	/// tszcpy : utf8/16/32/wchar_t ver. of strcpy_s
	/// </summary>
	/// <param name="pszDest"></param>
	/// <param name="sizeDest"></param>
	/// <param name="pszSrc"></param>
	/// <returns>
	/// 0 : ok.
	/// EINVAL : if !pszDest || !pszSrc
	/// ERANGE : if sizeDest is smaller
	/// </returns>
	template < gtlc::string_elem tchar >
	constexpr [[deprecated("NOT Secure")]]errno_t tszcpy(tchar* const& pszDest, size_t sizeDest, tchar const*const& pszSrc) {
		if (!pszDest or !sizeDest or (sizeDest > RSIZE_MAX))
			return EINVAL;
		tchar* pos = pszDest;
		if (!pszSrc) {
			*pos = 0;
			return EINVAL;
		}
		tchar const* posSrc = pszSrc;
		tchar* const pszMax = pszDest+sizeDest;
		for (; pos < pszMax; pos++, posSrc++) {
			*pos = *posSrc;
			if (!*posSrc)
				return 0;
		}
		pszDest[0] = 0;
		return ERANGE;
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]]errno_t tszcpy(tchar (&szDest)[sizeDest], tchar const*const& pszSrc) {
		return tszcpy(szDest, sizeDest, pszSrc);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]]errno_t tszcpy(std::array<tchar, sizeDest>& szDest, tchar const*const& pszSrc) {
		return tszcpy(szDest.data(), sizeDest, pszSrc);
	}

	/// <summary>
	/// tszcpy : utf8/16/32/wchar_t ver. of strcpy_s
	/// </summary>
	/// <param name="pszDest">dest. buffer</param>
	/// <param name="sizeDest">size of dest. including null terminating char.</param>
	/// <param name="svSrc">source string. do not need to be NULL terminated string</param>
	/// <returns>
	/// 0 : ok.
	/// EINVAL : if !pszDest
	/// ERANGE : if sizeDest is not enough.
	/// </returns>
	template < gtlc::string_elem tchar >
	constexpr errno_t tszcpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc) {
		if (!pszDest or !sizeDest)
			return EINVAL;
		tchar* pos = pszDest;
		if (sizeDest <= svSrc.size()) {
			*pos = 0;
			return ERANGE;
		}
		tchar const* s = svSrc.data();
		tchar const* const e = s + svSrc.size();
		while  (s < e)
			*pos++ = *s++;
		*pos = 0;
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszcpy(tchar (&szDest)[sizeDest], std::basic_string_view<tchar> svSrc) {
		return tszcpy(szDest, sizeDest, svSrc);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszcpy(std::array<tchar, sizeDest>& szDest, std::basic_string_view<tchar> svSrc) {
		return tszcpy(szDest.data(), sizeDest, svSrc);
	}
	template < gtlc::string_elem tchar >
	constexpr inline errno_t tszcpy(tchar* pszDest, size_t sizeDest, std::basic_string<tchar> const& strSrc) {
		return tszcpy(pszDest, sizeDest, (std::basic_string_view<tchar>)strSrc);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszcpy(tchar (&szDest)[sizeDest], std::basic_string<tchar> const& strSrc) {
		return tszcpy(szDest, sizeDest, (std::basic_string_view<tchar>)strSrc);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszcpy(std::array<tchar, sizeDest>& szDest, std::basic_string<tchar> const& strSrc) {
		return tszcpy(szDest.data(), sizeDest, (std::basic_string_view<tchar>)strSrc);
	}

	// tszncpy
	template < gtlc::string_elem tchar >
	constexpr [[deprecated("NOT Secure")]] errno_t tszncpy(tchar*const& pszDest, size_t sizeDest, tchar const* pszSrc, size_t nCount) {
		if (!pszDest or !sizeDest or (sizeDest > RSIZE_MAX) )
			return EINVAL;
		tchar* pos = pszDest;
		if (!pszSrc) {
			*pos = 0;
			return EINVAL;
		}
		if (nCount == _TRUNCATE) {
			nCount = sizeDest-1;
		} else if (sizeDest <= nCount) {
			*pos = 0;
			return ERANGE;
		}

		tchar const* posSrc = pszSrc;
		tchar* const pszMax = pszDest + nCount;	// not sizeDest
		for (; pos < pszMax; pos++, posSrc++) {
			*pos = *posSrc;
			if (!*posSrc)
				return 0;
		}
		*pos = 0;	// null terminator
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]] errno_t tszncpy(tchar (&szDest)[sizeDest], tchar const* pszSrc, size_t nCount) {
		return tszncpy(szDest, sizeDest, pszSrc, nCount);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]] errno_t tszncpy(std::array<tchar, sizeDest>& szDest, tchar const* pszSrc, size_t nCount) {
		return tszncpy(szDest.data(), sizeDest, pszSrc, nCount);
	}

	// tszncpy (sv)
	template < gtlc::string_elem tchar >
	constexpr errno_t tszncpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc, size_t nCount) {
		if (!pszDest or !sizeDest or (sizeDest > RSIZE_MAX) or (svSrc.size() > RSIZE_MAX))
			return EINVAL;
		tchar* pos = pszDest;
		if (nCount == _TRUNCATE) {
			nCount = std::min(sizeDest-1, svSrc.size());
		} else if (sizeDest <= nCount) {	// svSrc might be smaller and it could be ok sometimes. but it's safer to make some noise, here.
			*pos = 0;
			return ERANGE;
		} else {
			nCount = std::min(nCount, svSrc.size());
		}

		tchar const* posSrc = svSrc.data();
		tchar* const MaxEnd = pszDest + nCount;	// not sizeDest
		for (; pos < MaxEnd; pos++, posSrc++) {
			*pos = *posSrc;
		}
		*pos = 0;	// null terminator
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszncpy(tchar (&szDest)[sizeDest], std::basic_string_view<tchar> svSrc, size_t nCount) {
		return tszncpy(szDest, sizeDest, svSrc, nCount);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszncpy(std::array<tchar, sizeDest>& szDest, std::basic_string_view<tchar> svSrc, size_t nCount) {
		return tszncpy(szDest.data(), sizeDest, svSrc, nCount);
	}
	template < gtlc::string_elem tchar >
	constexpr inline errno_t tszncpy(tchar* pszDest, size_t sizeDest, std::basic_string<tchar> const& strSrc, size_t nCount) {
		return tszncpy(pszDest, sizeDest, std::basic_string_view<tchar>{strSrc.data(), strSrc.end()}, nCount);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszncpy(tchar (&szDest)[sizeDest], std::basic_string<tchar> const& strSrc, size_t nCount) {
		return tszncpy(szDest, sizeDest, std::basic_string_view<tchar>{strSrc.data(), strSrc.end()}, nCount);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline errno_t tszncpy(std::array<tchar, sizeDest>& szDest, std::basic_string<tchar> const& strSrc, size_t nCount) {
		return tszncpy(szDest.data(), sizeDest, std::basic_string_view<tchar>{strSrc.data(), strSrc.end()}, nCount);
	}

	// tszcat
	template < gtlc::string_elem tchar >
	constexpr [[deprecated("NOT Secure")]] errno_t tszcat(tchar* pszDest, size_t sizeDest, tchar const* pszSrc) {
		if (!pszDest or !sizeDest or (sizeDest > RSIZE_MAX) )
			return EINVAL;
		if (!pszSrc) {
			pszDest[0] = 0;
			return EINVAL;
		}
		auto const* const pszMax = pszDest + sizeDest;
		auto nCount = tszlen(pszDest, pszMax);
		if (nCount > RSIZE_MAX)
			return ERANGE;
		tchar* pos = pszDest + nCount;
		tchar const* posSrc = pszSrc;
		for (; pos < pszMax; pos++, posSrc++) {
			*pos = *posSrc;
			if (!*posSrc)
				return 0;
		}
		// pszSrc is longer
		pszDest[0] = 0;
		return ERANGE;
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]] errno_t tszcat(tchar (&szDest)[sizeDest], tchar const* pszSrc) {
		return tszcat(szDest, sizeDest, pszSrc);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]] errno_t tszcat(std::array<tchar, sizeDest>& szDest, tchar const* pszSrc) {
		return tszcat(szDest.data(), sizeDest, pszSrc);
	}
	template < gtlc::string_elem tchar >
	constexpr errno_t tszcat(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc) {
		if (!pszDest or !sizeDest or (sizeDest > RSIZE_MAX) or (svSrc.size() > RSIZE_MAX))
			return EINVAL;
		auto const* const pszMax = pszDest + sizeDest;
		auto nCount = tszlen(pszDest, pszMax);
		if (nCount > RSIZE_MAX)
			return ERANGE;
		tchar* pos = pszDest + nCount;
		if (sizeDest <= nCount + svSrc.size() + 1) {
			*pos = 0;
			return ERANGE;
		}
		tchar const* posSrc = svSrc.data();
		tchar const* const pszSrcEnd = posSrc + svSrc.size();
		for (; (pos < pszMax) and (posSrc < pszSrcEnd); pos++, posSrc++) {
			*pos = *posSrc;
		}
		*pos = 0;
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]] errno_t tszcat(tchar (&szDest)[sizeDest], std::basic_string_view<tchar> svSrc) {
		return tszcat(szDest, sizeDest, svSrc);
	}
	template < gtlc::string_elem tchar, int sizeDest >
	constexpr inline [[deprecated("NOT Secure")]] errno_t tszcat(std::array<tchar, sizeDest>& szDest, std::basic_string_view<tchar> svSrc) {
		return tszcat(szDest.data(), sizeDest, svSrc);
	}

	template < gtlc::string_elem tchar >
	constexpr [[deprecated("Not Secure")]] size_t tszrmchar(tchar* const psz, int chRemove) {
		if (!psz || !chRemove)
			return 0;
		tchar* pos = psz;
		tchar* p2 {};
		for (; *pos; pos++) {
			if (*pos == chRemove) {
				p2 = pos + 1;
				break;
			}
		}
		if (p2) {
			for (; ; p2++) {
				if (*p2 == chRemove)
					continue;
				if (!(*pos++ = *p2))
					break;
			}
		}
		return pos - psz;
	}
	template < gtlc::string_elem tchar >
	constexpr size_t tszrmchar(tchar* const psz, tchar const* const pszMax, tchar chRemove) {
		if (!psz || !chRemove)
			return 0;
		if constexpr (false) {
			// don't detect null terminator.
			auto end = std::erase(std::remove(psz, pszMax, chRemove), pszMax);
			if (end != psz)
				end--;
			*end = 0;	// null terminating.
			return end - psz;
		}
		else {
			tchar* pos = psz;
			tchar* p2 {};
			for (; (pos < pszMax) and *pos; pos++) {
				if (*pos == chRemove) {
					p2 = pos + 1;
					break;
				}
			}
			if (p2) {
				for (; (pos < pszMax) and (p2 < pszMax); p2++) {
					if (*p2 == chRemove)
						continue;
					if (!(*pos++ = *p2))
						break;
				}
			}
			return pos - psz;
		}
	}
	template < gtlc::string_elem tchar, int sizeBuffer > constexpr inline size_t tszrmchar(tchar(&sz)[sizeBuffer], tchar chRemove) {
		return tszrmchar(sz, sz+sizeBuffer, chRemove);
	}
	template < gtlc::string_elem tchar, int sizeBuffer > constexpr inline size_t tszrmchar(std::array<tchar, sizeBuffer>& sz, tchar chRemove) {
		return tszrmchar(sz.data(), sz.data()+sz.size(), chRemove);
	}


	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] [[deprecated("Not Secure")]] int tszcmp(tchar const* pszA, tchar const* pszB) {
		if (!pszA && !pszB)	// if both are nullptr, return 0;
			return 0;
		if (pszA && !pszB)	// if only one has value, its bigger.
			return *pszA;
		else if (!pszA && pszB)
			return -*pszB;

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

	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] int tszncmp(tchar const* pszA, tchar const* pszB, size_t nCount) {
		if ((!pszA and !pszB) or !nCount or (nCount > RSIZE_MAX))
			return 0;
		if (pszA && !pszB)
			return *pszA;
		else if (!pszA && pszB)
			return -*pszB;

		if (!nCount)
			return 0;

		for (; !*pszA || !*pszB; pszA++, pszB++) {
			auto r = *pszA - *pszB;
			if (!r)
				return r;
			if (--nCount == 0)
				return 0;
		}
		if (!*pszA && !*pszB)
			return 0;
		auto r = *pszA - *pszB;
		return r;
	}
	template < gtlc::string_elem tchar >
	int tszicmp(tchar const* pszA, tchar const* pszB) {
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
	template < gtlc::string_elem tchar >
	int tsznicmp(tchar const* pszA, tchar const* pszB, size_t nCount) {
		if (!pszA && !pszB)
			return 0;
		if (pszA && !pszB)
			return *pszA;
		else if (!pszA && pszB)
			return *pszB;

		if (!nCount)
			return 0;

		for (; !*pszA || !*pszB; pszA++, pszB++) {
			auto r = ToLower(*pszA) - ToLower(*pszB);
			if (!r)
				return r;
			if (--nCount == 0)
				return 0;
		}
		if (!*pszA && !*pszB)
			return 0;
		auto r = *pszA - *pszB;
		return r;
	}

	template < gtlc::string_elem tchar >
	errno_t tszupr(tchar* sz, size_t nCount) {
		if (!sz)
			return EINVAL;
		for (size_t i = 0; i < nCount; i++) {
			auto& c = *sz++;
			if (!c)
				break;
			c = ToUpper(c);
		}
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeBuffer >
	errno_t tszupr(tchar (&sz)[sizeBuffer]) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToUpper(c);
		}
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeBuffer >
	errno_t tszupr(std::array<tchar, sizeBuffer>& sz) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToUpper(c);
		}
		return 0;
	}

	template < gtlc::string_elem tchar >
	errno_t tszlwr(tchar* sz, size_t nCount) {
		if (!sz)
			return EINVAL;
		for (size_t i = 0; i < nCount; i++) {
			auto& c = *sz++;
			if (!c)
				break;
			c = ToLower(c);
		}
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeBuffer >
	errno_t tszlwr(tchar(&sz)[sizeBuffer]) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToLower(c);
		}
		return 0;
	}
	template < gtlc::string_elem tchar, int sizeBuffer >
	errno_t tszlwr(std::array<tchar, sizeBuffer>& sz) {
		for (auto& c : sz) {
			if (!c)
				break;
			c = ToLower(c);
		}
		return 0;
	}

	template < gtlc::string_elem tchar >
	tchar* tszrev(tchar* psz) {
		if (!psz)
			return nullptr;
		std::reverse(psz, psz + tszlen(psz));
	}
	template < gtlc::string_elem tchar >
	tchar const* tszsearch(tchar const* psz, tchar c) {
		if (!psz)
			return nullptr;
		auto const* const end = psz + tszlen(psz);
		auto const* p = std::search(psz, end, c);
		if (p == end)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar >
	tchar* tszsearch(tchar* psz, tchar c) {
		if (!psz)
			return nullptr;
		auto* const end = psz + tszlen(psz);
		auto* p = std::search(psz, end, c);
		if (p == end)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar >
	tchar const* tszsearch(tchar const* psz, tchar const* pszSub) {
		if (!psz || !pszSub)
			return nullptr;
		auto const* const end = psz + tszlen(psz);
		auto const* const endSub = pszSub + tszlen(pszSub);
		auto const* p = std::search(psz, end, pszSub, endSub);
		if (p == end)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar >
	tchar* tszsearch(tchar* psz, tchar* pszSub) {
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
	/// <param name="pszMax"></param>
	/// <param name="ppszStopped">where conversion stopped</param>
	/// <param name="radix">radix</param>
	/// <param name="cSplitter">digit splitter. such as ',' (thousand sepperator) or '\'' (like c++v14 notation)</param>
	/// <returns>number value. (no overflow checked)</returns>
	template < std::integral tvalue, gtlc::string_elem tchar>
	[[nodiscard]] tvalue tsztoi(std::basic_string_view<tchar> svNumberString, tchar** ppszStopped, int radix, int cSplitter) {
		if (svNumberString.empty())
			return {};

		tchar const* psz = svNumberString.data();
		tchar const* const pszEnd = svNumberString.data() + svNumberString.size();

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

		tvalue value{};
		constexpr static auto const tbl = internal::digit_table<uint8_t, 256>{};

		for (; (psz < pszEnd) && *psz; psz++) {
			if (cSplitter == *psz)
				continue;
			auto c = *psz;
			if constexpr (sizeof(tchar) > sizeof(uint8_t)) {
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
			*ppszStopped = const_cast<tchar*>(psz);

		return value;
	}

	template < std::integral tvalue, gtlc::string_elem tchar >
	inline tvalue tsztoi(std::basic_string<tchar> const& str, tchar** ppszStopped, int radix, tchar cSplitter) {
		return tsztoi<tvalue, tchar>((std::basic_string_view<tchar>)str, ppszStopped, radix, cSplitter);
	}
	template < std::integral tvalue, gtlc::string_elem tchar >
	inline tvalue tsztoi(tchar const*& psz, tchar** ppszStopped, int radix, tchar cSplitter) {
		return tsztoi<tvalue, tchar>(std::basic_string_view<tchar>{ psz, psz + tszlen(psz) }, ppszStopped, radix, cSplitter);
	}
	template < std::integral tvalue, gtlc::string_elem tchar, int size >
	inline tvalue tsztoi(tchar const (&sz)[size], tchar** ppszStopped, int radix, tchar cSplitter) {
		return tsztoi<tvalue, tchar>(std::basic_string_view<tchar>{ sz, sz + tszlen(sz, size) }, ppszStopped, radix, cSplitter);
	}



	template < std::floating_point tvalue, gtlc::string_elem tchar >
	[[deprecated("NOT STANDARD CONVERTING !")]] constexpr tvalue _tsztod(tchar const* psz, tchar const* pszEnd, tchar** ppszStopped, tchar cSplitter) {
		if (!psz)
			return {};

		// skip white space
		while ((psz < pszEnd) && is_space(*psz))
			psz++;

		// Check sign (+/-)
		bool bMinus{};
		if (*psz == '-')
			psz++, bMinus = true;
		else if (*psz == '+')
			psz++;

		tvalue value{};
		constexpr static auto const tbl = internal::digit_table<uint8_t, 256>{};

		// mantissa
		for (; (psz < pszEnd) && *psz; psz++) {
			if (cSplitter == *psz)
				continue;
			auto c{ *psz };
			if constexpr (sizeof(tchar) > sizeof(uint8_t)) {
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
				if constexpr (sizeof(tchar) > sizeof(uint8_t)) {
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
			tchar* pszStopped_local{};
			int e = tsztoi(psz, pszEnd, &pszStopped_local, 10, cSplitter);
			psz = pszStopped_local;
			if (e != 0)
				value *= std::pow(10, e);
		}

		if (bMinus)
			value = -value;

		if (ppszStopped)
			*ppszStopped = const_cast<tchar*>(psz);

		return value;
	}

	template < std::floating_point tvalue, gtlc::string_elem tchar >
	inline tvalue tsztod(std::basic_string_view<tchar> sv, tchar** ppszStopped) {
		if constexpr (sizeof(tchar) == sizeof(char)) {
			tvalue value;
			auto [ptr, ec] = std::from_chars(sv.data(), sv.data()+sv.size(), value, std::chars_format::general);
			if (ppszStopped)
				*ppszStopped = const_cast<tchar*>(ptr);
			return value;
		}
		// ... not secure.
		//else if constexpr (sizeof(tchar) == sizeof(wchar_t)) {
		//	return wcstod((wchar_t const*)sv.data(), ppszStopped);
		//}
		else {
			std::string str;
			str.reserve(16);
			tchar const* pos = sv.data();
			tchar const* end = sv.data() + sv.size();
			while (pos < end && is_space(*pos))
				pos++;
			for (; pos < end; pos++) {
				static_assert(std::is_unsigned_v<tchar>);
				if (*pos > 127 || !std::strchr("+-.0123456789eE", *pos))
					break;
				str += *pos;
			}
			return tsztod<tvalue, char>(str, ppszStopped);
		}
	}

	template < std::floating_point tvalue, gtlc::string_elem tchar>
	inline tvalue tsztod(std::basic_string<tchar> const& str, tchar** ppszStopped) {
		return tsztod<tvalue, tchar>((std::basic_string_view<tchar>)str, ppszStopped);
	}
	template < std::floating_point tvalue, gtlc::string_elem tchar>
	inline tvalue tsztod(tchar const* psz, tchar** ppszStopped) {
		return tsztod<tvalue, tchar>(std::basic_string_view<tchar>{ psz, psz+tszlen(psz) }, ppszStopped);
	}


	/// @brief Compare strings (containing numbers)
	template < gtlc::string_elem tchar, bool bIgnoreCase >
	constexpr int CompareStringContainingNumbers(const tchar* pszA, const tchar* pszB);

	template < gtlc::string_elem tchar >
	constexpr inline int tdszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB) {
		return CompareStringContainingNumbers<tchar, false>(svA, svB);
	}
	template < gtlc::string_elem tchar >
	constexpr inline int tdszcmp(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB) {
		return CompareStringContainingNumbers<tchar, false>((std::basic_string_view<tchar>)strA, (std::basic_string_view<tchar>)strB);
	}
	template < gtlc::string_elem tchar >
	constexpr inline int tdszicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB) {
		return CompareStringContainingNumbers<tchar, true>(svA, svB);
	}
	template < gtlc::string_elem tchar >
	constexpr inline int tdszicmp(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB) {
		return CompareStringContainingNumbers<tchar, true>((std::basic_string_view<tchar>)strA, (std::basic_string_view<tchar>)strB);
	}



	/// @brief  tszsearch_oneof
	/// @param psz    : string
	/// @param pszSet : chars to find
	/// @return position of found char.
	template < gtlc::string_elem tchar >
	tchar* tszsearch_oneof(tchar* psz, tchar const * const pszSet) {
		for (; *psz; psz++) {
			if (tszsearch(pszSet, *psz))
				return psz;
		}
		return nullptr;
	}
	template < gtlc::string_elem tchar >
	tchar const* tszsearch_oneof(tchar const* psz, tchar const* const pszSet) {
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
	template < gtlc::arithmetic tvalue, gtlc::string_elem tchar>
	tvalue tszto(const tchar* psz, tchar const* pszEnd, tchar** pszStopped, int radix, int cSplitter) {
		if constexpr (std::is_integral_v<tvalue>) {
			return tsztoi<tvalue>(psz, pszEnd, pszStopped, radix, cSplitter);
		} else {
			return tsztod<tvalue>(psz, pszEnd, pszStopped, cSplitter);
		}
	}


	/// @brief Compare two Strings. according to number (only for '0'-'9' are taken as number. no '-' sign, neither '.' for floating point 
	///  ex)
	///      "123" > "65"         // 123 > 65
	///      "abc123" > "abc6"    // 123 > 65 ("abc" == "abc")
	///      "-100" > "-125"      // '-' is just taken as part of string. ===> 100 > 125
	///      "00001" < "0000003"  // 1 < 3
	///      "01" < "001"         // if same (1 == 1) ===> longer gets winner.
	/// @param pszA
	/// @param pszB
	/// @return 
	template < gtlc::string_elem tchar, bool bIgnoreCase >
	constexpr int/*std::strong_ordering*/ CompareStringContainingNumbers(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB) {
		tchar const* pszA				= svA.data();
		tchar const* const pszAend	= svA.data() + svA.size();
		tchar const* pszB				= svB.data();
		tchar const* const pszBend	= svB.data() + svB.size();

		for (; (pszA < pszAend) && (pszB < pszBend); pszA++, pszB++) {
			if (IsDigit(*pszA) && IsDigit(*pszB)) {	// for numbers
				tchar const* const pszAs {pszA};
				tchar const* const pszBs {pszB};

				// skip '0'
				auto Skip0 = [](tchar const*& psz, tchar const* const pszEnd) {
					for ( ; (psz < pszEnd) and ('0' == *psz); psz++)
						;
				};
				Skip0(pszA, pszAend);
				Skip0(pszB, pszBend);

				// Count Digit Length
				auto CountDigitLength = [](tchar const*& psz, tchar const* const pszEnd) -> int {
					int nDigit{};
					for (; (psz + nDigit < pszEnd) and IsDigit(psz[nDigit]); nDigit++)
						;
					return nDigit;
				};
				int const nDigitA = CountDigitLength(pszA, pszAend);
				int const nDigitB = CountDigitLength(pszB, pszBend);

				// Compare digit length
				if (auto diff = nDigitA - nDigitB; diff)
					return diff;

				if (nDigitA == 0) {
					continue;
				}
				for (int nDigit {nDigitA}; (nDigit > 0); pszA++, pszB++, nDigit--) {
					if (*pszA == *pszB)
						continue;
					return *pszA - *pszB;
				}
				if (auto diff = (pszA-pszAs) - (pszB-pszBs); diff)
					return (int)diff;
				pszA--;
				pszB--;
			} else {
				tchar cA = *pszA;
				tchar cB = *pszB;
				if constexpr (bIgnoreCase) { cA = (tchar)ToLower(cA); cB = (tchar)ToLower(cB); }
				auto r = cA - cB;
				if (r == 0)
					continue;
				return r;
			}
		}
		if (pszA < pszAend)
			return *pszA;
		else if (pszB < pszBend)
			return -*pszB;

		if constexpr (bIgnoreCase)
			return tszicmp(pszA, pszB);
		return tszcmp(pszA, pszB);
	}
	template < gtlc::string_elem tchar >
	constexpr int CompareStringContainingNumbers(std::basic_string<tchar> const& strA, std::basic_string<tchar> const& strB, bool bIgnoreCase) {
		return bIgnoreCase
			? CompareStringContainingNumbers<tchar, true>((std::basic_string_view<tchar>)strA, (std::basic_string_view<tchar>)strB)
			: CompareStringContainingNumbers<tchar, false>((std::basic_string_view<tchar>)strA, (std::basic_string_view<tchar>)strB);
	}
	//template < gtlc::string_elem tchar, bool bIgnoreCase >
	//constexpr inline int/*std::strong_ordering*/ CompareStringContainingNumbers(tchar const* pszA, tchar const* pszB) {
	//	return CompareStringContainingNumbers<tchar, bIgnoreCase>({pszA, pszA+tszlen(pszA)}, {pszB, pszB+tszlen(pszB)});
	//}

#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__BASIC_STRING_IMPL
