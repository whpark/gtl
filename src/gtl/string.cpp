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

#include "gtl/string/basic_string.h"
#include "gtl/string/convert_codepage.h"
#include "gtl/string/convert_codepage_kssm.h"
#include "gtl/string/old_format.h"
#include "gtl/string.h"
//#include "gtl/archive.h"
#include "HangeulCodeTable.h"

namespace gtl {

	constexpr static bool const IsUTF8SigChar(char c) { return (c & 0b1100'0000) == 0b1000'0000; }

	namespace internal {
		template < typename tchar_t >
		inline std::optional<std::basic_string<tchar_t>> TransformEndian(std::basic_string_view<tchar_t>& sv, int eCodepage, int eCodepageOtherEndian) {
			if (eCodepage != eCodepageOtherEndian)
				return nullptr;
			std::basic_string<tchar_t> strOther;
			strOther.reserve(sv.size());
			for (auto c : sv) {
				strOther->push_back(GetByteSwap(c));
			}
			sv = strOther.value();
			return strOther;
		}
		template < typename tchar_t >
		inline void TransformEndian(std::basic_string<tchar_t>& str, int eCodepage, int eCodepageOtherEndian) {
			if (eCodepage == eCodepageOtherEndian) {
				for (auto& c : str)
					ByteSwap(c));
			}
		}
	}

#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY) && defined(_WINDOWS)
	std::string ConvUTF16_MBCS(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			throw std::invalid_argument{"string is too long."};
		}

		std::string str;
		if (svFrom.empty())
			return str;

		// check endian 
		auto strOther = internal::TransformEndian(svFrom, codepage.from, eCODEPAGE::_UTF16_other);

		auto* pszFrom = (wchar_t const*)svFrom.data();
		auto n = WideCharToMultiByte(codepage.to, 0, pszFrom, (int)svFrom.size(), nullptr, 0, nullptr, nullptr);
		if (n <= 0)
			return str;
		str.resize(n);
		auto n2 = WideCharToMultiByte(codepage.to, 0, pszFrom, (int)svFrom.size(), (LPSTR)str.data(), (int)str.capacity(), nullptr, nullptr);
		// check endian, transform
		return str;
	}
	std::u16string ConvMBCS_UTF16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			throw std::invalid_argument{"string is too long."};
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
		MultiByteToWideChar(codepage.from, 0, pszFrom, svFrom.size(), (LPWSTR)str.data(), (int)str.size());
		return str;
	}
#else
	std::string ConvUTF16_MBCS(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			throw std::invalid_argument{"string is too long."};
		}

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		std::locale loc(fmt::format(".{}", codepage.from));
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

		if (result != std::codecvt_base::error)
			throw std::invalid_argument{"String Cannot be transformed!"};
		return str;
	}
	std::u16string ConvMBCS_UTF16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u16string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			throw std::invalid_argument{"string is too long."};
		}

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		std::locale loc(fmt::format(".{}", codepage.from));
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

		if (result != std::codecvt_base::error)
			throw std::invalid_argument{"String Cannot be transformed!"};
		return str;
	}
#endif

#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY) && defined(_WINDOWS)
	std::u8string ConvUTF16_UTF8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (svFrom.size() > static_cast<size_t>(INT_MAX)) {
			throw std::invalid_argument{"string is too long."};
		}

		std::u8string str;
		if (svFrom.empty())
			return str;

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
			throw std::invalid_argument{"string is too long."};
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
		return str;
	}

#else

	std::u8string ConvUTF16_UTF8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u8string str;
		if (svFrom.empty())
			return str;

		size_t nOutputLen = 0;
		for (auto c : svFrom) {
			if (c < 0x80)
				nOutputLen ++;
			else if (c < 0x0800)
				nOutputLen += 2;
			else if ( (c < 0xd800) || ( (c >= 0xe000) && (c <= 0xffff) ) )
				nOutputLen += 3;
			else {
				throw std::invalid_argument{"Cannot Convert from u16 to u8."};
				nOutputLen ++;
			}
		}

		if (nOutputLen <= 0)
			return str;

		str.reserve(nOutputLen);
		for (auto c : svFrom) {
			if (c < 0x80)
				str.push_back(static_cast<char8_t>(c));
			else if (c < 0x0800) {
				str.push_back(static_cast<char8_t>(0xc0 | (((c)>>6)&0x03f)));
				str.push_back(static_cast<char8_t>(0x80 | ((c)&0x3F)));
			} else if ( (c < 0xd800) || ( (c >= 0xe000) && (c <= 0xffff) ) ) {
				str.push_back(static_cast<char8_t>(0xe0 | (((c)>>12)&0x0f)));
				str.push_back(static_cast<char8_t>(0x80 | (((c)>> 6)&0x3f)));
				str.push_back(static_cast<char8_t>(0x80 | ((c)&0x3f)));
			} else {
				throw std::invalid_argument{"Cannot Convert from u16 to u8."};
			}
		}

		return str;
	}
	std::u16string ConvUTF8_UTF16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u16string str;
		if (svFrom.empty())
			return str;

		size_t nOutputLen = 0;
		auto const* pszEnd = svFrom.data() + svFrom.size();
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			auto const c1 = psz[0];
			auto const c2 = psz[0] && (psz+1 < pszEnd) ? psz[1] : 0;
			auto const c3 = psz[1] && (psz+2 < pszEnd) ? psz[2] : 0;

			if ( c1 && c2 && ((c1 & 0b1110'0000) == 0b1100'0000) && IsUTF8SigChar(c2) ) {		// 110xxxxx 10xxxxxx
				char16_t b = ((c1 & 0b0001'1111) << 6) | (c2 & 0b0011'1111);
				if ( (b >= 0x0080) && (b <= 0x07ff) ) {
					psz += 1;
				}
			} else if (c1 && c2 && c3 && ((c1&0b1111'0000) == 0b1110'0000) && IsUTF8SigChar(c2) && IsUTF8SigChar(c3) ) {
				char16_t b = ((c1 & 0b0000'1111) << 12) | ((c2 & 0b0011'1111) << 6) | (c3 & 0b0011'1111);
				if ( (b >= 0x0800) && (b <= 0xFFFF) ) {
					psz += 2;
				}
			}
			nOutputLen++;
		}

		if (nOutputLen <= 0)
			return str;
		str.reserve(nOutputLen);
		for (auto const* psz = svFrom.data(); psz < pszEnd; psz++) {
			auto const c1 = psz[0];
			auto const c2 = psz[0] && (psz+1 < pszEnd) ? psz[1] : 0;
			auto const c3 = psz[1] && (psz+2 < pszEnd) ? psz[2] : 0;

			if ( c1 && c2 && ((c1&0b1110'0000) == 0b1100'0000) && IsUTF8SigChar(c2) ) {		// 110xxxxx 10xxxxxx
				char16_t b = ((c1 & 0b0001'1111) << 6) | (c2 & 0b0011'1111);
				if ( (b >= 0x0080) && (b <= 0x07ff) ) {
					str.push_back(b);
					psz += 1;
				} else {
					str.push_back(*psz);
				}
			} else if (c1 && c2 && c3 && ((c1&0b1111'0000) == 0b1110'0000) && IsUTF8SigChar(c2) && IsUTF8SigChar(c3) ) {
				char16_t b = ((c1 & 0b0000'1111) << 12) | ((c2 & 0b0011'1111) << 6) | (c3 & 0b0011'1111);
				if ( (b >= 0x0800) && (b <= 0xFFFF) ) {
					str.push_back(b);
					psz += 2;
				} else {
					str.push_back(*psz);
				}
			} else {
				str.push_back(c1);
			}
		}

		return str;
	}
#endif


	std::u32string ConvUTF16_UTF32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		// _Convert_wide_to_utf32 from <filesystem> (MSVC)
		std::u32string str;
		str.reserve(svFrom.size());

		char16_t const* first { svFrom.data() };
		char16_t const* iter {first};
		char16_t const* const last { first + svFrom.size()};

		for (; iter != last; ++iter) {
			if (*iter <= 0xd7ffu) {
				str.push_back(*iter);
			} else if (*iter <= 0xdbffu) { // found leading surrogate
				const char32_t cLeading = *iter; // widen for later math

				++iter;

				if (iter == last) { // missing trailing surrogate
					throw std::invalid_argument{"cannot convert string from utf16 to utf32. missing trailing surrogate"};
				}

				char32_t const cTrailing = *iter; // widen for later math

				if (0xdc00u <= cTrailing && cTrailing <= 0xdfffu) { // valid trailing surrogate
					str.push_back(0xfca02400u + (cLeading << 10) + cTrailing);
				} else { // invalid trailing surrogate
					throw std::invalid_argument{"cannot convert string from utf16 to utf32. invalid trailing surrogate"};
				}
			} else if (*iter <= 0xdfffu) { // found trailing surrogate by itself, invalid
				throw std::invalid_argument{"cannot convert string from utf16 to utf32. found trailing surrogate by itself, invalid"};
			} else {
				str.push_back(*iter);
			}
		}

		return str;
	}

	std::u16string ConvUTF32_UTF16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		// function _Convert_utf32_to_wide from <filesystem> (MSVC)
		std::u16string str;

		str.reserve(svFrom.size());

		for (auto const c : svFrom) {
			if (c <= 0xd7ffu) {
				str.push_back(static_cast<std::u16string::value_type>(c));
			} else if (c <= 0xdfffu) {
				throw std::invalid_argument{"cannot convert string from utf32 to utf16. 0xdffu < c < 0xdfffu"};
			} else if (c <= 0xffffu) {
				str.push_back(static_cast<std::u16string::value_type>(c));
			} else if (c <= 0x10ffffu) {
				str.push_back(static_cast<char16_t>(0xd7c0u + (c >> 10)));
				str.push_back(static_cast<char16_t>(0xdc00u + (c & 0x3ffu)));
			} else {
				throw std::invalid_argument{"cannot convert string from utf32 to utf16. 0x10FFFFU < c"};
			}
		}

		return str;
	}


	namespace internal {
		/// @brief static type cast (wide string/string_view) -> char16_t/char32_t string/string_vew. (according to its size), NO code conversion.
		/// @tparam tchar_t 
		/// @param str : basic_string or basic_string_view. (or whatever )
		/// @return same container with char??_t
		template < typename tchar_t, template <typename tchar_t, typename ... tstr_args> typename tstr, typename ... tstr_args >
		auto WideAsCharXX(tstr<tchar_t, tstr_args...>& str) {
			if constexpr (sizeof(tchar_t) == sizeof(char8_t)) {
				return (tstr<char8_t>)(tstr<char8_t>&)str;
			} else if constexpr (sizeof(tchar_t) == sizeof(char16_t)) {
				return (tstr<char16_t>)(tstr<char16_t>&)str;
			} else if constexpr (sizeof(tchar_t) == sizeof(char32_t)) {
				return (tstr<char32_t>)(tstr<char32_t>&)str;
			}
		}

		/// @brief static type cast (char16_t/char32_t string/string_view) -> char16_t/char32_t string/string_view. (according to its size), NO code conversion.
		template < typename tchar_t, template <typename tchar_t, typename ... tstr_args> typename tstr, typename ... tstr_args >
		tstr<wchar_t>& CharAsWideXX(tstr<tchar_t, tstr_args...>& str) {
			return (tstr<wchar_t>&)(str);
		}
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

	/// @brief Converts Codepage To StringA (MBCS)
	std::string ToStringA(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (codepage.from == codepage.to) {
			return std::string{svFrom};
		} else {
			auto strU = ConvMBCS_UTF16(svFrom, {.from = codepage.from});
			return ConvUTF16_MBCS(strU, {.to = codepage.to});
		}
	}
	std::string ToStringA(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringA(internal::WideAsCharXX(svFrom), codepage);
	}
	std::string ToStringA(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvUTF8_UTF16(svFrom, {.from = codepage.from});
		return ConvUTF16_MBCS(str, {.to = codepage.to});
	}
	std::string ToStringA(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF16_MBCS(svFrom, codepage);
	}
	std::string ToStringA(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvUTF32_UTF16(svFrom, {.from = codepage.from});
		return ConvUTF16_MBCS(str, {.to = codepage.to});
	}

	template < typename tstr >
	auto ToStringWide(tstr&& s, S_CODEPAGE_OPTION codepage) {
		if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
			auto str = ToStringU16(std::forward<tstr>(s), codepage);
			return internal::CharAsWideXX(str);
		} else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
			auto str = ToStringU32(std::forward<tstr>(s), codepage);
			return internal::CharAsWideXX(str);
		} else {
			static_assert(false, "no way!");
		}
	};

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	// alias to U16 / U32
	std::wstring ToStringW(std::string_view svFrom, S_CODEPAGE_OPTION codepage)    { return ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage)   { return ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage)	{ return ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage)	{ return ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage)	{ return ToStringWide(svFrom, codepage); }

	/// @brief Converts Codepage To utf-8
	std::u8string ToStringU8(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvMBCS_UTF16(svFrom, {.from = codepage.from});
		return ConvUTF16_UTF8(str, {.to = codepage.to});
	}
	std::u8string ToStringU8(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU8(internal::WideAsCharXX(svFrom), codepage);
	}
	std::u8string ToStringU8(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		//if (codepage.from == codepage.to) 

		return std::u8string{svFrom};
	}
	std::u8string ToStringU8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF16_UTF8(svFrom, codepage);
	}
	std::u8string ToStringU8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvUTF32_UTF16(svFrom, {.from = codepage.from});
		return ConvUTF16_UTF8(str, {.to = codepage.to});
	}

	/// @brief Converts Codepage To utf-16
	std::u16string ToStringU16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvMBCS_UTF16(svFrom, codepage);
	}
	std::u16string ToStringU16(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU16(internal::WideAsCharXX(svFrom), codepage);
	}
	std::u16string ToStringU16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF8_UTF16(svFrom, codepage);
	}
	std::u16string ToStringU16(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u16string str{svFrom};
		if (codepage.from != codepage.to) {
			[[unlikely]]
			if ( ( (codepage.from == eCODEPAGE::UTF16BE) && (codepage.to == eCODEPAGE::UTF16LE) )
				|| ( (codepage.from == eCODEPAGE::UTF16LE) && (codepage.to == eCODEPAGE::UTF16BE) ) )
				for (auto& c : str)
					c = _byteswap_ushort(c);
		}
		return str;
	}
	std::u16string ToStringU16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF32_UTF16(svFrom, codepage);
	}

	/// @brief Converts Codepage To utf-32
	std::u32string ToStringU32(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvMBCS_UTF16(svFrom, {.from = codepage.from});
		return ConvUTF16_UTF32(str, {.to = codepage.to});
	}
	std::u32string ToStringU32(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU32(internal::WideAsCharXX(svFrom), codepage);
	}
	std::u32string ToStringU32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvUTF8_UTF16(svFrom, {.from = codepage.from});
		return ConvUTF16_UTF32(str, {.to = codepage.to});
	}
	std::u32string ToStringU32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF16_UTF32(svFrom, codepage);
	}
	std::u32string ToStringU32(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u32string str{svFrom};
		if (codepage.from != codepage.to) {
			[[unlikely]]
			if ( ( (codepage.from == eCODEPAGE::UTF32BE) && (codepage.to == eCODEPAGE::UTF32LE) )
				|| ( (codepage.from == eCODEPAGE::UTF32LE) && (codepage.to == eCODEPAGE::UTF32BE) ) )
				for (auto& c : str)
					c = _byteswap_ulong(c);
		}
		return str;
	}


	/*GTL_API */std::map<char16_t, uint16_t> const& GetHangeulCodeMap(char16_t, uint16_t) {
		return GetHangeulCodeMapUTF16toKSSM();
	}
	/*GTL_API */std::map<uint16_t, char16_t> const& GetHangeulCodeMap(uint16_t, char16_t) {
		return GetHangeulCodeMapKSSMtoUTF16();
	}

	//-----------------------------------------------------------------------------
	/*GTL_API */bool IsUTF8String(std::string_view str, int* pOutputBufferCount, bool* pbIsMSBSet) {
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
