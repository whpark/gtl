//////////////////////////////////////////////////////////////////////
//
// string_primitives.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

module;

#include <cerrno>
#include <cmath>
#include <cctype>
//#include <string>
#include <string_view>
#include <charconv>
#include <algorithm>
#include <stdexcept>
#include <concepts>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:string_primitives;
import :concepts;

export namespace gtl {


	/// @brief  tszlen (string length). you cannot input string literal. you don't need tszlen("testlen");. ==> just call "testlen"sv.size();
	/// @param psz : null terminating char_t* var. (no string literals)
	/// @return string length
	/// @param pszMax : (end+1) of the allocated buffer.
	/// @return string length. if not reached, SIZE_MAX.
	template < gtlc::string_elem tchar >				constexpr		 [[nodiscard]] GTL__DEPR_SEC size_t tszlen(tchar const*const& psz);
	template < gtlc::string_elem tchar >				constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, tchar const* const pszMax);
	template < gtlc::string_elem tchar >				constexpr inline [[nodiscard]] size_t tszlen(tchar const* psz, size_t sizeOfBuf);
	template < gtlc::contiguous_string_container tstring_buf >	constexpr inline [[nodiscard]] size_t tszlen(tstring_buf const& v);

	// todo : documents...

	/// @brief utf8/16/32/wchar_t ver. of strcpy_s
	/// @param pszDest 
	/// @param sizeDest 
	/// @param pszSrc 
	/// @return 0 : ok.
	///  EINVAL : if !pszDest || !pszSrc
	///  ERANGE : if sizeDest is smaller
	template < gtlc::string_elem tchar >
	constexpr		 GTL__DEPR_SEC errno_t tszcpy(tchar* const& pszDest, size_t sizeDest, tchar const*const& pszSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline GTL__DEPR_SEC errno_t tszcpy(tstring_buf& szDest, tchar const* const& pszSrc);


	/// @brief tszcpy utf8/16/32/wchar_t ver. of strcpy_s
	/// @param pszDest dest. buffer
	/// @param sizeDest size of dest. including null terminating char.</param>
	/// @param szDest dest. string buffer. 
	///  ex) char szDest[30];</param>
	///      std::array<char16_t> szDest;</param>
	///      std::vector<char> szDest;</param>
	/// @param svSrc source string. do not need to be NULL terminated string</param>
	/// @param strSrc null terminated source string.
	/// @return 
	///  0 : ok.
	///  EINVAL : if !pszDest
	///  ERANGE : if sizeDest is not enough.
	template < gtlc::string_elem tchar >
	constexpr errno_t tszcpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc);

	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string_view<tchar> svSrc);

	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string<tchar> const& strSrc);

	// tszncpy
	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszncpy(tchar*const& pszDest, size_t sizeDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf& szDest, tchar const* pszSrc, size_t nCount = _TRUNCATE);

	// tszncpy (sv)
	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszncpy(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string_view<tchar> svSrc, size_t nCount = _TRUNCATE);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string<tchar> const& svSrc, size_t nCount = _TRUNCATE);

	// tszcat
	template < gtlc::string_elem tchar >
	constexpr		 GTL__DEPR_SEC errno_t tszcat(tchar* pszDest, size_t sizeDest, tchar const* pszSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline GTL__DEPR_SEC errno_t tszcat(tstring_buf& szDest, tchar const* pszSrc);

	template < gtlc::string_elem tchar >
	constexpr		 errno_t tszcat(tchar* pszDest, size_t sizeDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string_view<tchar> svSrc);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string<tchar> const& strSrc);


	/// @brief Remove Charactors from str.
	/// @param psz 
	/// @param chRemove 
	/// @return str length
	template < gtlc::string_elem tchar >
	constexpr GTL__DEPR_SEC	size_t tszrmchar(tchar* const& psz, tchar chRemove);
	template < gtlc::string_elem tchar >
	constexpr size_t tszrmchar(tchar* const psz, tchar const* const pszMax, tchar chRemove);
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline size_t tszrmchar(tstring_buf &sz, tchar chRemove);


	template < gtlc::string_elem tchar > constexpr inline GTL__DEPR_SEC [[nodiscard]] int tszcmp(tchar const* pszA, tchar const* pszB);
	template < gtlc::string_elem tchar > constexpr inline GTL__DEPR_SEC [[nodiscard]] int tszncmp(tchar const* pszA, tchar const* pszB, size_t nCount);
	template < gtlc::string_elem tchar > constexpr inline GTL__DEPR_SEC [[nodiscard]] int tszicmp(tchar const* pszA, tchar const* pszB);
	template < gtlc::string_elem tchar > constexpr inline GTL__DEPR_SEC [[nodiscard]] int tsznicmp(tchar const* pszA, tchar const* pszB, size_t nCount);

	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tszcmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tszncmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tszicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB);
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] int tsznicmp(std::basic_string_view<tchar> svA, std::basic_string_view<tchar> svB, size_t nCount);

	template < gtlc::contiguous_string_container tstring_buf >	errno_t tszupr(tstring_buf& buf);
	template < gtlc::string_elem tchar >			 	errno_t tszupr(tchar* const psz, size_t size);
	template < gtlc::contiguous_string_container tstring_buf >	errno_t tszlwr(tstring_buf& buf);
	template < gtlc::string_elem tchar >			 	errno_t tszlwr(tchar* const& psz, size_t size);


	//template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar* tszrev(tchar* psz);
	template < gtlc::string_elem tchar > 			  [[nodiscard]] tchar* tszrev(tchar* psz, tchar const* const pszEnd);
	template < gtlc::contiguous_string_container tstring_buf > std::remove_cvref_t<decltype(tstring_buf{}[0])>* tszrev(tstring_buf& buf);

#if 0     // use string_view, or stl
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar c);
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar* tszsearch(tchar* psz, tchar c);
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* pszSub);
	template < gtlc::string_elem tchar > GTL__DEPR_SEC [[nodiscard]] tchar* tszsearch(tchar* psz, tchar* pszSub);

	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar c);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar c);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar const* tszsearch(tchar const* psz, tchar const* const pszEnd, tchar const* pszSub, tchar const* const pszSubEnd);
	template < gtlc::string_elem tchar > constexpr [[nodiscard]] tchar* tszsearch(tchar* psz, tchar const* const pszEnd, tchar* pszSub, tchar const* const pszSubEnd);


	/// @brief  tszsearch_oneof
	/// @param psz    : string
	/// @param pszSet : chars to find
	/// @return position of found char.
	template < gtlc::string_elem tchar >
	GTL__DEPR_SEC tchar* tszsearch_oneof(tchar* psz, tchar const * const pszSet);
	template < gtlc::string_elem tchar >
	GTL__DEPR_SEC tchar const* tszsearch_oneof(tchar const* psz, tchar const* const pszSet);

	template < gtlc::string_elem tchar >
	constexpr tchar* tszsearch_oneof(tchar* psz, tchar* pszEnd, tchar const * const pszSet, tchar const* const pszSetEnd);
	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(tchar const* psz, tchar const* pszEnd, tchar const * const pszSet, tchar const* const pszSetEnd);
	template < gtlc::string_elem tchar >
	constexpr tchar const* tszsearch_oneof(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svSet);
#endif


};	// namespace gtl;

export namespace gtl {

	/// @brief  tszlen (string length). you cannot input string literal. you don't need tszlen("testlen");. ==> just call "testlen"sv.size();
	/// @param psz : null terminating char_t* var. (no string literals)
	/// @return string length
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] GTL__DEPR_SEC size_t tszlen(tchar const*const& psz) {
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
	template < gtlc::contiguous_string_container tstring_buf >
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
	constexpr GTL__DEPR_SEC errno_t tszcpy(tchar* const& pszDest, size_t sizeDest, tchar const* const& pszSrc) {
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
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline GTL__DEPR_SEC errno_t tszcpy(tstring_buf& szDest, tchar const* const& pszSrc) {
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

	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string_view<tchar> svSrc) {
		tchar* const pszDest = std::data(szDest);
		size_t sizeDest = std::size(szDest);
		return tszcpy(pszDest, sizeDest, svSrc);
	}
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcpy(tstring_buf& szDest, std::basic_string<tchar> const& strSrc) {
		tchar* const pszDest = std::data(szDest);
		size_t sizeDest = std::size(szDest);
		return tszcpy(pszDest, sizeDest, std::basic_string_view{strSrc.data(), strSrc.size()});

	}

#if 0	// any type <- any type.... �ʹ� ���� �ɼ�.
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
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
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
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string_view<tchar> svSrc, size_t nCount) {
		return tszncpy(std::data(szDest), std::size(szDest), svSrc, nCount);
	}
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszncpy(tstring_buf &szDest, std::basic_string<tchar> const& strSrc, size_t nCount) {
		return tszncpy(std::data(szDest), std::size(szDest), std::basic_string_view<tchar>{strSrc}, nCount);
	}

	// tszcat
	template < gtlc::string_elem tchar >
	constexpr GTL__DEPR_SEC errno_t tszcat(tchar* pszDest, size_t sizeDest, tchar const* pszSrc) {
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
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline GTL__DEPR_SEC errno_t tszcat(tstring_buf &szDest, tchar const* pszSrc) {
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
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string_view<tchar> svSrc) {
		return tszcat(std::data(szDest), std::size(szDest), svSrc);
	}
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
	constexpr inline errno_t tszcat(tstring_buf &szDest, std::basic_string<tchar> const& strSrc) {
		return tszcat(std::data(szDest), std::size(szDest), std::basic_string_view{strSrc.data(), strSrc.size()});
	}

	template < gtlc::string_elem tchar >
	constexpr GTL__DEPR_SEC size_t tszrmchar(tchar* const& psz, int chRemove) {
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
	template < gtlc::string_elem tchar, gtlc::contiguous_type_string_container<tchar> tstring_buf >
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
	constexpr inline GTL__DEPR_SEC [[nodiscard]] int tszcmp(tchar const* pszA, tchar const* pszB) {
		return gtl::internal::tszcmp<tchar, false>(pszA, pszB, {}, [](auto v) -> int { return (int)v; });
	}
	template < gtlc::string_elem tchar >
	constexpr inline GTL__DEPR_SEC [[nodiscard]] int tszncmp(tchar const* pszA, tchar const* pszB, size_t nCount) {
		return gtl::internal::tszcmp<tchar, true>(pszA, pszB, nCount, [](auto v) -> int { return (int)v; });
	}
	template < gtlc::string_elem tchar >
	constexpr inline GTL__DEPR_SEC [[nodiscard]] int tszicmp(tchar const* pszA, tchar const* pszB) {
		return gtl::internal::tszcmp<tchar, false>(pszA, pszB, {}, [](auto v) -> int { return std::tolower(v); });
	}
	template < gtlc::string_elem tchar >
	constexpr inline GTL__DEPR_SEC [[nodiscard]] int tsznicmp(tchar const* pszA, tchar const* pszB, size_t nCount) {
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


	template < gtlc::contiguous_string_container tstring_buf >
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

	template < gtlc::contiguous_string_container tstring_buf >
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
	//GTL__DEPR_SEC tchar* tszrev(tchar* psz) {
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
	template < gtlc::contiguous_string_container tstring_buf >
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
	GTL__DEPR_SEC tchar* tszsearch_oneof(tchar* psz, tchar const * const pszSet) {
		for (; *psz; psz++) {
			if (tszsearch(pszSet, *psz))
				return psz;
		}
		return nullptr;
	}
	template < gtlc::string_elem tchar >
	GTL__DEPR_SEC tchar const* tszsearch_oneof(tchar const* psz, tchar const* const pszSet) {
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

};	// namespace gtl;

