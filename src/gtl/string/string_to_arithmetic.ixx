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

export module gtl:string_to_arithmetic;
import :concepts;
import :string_misc;

export namespace gtl {


	/// @brief tszto_number
	/// @param psz 
	/// @param pszEnd 
	/// @param radix 
	/// @return number
	template < gtlc::arithmetic tvalue, gtlc::string_elem tchar>
	tvalue tszto(tchar const* psz, tchar const* pszEnd, tchar const** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);


	/// <summary>
	/// digit contants to integral type value.
	///  - radix detecting (c++ notation)
	///	   ex "0b1001'1100", "0xABCD1234", "0123456"
	///  - digit seperator (such as ',' or '\'' for c++ notation.)
	///  - only throws when (radix > 36)
	/// </summary>
	/// <param name="psz"></param>
	/// <param name="pszEnd"></param>
	/// <param name="ppszStopped">where conversion stopped</param>
	/// <param name="radix">radix</param>
	/// <param name="cSplitter">digit splitter. such as ',' (thousand sepperator) or '\'' (like c++v14 notation)</param>
	/// <returns>number value. (no overflow checked)</returns>
	template < std::integral tvalue = int, gtlc::string_elem tchar >
	constexpr [[nodiscard]] tvalue tsztoi(std::basic_string_view<tchar> svNumberString, tchar const** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);
	template < std::integral tvalue = int, gtlc::string_elem tchar >
	inline [[nodiscard]] tvalue tsztoi(std::basic_string<tchar> const& str, tchar const** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);
	template < std::integral tvalue = int, gtlc::string_elem tchar >
	GTL__DEPR_SEC inline [[nodiscard]] tvalue tsztoi(tchar const* const& psz, tchar const** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);
	template < std::integral tvalue = int, gtlc::string_elem tchar, size_t size >
	constexpr inline [[nodiscard]] tvalue tsztoi(tchar const (&sz)[size], tchar const** ppszStopped = nullptr, int radix = 0, tchar cSplitter = 0);

	template < std::floating_point tvalue = double, gtlc::string_elem tchar = char16_t >
	[[deprecated("NOT A STANDARD CONVERTING !")]] constexpr tvalue _tsztod(tchar const* psz, tchar const* pszEnd, tchar const** ppszStopped = nullptr, tchar cSplitter = 0);

	template < std::floating_point tvalue = double, gtlc::string_elem tchar >
	[[nodiscard]] tvalue tsztod(std::basic_string_view<tchar> sv, tchar const** ppszStopped = nullptr, tchar cSplitter = 0);
	template < std::floating_point tvalue = double, gtlc::string_elem tchar >
	inline [[nodiscard]] tvalue tsztod(std::basic_string<tchar> const& str, tchar const** ppszStopped = nullptr, tchar cSplitter = 0);
	template < std::floating_point tvalue = double, gtlc::string_elem tchar >
	GTL__DEPR_SEC inline [[nodiscard]] tvalue tsztod(tchar const* psz, tchar const** ppszStopped = nullptr, tchar cSplitter = 0);


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


	/// @brief tszto_number
	/// @param psz 
	/// @param pszEnd 
	/// @param radix 
	/// @return number
	template < gtlc::arithmetic tvalue, gtlc::string_elem tchar>
	tvalue tszto(tchar const* psz, tchar const* const pszEnd, tchar const** ppszStopped, int radix, tchar cSplitter) {
		if constexpr (std::is_integral_v<tvalue>) {
			return tsztoi<tvalue>(std::basic_string_view<tchar>{psz, pszEnd}, ppszStopped, radix, cSplitter);
		} else {
			return tsztod<tvalue>(std::basic_string_view<tchar>{psz, pszEnd}, ppszStopped, cSplitter);
		}
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
	constexpr [[nodiscard]] tvalue tsztoi(std::basic_string_view<tchar> svNumberString, tchar const** ppszStopped, int radix, tchar cSplitter) {
		if (svNumberString.empty()) {
			if (ppszStopped)
				*ppszStopped = svNumberString.data();
			return {};
		}

		tchar const* psz = svNumberString.data();
		tchar const* const pszEnd = svNumberString.data() + svNumberString.size();

		// skip white space
		while ((psz < pszEnd) && IsSpace(*psz))
			psz++;

		if (psz >= pszEnd) {
			if (ppszStopped)
				*ppszStopped = psz;
			return {};
		}

		// Check sign (+/-)
		bool bMinus{};
		if (*psz == '-')
			psz++, bMinus = true;
		else if (*psz == '+')
			psz++;

		// skip white space
		while ((psz < pszEnd) && IsSpace(*psz))
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
			value = 0-value;	// not just '-value' but '0-value' to suppress warning.

		if (ppszStopped)
			*ppszStopped = psz;

		return value;
	}

	template < std::integral tvalue, gtlc::string_elem tchar >
	inline [[nodiscard]] tvalue tsztoi(std::basic_string<tchar> const& str, tchar const** ppszStopped, int radix, tchar cSplitter) {
		return tsztoi<tvalue, tchar>((std::basic_string_view<tchar>)str, ppszStopped, radix, cSplitter);
	}
	template < std::integral tvalue, gtlc::string_elem tchar >
	GTL__DEPR_SEC inline [[nodiscard]] tvalue tsztoi(tchar const* const& psz, tchar const** ppszStopped, int radix, tchar cSplitter) {
		return tsztoi<tvalue, tchar>(std::basic_string_view<tchar>{ psz, psz + tszlen(psz) }, ppszStopped, radix, cSplitter);
	}
	template < std::integral tvalue, gtlc::string_elem tchar, size_t size >
	constexpr inline [[nodiscard]] tvalue tsztoi(tchar const (&sz)[size], tchar const** ppszStopped, int radix, tchar cSplitter) {
		return tsztoi<tvalue, tchar>(std::basic_string_view<tchar>{ sz, sz + tszlen(sz, size) }, ppszStopped, radix, cSplitter);
	}


	template < std::floating_point tvalue, gtlc::string_elem tchar >
	[[deprecated("NOT STANDARD CONVERTING !")]] constexpr tvalue _tsztod(tchar const* psz, tchar const* pszEnd, tchar const** ppszStopped, tchar cSplitter) {
		if (!psz)
			return {};

		// skip white space
		while ((psz < pszEnd) && IsSpace(*psz))
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
			*ppszStopped = psz;

		return value;
	}

	template < std::floating_point tvalue, gtlc::string_elem tchar >
	tvalue tsztod(std::basic_string_view<tchar> sv, tchar const** ppszStopped, tchar cSplitter) {
		if constexpr (sizeof(tchar) == sizeof(char)) {
			if (cSplitter == 0) {
				// 주의!!!!!!! cSplitter 가 0일 경우에만..
				tvalue value;
				auto [ptr, ec] = std::from_chars(sv.data(), sv.data()+sv.size(), value, std::chars_format::general);
				if (ppszStopped)
					*ppszStopped = ptr;
				return value;
			}
		}

		char buf[1024];
		char* str = buf;
		char const* const end_str = buf+sizeof(buf)-1;
		tchar const* pos = sv.data();
		tchar const* end = sv.data() + sv.size();
		while (pos < end && IsSpace(*pos))
			pos++;
		for (; (pos < end) and (str < end_str); pos++) {
			auto const c = *pos;
			if (c == cSplitter)
				continue;
			//if constexpr (std::is_unsigned_v<tchar>) {
			//	if (c > 127)
			//		break;
			//}
			//else {
			//	if (c <= 0)
			//		break;
			//}
			if ((c > 127) or (c <= 0) or !std::strchr("+-.0123456789eE", c))
				break;
			*str++ += (char)*pos;
		}
		*str = 0;
		char const* pszStopped{};
		auto value = tsztod<tvalue, char>(std::string_view{buf, str+1}, &pszStopped, 0);
		if (ppszStopped) {
			*ppszStopped = sv.data() + (pszStopped-buf);
		}
		return value;
	}

	template < std::floating_point tvalue, gtlc::string_elem tchar>
	inline tvalue tsztod(std::basic_string<tchar> const& str, tchar const** ppszStopped, tchar cSplitter) {
		return tsztod<tvalue, tchar>((std::basic_string_view<tchar>)str, ppszStopped, cSplitter);
	}
	template < std::floating_point tvalue, gtlc::string_elem tchar>
	inline tvalue tsztod(tchar const* psz, tchar const** ppszStopped, tchar cSplitter) {
		return tsztod<tvalue, tchar>(std::basic_string_view<tchar>{ psz, psz+tszlen(psz) }, ppszStopped, cSplitter);
	}


};	// namespace gtl;

