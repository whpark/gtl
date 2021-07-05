//////////////////////////////////////////////////////////////////////
//
// iconv_wrapper.h:
//
// PWH
//
// 2021.05.07. libiconv wrapper
// 
//   https://www.gnu.org/software/libiconv/
//
//   이거 있는줄 알았으면,... codepage converter 안 만들었을.......
//
//////////////////////////////////////////////////////////////////////

module;

#include <string_view>
#include <optional>
#include "iconv.h"

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:iconv_wrapper;
import :concepts;

/*******************************************


		European languages
			ASCII, ISO-8859-{1,2,3,4,5,7,9,10,13,14,15,16}, KOI8-R, KOI8-U, KOI8-RU, CP{1250,1251,1252,1253,1254,1257}, CP{850,866,1131}, Mac{Roman,CentralEurope,Iceland,Croatian,Romania}, Mac{Cyrillic,Ukraine,Greek,Turkish}, Macintosh
		Semitic languages
			ISO-8859-{6,8}, CP{1255,1256}, CP862, Mac{Hebrew,Arabic}
		Japanese
			EUC-JP, SHIFT_JIS, CP932, ISO-2022-JP, ISO-2022-JP-2, ISO-2022-JP-1, ISO-2022-JP-MS
		Chinese
			EUC-CN, HZ, GBK, CP936, GB18030, EUC-TW, BIG5, CP950, BIG5-HKSCS, BIG5-HKSCS:2004, BIG5-HKSCS:2001, BIG5-HKSCS:1999, ISO-2022-CN, ISO-2022-CN-EXT
		Korean
			EUC-KR, CP949, ISO-2022-KR, JOHAB
		Armenian
			ARMSCII-8
		Georgian
			Georgian-Academy, Georgian-PS
		Tajik
			KOI8-T
		Kazakh
			PT154, RK1048
		Thai
			ISO-8859-11, TIS-620, CP874, MacThai
		Laotian
			MuleLao-1, CP1133
		Vietnamese
			VISCII, TCVN, CP1258
		Platform specifics
			HP-ROMAN8, NEXTSTEP
		Full Unicode
			UTF-8
			UCS-2, UCS-2BE, UCS-2LE
			UCS-4, UCS-4BE, UCS-4LE
			UTF-16, UTF-16BE, UTF-16LE
			UTF-32, UTF-32BE, UTF-32LE
			UTF-7
			C99, JAVA 
		Full Unicode, in terms of uint16_t or uint32_t (with machine dependent endianness and alignment)
			UCS-2-INTERNAL, UCS-4-INTERNAL
		Locale dependent, in terms of `char' or `wchar_t' (with machine dependent endianness and alignment, and with OS and locale dependent semantics)
			char, wchar_t
			The empty encoding name "" is equivalent to "char": it denotes the locale dependent character encoding.

		When configured with the option --enable-extra-encodings, it also provides support for a few extra encodings:

		European languages
			CP{437,737,775,852,853,855,857,858,860,861,863,865,869,1125}
		Semitic languages
			CP864
		Japanese
			EUC-JISX0213, Shift_JISX0213, ISO-2022-JP-3
		Chinese
			BIG5-2003 (experimental)
		Turkmen
			TDS565
		Platform specifics
			ATARIST, RISCOS-LATIN1 


*******************************************/

export namespace gtl {

	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, size_t initial_dst_buf_size = 1024 >
	class Ticonv {
		libiconv_t cd_ {(iconv_t)-1};

	public:
		Ticonv(char const* to = nullptr, char const* from = nullptr) {
			if (!to)
				to = GuessCodeFromType<tchar_to>();
			if (!from)
				from = GuessCodeFromType<tchar_from>();
			cd_ = iconv_open(to, from);
		}
		Ticonv(char const* to, char const* from, bool bTransliterate, bool bDiscardIlseq) : Ticonv(to, from) {
			SetTransliterate(bTransliterate);
			SetDiscardIsseq(bDiscardIlseq);
		}
		~Ticonv() {
			if (IsOpen())
				iconv_close(cd_);
		}

		bool IsOpen() const {
			return cd_ != (iconv_t)-1;
		}

		bool IsTrivial() const {
			return IsFlag(ICONV_TRIVIALP);
		}
		bool IsTransliterate() const {
			return IsFlag(ICONV_GET_TRANSLITERATE);
		}
		bool IsDiscardIlseq() const {
			return IsFlag(ICONV_SET_DISCARD_ILSEQ);
		}
		void SetTransliterate(bool bOn = true) {
			SetFlag(ICONV_SET_TRANSLITERATE, bOn);
		}
		void SetDiscardIsseq(bool bOn = true) {
			SetFlag(ICONV_SET_DISCARD_ILSEQ, bOn);
		}

	public:
		std::optional<std::basic_string<tchar_to>> Convert(std::basic_string<tchar_from> const& strFrom) {
			return Convert(std::basic_string_view<tchar_from>{strFrom});
		}
		std::optional<std::basic_string<tchar_to>> Convert(std::basic_string_view<tchar_from> svFrom) {
			if (!IsOpen())
				return {};

			// source
			using char_source_buf_t = char;	// or char const

			char_source_buf_t* src = std::bit_cast<char_source_buf_t*>(svFrom.data());
			size_t remnant_src = svFrom.size()*sizeof(tchar_from);

			auto Conv = [](iconv_t cd, std::basic_string<tchar_to>& strTo, char_source_buf_t** psrc, auto& remnant_src, auto& remnant_dst) -> std::optional<std::basic_string<tchar_to>> {
				while (remnant_src) {
					char* out = std::bit_cast<char*>(strTo.data() + strTo.size() - remnant_dst/sizeof(tchar_to));
					if (iconv(cd, psrc, &remnant_src, &out, &remnant_dst) == (size_t)-1) {
						if (errno == E2BIG) {
							//size_t const old_size = strTo.size() * sizeof(tchar_to);
							//size_t const new_size = strTo.size() * sizeof(tchar_to) * 2;
							remnant_dst += strTo.size() * sizeof(tchar_to);
							strTo.resize(strTo.size()*2);
							continue;
						}
						return {};
					}
				}

				strTo.resize(strTo.size() - remnant_dst/sizeof(tchar_to));
				return std::move(strTo);
			};


			if constexpr (initial_dst_buf_size != 0) {
				tchar_to initial_dst_buf[initial_dst_buf_size]{};
				size_t remnant_dst = sizeof(initial_dst_buf);
				char* out = (char*)initial_dst_buf;
				if (iconv(cd_, &src, &remnant_src, &out, &remnant_dst) == (size_t)-1) {
					if (errno == E2BIG) {
						std::basic_string<tchar_to> strTo;
						size_t const old_size = sizeof(initial_dst_buf);
						size_t const new_size = std::max(old_size*2, svFrom.size());
						strTo.resize(new_size/sizeof(tchar_to));
						std::memmove(strTo.data(), initial_dst_buf, old_size - remnant_dst);
						remnant_dst += (new_size - old_size);

						return Conv(cd_, strTo, &src, remnant_src, remnant_dst);
					}
					return {};
				}
				return std::move(std::basic_string<tchar_to>(initial_dst_buf, std::size(initial_dst_buf)-remnant_dst/sizeof(tchar_to)));

			} else {
				std::basic_string<tchar_to> strTo;
				strTo.resize(svFrom.size());
				size_t remnant_dst = strTo.size()*sizeof(tchar_to);

				return Conv(cd_, strTo, &src, remnant_src, remnant_dst);
			}
		}

	public:
		template < gtlc::string_elem tchar >
		constexpr static char const* GuessCodeFromType() {
			using namespace std::literals;
			if constexpr (std::is_same_v<tchar, char8_t>) {
				return "UTF-8";
			}
			else if (std::is_same_v<tchar, char16_t>) {
				if constexpr (std::endian::native == std::endian::little) {
					return "UTF-16LE";
				} else {
					return "UTF-16BE";
				}
			} else if (std::is_same_v<tchar, char32_t>) {
				if constexpr (std::endian::native == std::endian::little) {
					return "UTF-32LE";
				} else {
					return "UTF-32BE";
				}
			}
			else if (std::is_same_v<tchar, wchar_t>) {
				return "wchar_t";
			} else if (std::is_same_v<tchar, char>) {
				return "char";
			}
		#if (GTL__STRING_SUPPORT_CODEPAGE_KSSM)
			else if (std::is_same_v<tchar, uint16_t>) {
				return "JOHAB";
			}
		#endif
			return nullptr;
		};

	protected:
		bool IsFlag(int flag) const {
			int value {};
			iconvctl(cd_, flag, &value);
			return value != 0;
		}
		void SetFlag(int flag, bool bOn) {
			int bValue { bOn };
			iconvctl(cd_, flag, &bValue);
		}
	};

	// helper
	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, size_t initial_dst_buf_size = 1024 >
	std::optional<std::basic_string<tchar_to>> ToString_iconv(std::basic_string_view<tchar_from> svFrom, char const* szCodeTo = nullptr, char const* szCodeFrom = nullptr) {
		return Ticonv<tchar_to, tchar_from, initial_dst_buf_size>{szCodeTo, szCodeFrom}.Convert(svFrom);
	}

	// helper
	template < gtlc::string_elem tchar_to, gtlc::string_elem tchar_from, size_t initial_dst_buf_size = 1024 >
	std::optional<std::basic_string<tchar_to>> ToString_iconv(std::basic_string<tchar_from> const& strFrom, char const* szCodeTo = nullptr, char const* szCodeFrom = nullptr) {
		return Ticonv<tchar_to, tchar_from, initial_dst_buf_size>{szCodeTo, szCodeFrom}.Convert(strFrom);
	}

}
