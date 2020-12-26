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
#include "gtl/string/convert_utf.h"
#include "gtl/string/convert_codepage_kssm.h"
#include "gtl/string/HangeulCodeMap.h"
#include "gtl/string/old_format.h"
#include "gtl/string.h"
//#include "gtl/archive.h"

namespace gtl {

	GTL_DATA eCODEPAGE eMBCS_Codepage_g = static_cast<eCODEPAGE>(GTL_DEFAULT_CODEPAGE);

	template < typename tchar >
	inline [[nodiscard]] std::optional<std::basic_string<tchar>> CheckAndConvertEndian(std::basic_string_view<tchar> sv, eCODEPAGE eCodepage) {
		if (eCODEPAGE_OTHER_ENDIAN<tchar> != eCodepage) {
			[[likely]]
			return {};
		}
		std::basic_string<tchar> strOther;
		strOther.reserve(sv.size());
		for (auto c : sv) {
			strOther.push_back(GetByteSwap(c));
		}
		return strOther;
	}

	template < typename tchar, template < typename, typename ... tstr_args> typename tstr, typename ... tstr_args >
	inline bool CheckAndConvertEndian(tstr<tchar, tstr_args...>& str, eCODEPAGE eCodepage) {
		if (eCODEPAGE_OTHER_ENDIAN<tchar> != eCodepage) {
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
		std::string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

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
		std::u16string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		auto* pszFrom = (char const*)svFrom.data();
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
		std::string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		auto eCodepageTo = codepage.To<char>();
		std::locale loc(fmt::format(".{}", eCodepageTo));

#ifdef _WINDOWS
		using utf16_t = wchar_t;
#else
		using utf16_t = char16_t;
#endif
		auto const& facet = std::use_facet<std::codecvt<utf16_t, char, mbstate_t>>(loc);

		std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
		auto len = svFrom.size()*sizeof(char16_t);//facet.length(state, pszSourceBegin, pszSourceEnd, svFrom.size());
		if (len <= 0)
			return str;
		str.resize(len);

		state = {}; // init.
		std::remove_cvref_t<decltype(svFrom)>::value_type const* pszSourceNext{};
		std::remove_cvref_t<decltype(str)>::value_type* pszDestNext{};
		auto result = facet.out(state, (utf16_t const*)pszSourceBegin, (utf16_t const*)pszSourceEnd, (utf16_t const*&)pszSourceNext,
					   str.data(), str.data()+len, pszDestNext);

		if (result == std::codecvt_base::error)
			throw std::invalid_argument{ GTL__FUNCSIG "String Cannot be transformed!" };

		len = tszlen(str.data(), str.data()+len+1);
		if (str.size() > len)
			str.resize(len);
		return str;
	}
	std::u16string ConvMBCS_UTF16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u16string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		auto eCodepageFrom = codepage.From<char>();

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

		len = (int)((char16_t*)pszDestNext - str.data());
		if (str.size() != len)
			str.resize(len);

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}
#endif

#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY) && defined(_WINDOWS)
	std::u8string ConvUTF16_UTF8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u8string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

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
		std::u16string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

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

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		size_t nOutputLen = 0;
		auto const* const end = svFrom.data() + svFrom.size();
		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char8_t, char16_t, false, true>(pos, end, nOutputLen);
		}

		if (nOutputLen <= 0)
			return str;

		str.reserve(nOutputLen);
		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char8_t, char16_t, true, false>(pos, end, str);
		}

		return str;
	}
	std::u16string ConvUTF8_UTF16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u16string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = 0;
		auto const* const end = svFrom.data() + svFrom.size();
		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char16_t, char8_t, false, true>(pos, end, nOutputLen);
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char16_t, char8_t, true, false>(pos, end, str);
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

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		size_t nOutputLen = 0;
		auto const* const end = svFrom.data() + svFrom.size();
		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char32_t, char16_t, false, true>(pos, end, nOutputLen);
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char32_t, char16_t, true, false>(pos, end, str);
		}

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

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		size_t nOutputLen = 0;
		auto const* const end = svFrom.data() + svFrom.size();
		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char16_t, char32_t, false, true>(pos, end, nOutputLen);
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char16_t, char32_t, true, false>(pos, end, str);
		}

		// check endian, Convert
		CheckAndConvertEndian(str, codepage.to);

		return str;
	}

	std::u8string ConvUTF32_UTF8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u8string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) [[unlikely]] { svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		size_t nOutputLen = 0;
		auto const* const end = svFrom.data() + svFrom.size();
		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char8_t, char32_t, false, true>(pos, end, nOutputLen);
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char8_t, char32_t, true, false>(pos, end, str);
		}

		return str;
	}
	std::u32string ConvUTF8_UTF32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u32string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		size_t nOutputLen = 0;
		auto const* const end = svFrom.data() + svFrom.size();
		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char32_t, char8_t, false, true>(pos, end, nOutputLen);
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);

		for (auto const* pos = svFrom.data(); pos < end; ) {
			internal::UTFCharConverter<char32_t, char8_t, true, false>(pos, end, str);
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

			auto IsUTF8SigChar = [](char c) -> bool { return (c & 0b1100'0000) == 0b1000'0000; };

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
