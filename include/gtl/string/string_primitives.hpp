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
	/// 
	template < gtlc::string_elem tchar > [[nodiscard]] constexpr std::basic_string_view<tchar> GetSpaceString() {
		if constexpr (std::is_same_v<tchar, char>) { return _A(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, char8_t>) { return _u8(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, char16_t>) { return _u(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, char32_t>) { return _U(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, wchar_t>) { return _W(SPACE_STRING); }
		else if constexpr (std::is_same_v<tchar, uint16_t>) { return (uint16_t*)_u(SPACE_STRING); }
		else { static_assert(false, "tchar must be one of (char, char8_t, wchar_t) !"); }
	}
	//template < gtlc::string_elem tchar > [[nodiscard]] constexpr std::basic_string_view<tchar> GetSpaceString();
	//template <> [[nodiscard]] constexpr std::basic_string_view<char>     GetSpaceString() { return SPACE_STRING; }
	//template <> [[nodiscard]] constexpr std::basic_string_view<char8_t>  GetSpaceString() { return _u8(SPACE_STRING); }
	//template <> [[nodiscard]] constexpr std::basic_string_view<char16_t> GetSpaceString() { return _u(SPACE_STRING); }
	//template <> [[nodiscard]] constexpr std::basic_string_view<char32_t> GetSpaceString() { return _U(SPACE_STRING); }
	//template <> [[nodiscard]] constexpr std::basic_string_view<wchar_t>  GetSpaceString() { return _W(SPACE_STRING); }


	constexpr static inline bool is_space(int const c) { return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' '); }


	//-----------------------------------------------------------------------------
	// Trim
	template < gtlc::string_elem tchar > void TrimLeft(std::basic_string<tchar>& str, std::basic_string_view<tchar> svTrim) {
		auto pos = str.find_first_not_of(svTrim);
		if (pos == str.npos)
			str.clear();
		else
			str.erase(str.begin(), str.begin()+pos);
	}
	template < gtlc::string_elem tchar > void TrimRight(std::basic_string<tchar>& str, std::basic_string_view<tchar> svTrim) {
		str.erase(str.begin() + (str.find_last_not_of(svTrim) + 1), str.end());
	}
	template < gtlc::string_elem tchar > void Trim(std::basic_string<tchar>& str, std::basic_string_view<tchar> svTrim) {
		TrimRight(str, svTrim);
		TrimLeft(str, svTrim);
	}

	template < gtlc::string_elem tchar > void TrimLeft(std::basic_string<tchar>& str, tchar const cTrim) {
		auto pos = str.find_first_not_of(cTrim);
		if (pos == str.npos)
			str.clear();
		else
			str.erase(str.begin(), str.begin()+pos);
	}
	template < gtlc::string_elem tchar > void TrimRight(std::basic_string<tchar>& str, tchar const cTrim) {
		str.erase(str.begin() + (str.find_last_not_of(cTrim) + 1), str.end());
	}
	template < gtlc::string_elem tchar > void Trim(std::basic_string<tchar>& str, tchar const cTrim) {
		TrimRight(str, cTrim);
		TrimLeft(str, cTrim);
	}

	template < gtlc::string_elem tchar > void TrimLeft(std::basic_string<tchar>& str)	{
		TrimLeft(str, GetSpaceString<tchar>());
	}
	template < gtlc::string_elem tchar > void TrimRight(std::basic_string<tchar>& str)	{
		TrimRight(str, GetSpaceString<tchar>());
	}
	template < gtlc::string_elem tchar > void Trim(std::basic_string<tchar>& str) {
		Trim(str, GetSpaceString<tchar>());
	}


	// Trim-View
	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimLeftView(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svTrim) {
		if (auto pos = sv.find_first_not_of(svTrim); pos != sv.npos)
			return { sv.begin()+pos, sv.end() };
		else
			return {};
	}
	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimRightView(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svTrim) {
		return { sv.begin(), sv.begin() + (sv.find_last_not_of(svTrim)+1) };
	}
	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimView(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svTrim) {
		return TrimRightView(TrimLeftView(sv, svTrim), svTrim);
	}

	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimLeftView(std::basic_string_view<tchar> sv, tchar const cTrim) {
		if (auto pos = sv.find_first_not_of(cTrim); pos != sv.npos)
			return { sv.begin()+pos, sv.end() };
		else
			return {};
	}
	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimRightView(std::basic_string_view<tchar> sv, tchar const cTrim) {
		return { sv.begin(), sv.begin() + (sv.find_last_not_of(cTrim)+1) };
	}
	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimView(std::basic_string_view<tchar> sv, tchar const cTrim) {
		return TrimRightView(TrimLeftView(sv, cTrim), cTrim);
	}

	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimLeftView(std::basic_string_view<tchar> sv) {
		return TrimLeftView(sv, GetSpaceString<tchar>());
	}
	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimRightView(std::basic_string_view<tchar> sv) {
		return TrimRightView(sv, GetSpaceString<tchar>());
	}
	template < gtlc::string_elem tchar >
	std::basic_string_view<tchar> TrimView(std::basic_string_view<tchar> sv) {
		return TrimView(sv, GetSpaceString<tchar>());
	}

	/// @brief ToLower, ToUpper, ToDigit, IsSpace ... (locale irrelavant)
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToLower(tchar c/* Locale Irrelavant */) { if ((c >= 'A') && (c <= 'Z')) return c - 'A' + 'a'; return c; }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToUpper(tchar c/* Locale Irrelavant */) { if ((c >= 'a') && (c <= 'z')) return c - 'a' + 'A'; return c; }
	template < gtlc::string_elem tchar > constexpr inline               void MakeLower(tchar& c/* Locale Irrelavant */) { if ((c >= 'A') && (c <= 'Z')) c += 'a' - 'A'; }
	template < gtlc::string_elem tchar > constexpr inline               void MakeUpper(tchar& c/* Locale Irrelavant */) { if ((c >= 'a') && (c <= 'z')) c -= 'a' - 'A'; }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsDigit(tchar const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '9'); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsOdigit(tchar const c/* Locale Irrelavant */) { return (c >= '0') && (c <= '7'); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsXdigit(tchar const c/* Locale Irrelavant */) { return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsSpace(tchar const c/* Locale Irrelavant */) { return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' '); }
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsNotSpace(tchar const c/* Locale Irrelavant */) { return !IsSpace(c); }



	/// @brief  tszlen (string length). you cannot input string literal. you don't need tszlen("testlen");. ==> just call "testlen"sv.size();
	/// @param psz : null terminating char_t* var. (no string literals)
	/// @return string length
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] GTL_DEPR_SEC size_t tszlen(tchar const*const& psz) {
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
			return pszMax-psz;	// max size
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
		return tszlen(psz, psz + sizeOfBuf);
	}
	template < gtlc::string_buffer_fixed tstring_buf >
	constexpr inline [[nodiscard]] size_t tszlen(tstring_buf const& v) {
		return tszlen(std::data(v), std::size(v));
	}


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
	constexpr GTL_DEPR_SEC errno_t tszcpy(tchar* const& pszDest, size_t sizeDest, tchar const* const& pszSrc) {
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
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline GTL_DEPR_SEC errno_t tszcpy(tstring_buf& szDest, tchar const* const& pszSrc) {
#pragma warning(suppress:4996)
		return tszcpy(std::data(szDest), std::size(szDest), pszSrc);
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
		auto sizeSrc = size(svSrc);
		if (sizeDest <= sizeSrc) {
			*pos = 0;
			return ERANGE;
		}
		tchar const* s = data(svSrc);
		tchar const* const e = s + sizeSrc;
		while  (s < e)
			*pos++ = *s++;
		*pos = 0;
		return 0;
	}

	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string_view<tchar> svSrc) {
		tchar* const pszDest = std::data(szDest);
		size_t sizeDest = std::size(szDest);
		return tszcpy(pszDest, sizeDest, svSrc);
	}
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string<tchar> const& strSrc) {
		tchar* const pszDest = std::data(szDest);
		size_t sizeDest = std::size(szDest);
		return tszcpy(pszDest, sizeDest, std::basic_string_view{strSrc.data(), strSrc.size()});

	}

#if 0	// any type <- any type.... 너무 많은 옵션.
	template < gtlc::string_buffer_fixed tcontainer1, gtlc::string_buffer_fixed tcontainer2 >
	requires requires (tcontainer1 t1, tcontainer2 t2) {
		requires std::is_same_v<std::remove_cvref_t<decltype(t1[0])>, std::remove_cvref_t<decltype(t2[0])> >
		&& gtlc::string_elem<std::remove_cvref_t<decltype(t1[0])>>;
	}
	constexpr errno_t tszcpy(tcontainer1& szDest, tcontainer2 const& svSrc) {
		using tchar = std::remove_cvref_t<decltype(szDest[0])>;
		tchar* const pszDest = std::data(szDest);
		size_t sizeDest = std::size(szDest);
		if (!pszDest or !sizeDest)
			return EINVAL;
		tchar* pos = pszDest;
		auto sizeSrc = tszlen(svSrc);
		if (sizeDest <= sizeSrc) {
			*pos = 0;
			return ERANGE;
		}
		tchar const* s = std::data(svSrc);
		tchar const* const e = s + sizeSrc;
		while  (s < e) {
			*pos++ = *s++;
		}
		*pos = 0;
		return 0;
	}
#endif

	// tszncpy
	template < gtlc::string_elem tchar >
	constexpr errno_t tszncpy(tchar*const& pszDest, size_t sizeDest, tchar const* pszSrc, size_t nCount) {
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
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, tchar const* pszSrc, size_t nCount) {
		return tszncpy(std::data(szDest), std::size(szDest), pszSrc, nCount);
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
			if (!*pos)
				return 0;
		}
		*pos = 0;	// null terminator
		return 0;
	}
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string_view<tchar> svSrc, size_t nCount) {
		return tszncpy(std::data(szDest), std::size(szDest), svSrc, nCount);
	}
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string<tchar> const& strSrc, size_t nCount) {
		return tszncpy(std::data(szDest), std::size(szDest), std::basic_string_view<tchar>{strSrc}, nCount);
	}

	// tszcat
	template < gtlc::string_elem tchar >
	constexpr GTL_DEPR_SEC errno_t tszcat(tchar* pszDest, size_t sizeDest, tchar const* pszSrc) {
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
			if (!*pos)
				return 0;
		}
		// pszSrc is longer
		pszDest[0] = 0;
		return ERANGE;
	}
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline GTL_DEPR_SEC errno_t tszcat(tstring_buf &szDest, tchar const* pszSrc) {
#pragma warning(suppress:4996)
		return tszcat(std::data(szDest), std::size(szDest), pszSrc);
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
			if (!*pos)
				return 0;
		}
		*pos = 0;
		return 0;
	}
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string_view<tchar> svSrc) {
		return tszcat(std::data(szDest), std::size(szDest), svSrc);
	}
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string<tchar> const& strSrc) {
		return tszcat(std::data(szDest), std::size(szDest), std::basic_string_view{strSrc.data(), strSrc.size()});
	}

	template < gtlc::string_elem tchar >
	constexpr GTL_DEPR_SEC size_t tszrmchar(tchar* const& psz, int chRemove) {
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
				if (!(*pos = *p2))
					break;
				pos++;
			}
		}
		return pos - psz;
	}
	template < gtlc::string_elem tchar >
	constexpr size_t tszrmchar(tchar* const psz, tchar const* const pszMax, tchar chRemove) {
		if (!psz || !chRemove)
			return 0;
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
				if (!(*pos = *p2))
					break;
				pos++;
			}
		}
		return pos - psz;
	}
	template < gtlc::string_elem tchar, gtlc::string_buffer_fixed_c<tchar> tstring_buf >
	constexpr inline size_t tszrmchar(tstring_buf &sz, tchar chRemove) {
		return tszrmchar(std::data(sz), std::data(sz) + std::size(sz), chRemove);
	}

	namespace internal {
		template < gtlc::string_elem tchar, bool bUseCount, class Eval >
		constexpr [[nodiscard]] int tszcmp(tchar const* pszA, tchar const* pszB, size_t nCount, Eval&& eval) {
			if constexpr (bUseCount) {
				if (!nCount)
					return 0;
				if (nCount > RSIZE_MAX)
					throw std::invalid_argument(GTL__FUNCSIG "nCount is too big!");
			}
			if (!pszA && !pszB)	// if both are nullptr, return 0;
				return 0;
			if (pszA && !pszB)	// if only one has value, its bigger.
				return *pszA ? *pszA : 0;//1;
			else if (!pszA && pszB)
				return *pszB ? -*pszB : 0;//-1;

			for (; *pszA && *pszB; pszA++, pszB++) {
				int r = eval(*pszA) - eval(*pszB);
				if (r)
					return r;
				if constexpr (bUseCount) {
					if (--nCount == 0)
						return 0;
				}
			}
			if (!*pszA && !*pszB)
				return 0;
			int r = eval(*pszA) - eval(*pszB);
			return r;
		}

		template < gtlc::string_elem tchar, bool bUseCount, class Eval >
		constexpr [[nodiscard]] int tszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount, Eval&& eval) {
			if constexpr (bUseCount) {
				if (!nCount)
					return 0;
				if (nCount > RSIZE_MAX)
					throw std::invalid_argument(GTL__FUNCSIG "nCount is too big!");
			}
			bool bEmptyA = svA.empty();
			bool bEmptyB = svB.empty();
			if (bEmptyA || bEmptyB) {
				if (bEmptyA and bEmptyB)
					return 0;
				if (!bEmptyA)
					return 1;
				if (!bEmptyB)
					return -1;
			}
			tchar const* pszA = svA.data();
			tchar const* pszB = svB.data();
			tchar const* const pszAEnd = svA.data() + svA.size();
			tchar const* const pszBEnd = svB.data() + svB.size();

			for (; pszA < pszAEnd and pszB < pszBEnd; pszA++, pszB++) {
				int r = eval(*pszA) - eval(*pszB);
				if (r)
					return r;
				if constexpr (bUseCount) {
					if (--nCount == 0)
						return 0;
				}
				if (!*pszA)	[[unlikely]]
					return 0;
			}
			//if ((pszA < pszAEnd) and (pszB < pszBEnd)) [[unlkely]] // actually, never happens here.
			//{
			//	
			//	return eval(*pszA, *pszB);
			//}
			//else {
				if (pszA < pszAEnd)
					return eval(*pszA);
				if (pszB < pszBEnd)
					return -eval(*pszB);
			//}

			return 0;
		}
	}


	template < gtlc::string_elem tchar >
	constexpr inline GTL_DEPR_SEC [[nodiscard]] int tszcmp(tchar const* pszA, tchar const* pszB) {
		return gtl::internal::tszcmp<tchar, false>(pszA, pszB, {}, [](auto v) -> int { return (int)v; });
	}
	template < gtlc::string_elem tchar >
	constexpr inline GTL_DEPR_SEC [[nodiscard]] int tszncmp(tchar const* pszA, tchar const* pszB, size_t nCount) {
		return gtl::internal::tszcmp<tchar, true>(pszA, pszB, nCount, [](auto v) -> int { return (int)v; });
	}
	template < gtlc::string_elem tchar >
	constexpr inline GTL_DEPR_SEC [[nodiscard]] int tszicmp(tchar const* pszA, tchar const* pszB) {
		return gtl::internal::tszcmp<tchar, false>(pszA, pszB, {}, [](auto v) -> int { return std::tolower(v); });
	}
	template < gtlc::string_elem tchar >
	constexpr inline GTL_DEPR_SEC [[nodiscard]] int tsznicmp(tchar const* pszA, tchar const* pszB, size_t nCount) {
		return gtl::internal::tszcmp<tchar, true>(pszA, pszB, nCount, [](auto v) -> int { return std::tolower(v); });
	}

	template < gtlc::string_elem tchar >
	constexpr inline [[nodiscard]] int tszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB) {
		return gtl::internal::tszcmp<tchar, false>(svA, svB, {}, [](auto v) -> int { return (int)v; });
	}
	template < gtlc::string_elem tchar >
	constexpr inline [[nodiscard]] int tszncmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount) {
		return gtl::internal::tszcmp<tchar, true>(svA, svB, nCount, [](auto v) -> int { return (int)v; });
	}
	template < gtlc::string_elem tchar >
	constexpr inline [[nodiscard]] int tszicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB) {
		return gtl::internal::tszcmp<tchar, false>(svA, svB, {}, [](auto v) -> int { return std::tolower(v); });
	}
	template < gtlc::string_elem tchar >
	constexpr inline [[nodiscard]] int tsznicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount) {
		return gtl::internal::tszcmp<tchar, true>(svA, svB, nCount, [](auto v) -> int { return std::tolower(v); });
	}


	template < gtlc::string_buffer_fixed tstring_buf >
	errno_t tszupr(tstring_buf& buf) {
		if (!std::data(buf))
			return EINVAL;
		if (std::size(buf) > RSIZE_MAX)
			return ERANGE;
		for (auto& c : buf) {
			if (!c)
				break;
			MakeUpper(c);
		}
		return 0;
	}
	template < gtlc::string_elem tchar >
	errno_t tszupr(tchar* const psz, size_t size) {
		if (!psz)
			return EINVAL;
		if (size > RSIZE_MAX)
			return ERANGE;
		tchar* pos = psz;
		for (size_t i = 0; i < size; i++) {
			if (!*pos)
				break;
			MakeUpper(*pos);
		}
		return 0;
	}

	template < gtlc::string_buffer_fixed tstring_buf >
	errno_t tszlwr(tstring_buf& buf) {
		if (!std::data(buf))
			return EINVAL;
		if (std::size(buf) > RSIZE_MAX)
			return ERANGE;
		for (auto& c : buf) {
			if (!c)
				break;
			MakeLower(c);
		}
		return 0;
	}
	template < gtlc::string_elem tchar >
	errno_t tszlwr(tchar* const& psz, size_t size) {
		if (!psz)
			return EINVAL;
		if (size > RSIZE_MAX)
			return ERANGE;
		tchar* pos = psz;
		for (size_t i = 0; i < size; i++) {
			if (!*pos)
				break;
			MakeLower(*pos);
		}
		return 0;
	}

	//template < gtlc::string_elem tchar >
	//GTL_DEPR_SEC tchar* tszrev(tchar* psz) {
	//	if (!psz)
	//		return nullptr;
	//	std::reverse(psz, psz + tszlen(psz));
	//	return psz;
	//}
	template < gtlc::string_elem tchar >
	tchar* tszrev(tchar* psz, tchar const* const pszEnd) {
		if (!psz)
			return nullptr;
		std::reverse(psz, pszEnd);
		return psz;
	}
	template < gtlc::string_buffer_fixed tstring_buf >
	std::remove_cvref_t<decltype(tstring_buf{}[0])>* tszrev(tstring_buf& buf) {
		std::reverse(std::data(buf), std::data(buf)+tszlen(buf));
		return std::data(buf);
	}

#if 0
	template < gtlc::string_elem tchar >
	tchar const* tszsearch(tchar const* psz, tchar c) {
		if (!psz)
			return nullptr;
		for (tchar const* pos = psz; *pos; pos++) {
			if (*pos == c)
				return pos;
		}
		return nullptr;
	}
	template < gtlc::string_elem tchar >
	tchar* tszsearch(tchar* psz, tchar c) {
		if (!psz)
			return nullptr;
		for (tchar* pos = psz; *pos; pos++) {
			if (*pos == c)
				return pos;
		}
		return nullptr;
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


	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar c) {
		if (!psz)
			return nullptr;
		auto* p = std::search(psz, pszEnd, c);
		if (p == pszEnd)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar c) {
		if (!psz)
			return nullptr;
		auto* p = std::search(psz, pszEnd, c);
		if (p == pszEnd)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar const* pszSub, tchar const* const pszSubEnd) {
		if (!psz || !pszSub)
			return nullptr;
		auto* p = std::search(psz, pszEnd, pszSub, pszSubEnd);
		if (p == pszEnd)
			return nullptr;
		return p;
	}
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar* pszSub, tchar const* const pszSubEnd) {
		if (!psz || !pszSub)
			return nullptr;
		auto* p = std::search(psz, pszEnd, pszSub, pszSubEnd);
		if (p == pszEnd)
			return nullptr;
		return p;
	}


	/// @brief  tszsearch_oneof
	/// @param psz    : string
	/// @param pszSet : chars to find
	/// @return position of found char.
	template < gtlc::string_elem tchar >
	GTL_DEPR_SEC tchar* tszsearch_oneof(tchar* psz, tchar const * const pszSet) {
		for (; *psz; psz++) {
			if (tszsearch(pszSet, *psz))
				return psz;
		}
		return nullptr;
	}
	template < gtlc::string_elem tchar >
	GTL_DEPR_SEC tchar const* tszsearch_oneof(tchar const* psz, tchar const* const pszSet) {
		for (; *psz; psz++) {
			if (tszsearch(pszSet, *psz))
				return psz;
		}
		return nullptr;
	}
	template < gtlc::string_elem tchar >
	constexpr tchar* tszsearch_oneof(tchar* psz, tchar* pszEnd, tchar const* const pszSet, tchar const* const pszSetEnd) {
		if (!psz || !pszSet)
			return nullptr;
		for (; (psz < pszEnd) and *psz; psz++) {
			if (tszsearch(pszSet, pszSetEnd, *psz))
				return psz;
		}
		return nullptr;
	}
	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(tchar const* psz, tchar const* pszEnd, tchar const* const pszSet, tchar const* const pszSetEnd) {
		if (!psz || !pszSet)
			return nullptr;
		for (; (psz < pszEnd) and *psz; psz++) {
			if (tszsearch(pszSet, pszSetEnd, *psz))
				return psz;
		}
		return nullptr;
	}

	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svSet) {
		tchar const* psz = sv.data();
		tchar const* pszEnd = sv.data() + sv.size();
		for (; (psz < pszEnd) and *psz; psz++) {
			if (tszsearch(svSet, *psz))
				return psz;
		}
		return nullptr;
	}
#endif

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
	///	   ex "0b1001'1100", "0xABCD1234", "0123456"
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

		return 0;
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
