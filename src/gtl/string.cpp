#include "pch.h"

//////////////////////////////////////////////////////////////////////
//
// string.cpp
//
// PWH
// 2018.12.02.
// 2019.07.24. QL -> GTL
// 2020.11.12.
//
//////////////////////////////////////////////////////////////////////

#include "gtl/string/string_primitives.h"
#include "gtl/string/convert_codepage.h"
#include "gtl/string/convert_codepage_kssm.h"
#include "gtl/string/HangeulCodeTable.h"
#include "gtl/string/old_format.h"
#include "gtl/string.h"
//#include "gtl/archive.h"

namespace gtl {

	constexpr static bool const IsUTF8SigChar(char c) { return (c & 0b1100'0000) == 0b1000'0000; }

	template <typename tchar_t>
	inline constexpr int eCODEPAGE_OTHER;
	template <>
	inline constexpr int eCODEPAGE_OTHER<wchar_t> = eCODEPAGE::_UCS2_other;
	template <>
	inline constexpr int eCODEPAGE_OTHER<char16_t> = eCODEPAGE::_UTF16_other;
	template <>
	inline constexpr int eCODEPAGE_OTHER<char32_t> = eCODEPAGE::_UTF16_other;

	template < typename tchar_t >
	inline [[nodiscard]] std::optional<std::basic_string<tchar_t>> CheckAndConvertEndian(std::basic_string_view<tchar_t> sv, int eCodepage) {
		if (eCODEPAGE_OTHER<tchar_t> != eCodepage) {
			[[likely]]
			return {};
		}
		std::basic_string<tchar_t> strOther;
		strOther.reserve(sv.size());
		for (auto c : sv) {
			strOther.push_back(GetByteSwap(c));
		}
		return strOther;
	}

	template < typename tchar_t, template < typename, typename ... tstr_args> typename tstr, typename ... tstr_args >
	inline bool CheckAndConvertEndian(tstr<tchar_t, tstr_args...>& str, int eCodepage) {
		if (eCODEPAGE_OTHER<tchar_t> != eCodepage) {
			[[likely]]
			return false;
		}
		for (auto& c : str) {
			ByteSwap(c);
		}
		return true;
	}


#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY) && defined(_WINDOWS)
	std::string ConvUTF16_MBCS(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		std::string str;
		if (svFrom.empty())
			return str;

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		auto* pszFrom = (wchar_t const*)svFrom.data();
		auto n = WideCharToMultiByte(codepage.to, 0, pszFrom, (int)svFrom.size(), nullptr, 0, nullptr, nullptr);
		if (n <= 0)
			return str;
		str.resize(n);
		auto n2 = WideCharToMultiByte(codepage.to, 0, pszFrom, (int)svFrom.size(), (LPSTR)str.data(), (int)str.capacity(), nullptr, nullptr);
		return str;
	}
	std::u16string ConvMBCS_UTF16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		std::u16string str;
		if (svFrom.empty())
			return str;

		auto* pszFrom = (char const*)svFrom.data();
		// todo : test return value is size or length?
		auto n = MultiByteToWideChar(codepage.from, 0, pszFrom, (int)svFrom.size(), nullptr, 0);
		if (n <= 0)
			return str;
		str.resize(n);
		MultiByteToWideChar(codepage.from, 0, pszFrom, (int)svFrom.size(), (LPWSTR)str.data(), (int)str.size());

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}
#else
	std::string ConvUTF16_MBCS(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		// todo : to be tested.
		std::string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		auto eCodepageFrom = codepage.From();

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, eCodepageFrom);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		std::locale loc(fmt::format(".{}", eCodepageFrom));
		auto const& facet = std::use_facet<std::codecvt<char, char16_t, mbstate_t>>(loc);

		std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
		auto len = facet.length(state, pszSourceBegin, pszSourceEnd, svFrom.size());
		if (len <= 0)
			return str;
		str.resize(len);

		state = {}; // init.
		std::remove_cvref_t<decltype(svFrom)>::value_type const* pszSourceNext{};
		std::remove_cvref_t<decltype(str)>::value_type* pszDestNext{};
		auto result = facet.in(state, pszSourceBegin, pszSourceEnd, pszSourceNext,
					   str.data(), str.data()+len, pszDestNext);

		if (result == std::codecvt_base::error)
			throw std::invalid_argument{ GTL__FUNCSIG "String Cannot be transformed!" };
		return str;
	}
	std::u16string ConvMBCS_UTF16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		// todo : to be tested.
		std::u16string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		auto eCodepageFrom = codepage.From();

		std::locale loc(fmt::format(".{}", eCodepageFrom));
	#ifdef _WINDOWS
		using utf16_t = wchar_t;
	#else
		using utf16_t = char16_t;
	#endif
		auto const& facet = std::use_facet<std::codecvt<utf16_t, char, mbstate_t>>(loc);

		std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
		auto len = facet.length(state, pszSourceBegin, pszSourceEnd, svFrom.size());
		if (len <= 0)
			return str;
		str.resize(len);

		state = {}; // init.
		std::remove_cvref_t<decltype(svFrom)>::value_type const* pszSourceNext{};
		utf16_t* pszDestNext{};
		auto result = facet.in(state, pszSourceBegin, pszSourceEnd, pszSourceNext,
							   (utf16_t*)str.data(), (utf16_t*)str.data()+len, pszDestNext);

		if (result == std::codecvt_base::error)
			throw std::invalid_argument{ GTL__FUNCSIG "String Cannot be transformed!" };

		len = (char16_t*)pszDestNext - str.data();
		if (str.size() != len)
			str.resize(len);

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}
#endif

#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY) && defined(_WINDOWS)
	std::u8string ConvUTF16_UTF8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		std::u8string str;
		if (svFrom.empty())
			return str;

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);


		wchar_t const* pszFrom = (wchar_t const*)svFrom.data();
		auto n = WideCharToMultiByte(CP_UTF8, 0, pszFrom, (int)svFrom.size(), nullptr, 0, nullptr, nullptr);
		if (n <= 0)
			return str;
		str.resize(n);
		WideCharToMultiByte(CP_UTF8, 0, pszFrom, -1, (LPSTR)str.data(), (int)str.size(), nullptr, nullptr);
		return str;
	}
	std::u16string ConvUTF8_UTF16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		std::u16string str;
		if (svFrom.empty())
			return str;

		char const* pszFrom = (char const*)svFrom.data();
		auto n = MultiByteToWideChar(CP_UTF8, 0, pszFrom, (int)svFrom.size(), nullptr, 0);
		if (n <= 0)
			return str;
		str.resize(n);
		MultiByteToWideChar(CP_UTF8, 0, pszFrom, -1, (LPWSTR)str.data(), (int)str.size());

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}

#else

	std::u8string ConvUTF16_UTF8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u8string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		size_t nOutputLen = 0;
		for (auto c : svFrom) {
			if (c <= 0x7f)
				nOutputLen++;
			else if (c <= 0x07ffu)
				nOutputLen += 2;
			else if ((c <= 0xd7ffu) || (c >= 0xe000u))
				nOutputLen += 3;
			else if (c <= utf_const::fSurrogateW1.second) {
				nOutputLen += 4;
			}
			else {
				throw std::invalid_argument{ GTL__FUNCSIG "Cannot Convert from u32 to u8." };
				nOutputLen++;
			}
		}

		if (nOutputLen <= 0)
			return str;

		str.reserve(nOutputLen);
		for (auto sv = utf_string_view{ svFrom }; auto c : sv) {
			constexpr uint8_t mask_3bits = 0b0000'0111;
			constexpr uint8_t mask_4bits = 0b0000'1111;
			constexpr uint8_t mask_6bits = 0b0011'1111;
			if (c <= 0x7f)
				str.push_back(static_cast<char8_t>(c));
			else if (c <= 0x07ff) {
				str.push_back(static_cast<char8_t>(0xc0 | (((c) >> 6) & mask_6bits)));
				str.push_back(static_cast<char8_t>(0x80 | ((c)&mask_6bits)));
			}
			else if (c <= 0xffff) /*((c < 0xd800) || ((c >= 0xe000) && (c <= 0xffff)))*/ {
				str.push_back(static_cast<char8_t>(0xe0 | (((c) >> 12) & mask_4bits)));		// 4 bits
				str.push_back(static_cast<char8_t>(0x80 | (((c) >> 6) & mask_6bits)));		// 6 bits
				str.push_back(static_cast<char8_t>(0x80 | ((c)&mask_6bits)));				// 6 bits
			}
			else if (c <= 0x10'ffff) {
				str.push_back(static_cast<char8_t>(0xf0 | (((c) >> 18) & mask_3bits)));		// 3 bits
				str.push_back(static_cast<char8_t>(0x80 | (((c) >> 12) & mask_6bits)));		// 6 bits
				str.push_back(static_cast<char8_t>(0x80 | (((c) >> 6) & mask_6bits)));		// 6 bits
				str.push_back(static_cast<char8_t>(0x80 | ((c)&mask_6bits)));				// 6 bits
			}
			else {
				throw std::invalid_argument{ GTL__FUNCSIG "Cannot Convert from u16 to u8." };
			}
		}

		return str;
	}
	std::u16string ConvUTF8_UTF16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u16string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = 0;
		auto const* const end = svFrom.data() + svFrom.size();
		for (auto const* pos = svFrom.data(); pos < end; pos++) {
			if (pos[0] <= 0x7f) {
				nOutputLen++;
			}
			else if ((pos[0] & 0b1110'0000) == 0b1100'0000) {	// 0~0x7ff
				if ((pos + 1 >= end)
					|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				pos += 1;
				nOutputLen += 1;
			}
			else if ((pos[0] & 0b1111'0000) == 0b1110'000) {	// ~0xffff
				if ((pos + 2 >= end)
					|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
					|| ((pos[2] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				pos += 2;
				nOutputLen++;
			}
			else if ((pos[0] & 0b1111'1000) == 0b1111'0000) {	// ~0x10'ffff
				if ((pos + 3 >= end)
					|| ((pos[1] & 0b1100'0000) != 0b1000'0000)
					|| ((pos[2] & 0b1100'0000) != 0b1000'0000)
					|| ((pos[3] & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
				pos += 3;
				nOutputLen += 2;
			}
			else {
				throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}

		}


		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		for (auto const* pos = svFrom.data(); pos < end; pos++) {

			namespace uc = utf_const;

			if (pos[0] <= 0x7f) {
				str.push_back(pos[0]);
			}
			else if ((pos[0] & 0b1110'0000) == 0b1100'0000) {	// 0~0x7ff
				str.push_back(((pos[0] & uc::mask_6bit) << 6) | (pos[1] & uc::mask_6bit));
				pos += 1;
			}
			else if ((pos[0] & 0b1111'0000) == 0b1110'000) {	// ~0xffff
				str.push_back(((pos[0] & uc::mask_4bit) << 12) | ((pos[1] & uc::mask_6bit) << 6) | (pos[2] & uc::mask_6bit));
				pos += 2;
			}
			else if ((pos[0] & 0b1111'1000) == 0b1111'0000) {	// ~0x10'ffff
				//char32_t c = ((pos[0] & uc::mask_3bit) << 18) | ((pos[1] & uc::mask_6bit) << 12) | ((pos[2] & uc::mask_6bit) << 6) | (pos[3] & uc::mask_6bit);
				constexpr static auto const preH = uc::fSurrogateW1.first - (0x1'0000u >> uc::nBitSurrogate);
				constexpr static auto const preL = uc::fSurrogateW2.first;

				str.push_back(preH + ( (pos[0] & uc::mask_3bit) << 8) | ((pos[1] & uc::mask_6bit) << 2) | ((pos[2]>> 4) & 0b0011) );
				str.push_back(preL + ( (pos[2] & uc::mask_4bit) << 6) | (pos[3] & uc::mask_6bit) );
				pos += 3;
			}
			else {
				throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}
		}

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}
#endif


	std::u32string ConvUTF16_UTF32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u32string str;

		if (svFrom.size() <= 0)
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		str.reserve(svFrom.size());

		for (auto c : utf_string_view{ svFrom }) {
			str.push_back(c);
		}

		//constexpr char16_t const fSurrogateW1_B{ 0xd800u };	// 0xd800u ~ 0xdbffu;
		//constexpr char16_t const fSurrogateW1_E{ 0xdbffu };	// 0xdbffu ~ 0xdbffu;
		//constexpr char16_t const fSurrogateW2_B{ 0xdc00u };	// 0xdc00u ~ 0xdfffu;
		//constexpr char16_t const fSurrogateW2_E{ 0xdfffu };	// 0xdfffu ~ 0xdfffu;
		//constexpr int const nBitSurrogate{ 10 };
		//char16_t const* const last { svFrom.data() + svFrom.size()};
		//for (char16_t const* pos = svFrom.data(); pos < last; ++pos) {
		//	if (*pos < fSurrogateW1_B) [[likely]] {
		//		str.push_back(*pos);
		//	} else if (*pos <= fSurrogateW1_E) [[unlikely]] { // leading surrogate
		//		char32_t const cLeading = *pos; // widen for later math

		//		if (++pos == last) { // missing trailing surrogate
		//			throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. missing trailing surrogate"};
		//		}

		//		char32_t const cTrailing = *pos; // widen for later math

		//		if (fSurrogateW2_B <= cTrailing && cTrailing <= fSurrogateW2_E) { // valid trailing surrogate
		//			//str.push_back(0xfca02400u + (cLeading << 10) + cTrailing);
		//			constexpr static uint32_t const pre = 0x1'0000 - ((fSurrogateW1_B << nBitSurrogate) + fSurrogateW2_B);
		//			str.push_back(pre + (cLeading << nBitSurrogate) + cTrailing);
		//		} else { // invalid trailing surrogate
		//			throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. invalid trailing surrogate"};
		//		}
		//	} else if (*pos <= 0xdfffu) { // found trailing surrogate by itself, invalid
		//		[[unlikely]]
		//		throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf16 to utf32. found trailing surrogate by itself, invalid"};
		//	} else {
		//		[[likely]]
		//		str.push_back(*pos);
		//	}
		//}

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}

	std::u16string ConvUTF32_UTF16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		// function _Convert_utf32_to_wide from <filesystem> (MSVC)
		// Original License :
				// Copyright (c) Microsoft Corporation.
				// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
		std::u16string str;

		if (svFrom.size() <= 0)
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		str.reserve(svFrom.size());

		for (auto const c : svFrom) {
			if (c <= 0xd7ffu) {
				str.push_back(static_cast<std::u16string::value_type>(c));
			} else if (c <= 0xdfffu) {
				throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf32 to utf16. 0xdffu < c < 0xdfffu"};
			} else if (c <= 0xffffu) {
				str.push_back(static_cast<std::u16string::value_type>(c));
			} else if (c <= 0x10'ffffu) {
				str.push_back(static_cast<char16_t>(0xd7c0u + (c >> 10)));
				str.push_back(static_cast<char16_t>(0xdc00u + (c & 0x3ffu)));
			} else {
				throw std::invalid_argument{ GTL__FUNCSIG "cannot convert string from utf32 to utf16. 0x10FFFFU < c"};
			}
		}

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}

	std::u8string ConvUTF32_UTF8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u8string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		size_t nOutputLen = 0;
		for (auto c : svFrom) {
			if (c < 0x80)
				nOutputLen++;
			else if (c < 0x0800)
				nOutputLen += 2;
			else if (c <= 0xffff) /*((c < 0xd800) || ((c >= 0xe000) && (c <= 0xffff)))*/
				nOutputLen += 3;
			else if (c <= 0x10'ffff)
				nOutputLen += 4;
			else {
				throw std::invalid_argument{ GTL__FUNCSIG "Cannot Convert from u32 to u8." };
				nOutputLen++;
			}
		}

		if (nOutputLen <= 0)
			return str;

		str.reserve(nOutputLen);
		for (auto c : svFrom) {
			constexpr uint8_t mask_3bits = 0b0000'0111;
			constexpr uint8_t mask_4bits = 0b0000'1111;
			constexpr uint8_t mask_6bits = 0b0011'1111;
			if (c <= 0x7f)
				str.push_back(static_cast<char8_t>(c));
			else if (c <= 0x07ff) {
				str.push_back(static_cast<char8_t>(0xc0 | (((c) >> 6) & mask_6bits)));
				str.push_back(static_cast<char8_t>(0x80 | ((c) &mask_6bits)));
			}
			else if (c <= 0xffff) /*((c < 0xd800) || ((c >= 0xe000) && (c <= 0xffff)))*/ {
				str.push_back(static_cast<char8_t>(0xe0 | (((c) >> 12) & mask_4bits)));		// 4 bits
				str.push_back(static_cast<char8_t>(0x80 | (((c) >> 6) & mask_6bits)));		// 6 bits
				str.push_back(static_cast<char8_t>(0x80 | ((c) & mask_6bits)));				// 6 bits
			}
			else if (c <= 0x10'ffff) {
				str.push_back(static_cast<char8_t>(0xf0 | (((c) >> 18) & mask_3bits)));		// 3 bits
				str.push_back(static_cast<char8_t>(0x80 | (((c) >> 12) & mask_6bits)));		// 6 bits
				str.push_back(static_cast<char8_t>(0x80 | (((c) >> 6) & mask_6bits)));		// 6 bits
				str.push_back(static_cast<char8_t>(0x80 | ((c) & mask_6bits)));				// 6 bits
			}

			else {
				throw std::invalid_argument{ GTL__FUNCSIG "Cannot Convert from u16 to u8." };
			}
		}

		return str;
	}
	std::u32string ConvUTF8_UTF32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u32string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = 0;
		auto const* pszEnd = svFrom.data() + svFrom.size();
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			auto const c = psz[0];

			if (c <= 0x7f)
				;
			else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7f
				if ( (psz+1 >= pszEnd)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}
			else if ((c & 0b1111'0000) == 0b1110'000) {	// ~0x7ff
				if ((psz+2 >= pszEnd)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}
			else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0xffff
				if ((psz+3 >= pszEnd)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					|| ((*++psz & 0b1100'0000) != 0b1000'0000)
					)
					throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}
			else {
				throw std::invalid_argument{ GTL__FUNCSIG "not a utf-8" };
			}

			nOutputLen++;
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			constexpr uint8_t mask_3bits { 0b0000'0111 };
			constexpr uint8_t mask_4bits { 0b0000'1111 };
			constexpr uint8_t mask_6bits { 0b0011'1111 };

			auto const c { *psz };

			char32_t v{};
			if (c <= 0x7f)
				v = c;
			else if ((c & 0b1110'0000) == 0b1100'0000) {	// 0~0x7f
				v = (*psz & mask_6bits) << 6;
				v |= *++psz & mask_6bits;
			}
			else if ((c & 0b1111'0000) == 0b1110'0000) {	// ~0x7ff
				v = (*psz & mask_4bits) << 12;
				v |= (*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}
			else if ((c & 0b1111'1000) == 0b1111'0000) {	// ~0xffff
				v = (*psz & mask_3bits) << 18;
				v |= (*++psz & mask_6bits) << 12;
				v |= (*++psz & mask_6bits) << 6;
				v |= (*++psz & mask_6bits);
			}

			str.push_back(v);
		}

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}



//#pragma warning(push)
//#pragma warning(disable: 4996)
//	std::u32string ConvUTF8_UTF32(std::u8string_view sv) {
//		std::wstring_convert<std::codecvt<char32_t, char8_t, std::mbstate_t>, char32_t> conversion;
//		return conversion.from_bytes((char const*)sv.data());
//	}
//
//	std::u8string ConvUTF32_UTF8(std::u32string_view sv) {
//		std::wstring_convert<std::codecvt<char32_t, char8_t, std::mbstate_t>, char32_t> conversion;
//		return (std::u8string&)conversion.to_bytes(sv.data());
//	}
//#pragma warning(pop)


	//-----------------------------------------------------------------------------
	bool IsUTF8String(std::string_view str, int* pOutputBufferCount, bool* pbIsMSBSet) {
		if (pOutputBufferCount)
			*pOutputBufferCount = 0;
		if (pbIsMSBSet)
			*pbIsMSBSet = false;

		if (str.empty())
			return false;

		int nOutputBufferCount = 0;
		bool bMSB = false;
		bool bUTF8 = true;
		for (size_t iPos = 0; iPos < (size_t)str.size(); iPos++, nOutputBufferCount++) {
			const char c = str[iPos];
			const char c1 = str[iPos];
			const char c2 = iPos+1 < str.size() ? str[iPos+1] : 0;
			const char c3 = iPos+2 < str.size() ? str[iPos+2] : 0;

			if (c1 && 0b1000'0000) {
				bMSB = true;
				if ( ((c1 & 0b1110'0000) == 0b1100'0000) && IsUTF8SigChar(c2) ) {
					wchar_t b = ((c1 & 0b0001'1111) << 6) | (c2 & 0b0011'1111);
					if ( (b >= 0x0080) && (b <= 0x07ff) ) {
						iPos++;
					} else {
						bUTF8 = false;
						break;
					}
				} else if ( ((c1 & 0b1111'0000) == 0b1110'0000) && IsUTF8SigChar(c2) && IsUTF8SigChar(c3) ) {
					wchar_t b = ((c1 & 0b0000'1111) << 12) | ((c2 & 0b0011'1111) << 6) | (c3 & 0b0011'1111);
					if ( (b >= 0x0800) && (b <= 0xFFFF) ) {
						iPos += 2;
					} else {
						bUTF8 = false;
						break;
					}
				} else {
					bUTF8 = false;
					break;
				}
			} else {
			}
		}

		if (pOutputBufferCount)
			*pOutputBufferCount = bUTF8 ? (nOutputBufferCount+1) : 0;

		if (pbIsMSBSet)
			*pbIsMSBSet = bMSB;

		return bUTF8;
	}





}	// namespace gtl
