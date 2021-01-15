//////////////////////////////////////////////////////////////////////
//
// convert_codepage.h:
//
// PWH
// 2020.11.13.
// 2020.12.28. 
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GTL_HEADER__STRING_CONVERT_CODEPAGE
#define GTL_HEADER__STRING_CONVERT_CODEPAGE

#include <experimental/generator>

#include "gtl/misc.h"
#include "string_primitives.h"
#include "utf_char_view.h"

namespace gtl {
#pragma pack(push, 8)

	//------------------------------------------------------------------------------------------
	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	enum class eCODEPAGE : int {
		DEFAULT__OR_USE_MBCS_CODEPAGE = 0,	// Default or use MBCS Codepage (eCODEPAGE eMBCS_Codepage_g;)
		DEFAULT = 0,

		UTF7 = 65000,
		UTF8 = 65001,

		UTF16LE = 1200,
		UTF16BE = 1201,
		UTF16 = (std::endian::native == std::endian::little) ? UTF16LE : UTF16BE,
		_UTF16_other = (UTF16 == UTF16BE) ? UTF16LE : UTF16BE,

		UTF32LE = 12000,
		UTF32BE = 12001,
		UTF32 = (std::endian::native == std::endian::little) ? UTF32LE : UTF32BE,
		_UTF32_other = (UTF32 == UTF32BE) ? UTF32LE : UTF32BE,

		UCS2LE = sizeof(wchar_t) == sizeof(char16_t) ? UTF16LE : UTF32LE,
		UCS2BE = sizeof(wchar_t) == sizeof(char16_t) ? UTF16BE : UTF32BE,
		UCS2 = (std::endian::native == std::endian::little) ? UCS2LE : UCS2BE,
		_UCS2_other = (UCS2 == UCS2BE) ? UCS2LE : UCS2BE,


	//------------------------

		KO_KR_949 = 949,
#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM)
		KO_KR_JOHAB_KSSM_1361 = 1361,	// 조합형 KSSM
#endif


		//... your codepages, here.


	};

#if 0
	struct eCODEPAGE_ {
		enum : int {
			DEFAULT = -1,
			//MBCS = 0,

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


		//------------------------

			ACP = 0,

			KO_KR_949 = 949,
	#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM)
			KO_KR_JOHAB_KSSM_1361 = 1361,	// 조합형 KSSM
	#endif


			//... your codepages, here.


		};

	private :
		int value_{DEFAULT};
	};
#endif

	template <typename tchar>	constexpr inline eCODEPAGE const eCODEPAGE_DEFAULT = eCODEPAGE::DEFAULT;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_DEFAULT<char> = eCODEPAGE::DEFAULT;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_DEFAULT<char8_t> = eCODEPAGE::UTF8;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_DEFAULT<char16_t> = eCODEPAGE::UTF16;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_DEFAULT<char32_t> = eCODEPAGE::UTF32;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_DEFAULT<wchar_t> =
		sizeof(wchar_t) == sizeof(char16_t) ? eCODEPAGE::UCS2LE : (sizeof(wchar_t) == sizeof(char32_t) ? eCODEPAGE::UTF32: eCODEPAGE::DEFAULT);

	template <typename tchar>	constexpr inline eCODEPAGE const eCODEPAGE_OTHER_ENDIAN = eCODEPAGE::DEFAULT;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_OTHER_ENDIAN<char16_t> = eCODEPAGE::_UTF16_other;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_OTHER_ENDIAN<char32_t> = eCODEPAGE::_UTF32_other;
	template <>					constexpr inline eCODEPAGE const eCODEPAGE_OTHER_ENDIAN<wchar_t> = eCODEPAGE::_UCS2_other;


	template < eCODEPAGE eCodepage > struct char_type_from {
		using char_type = char;
	};

	template <> struct char_type_from<eCODEPAGE::UTF8> {
		using char_type = char8_t;
	};
	template <> struct char_type_from<eCODEPAGE::UTF16LE> {
		using char_type = char16_t;
	};
	template <> struct char_type_from<eCODEPAGE::UTF16BE> {
		using char_type = char16_t;
	};
	template <> struct char_type_from<eCODEPAGE::UTF32LE> {
		using char_type = char32_t;
	};
	template <> struct char_type_from<eCODEPAGE::UTF32BE> {
		using char_type = char32_t;
	};



	/// @brief default codepage for MBCS (windows)
	/// you can set this value for your region.
	GTL_DATA extern eCODEPAGE eMBCS_Codepage_g;


	constexpr static inline [[nodiscard]] std::string_view GetCodepageBOM(eCODEPAGE eCodepage) {
		using namespace std::literals;
		switch (eCodepage) {
		case eCODEPAGE::DEFAULT : return {};
		case eCODEPAGE::UTF8 : return "\xEF\xBB\xBF"sv;
		case eCODEPAGE::UTF16LE : return "\xFF\xFE"sv;
		case eCODEPAGE::UTF16BE : return "\xFE\xFF"sv;
		case eCODEPAGE::UTF32LE : return "\xFF\xFE\x00\x00"sv;
		case eCODEPAGE::UTF32BE : return "\x00\x00\xFE\xFF"sv;
		}
		return {};
	}



	struct S_CODEPAGE_OPTION {
		eCODEPAGE from {eCODEPAGE::DEFAULT};
		eCODEPAGE to {eCODEPAGE::DEFAULT};

		// no constructors. for designated initializer...
		template < typename tchar_from > eCODEPAGE From() const { return GetCodepage<tchar_from>(from); }
		template < typename tchar_to >	 eCODEPAGE To() const   { return GetCodepage<tchar_to>(to); }

	private:
		template < typename tchar >
		constexpr inline static eCODEPAGE GetCodepage(eCODEPAGE codepage) {
			if (codepage == eCODEPAGE::DEFAULT__OR_USE_MBCS_CODEPAGE) {	// wrong or default value
				if constexpr (gtlc::is_same_utf<tchar, char>) {
					return eMBCS_Codepage_g;
				}
				else {
					if (eCODEPAGE_DEFAULT<tchar> != eCODEPAGE::DEFAULT)
						return eCODEPAGE_DEFAULT<tchar>;
				}
			}
			return codepage;
		}

	};


	/// @brief Convert Codepage (from -> to) ex, wstring -> u32string, u8string -> string, u16string -> string, string -> u16string...
	/// @param svFrom 
	/// @param codepage 
	/// @return 
	template < gtlc::string_elem_utf tchar_to, gtlc::string_elem_utf tchar_from, bool bCOUNT_FIRST = true >
	std::basic_string<tchar_to> ToUTFString(std::basic_string_view<tchar_from> svFrom, S_CODEPAGE_OPTION codepage = {});


	/// @brief Convert Codepage (from -> to) ex, wstring -> u32string, u8string -> string, u16string -> string, string -> u16string...
	/// @param svFrom 
	/// @param codepage 
	/// @return 
	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, bool bCOUNT_FIRST = true >
	std::basic_string<tchar_to> ToString(std::basic_string_view<tchar_from> svFrom, S_CODEPAGE_OPTION codepage = {});


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

	GTL_API std::string		ConvWide_MBCS(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage);
	GTL_API std::wstring	ConvMBCS_Wide(std::string_view svFrom, S_CODEPAGE_OPTION codepage);
	//GTL_API std::u16string	ConvUTF8_UTF16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage);
	//GTL_API std::u32string	ConvUTF8_UTF32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage);
	//GTL_API std::u8string	ConvUTF16_UTF8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage);
	//GTL_API std::u32string	ConvUTF16_UTF32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage);
	//GTL_API std::u8string	ConvUTF32_UTF8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage);
	//GTL_API std::u16string	ConvUTF32_UTF16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage);

	GTL_API bool IsUTF8String(std::string_view str, int* pOutputBufferCount = nullptr, bool* pbIsMSBSet = nullptr);


	namespace internal {

		/// @brief static type cast (wide string/string_view) -> char16_t/char32_t string/string_vew. (according to its size), NO code conversion.
		/// @tparam tchar 
		/// @param str : basic_string or basic_string_view. (or whatever )
		/// @return same container with char??_t
		template < typename tchar >
		std::basic_string<gtlc::as_utf_t<tchar>>& ReinterpretAsUTF(std::basic_string<tchar>& str) {
			return reinterpret_cast<std::basic_string<gtlc::as_utf_t<tchar>>&>(str);
		}
		template < typename tchar >
		std::basic_string<gtlc::as_utf_t<tchar>> const& ReinterpretAsUTF(std::basic_string<tchar> const& str) {
			return reinterpret_cast<std::basic_string<gtlc::as_utf_t<tchar>> const&>(str);
		}
		template < typename tchar >
		std::basic_string_view<gtlc::as_utf_t<tchar>>& ReinterpretAsUTF(std::basic_string_view<tchar>& sv) {
			return reinterpret_cast<std::basic_string_view<gtlc::as_utf_t<tchar>>&>(sv);
		}

		/// @brief static type cast (char16_t/char32_t string/string_view) -> char16_t/char32_t string/string_view. (according to its size), NO code conversion.
		template < gtlc::string_elem tchar>
		std::basic_string_view<gtlc::as_wide_t<tchar>>& ReinterpretAsWide(std::basic_string_view<tchar>& sv) {
			return reinterpret_cast<std::basic_string_view<gtlc::as_wide_t<tchar>>&>(sv);
		}
		template < gtlc::string_elem tchar>
		std::basic_string_view<gtlc::as_wide_t<tchar>> const& ReinterpretAsWide(std::basic_string_view<tchar> const& sv) {
			return reinterpret_cast<std::basic_string_view<gtlc::as_wide_t<tchar>> const&>(sv);
		}
		template < gtlc::string_elem tchar>
		std::basic_string<gtlc::as_wide_t<tchar>>& ReinterpretAsWide(std::basic_string<tchar>& str) {
			return reinterpret_cast<std::basic_string<gtlc::as_wide_t<tchar>>&>(str);
		}

		template < gtlc::string_elem tchar >
		inline [[nodiscard]] std::wstring ToStringWide(std::basic_string_view<tchar>& s, S_CODEPAGE_OPTION codepage) {
			if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
				return (std::wstring&)ToStringU16(s, codepage);
			}
			else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
				return (std::wstring&)ToStringU32(s, codepage);
			}
			else {
				static_assert(false, "no way!");
			}
		};
	}

	/// @brief Converts Codepage To StringA (MBCS)
	std::string ToStringA(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (codepage.From<char>() == codepage.To<char>()) {
			return std::string{ svFrom };
		}
		else {
			auto strU = ConvMBCS_Wide(svFrom, { .from = codepage.From<char>() });
			return ConvWide_MBCS(strU, { .to = codepage.To<char>() });
		}
	}
	std::string ToStringA(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringA(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	std::string ToStringA(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ToUTFString<wchar_t, char8_t, false>(svFrom, { .from = codepage.from });
		return ConvWide_MBCS(str, { .to = codepage.to });
	}
	std::string ToStringA(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if constexpr (gtlc::is_same_utf<wchar_t, char16_t>) {
			return ConvWide_MBCS((std::wstring_view&)svFrom, codepage);
		}
		else {
			auto str = ToUTFString<wchar_t, char16_t, false>(svFrom, {.from = codepage.from});
			return ConvWide_MBCS(str, {.to = codepage.to});
		}
	}
	std::string ToStringA(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if constexpr (gtlc::is_same_utf<wchar_t, char32_t>) {
			return ConvWide_MBCS((std::wstring_view&)svFrom, codepage);
		}
		else {
			auto str = ToUTFString<wchar_t, char32_t, false>(svFrom, { .from = codepage.from });
			return ConvWide_MBCS(str, { .to = codepage.to });
		}
	}

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	// alias to U16 / U32
	std::wstring ToStringW(std::string_view svFrom, S_CODEPAGE_OPTION codepage)    { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage)   { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage)  { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide(svFrom, codepage); }
	std::wstring ToStringW(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide(svFrom, codepage); }

	/// @brief Converts Codepage To utf-8
	std::u8string ToStringU8(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvMBCS_Wide(svFrom, { .from = codepage.from });
		return ToUTFString<char8_t, wchar_t>(str, { .to = codepage.to });
	}
	std::u8string ToStringU8(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU8(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	std::u8string ToStringU8(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		//if (codepage.from == codepage.to) 

		return std::u8string{ svFrom };
	}
	std::u8string ToStringU8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char8_t>(svFrom, codepage);
	}
	std::u8string ToStringU8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char8_t>(svFrom, codepage);
	}

	/// @brief Converts Codepage To utf-16
	std::u16string ToStringU16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if constexpr (gtlc::is_same_utf<char16_t, wchar_t>) {
			return (std::u16string&)ConvMBCS_Wide(svFrom, codepage);
		}
		else {
			auto str = ConvMBCS_Wide(svFrom, {.from = codepage.from});
			return ToUTFString<char16_t, wchar_t, false>(str, {.to = codepage.to});
		}
	}
	std::u16string ToStringU16(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU16(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	std::u16string ToStringU16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char16_t, char8_t>(svFrom, codepage);
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
		return ToUTFString<char16_t, char32_t>(svFrom, codepage);
	}

	/// @brief Converts Codepage To utf-32
	std::u32string ToStringU32(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvMBCS_Wide(svFrom, { .from = codepage.from });
		return ToUTFString<char32_t, wchar_t>(str, { .to = codepage.to });
	}
	std::u32string ToStringU32(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU32(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	std::u32string ToStringU32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char32_t, char8_t>(svFrom, codepage);
	}
	std::u32string ToStringU32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char32_t, char16_t>(svFrom, codepage);
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


	namespace internal {
		template < typename tchar >
		inline [[nodiscard]] std::optional<std::basic_string<tchar>> CheckAndConvertEndian(std::basic_string_view<tchar> sv, eCODEPAGE eCodepage) {
			if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				return {};
			}
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
			if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				return false;
			}
			if (eCODEPAGE_OTHER_ENDIAN<tchar> != eCodepage) {
				[[likely]]
				return false;
			}
			for (auto& c : str) {
				ByteSwap(c);
			}
			return true;
		}
	}


	/// @brief Convert Codepage (from -> to) ex, wstring -> u32string, u8string -> string, u16string -> string, string -> u16string...
	/// @param svFrom 
	/// @param codepage 
	/// @return 
	template < gtlc::string_elem_utf tchar_to, gtlc::string_elem_utf tchar_from, bool bCOUNT_FIRST >
	std::basic_string<tchar_to> ToUTFString(std::basic_string_view<tchar_from> svFrom, S_CODEPAGE_OPTION codepage) {
		std::basic_string<tchar_to> str;
		if (svFrom.size() <= 0)
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = internal::CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		// Count Converted-String Length
		size_t nOutputLen = 0;
		auto const* pos = svFrom.data();
		auto const* const end = svFrom.data() + svFrom.size();
		if constexpr (bCOUNT_FIRST) {
			while (pos < end) {
				internal::UTFCharConverter<tchar_to, tchar_from, false, true>(pos, end, nOutputLen);
			}
			if (nOutputLen <= 0)
				return str;
		}
		else {
			nOutputLen = svFrom.size();
		}

		// Convert String
		str.reserve(nOutputLen);
		pos = svFrom.data();
		while (pos < end) {
			internal::UTFCharConverter<tchar_to, tchar_from, true, false>(pos, end, str);
		}

		// check endian, Convert
		internal::CheckAndConvertEndian(str, codepage.to);

		return str;

	}


	/// @brief Convert Codepage (from -> to) ex, wstring -> u32string, u8string -> string, u16string -> string, string -> u16string...
	/// @param svFrom 
	/// @param codepage 
	/// @return 
	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, bool bCOUNT_FIRST >
	[[nodiscard]] std::basic_string<tchar_to> ToString(std::basic_string_view<tchar_from> svFrom, S_CODEPAGE_OPTION codepage) {

		if constexpr (gtlc::is_same_utf<tchar_to, char> and gtlc::is_same_utf<tchar_from, char>) {
			if (codepage.from == codepage.to)
				return std::basic_string<tchar_to>{svFrom};
			return ConvWide_MBCS(ConvMBCS_Wide(svFrom, {.from = codepage.from}), {.to = codepage.to});
		}
		else if constexpr (gtlc::is_same_utf<tchar_to, char>) {
			if constexpr (gtlc::is_same_utf<tchar_from, wchar_t>) {
				return ConvWide_MBCS((std::wstring_view&)svFrom, codepage);
			}
			else {
				return ConvWide_MBCS(
							ToUTFString<wchar_t, tchar_from, false>(svFrom, {.from = codepage.from}),
							{.to = codepage.to});
			}
		}
		else if constexpr (gtlc::is_same_utf<tchar_from, char>) {
			if constexpr (gtlc::is_same_utf<tchar_to, wchar_t>) {
				return (std::basic_string<tchar_to>&)ConvMBCS_Wide(svFrom, codepage);
			}
			else {
				return ToUTFString<tchar_to, wchar_t, bCOUNT_FIRST>(
							ConvMBCS_Wide(svFrom, {.from = codepage.from}),
							{.to = codepage.to});
			}
		}
		else if constexpr (gtlc::is_one_of< tchar_from, char8_t, char16_t, char32_t, wchar_t>
			and gtlc::is_one_of< tchar_to, char8_t, char16_t, char32_t, wchar_t>) {

			return ToUTFString<tchar_to, tchar_from, bCOUNT_FIRST>(svFrom, codepage);
		}
		else {
			static_assert(false);
		}
	}

	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, bool bCOUNT_FIRST = true >
	inline void ToString(std::basic_string_view<tchar_from> svFrom, std::basic_string<tchar_to>& strTo, S_CODEPAGE_OPTION codepage = {}) {
		strTo = ToString<tchar_to, tchar_from, bCOUNT_FIRST>(svFrom);
	}


#pragma pack(pop)
};	// namespace gtl;


#endif	// GTL_HEADER__STRING_CONVERT_CODEPAGE
