//////////////////////////////////////////////////////////////////////
//
// convert_codepage.h:
//
// PWH
// 2020.11.13.
// 2020.12.28. 
//
//////////////////////////////////////////////////////////////////////

module;

#include <bit>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <locale>
#include <format>

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:string_convert_codepage;
import :concepts;
import :misc;
import :string_primitives;
import :string_utf_char_view;
export import :string_convert_codepage_kssm;

export namespace gtl {

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
	extern eCODEPAGE eMBCS_Codepage_g;


	constexpr [[nodiscard]] std::string_view GetCodepageBOM(eCODEPAGE eCodepage) {
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

	/// @brief Converts Codepage (Unicode <-> MBCS ...)
	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, bool bCOUNT_FIRST = true>
	inline void ToString(std::basic_string_view<tchar_from> svFrom, std::basic_string<tchar_to>& strTo, S_CODEPAGE_OPTION codepage = {});

	/// @brief Wide <-> MBCS
	/// @param svFrom 
	/// @param codepage 
	/// @return 
	template < bool bCOUNT_FIRST = true> std::string	ConvWide_MBCS(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage);
	template < bool bCOUNT_FIRST = true> std::wstring	ConvMBCS_Wide(std::string_view svFrom, S_CODEPAGE_OPTION codepage);


	/// @brief Converts Codepage To StringA (MBCS)
	template < bool bCOUNT_FIRST = true> inline std::string		ToStringA(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::string		ToStringA(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::string		ToStringA(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::string		ToStringA(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::string		ToStringA(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	template < bool bCOUNT_FIRST = true> inline std::wstring	ToStringW(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::wstring	ToStringW(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::wstring	ToStringW(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::wstring	ToStringW(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::wstring	ToStringW(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-8
	template < bool bCOUNT_FIRST = true> inline std::u8string	ToStringU8(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u8string	ToStringU8(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u8string	ToStringU8(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u8string	ToStringU8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u8string	ToStringU8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-16
	template < bool bCOUNT_FIRST = true> inline std::u16string	ToStringU16(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u16string	ToStringU16(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u16string	ToStringU16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u16string	ToStringU16(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u16string	ToStringU16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	/// @brief Converts Codepage To utf-32
	template < bool bCOUNT_FIRST = true> inline std::u32string	ToStringU32(std::string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u32string	ToStringU32(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u32string	ToStringU32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u32string	ToStringU32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage = {});
	template < bool bCOUNT_FIRST = true> inline std::u32string	ToStringU32(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage = {});

	inline bool IsUTF8String(std::string_view sv, size_t* pOutputBufferCount = nullptr, bool* pbIsMSBSet = nullptr) {
		if (pOutputBufferCount)
			*pOutputBufferCount = 0;
		if (pbIsMSBSet)
			*pbIsMSBSet = false;

		size_t nOutputLen = 0;
		auto const* pos = sv.data();
		auto const* const end = sv.data() + sv.size();
		while (pos < end) {
			if (!gtl::internal::UTFCharConverter<char32_t, char8_t, false, true, false>((char8_t const*&)pos, (char8_t const*)end, nOutputLen))
				return false;
		}

		if (pOutputBufferCount)
			*pOutputBufferCount = nOutputLen;
		if (pbIsMSBSet && (nOutputLen != sv.size()))
			*pbIsMSBSet = true;

		return true;
	}


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

		template < gtlc::string_elem tchar, bool bCOUNT_FIRST >
		inline [[nodiscard]] std::wstring ToStringWide(std::basic_string_view<tchar>& s, S_CODEPAGE_OPTION codepage) {
			if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
				return (std::wstring&)ToStringU16<bCOUNT_FIRST>(s, codepage);
			}
			else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
				return (std::wstring&)ToStringU32<bCOUNT_FIRST>(s, codepage);
			}
			else {
				static_assert(false, "no way!");
			}
		};
	}

	/// @brief Converts Codepage To StringA (MBCS)
	template < bool bCOUNT_FIRST >
	std::string ToStringA(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if (codepage.From<char>() == codepage.To<char>()) {
			return std::string{ svFrom };
		}
		else {
			auto strU = ConvMBCS_Wide<bCOUNT_FIRST>(svFrom, { .from = codepage.From<char>() });
			return ConvWide_MBCS<bCOUNT_FIRST>(strU, { .to = codepage.To<char>() });
		}
	}
	template < bool bCOUNT_FIRST >
	std::string ToStringA(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringA<bCOUNT_FIRST>(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	template < bool bCOUNT_FIRST >
	std::string ToStringA(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ToUTFString<wchar_t, char8_t, false>(svFrom, { .from = codepage.from });
		return ConvWide_MBCS<bCOUNT_FIRST>(str, { .to = codepage.to });
	}
	template < bool bCOUNT_FIRST >
	std::string ToStringA(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if constexpr (gtlc::is_same_utf<wchar_t, char16_t>) {
			return ConvWide_MBCS<bCOUNT_FIRST>((std::wstring_view&)svFrom, codepage);
		}
		else {
			auto str = ToUTFString<wchar_t, char16_t, false>(svFrom, {.from = codepage.from});
			return ConvWide_MBCS<bCOUNT_FIRST>(str, {.to = codepage.to});
		}
	}
	template < bool bCOUNT_FIRST >
	std::string ToStringA(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if constexpr (gtlc::is_same_utf<wchar_t, char32_t>) {
			return ConvWide_MBCS<bCOUNT_FIRST>((std::wstring_view&)svFrom, codepage);
		}
		else {
			auto str = ToUTFString<wchar_t, char32_t, false>(svFrom, { .from = codepage.from });
			return ConvWide_MBCS<bCOUNT_FIRST>(str, { .to = codepage.to });
		}
	}

	/// @brief Converts Codepage To StringW (unicode ~ utf16)
	// alias to U16 / U32
	template < bool bCOUNT_FIRST > std::wstring ToStringW(std::string_view svFrom, S_CODEPAGE_OPTION codepage)    { return internal::ToStringWide<char, bCOUNT_FIRST>(svFrom, codepage); }
	template < bool bCOUNT_FIRST > std::wstring ToStringW(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage)   { return internal::ToStringWide<wchar_t, bCOUNT_FIRST>(svFrom, codepage); }
	template < bool bCOUNT_FIRST > std::wstring ToStringW(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage)  { return internal::ToStringWide<char8_t, bCOUNT_FIRST>(svFrom, codepage); }
	template < bool bCOUNT_FIRST > std::wstring ToStringW(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide<char16_t, bCOUNT_FIRST>(svFrom, codepage); }
	template < bool bCOUNT_FIRST > std::wstring ToStringW(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) { return internal::ToStringWide<char32_t, bCOUNT_FIRST>(svFrom, codepage); }

	/// @brief Converts Codepage To utf-8
	template < bool bCOUNT_FIRST >
	std::u8string ToStringU8(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvMBCS_Wide(svFrom, { .from = codepage.from });
		return ToUTFString<char8_t, wchar_t, bCOUNT_FIRST>(str, { .to = codepage.to });
	}
	template < bool bCOUNT_FIRST >
	std::u8string ToStringU8(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU8<bCOUNT_FIRST>(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	template < bool bCOUNT_FIRST >
	std::u8string ToStringU8(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		//if (codepage.from == codepage.to) 

		return std::u8string{ svFrom };
	}
	template < bool bCOUNT_FIRST >
	std::u8string ToStringU8(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char8_t, char16_t, bCOUNT_FIRST>(svFrom, codepage);
	}
	template < bool bCOUNT_FIRST >
	std::u8string ToStringU8(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char8_t, char32_t, bCOUNT_FIRST>(svFrom, codepage);
	}

	/// @brief Converts Codepage To utf-16
	template < bool bCOUNT_FIRST >
	std::u16string ToStringU16(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		if constexpr (gtlc::is_same_utf<char16_t, wchar_t>) {
			return (std::u16string&)ConvMBCS_Wide<bCOUNT_FIRST>(svFrom, codepage);
		}
		else {
			auto str = ConvMBCS_Wide<false>(svFrom, {.from = codepage.from});
			return ToUTFString<char16_t, wchar_t, bCOUNT_FIRST>(str, {.to = codepage.to});
		}
	}
	template < bool bCOUNT_FIRST >
	std::u16string ToStringU16(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU16<bCOUNT_FIRST>(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	template < bool bCOUNT_FIRST >
	std::u16string ToStringU16(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char16_t, char8_t, bCOUNT_FIRST>(svFrom, codepage);
	}
	template < bool bCOUNT_FIRST >
	std::u16string ToStringU16(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u16string str{ svFrom };
		if (codepage.from != codepage.to) [[unlikely]] {
			if (((codepage.from == eCODEPAGE::UTF16BE) && (codepage.to == eCODEPAGE::UTF16LE))
				|| ((codepage.from == eCODEPAGE::UTF16LE) && (codepage.to == eCODEPAGE::UTF16BE)))
				for (auto& c : str)
					c = gtl::GetByteSwap(c);
		}
		return str;
	}
	template < bool bCOUNT_FIRST >
	std::u16string ToStringU16(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char16_t, char32_t, bCOUNT_FIRST>(svFrom, codepage);
	}

	/// @brief Converts Codepage To utf-32
	template < bool bCOUNT_FIRST >
	std::u32string ToStringU32(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		auto str = ConvMBCS_Wide<false>(svFrom, { .from = codepage.from });
		return ToUTFString<char32_t, wchar_t, bCOUNT_FIRST>(str, { .to = codepage.to });
	}
	template < bool bCOUNT_FIRST >
	std::u32string ToStringU32(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToStringU32<bCOUNT_FIRST>(internal::ReinterpretAsUTF(svFrom), codepage);
	}
	template < bool bCOUNT_FIRST >
	std::u32string ToStringU32(std::u8string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char32_t, char8_t, bCOUNT_FIRST>(svFrom, codepage);
	}
	template < bool bCOUNT_FIRST >
	std::u32string ToStringU32(std::u16string_view svFrom, S_CODEPAGE_OPTION codepage) {
		return ToUTFString<char32_t, char16_t, bCOUNT_FIRST>(svFrom, codepage);
	}
	template < bool bCOUNT_FIRST >
	std::u32string ToStringU32(std::u32string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::u32string str{ svFrom };
		if (codepage.from != codepage.to) {
			[[unlikely]]
			if (((codepage.from == eCODEPAGE::UTF32BE) && (codepage.to == eCODEPAGE::UTF32LE))
				|| ((codepage.from == eCODEPAGE::UTF32LE) && (codepage.to == eCODEPAGE::UTF32BE)))
				for (auto& c : str)
					c = gtl::GetByteSwap(c);
		}
		return str;
	}


	namespace internal {
		template < typename tchar >
		inline [[nodiscard]] std::optional<std::basic_string<tchar>> CheckAndConvertEndian(std::basic_string_view<tchar> sv, eCODEPAGE eCodepage) {
			if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				return {};
			}
			else {
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
		}

		template < typename tchar, template < typename, typename ... tstr_args> typename tstr, typename ... tstr_args >
		inline bool CheckAndConvertEndian(tstr<tchar, tstr_args...>& str, eCODEPAGE eCodepage) {
			if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				return false;
			}
			else {
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
				internal::UTFCharConverter<tchar_to, tchar_from, false, true, true>(pos, end, nOutputLen);
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
			internal::UTFCharConverter<tchar_to, tchar_from, true, false, true>(pos, end, str);
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
	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, bool bCOUNT_FIRST >
	inline void ToString(std::basic_string_view<tchar_from> svFrom, std::basic_string<tchar_to>& strTo, S_CODEPAGE_OPTION codepage) {
		strTo = ToString<tchar_to, tchar_from, bCOUNT_FIRST>(svFrom);
	}


#if (GTL_STRING_PRIMITIVES__WINDOWS_FRIENDLY) && defined(_WINDOWS)
	template < bool bCOUNT_FIRST >
	std::string ConvWide_MBCS(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX/2)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = internal::CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		auto* pszFrom = svFrom.data();
		if constexpr (bCOUNT_FIRST) {
			auto n = WideCharToMultiByte((int)codepage.to, 0, pszFrom, (int)svFrom.size(), nullptr, 0, nullptr, nullptr);
			if (n <= 0)
				return str;
			str.resize(n);
			auto n2 = WideCharToMultiByte((int)codepage.to, 0, pszFrom, (int)svFrom.size(), (LPSTR)str.data(), (int)str.size(), nullptr, nullptr);
			return str;
		}
		else {
			auto n = svFrom.size()*2;
			if (n <= 0)
				return str;
			str.resize(n);
			n = WideCharToMultiByte((int)codepage.to, 0, pszFrom, (int)svFrom.size(), (LPSTR)str.data(), (int)str.size(), nullptr, nullptr);
			if (n >= 0) {
				str.resize(n);
			}
			else {
				n = WideCharToMultiByte((int)codepage.to, 0, pszFrom, (int)svFrom.size(), nullptr, 0, nullptr, nullptr);
				str.resize(n);
				WideCharToMultiByte((int)codepage.to, 0, pszFrom, (int)svFrom.size(), (LPSTR)str.data(), (int)str.size(), nullptr, nullptr);
				return str;
			}
		}
		return str;
	}
	template < bool bCOUNT_FIRST >
	std::wstring ConvMBCS_Wide(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::wstring str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			[[unlikely]]
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		auto* pszFrom = (char const*)svFrom.data();
		if constexpr (bCOUNT_FIRST) {
			auto n = MultiByteToWideChar((int)codepage.from, 0, pszFrom, (int)svFrom.size(), nullptr, 0);
			if (n <= 0)
				return str;
			str.resize(n);
			MultiByteToWideChar((int)codepage.from, 0, pszFrom, (int)svFrom.size(), str.data(), (int)str.size());
		}
		else {
			str.resize(svFrom.size());
			auto n = MultiByteToWideChar((int)codepage.from, 0, pszFrom, (int)svFrom.size(), str.data(), (int)str.size());
			if (n >= 0) {
				str.resize(n);
			} else {
				n = MultiByteToWideChar((int)codepage.from, 0, pszFrom, (int)svFrom.size(), nullptr, 0);
				str.resize(n);
				MultiByteToWideChar((int)codepage.from, 0, pszFrom, (int)svFrom.size(), str.data(), (int)str.size());
			}
		}

		// check endian, Convert
		internal::CheckAndConvertEndian(str, codepage.to);

		return str;
	}
#else
	template < bool bCOUNT_FIRST >
	std::string ConvWide_MBCS(std::wstring_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::string str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		// check endian, Convert
		auto strOther = internal::CheckAndConvertEndian(svFrom, codepage.from);
		if (strOther) { [[unlikely]] svFrom = strOther.value(); } //svFrom = strOther.value_or(svFrom);

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		auto eCodepageTo = codepage.To<char>();
		std::locale loc(std::format(".{}", (int)eCodepageTo));

		auto const& facet = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc);

		std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
		auto len = svFrom.size()*sizeof(char16_t);//facet.length(state, pszSourceBegin, pszSourceEnd, svFrom.size());
		str.resize(len);

		state = {}; // init.
		std::remove_cvref_t<decltype(svFrom)>::value_type const* pszSourceNext{};
		std::remove_cvref_t<decltype(str)>::value_type* pszDestNext{};
		auto result = facet.out(state, pszSourceBegin, pszSourceEnd, pszSourceNext,
					   str.data(), str.data()+len, pszDestNext);

		if (result == std::codecvt_base::error)
			throw std::invalid_argument{ GTL__FUNCSIG "String Cannot be transformed!" };

		// todo : test len.
		//len = tszlen(str.data(), str.data()+len+1);
		len = (int)(pszDestNext - str.data());
		str.resize(len);
		return str;
	}
	template < bool bCOUNT_FIRST >
	std::wstring ConvMBCS_Wide(std::string_view svFrom, S_CODEPAGE_OPTION codepage) {
		std::wstring str;
		if (svFrom.empty())
			return str;

		if (svFrom.size() > std::min((size_t)INT32_MAX, (size_t)RSIZE_MAX)) {
			throw std::invalid_argument{ GTL__FUNCSIG "string is too long." };
		}

		auto const* pszSourceBegin = svFrom.data();
		auto const* pszSourceEnd = svFrom.data()+svFrom.size();

		auto eCodepageFrom = codepage.From<char>();

		std::locale loc(std::format(".{}", (int)eCodepageFrom));

		auto const& facet = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc);

		std::mbstate_t state = {0}; // zero-initialization represents the initial conversion state for mbstate_t
		auto len = svFrom.size();//facet.length(state, pszSourceBegin, pszSourceEnd, svFrom.size());
		len *= 2;	// buffer. facet.length() does not return exact length.
		if (len <= 0)
			return str;
		str.resize(len);

		state = {}; // init.
		std::remove_cvref_t<decltype(svFrom)>::value_type const* pszSourceNext{};
		wchar_t* pszDestNext{};
		auto result = facet.in(state, pszSourceBegin, pszSourceEnd, pszSourceNext,
							   str.data(), str.data()+len, pszDestNext);

		if (result == std::codecvt_base::error)
			throw std::invalid_argument{ GTL__FUNCSIG "String Cannot be transformed!" };

		len = (int)(pszDestNext - str.data());
		if (str.size() != len)
			str.resize(len);

		// check endian, Convert
		internal::CheckAndConvertEndian(str, codepage.to);

		return str;
	}
#endif



#if 0
#pragma warning(push)
#pragma warning(disable: 4996)
	std::u32string ConvUTF8_UTF32(std::u8string_view sv) {
		std::wstring_convert<std::codecvt<char32_t, char8_t, std::mbstate_t>, char32_t> conversion;
		return conversion.from_bytes((char const*)sv.data());
	}

	std::u8string ConvUTF32_UTF8(std::u32string_view sv) {
		std::wstring_convert<std::codecvt<char32_t, char8_t, std::mbstate_t>, char32_t> conversion;
		return (std::u8string&)conversion.to_bytes(sv.data());
	}
#pragma warning(pop)
#endif


};	// namespace gtl;

