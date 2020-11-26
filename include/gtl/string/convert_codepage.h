//////////////////////////////////////////////////////////////////////
//
// convert_codepage.h:
//
// PWH
// 2020.11.13.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING_CONVERT_CODEPAGE
#define GTL_HEADER__STRING_CONVERT_CODEPAGE

#include <experimental/generator>

#include "string_primitives.h"
#include "utf_string_view.h"

namespace gtl {
#pragma pack(push, 8)

	//------------------------------------------------------------------------------------------
	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	struct eCODEPAGE { enum : uint32_t {
		Default = 0xFFFF'FFFF,	// CP_OEMCP

		UCS2LE = 1200,
		UCS2BE = 1201,
		UCS2 = (std::endian::native == std::endian::little) ? UCS2LE : UCS2BE,
		_UCS2_other = (UCS2 == UCS2BE) ? UCS2LE : UCS2BE,

		UTF7 = 65000,
		UTF8 = 65001,

		UTF16LE = UCS2LE,
		UTF16BE = UCS2BE,
		UTF16 = (std::endian::native == std::endian::little) ? UTF16LE : UTF16BE,
		_UTF16_other = (UTF16 == UTF16BE) ? UTF16LE : UTF16BE,

		UTF32LE = 12000,
		UTF32BE = 12001,
		UTF32 = (std::endian::native == std::endian::little) ? UTF32LE : UTF32BE,
		_UTF32_other = (UTF32 == UTF32BE) ? UTF32LE : UTF32BE,

		KO_KR_949 = 949,
		KO_KR_JOHAB_KSSM_1361 = 1361,	// Á¶ÇÕÇü KSSM
	}; };

	GTL_DATA extern int eGTLDefaultCodepage_g;

	struct S_CODEPAGE_OPTION {
		uint32_t from {eCODEPAGE::Default};
		uint32_t to {eCODEPAGE::Default};

		uint16_t From() const {
			if (from > 0xffff) {	// wrong or default value
				return eGTLDefaultCodepage_g;
			}
			return from;
		}
		uint16_t To() const {
			if (to > 0xffff) {	// wrong or default value
				return eGTLDefaultCodepage_g;
			}
			return to;
		}
	};


	/// @brief Converts Codepage To StringA (MBCS)
	inline std::string		ToStringA(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::string		ToStringA(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::string		ToStringA(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::string		ToStringA(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::string		ToStringA(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	inline std::wstring		ToStringW(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::wstring		ToStringW(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::wstring		ToStringW(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::wstring		ToStringW(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::wstring		ToStringW(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-8
	inline std::u8string	ToStringU8(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u8string	ToStringU8(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u8string	ToStringU8(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u8string	ToStringU8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u8string	ToStringU8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-16
	inline std::u16string	ToStringU16(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u16string	ToStringU16(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u16string	ToStringU16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u16string	ToStringU16(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u16string	ToStringU16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-32
	inline std::u32string	ToStringU32(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u32string	ToStringU32(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u32string	ToStringU32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u32string	ToStringU32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	inline std::u32string	ToStringU32(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});


	GTL_API std::string		ConvUTF16_MBCS(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u16string	ConvMBCS_UTF16(std::string_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u16string	ConvUTF8_UTF16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u32string	ConvUTF8_UTF32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u8string	ConvUTF16_UTF8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u32string	ConvUTF16_UTF32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u8string	ConvUTF32_UTF8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::u16string	ConvUTF32_UTF16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage);

	GTL_API bool IsUTF8String(std::string_view str, int* pOutputBufferCount = nullptr, bool* pbIsMSBSet = nullptr);


	namespace internal {

		/// @brief static type cast (wide string/string_view) -> char16_t/char32_t string/string_vew. (according to its size), NO code conversion.
		/// @tparam tchar 
		/// @param str : basic_string or basic_string_view. (or whatever )
		/// @return same container with char??_t
		template < typename tchar, template <typename tchar, typename ... tstr_args> typename tstr, typename ... tstr_args >
		auto WideAsCharXX(tstr<tchar, tstr_args...>& str) {
			if constexpr (sizeof(tchar) == sizeof(char8_t)) {
				return (tstr<char8_t>)(tstr<char8_t>&)str;
			}
			else if constexpr (sizeof(tchar) == sizeof(char16_t)) {
				return (tstr<char16_t>)(tstr<char16_t>&)str;
			}
			else if constexpr (sizeof(tchar) == sizeof(char32_t)) {
				return (tstr<char32_t>)(tstr<char32_t>&)str;
			}
		}

		/// @brief static type cast (char16_t/char32_t string/string_view) -> char16_t/char32_t string/string_view. (according to its size), NO code conversion.
		template < typename tchar, template <typename tchar, typename ... tstr_args> typename tstr, typename ... tstr_args >
		tstr<wchar_t>& CharAsWideXX(tstr<tchar, tstr_args...>& str) {
			return (tstr<wchar_t>&)(str);
		}

		template < typename tstr >
		auto ToStringWide(tstr&& s, S_CODEPAGE_OPTION codepage) {
			if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
				auto str = ToStringU16(std::forward<tstr>(s), codepage);
				return CharAsWideXX(str);
			}
			else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
				auto str = ToStringU32(std::forward<tstr>(s), codepage);
				return CharAsWideXX(str);
			}
			else {
				static_assert(false, "no way!");
			}
		};
	}

	/// @brief Converts Codepage To StringA (MBCS)
	std::string ToStringA(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (codepage.From() == codepage.To()) {
			return std::string{ svFrom };
		}
		else {
			auto strU = ConvMBCS_UTF16(svFrom, { .from = codepage.From() });
			return ConvUTF16_MBCS(strU, { .to = codepage.To() });
		}
	}
	std::string ToStringA(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringA(internal::WideAsCharXX(svFrom), codepage);
	}
	std::string ToStringA(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvUTF8_UTF16(svFrom, { .from = codepage.from });
		return ConvUTF16_MBCS(str, { .to = codepage.to });
	}
	std::string ToStringA(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF16_MBCS(svFrom, codepage);
	}
	std::string ToStringA(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvUTF32_UTF16(svFrom, { .from = codepage.from });
		return ConvUTF16_MBCS(str, { .to = codepage.to });
	}

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	// alias to U16 / U32
	std::wstring ToStringW(std::string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide(svFrom, codepage); }

	/// @brief Converts Codepage To utf-8
	std::u8string ToStringU8(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvMBCS_UTF16(svFrom, { .from = codepage.from });
		return ConvUTF16_UTF8(str, { .to = codepage.to });
	}
	std::u8string ToStringU8(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU8(internal::WideAsCharXX(svFrom), codepage);
	}
	std::u8string ToStringU8(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		//if (codepage.from == codepage.to) 

		return std::u8string{ svFrom };
	}
	std::u8string ToStringU8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF16_UTF8(svFrom, codepage);
	}
	std::u8string ToStringU8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF32_UTF8(svFrom, codepage);
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
		std::u16string str{ svFrom };
		if (codepage.from != codepage.to) {
			[[unlikely]]
			if (((codepage.from == eCODEPAGE::UTF16BE) && (codepage.to == eCODEPAGE::UTF16LE))
				|| ((codepage.from == eCODEPAGE::UTF16LE) && (codepage.to == eCODEPAGE::UTF16BE)))
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
		auto str = ConvMBCS_UTF16(svFrom, { .from = codepage.from });
		return ConvUTF16_UTF32(str, { .to = codepage.to });
	}
	std::u32string ToStringU32(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU32(internal::WideAsCharXX(svFrom), codepage);
	}
	std::u32string ToStringU32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF8_UTF32(svFrom, codepage);
	}
	std::u32string ToStringU32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ConvUTF16_UTF32(svFrom, codepage);
	}
	std::u32string ToStringU32(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u32string str{ svFrom };
		if (codepage.from != codepage.to) {
			[[unlikely]]
			if (((codepage.from == eCODEPAGE::UTF32BE) && (codepage.to == eCODEPAGE::UTF32LE))
				|| ((codepage.from == eCODEPAGE::UTF32LE) && (codepage.to == eCODEPAGE::UTF32BE)))
				for (auto& c : str)
					c = _byteswap_ulong(c);
		}
		return str;
	}

	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar1, gtlc::string_elem tchar2 >
	basic_string_t<tchar2> ToString(std::basic_string_view<tchar1> str, S_CODEPAGE_OPTION codepage = {}) {
		basic_string_t<tchar2> result;
		if constexpr (std::is_same_v<tchar2, char>) {
			result = ToStringA(str, codepage);
		} else if constexpr (std::is_same_v<tchar2, wchar_t>) {
			result = ToStringW(str, codepage);
		} else if constexpr (std::is_same_v<tchar2, char8_t>) {
			result = ToStringU8(str, codepage);
		} else if constexpr (std::is_same_v<tchar2, char16_t>) {
			result = ToStringU16(str, codepage);
		} else if constexpr (std::is_same_v<tchar2, char32_t>) {
			result = ToStringU32(str, codepage);
		}
		return result;
	}
	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar1, gtlc::string_elem tchar2 >
	void ToString(std::basic_string_view<tchar1> strFrom, std::basic_string<tchar2>& strTo, S_CODEPAGE_OPTION codepage = {}) {
		strTo = ToString<tchar1, tchar2>(strFrom);
	}

	//-------------------------------------------------------------
	// experimental
	//
	template < gtlc::string_elem_utf tchar_return, gtlc::string_elem_utf tchar >
	requires ( (sizeof(tchar) != sizeof(tchar_return)) && (sizeof(tchar) != sizeof(char32_t)) )
	std::experimental::generator<tchar_return> StringSequence(utf_string_view<tchar> sv) {
		using usv = utf_string_view<tchar>;
		if constexpr (sizeof(tchar_return) == sizeof(char8_t)) {
			for (auto const c : sv) {
				if (c <= 0x7f) {
					co_yield c;
				}
				else if (c <= 0x7ff) {
					co_yield 0xc0 + (c >> 6);
					co_yield 0x80 + (c & usv::bitmask_6bit);
				}
				else if (c <= 0xffff) {
					co_yield 0xe0 + (c >> 12);
					co_yield 0x80 + ((c >> 6) & usv::bitmask_6bit);
					co_yield 0x80 + ((c >> 0) & usv::bitmask_6bit);
				}
				else if (c <= 0x10'ffff) {
					co_yield 0xf0 + (c >> 18);
					co_yield 0x80 + ((c >> 12) & usv::bitmask_6bit);
					co_yield 0x80 + ((c >> 6) & usv::bitmask_6bit);
					co_yield 0x80 + ((c >> 0) & usv::bitmask_6bit);
				}
				else
					throw std::invalid_argument{ GTL__FUNCSIG "no utf" };
			}
		}
		else if constexpr (sizeof(tchar_return) == sizeof(char16_t)) {
			for (auto const c : sv) {
				if (c <= 0xffff) {
					[[likely]]
					co_yield c;
				}
				else if ((0x1'0000 <= c) && (c <= 0x10'ffff)) {
					constexpr auto const preH = usv::fSurrogateW1.first - 0x1'0000;
					co_yield preH + (c >> 10);
					constexpr auto const preL = usv::fSurrogateW2.first;
					co_yield preL + (c & 0b0011'1111);
				}
				else
					co_yield c;
			}
		}
		else if constexpr (sizeof(tchar_return) == sizeof(char32_t)) {
			for (auto const c : sv) {
				co_yield c;
			}
		}
		co_return;
	}


	//template < typename > requires(false)
	//[[deprecated]] inline std::u8string ConvUTF16_UTF8(std::u16string_view svFrom) {
	//	std::wstring_convert<std::codecvt<char16_t, char8_t, std::mbstate_t>, char16_t> conversion;
	//	return (std::u8string&)conversion.to_bytes(svFrom.data());
	//}
	//template < typename > requires(false)
	//[[deprecated]] inline std::u16string ConvUTF8_UTF16(std::u8string_view svFrom) {
	//	std::wstring_convert<std::codecvt<char16_t, char8_t, std::mbstate_t>, char16_t> conversion;
	//	return conversion.from_bytes((char const*)svFrom.data());
	//}
	//template < typename > requires(false)
	//[[deprecated]] inline std::u32string ConvUTF8_UTF32(std::u8string_view sv) {
	//	std::wstring_convert<std::codecvt<char32_t, char8_t,std::mbstate_t>, char32_t> conversion;
	//	return conversion.from_bytes((char const*)sv.data());
	//}
	//template < typename > requires(false)
	//[[deprecated]] inline std::u8string ConvUTF32_UTF8(std::u32string_view sv) {
	//	std::wstring_convert<std::codecvt<char32_t, char8_t,std::mbstate_t>, char32_t> conversion;
	//	return (std::u8string&)conversion.to_bytes(sv.data());
	//}


#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__STRING_CONVERT_CODEPAGE
