//////////////////////////////////////////////////////////////////////
//
// string_misc.hpp : inline functions (Don't include this header file directly!)
//
// PWH
// 2020.12.16.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__BASIC_STRING_MISC_IMPL
#define GTL_HEADER__BASIC_STRING_MISC_IMPL

#include "gtl/_default.h"
#include "latin_charset.h"
//#include <gtl/concepts.h>
//#include <gtl/string/string_primitives.h>

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
	namespace charset {
		template < gtlc::string_elem tchar >
		struct S_ULPair { tchar cUpper, cLower; };

		constexpr static inline S_ULPair<char16_t> latin1_diff { 0xC0, 0xE0 };	// À, à'
		constexpr static inline S_ULPair<char16_t> latin1_exception1 { 0x00D7, 0x00F7 }; // ×, ÷
		constexpr static inline S_ULPair<char16_t> latin1_exception2 { 0x039c, 0x00b5 }; // µ, Μ : 743
		constexpr static inline S_ULPair<char16_t> latin1_exception3 { 0x0178, 0x00ff }; // ÿ, Ÿ : 121


		template < typename tchar >
		struct S_MAP_TABLE {
			std::pair<tchar, tchar> const range;
			std::map<tchar, tchar> const& map;
		};

		static inline std::array<S_MAP_TABLE<char16_t>, 4> mapUL16 { {
			//{ rangeUL_latin1_g,			mapUL_latin1_g},
			{ rangeUL_latin_extended_g, mapUL_latin_extended_g },
			{ rangeUL_other1_g,			mapUL_other1_g },
			{ rangeUL_other2_g,			mapUL_other2_g },
			{ rangeUL_other3_g,			mapUL_other3_g },
		} };

		static inline std::array<S_MAP_TABLE<char16_t>, 5> mapLU16 { {
			{ rangeLU_latin1_g,			mapLU_latin1_g},
			{ rangeLU_latin_extended_g, mapLU_latin_extended_g },
			{ rangeLU_other1_g,			mapLU_other1_g },
			{ rangeLU_other2_g,			mapLU_other2_g },
			{ rangeLU_other3_g,			mapLU_other3_g },
		} };

		static inline std::array<S_MAP_TABLE<char32_t>, 3> mapUL32 { {
			{ rangeUL_other4_g,			mapUL_other4_g },
			{ rangeUL_other5_g,			mapUL_other5_g },
			{ rangeUL_other6_g,			mapUL_other6_g },
		} };

		static inline std::array<S_MAP_TABLE<char32_t>, 3> mapLU32 { {
			{ rangeLU_other4_g,			mapLU_other4_g },
			{ rangeLU_other5_g,			mapLU_other5_g },
			{ rangeLU_other6_g,			mapLU_other6_g },
		} };

	}

	/// @brief ToLower, ToUpper, ToDigit, IsSpace ... (locale irrelavant)
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToLower(tchar c) {
		if (c < 'A')
			return c;

		if (c <= 'Z')
			return c - 'A' + 'a';

		using namespace gtl::charset;

		if constexpr (sizeof(tchar) >= sizeof(char16_t)) {
			// Latin-1
			if (c < rangeUL_latin1_g.first)
				return c;
			if (c < rangeUL_latin1_g.second) {
				if (c != latin1_exception1.cUpper)
					return c + latin1_diff.cLower - latin1_diff.cUpper;
				return c;
			}
			// Latin Extended, others
			for (auto const& [range, map] : mapUL16) {
				if (c < range.first)
					return c;
				if (c < range.second) {
					if (auto iter = map.find(c); iter != map.end()) {
						return iter->second;
					}
					return c;
				}
			}
		}

		if constexpr (sizeof(tchar) >= sizeof(char32_t)) {
			for (auto const& [range, map] : mapUL32) {
				if (c < range.first)
					return c;
				if (c < range.second) {
					if (auto iter = map.find(c); iter != map.end()) {
						return iter->second;
					}
					return c;
				}
			}
		}
		return c;
	}
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar ToUpper(tchar c) {
		if (c < 'a')
			return c;
		if (c <= 'z')
			return c - 'a' + 'A';

		using namespace gtl::charset;

		if constexpr (sizeof(tchar) >= sizeof(char16_t)) {
			//// Latin-1
			//if (c < rangeLU_latin1_g.first) {
			//	if (c == latin1_exception2.cLower)		// { 0x039c, 0x00b5 }; // µ, Μ : 743
			//		return latin1_exception2.cUpper;
			//	return c;
			//}
			//if (c < rangeLU_latin1_g.second) {
			//	if (c == latin1_exception3.cLower)		// { 0x0178, 0x00ff }; // ÿ, Ÿ : 121
			//		return latin1_exception3.cUpper;
			//	if (c != latin1_exception1.cLower)
			//		return c - latin1_diff.cLower + latin1_diff.cUpper;
			//	return c;
			//}
			// Latin Extended, others
			for (auto const& [range, map] : mapLU16) {
				if (c < range.first)
					return c;
				if (c < range.second) {
					if (auto iter = map.find(c); iter != map.end()) {
						return iter->second;
					}
					return c;
				}
			}
		}

		if constexpr (sizeof(tchar) >= sizeof(char32_t)) {
			for (auto const& [range, map] : mapLU32) {
				if (c < range.first)
					return c;
				if (c < range.second) {
					if (auto iter = map.find(c); iter != map.end()) {
						return iter->second;
					}
					return c;
				}
			}
		}
		return c;
	}
	template < gtlc::string_elem tchar > constexpr inline               void MakeLower(tchar& c) {
		c = ToLower(c);
	}
	template < gtlc::string_elem tchar > constexpr inline               void MakeUpper(tchar& c) {
		c = ToUpper(c);
	}
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsDigit(tchar const c) {
		return (c >= '0') && (c <= '9');
	}
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsOdigit(tchar const c) {
		return (c >= '0') && (c <= '7');
	}
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsXdigit(tchar const c) {
		return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
	}
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsSpace(tchar const c) {
		return (c == '\t') || (c == '\r') || (c == '\n') || (c == ' ');
	}
	template < gtlc::string_elem tchar > constexpr inline [[nodiscard]] tchar IsNotSpace(tchar const c) {
		return !IsSpace(c);
	}


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


	namespace internal {
		template < gtlc::string_elem tchar, typename treturn >
		[[nodiscard]] std::vector<treturn> TSplit(std::basic_string_view<tchar> sv, std::function<bool(tchar)> func) {
			std::vector<treturn> r;
			if (sv.empty())
				return r;
			tchar const* pos = sv.data();
			tchar const* end = sv.data() + sv.size();
			tchar const* s = pos;
			for (; pos < end; pos++) {
				if (!func(*pos))
					continue;
				r.emplace_back(s, pos);
				s = pos+1;
			}
			r.emplace_back(s, end);

			return r;
		}
	}	// namespace internal
	/// @brief Split String into ...
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string<tchar>> Split(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svDelimiters) {
		return internal::TSplit<tchar, std::basic_string<tchar>>(sv, [svDelimiters](tchar c) { return svDelimiters.find(c) != svDelimiters.npos; });
	}
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string<tchar>> Split(std::basic_string_view<tchar> sv, tchar cDelimiter) {
		return internal::TSplit<tchar, std::basic_string<tchar>>(sv, [cDelimiter](tchar c) { return cDelimiter == c; });
	}
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string_view<tchar>> SplitView(std::basic_string_view<tchar> sv, std::basic_string_view<tchar> svDelimiters) {
		return gtl::internal::TSplit<tchar, std::basic_string_view<tchar>>(sv, [svDelimiters](tchar c) { return svDelimiters.find(c) != svDelimiters.npos; });
	}
	template < gtlc::string_elem tchar >
	inline [[nodiscard]] std::vector<std::basic_string_view<tchar>> SplitView(std::basic_string_view<tchar> sv, tchar cDelimiter) {
		return gtl::internal::TSplit<tchar, std::basic_string_view<tchar>>(sv, [cDelimiter](tchar c) { return cDelimiter == c; });
	}


	namespace internal {
		template < gtlc::string_elem tchar, size_t max_digit_len = 0 >
		constexpr bool CheckDigitLen_Progress(std::basic_string<tchar>& str, tchar const*& pos, tchar const* const end, int radix, tchar cFill, tchar cTerminating) {
			if constexpr (max_digit_len != 0) {
				if ( (pos + max_digit_len > end) or (*pos == cTerminating) )
					return false;
			}
			else {
				if ( (pos >= end) or (*pos == cTerminating) )
					return false;
			}

			// check end point
			tchar* pHexEnd{};
			char32_t v{};
			if constexpr (max_digit_len != 0) {
				v = tsztoi<char32_t>(std::basic_string_view<tchar>{pos, pos+max_digit_len}, &pHexEnd, radix);
				if (pos + max_digit_len != pHexEnd) {
					str += cFill;
					return false;
				}
			}
			else {
				v = tsztoi<char32_t>(std::basic_string_view<tchar>{pos, end}, &pHexEnd, radix);
			}

			// if no digits,
			if (pos == pHexEnd) {
				str += cFill;
				return false;
			}

			if constexpr (gtlc::is_one_of<tchar, char>) {
				str += (v <= 0xff) ? (char)v : cFill;
			}
			else if constexpr (gtlc::is_one_of<tchar, char8_t>) {//, char16_t, char32_t, wchar_t>) {
				if (v <= 0x7f) {
					str += (char8_t)v;
				} else {
					str += gtl::ToStringU8(std::basic_string_view<char32_t>{&v, &v+1});
				}
			}
			else if constexpr (gtlc::is_one_of<tchar, char16_t, wchar_t>) {
				if (v <= 0xffff) {
					str += (char16_t)v;
				} else {
					str += gtl::ToString<char32_t, tchar>(std::basic_string_view<char32_t>{&v, &v+1});
				}
			}
			else if constexpr (gtlc::is_one_of<tchar, char32_t>) {
				str += v;
			}
#if GTL_STRING_SUPPORT_CODEPAGE_KSSM
			else if constexpr (gtlc::is_one_of<tchar, uint16_t>) {
				str += (v < 0xffff) ? (uint16_t)v : cFill;
			}
#endif
			else {
				static_assert(false);
			}

			pos = pHexEnd-1;
		}
	}

	/// @brief Translate Escape Sequence String
	template < gtlc::string_elem tchar >
	constexpr [[nodiscard]] std::optional<std::basic_string<tchar>> TranslateEscapeSequence(std::basic_string_view<tchar> sv, tchar cFill, tchar cTerminating) {
		std::basic_string<tchar> str;
		str.reserve(sv.size());
		const tchar* pos{};
		const tchar* const end = sv.data() + sv.size();

		// first, check and count number of bytes to be translated into
		for (pos = sv.data(); (pos < end) and (*pos != cTerminating); pos++) {
			if (*pos == (tchar)'\\') {
				if ( (++pos >= end) or (*pos == cTerminating) )
					return {};

				if (*pos == (tchar)'x') { // Hexa ASCII Code
					pos++;
					if (!internal::CheckDigitLen_Progress<tchar, 0>(str, pos, end, 16, cFill, cTerminating))
						return {};
				} else if (*pos == (tchar)'u') { // Unicode (16 bit)
					pos++;
					if (!internal::CheckDigitLen_Progress<tchar, 4>(str, pos, end, 16, cFill, cTerminating))
						return {};
				} else if (*pos == (tchar)'U') {	// Unicode (32 bit)
					pos++;
					if (!internal::CheckDigitLen_Progress<tchar, 8>(str, pos, end, 16, cFill, cTerminating))
						return {};
				} else if (IsOdigit(*pos)) {		// Octal ASCII Code
					if (!internal::CheckDigitLen_Progress<tchar, 0>(str, pos, std::min(pos+3, end), 8, cFill, cTerminating))
						return {};
				} else {
					// constexpr map ... (not yet)
					constexpr static const struct {
						tchar cEscape;
						tchar cValue;
					} escapes[] = {
						{ (tchar)'a', (tchar)'\a' },
						{ (tchar)'b', (tchar)'\b' },
						{ (tchar)'f', (tchar)'\f' },
						{ (tchar)'n', (tchar)'\n' },
						{ (tchar)'r', (tchar)'\r' },
						{ (tchar)'t', (tchar)'\t' },
						{ (tchar)'v', (tchar)'\v' },
					};
					tchar v {*pos};
					auto iter = std::find_if(std::begin(escapes), std::end(escapes), [v](auto p){ return p.cEscape == v;});
					str += (iter == std::end(escapes)) ? v : iter->cValue;
					//for (auto const& [esc, value] : escapes) {
					//	if (esc != *pos)
					//		continue;
					//	v = value;
					//	break;
					//}
					//str += v;
				}
			} else {
				str += *pos;
			}
		}

		return str;
	}


#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__BASIC_STRING_MISC_IMPL
