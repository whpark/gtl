#pragma once

//////////////////////////////////////////////////////////////////////
//
// archive.h: TArchive : binary in/out, string encoding converting, Read/Write Text Strings (with encodings...)
//
// PWH
// 2018.12.10.
// 2019.07.24. QL -> GTL
// 2019.10.22. 2019.11.22. archive --> iarchive / oarchive 분리
// 2020.11.30. GTL.2020
//
// stream helper
//
//////////////////////////////////////////////////////////////////////

#include "_default.h"
#include "_lib_gtl.h"
#include "concepts.h"
#include "string.h"

#if 0

namespace gtl {
#pragma pack(push, 8)


	//-----------------------------------------------------------------------------
	//

	/// @brief TArchive : class for file io.
	///   i) Reads line by line for text file.
	///  ii) UTF Encoding/Decoding. ByteSwapping...
	/// @tparam stream_t : any of std::basic_stream
	/// @tparam bSWAP_BYTE_ORDER : swap bytes order for integral types. (ex, for network stream)
	/// @tparam bSTORE : is a storing stream
	/// @tparam bLOAD : is a loading stream
	template < typename tstream,
		bool bSWAP_BYTE_ORDER = false,
		bool bSTORE = std::is_base_of_v<std::ostream, tstream>,
		bool bLOAD = std::is_base_of_v<std::istream, tstream>//,
		//size_t PROCESSING_BUFFER_SIZE = 4096
	>
	class TArchive {

	#define CHECK_ARCHIVE_LOADABLE\
		if constexpr (bSTORE && bLOAD) { if (!IsLoad()) { throw std::ios_base::failure(GTL__FUNCSIG "NOT an archive for loading!"); } }
	#define CHECK_ARCHIVE_STORABLE\
		if constexpr (bSTORE && bLOAD) { if (!IsStore()) { throw std::ios_base::failure(GTL__FUNCSIG "NOT an archive for storing!"); } }


	protected:
		tstream& m_stream;
		eCODEPAGE m_eCodepage{ eCODEPAGE::UTF8 };
	public:
		bool const m_bStore;

	public:
		TArchive() = delete;
		TArchive(const TArchive&) = delete;
		TArchive(TArchive&&) = default;
		explicit TArchive(tstream& stream) requires ((bSTORE && !bLOAD) || (!bSTORE && bLOAD))
			: m_stream{ stream }, m_bStore{ bSTORE }
		{
		}

		TArchive(tstream& stream, bool bStore) requires (bSTORE && bLOAD)
			: m_stream{ stream }, m_bStore{ bStore }
		{
		}

	public:
		/// @brief Set/Get Codepage.
		int SetDefaultCodepage(int eCodepage) { return std::exchange(m_eCodepage, eCodepage); }
		int GetDefaultCodepage() const { return m_eCodepage; }

		// 
		constexpr bool IsSwapByteOrder() const { return bSWAP_BYTE_ORDER; }	// 'Get'SwapByteOrder  라고 이름을 바꾸면 안됨..... 헷갈림

		/// @brief tells its loading or storing.
		/// @return 
		bool IsStoring() const {
			if constexpr (bSTORE && bLOAD) {
				return m_bStore;
			}
			else {
				return bSTORE;
			}
		}
		bool IsLoading() const {
			if constexpr (bSTORE && bLOAD) {
				return !m_bStore;
			}
			else {
				return bLOAD;
			}
		}

		using is_storing_only_t = std::bool_constant<bSTORE && !bLOAD>;
		using is_loading_only_t = std::bool_constant<!bSTORE && bLOAD>;
		using is_storing_t = std::bool_constant<bSTORE>;
		using is_loading_t = std::bool_constant<bLOAD>;


		// todo : ....... Write/Read 정리. 2020.12.02
	public:
		/// @brief Write byte buffer
		inline void Write(void const* data, std::streamsize size) requires (bSTORE) {
			CHECK_ARCHIVE_STORABLE;
			m_stream.write((std::ostream::char_type const*)data, size);
		}
		/// @brief Write byte buffer
		template < gtlc::trivially_copyable TYPE > requires (bSTORE and !bSWAP_BYTE_ORDER)	// some structs can contain integral values which needs to be swapped byte ordering.
		inline void Write(TYPE const& v) {
			CHECK_ARCHIVE_STORABLE;
			Write(&v, sizeof(v));
		}
		///// @brief Write byte buffer
		//[[nodiscard]] bool WriteAndVerify(void const* data, std::streamsize size) requires (bSTORE) {
		//	CHECK_ARCHIVE_STORABLE;
		//	auto pos0 = m_stream.tellp();
		//	Write(data, size);
		//	auto pos1 = m_stream.tellp();
		//	return (pos1 - pos0 == size);
		//}


	public:
		//---------------------------------------------------------------------
		// Read byte buffer
		template < typename T > requires (bLOADING)
		inline std::streamsize Read(void* data, std::streamsize size) {
			CHECK_ARCHIVE_LOADABLE;
			auto pos0 = m_stream.tellg();
			if (m_stream.read((std::istream::char_type*)data, size)) {
#ifdef _DEBUG
				auto pos1 = m_stream.tellg();
				auto nRead = pos1 - pos0;
				return nRead;
#else
				return (m_stream.tellg() - pos0);
#endif
			}
			return -1;
		}
		template < gtlc::trivially_copyable TYPE > requires (bLOADING and !bSWAP_BYTE_ORDER)	// some structs can contain integral values which needs to be swapped byte ordering.
		inline std::streamsize Read(TYPE& v) {
			return Read(&v, sizeof(v));
		}


	protected:
		/// @brief Write Ints Swaps byte-order if (bSWAP_BYTE_ORDER)
		/// @param container 
		/// @param nCount 
		template < std::integral T_INT, int PROCESSING_BUFFER_SIZE = 4096 > requires (bSTORE)
		void WriteInts(T_INT const* data, std::streamsize nCount) {
			CHECK_ARCHIVE_STORABLE;
			if constexpr (bSWAP_BYTE_ORDER and (sizeof(T_INT) > 1)) {
				WriteBultKintsSwapByte<T_INT, PROCESSING_BUFFER_SIZE>(data, nCount);
			}
			else {
				Write(data, nCount*sizeof(T_INT), true);
			}
		}
		template < std::integral T_INT, gtlc::contiguous_container<T_INT> tcontainer > requires (bSTORE)
		inline void WriteInts(tcontainer const& container) {
			WriteInts(std::data(container), std::size(container));
		}

		/// @brief Read Ints. Swaps byte-order if (bSWAP_BYTE_ORDER)
		/// @param container ints.
		/// @param nCount count. (NOT size in bytes but count in item)
		/// @return nCount if OK,  or -1 if failed.
		template < std::integral T_INT > requires (bLOAD)
		std::streamsize ReadInts(T_INT* data, std::streamsize nCount) {
			CHECK_ARCHIVE_LOADABLE;
			auto nRead = Read(data, nCount*sizeof(T_INT));
			if (nRead <= 0)
				return nRead;
			std::streamsize nReadCount = nRead / sizeof(T_INT);
			if constexpr (bSWAP_BYTE_ORDER && sizeof(T_INT) > 1) {
				for (std::streamsize i = 0; i < nReadCount; i++) {
					SwapByteOrder(data[i]);
				}
			}
			return nReadCount;
		}
		template < std::integral T_INT, gtlc::contiguous_container<T_INT> tcontainer > requires (bLOAD)
		inline std::streamsize ReadInts(tcontainer& container) {
			return ReadInts(std::data(container), std::size(container));
		}


		/// @brief Write Ints Swaps byte-order if (bSWAP_BYTE_ORDER)
		/// @param container 
		/// @param nCount 
		template < std::integral T_INT, int PROCESSING_BUFFER_SIZE = 4096 > requires (bSTORE)
		void WriteIntsSwapByte(T_INT const* data, std::streamsize nCount) {
			CHECK_ARCHIVE_STORABLE;
			if constexpr (sizeof(T_INT) > 1)) {
				while (nCount >= 0) {
					std::array<T_INT, PROCESSING_BUFFER_SIZE/sizeof(T_INT)> buffer;
					std::streamsize nToWrite = std::min(nCount, buffer.size());
					for (std::streamsize i = 0; i < nToWrite; i++) {
						buffer[i] = GetByteSwap(data[i]);
					}
					Write(buffer.data(), nToWrite*sizeof(buffer[0]));
					data += nToWrite;
					nCount -= nToWrite;
				}
			}
			else {
				Write(data, nCount*sizeof(T_INT), true);
			}
		}
		template < std::integral T_INT, gtlc::contiguous_container<T_INT> tcontainer > requires (bSTORE)
		inline void WriteIntsSwapByte(tcontainer const& container) {
			WriteIntsSwapByte(std::data(container), std::size(container));
		}


		/// @brief Read Ints. Swaps byte-order if (bSWAP_BYTE_ORDER)
		/// @param container ints.
		/// @param nCount count. (NOT size in bytes but count in item)
		/// @return 
		template < std::integral T_INT > requires (bLOAD)
		std::streamsize ReadIntsSwapByte(T_INT* data, std::streamsize nCount) {
			CHECK_ARCHIVE_LOADABLE;
			auto nRead = Read(data, nCount*sizeof(T_INT));
			if (nRead <= 0)
				return nRead;
			std::streamsize nReadCount = nRead / sizeof(T_INT);
			if constexpr (sizeof(T_INT) > 1) {
				for (std::streamsize i = 0; i < nReadCount; i++) {
					SwapByteOrder(data[i]);
				}
			}
			return nReadCount;
		}
		template < std::integral T_INT, gtlc::contiguous_container<T_INT> tcontainer > requires (bLOAD)
		inline std::streamsize ReadIntsSwapByte(tcontainer& container) {
			return ReadIntsSwapByte(std::data(container), std::size(container));
		}


	public:
		template < typename T >
		constexpr bool IsBulkIOCapable() {
			if constexpr (!std::is_trivially_copyable_v<T>) {
				return false;
			}

			if constexpr (!bSWAP_BYTE_ORDER) {
				return true;
			}

			if constexpr (std::is_floating_point_v<T> or (sizeof(T) == 1)) {
				return true;
			}

			return false;
		}


		//---------------------------------------------------------------------
		// operator >>, <<, &
		template < gtlc::trivially_copyable TYPE > requires (bSTORE and IsBulkIOCapable<TYPE>())
		TArchive& operator << (TYPE const& v) {
			CHECK_ARCHIVE_STORABLE;
			if constexpr (bSWAP_BYTE_ORDER and std::is_integral_v<TYPE> and (sizeof(TYPE) > 1)) {
				TYPE v2 = GetSwapByteOrder(v);
				Write(&v2, sizeof(v2), true);
			}
			else {
				Write(&v, sizeof(v), true);
			}
			return *this;
		};
		template < gtlc::trivially_copyable TYPE > requires (bLOAD and IsBulkIOCapable<TYPE>())
		TArchive& operator >> (TYPE& v) {
			CHECK_ARCHIVE_LOADABLE;
			std::streamsize nRead = Read(&v, sizeof(v));
			if (nRead < (std::streamsize)sizeof(v))
				throw std::ios_base::failure(GTL__FUNCSIG "unexpected EOF");
			if constexpr (bSWAP_BYTE_ORDER and std::is_integral_v<TYPE> and (sizeof(TYPE) > 1)) {
				ByteSwap(v);
			}
			return *this;
		};
		template < typename TYPE > requires (IsBulkIOCapable<TYPE>())
		TArchive& operator & (TYPE& v) {
			if constexpr (bSTORE && bLOAD) {
				if (IsStore())
					return *this << v;
				else
					return *this >> v;
			} else if constexpr (bSTORE) {
				return *this << v;
			} else if constexpr (bLOAD) {
				return *this >> v;
			} else {
				static_assert(false, "no way");
				return *this;
			}
		};
		//template < gtlc::trivially_copyable TYPE > requires (bSTORE)
		//TArchive& operator & (TYPE const& v) {
		//	CHECK_ARCHIVE_STORABLE;
		//	return *this << v;
		//};


		//---------------------------------------------------------------------
		// Write BOM
		void WriteCodepageBOM(eCODEPAGE eCodepage) requires (bSTORE) {
			CHECK_ARCHIVE_STORABLE;
			if (eCodepage == eCODEPAGE::DEFAULT) {
				eCodepage = eCODEPAGE::UTF8;
			}
			std::string_view sv = GetCodepageBOM(eCodepage);
			if (sv.size() > 0)
				Write(sv.data(), sv.size());
			SetDefaultCodepage(eCodepage);
		};
		//---------------------------------------------------------------------
		// Read BOM
		eCODEPAGE ReadCodepageBOM(int eDefaultCodepage = eCODEPAGE::UTF8) requires (bLOAD) {
			CHECK_ARCHIVE_LOADABLE;
			unsigned char c = 0;

			auto get = [](tstream& stream) -> std::optional<char> {
				char c{};
				static_assert(sizeof(c) == sizeof(typename tstream::char_type));
				if (stream.get((typename tstream::char_type&)c))
					return c;
				return {};
			};
			auto peek = [](tstream& stream, std::string_view sv) -> bool {
				auto pos = stream.tellg();
				bool bMatch {true};
				for (auto c : sv) {
					if (auto ci = get(stream); ci == c) {
						bMatch = false;
						break;
					}
				}
				if (bMatch)
					return true;
				stream.seekg(pos);
				return false;
			};

			constexpr static std::vector<std::pair<std::string_view, eCODEPAGE>> const codepages{
				{GetCodepageBOM(eCODEPAGE::UTF32BE), eCODEPAGE::UTF32BE},	// in BOM Length order ...
				{GetCodepageBOM(eCODEPAGE::UTF32LE), eCODEPAGE::UTF32LE},
				{GetCodepageBOM(eCODEPAGE::UTF16BE), eCODEPAGE::UTF16BE},
				{GetCodepageBOM(eCODEPAGE::UTF16LE), eCODEPAGE::UTF16LE},
				{GetCodepageBOM(eCODEPAGE::UTF8), eCODEPAGE::UTF8},
			};

			for (auto const& [sv, codepage] : codepages) {
				if (peek(m_stream, sv)) {
					SetDefaultCodepage(codepage);
					return codepage;
				}
			}

			SetDefaultCodepage(eDefaultCodepage);

			return eDefaultCodepage;
		};


		//---------------------------------------------------------------------
		// Read / Write String
		template < typename tchar > requires (bLOAD)
		std::optional<std::basic_string<tchar>> ReadString(tchar cDelimiter = '\n', bool bTrimCR = true) {
			str.clear();
			CHECK_ARCHIVE_LOADABLE;

			bool bResult = false;
			auto eCodepage = GetCodepage();
			switch ((int)eCodepage) {
			default :
			case eCODEPAGE::DEFAULT :
			case eCODEPAGE::ACP :
				{
					if constexpr (std::is_same_v<tchar, char>) {
						bResult = std::getline(m_stream, str, cDelimiter).good();
					} else {
						std::basic_string<char> strLine;
						bResult = std::getline(m_stream, strLine, cDelimiter).good();
						str = ToString<tchar>(strLine, {.from = GetDefaultCodepage()});
					}
				}
				break;
			case eCODEPAGE::UTF8 :
				{
					if constexpr (std::is_same_v<tchar, char8_t>) {
						bResult = std::getline(m_stream, str, cDelimiter).good();
					} else {
						std::basic_string<char8_t> strLine;
						bResult = std::getline(m_stream, strLine, cDelimiter).good();
						S_CODEPAGE_OPTION option{.to = std::is_same_v<tchar, char> ? GetDefaultCodepage() : eCODEPAGE::DEFAULT};
						str = ToString<tchar>(strLine, option);
					}
				}
				break;

			case eCODEPAGE::UTF16LE :
			case eCODEPAGE::UTF16BE :
				{
					if constexpr (sizeof(tchar) == sizeof(char16_t)) {	// char16_t, wchar_t, uint16_t
						auto eStreamByteOrder = (eCodepage == eCODEPAGE::UTF16LE) ? std::endian::little : std::endian::big;
						bool bSwapByteOrder = (std::endian::native != eStreamByteOrder) xor bSWAP_BYTE_ORDER;
						if (bSwapByteOrder) [[unlikely]] {
							bResult = std::getline<char16_t>(m_stream, str, GetByteSwap<char16_t>(cDelimiter)).good();
							for (auto& c : str)
								ByteSwap(c);
						}
						else [[likely]] {
							bResult = std::getline(m_stream, str, cDelimiter).good();
						}
					}
					else {
						if (auto r = ReadString<char16_t>(cDelimiter, bTrimCR); r) {
							S_CODEPAGE_OPTION option{.to = std::is_same_v<tchar, char> ? GetDefaultCodepage() : eCODEPAGE::DEFAULT};
							return ToString(*r, option);
						}
						else {
							return {};
						}
					}
				}
				break;

				// todo : 2020.12.08. UTF32

			}

			if (!bResult and !str.empty() and m_stream.eof())	// 마지막 라인
				bResult = true;
			if (bTrimCR)
				TrimRight(str, '\r');
			return bResult;
		};

		inline std::optional<std::string>		ReadStringA(char cDelimiter = '\n', bool bTrimCR = true) requires (bLOAD) { return ReadString<char>(cDelimiter, bTrimCR); }
		inline std::optional<std::u8string>		ReadStringU8(char8_t cDelimiter = u8'\n', bool bTrimCR = true) requires (bLOAD) { return ReadString<char8_t>(cDelimiter, bTrimCR); }
		inline std::optional<std::u16string>	ReadStringU16(char16_t cDelimiter = u'\n', bool bTrimCR = true) requires (bLOAD) { return ReadString<char16_t>(cDelimiter, bTrimCR); }
		inline std::optional<std::u32string>	ReadStringU32(char32_t cDelimiter = U'\n', bool bTrimCR = true) requires (bLOAD) { return ReadString<char32_t>(cDelimiter, bTrimCR); }
		inline std::optional<std::wstring>		ReadStringW(wchar_t cDelimiter = L'\n', bool bTrimCR = true) requires (bLOAD) { return ReadString<wchar_t>(cDelimiter, bTrimCR); }

		template < typename tchar, bool bWriteNewLine = false > requires (bSTORE)
		void TWriteString(std::basic_string_view<tchar> sv) {
			CHECK_ARCHIVE_STORABLE;

			auto write_string = [&] <typename tchar_other> (eCODEPAGE eCodepage, bool bForceSwapByteOrder) {
				std::streamsize nWritten {};
				if constexpr (std::is_same_v<std::remove_cvref_t<tchar>, std::remove_cvref_t<tchar_other>>) {
					WriteInts(sv.data(), sv.size(), bForceSwapByteOrder);
				} else {
					std::basic_string<decltype(ch)> strDest;
					ConvCodepage(strDest, sv, eCodepage);
					WriteInts(strDest.data(), strDest.size(), bForceSwapByteOrder);
				}
			};

			bool bResult = false;
			auto eCodepage = GetDefaultCodepage();
			switch (GetCodepage()) {
			case eCHAR_ENCODING::AUTO :
			case eCHAR_ENCODING::MBCS :
				write_string(char(0), eCodepage, false);
				break;
			case eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN :
				write_string(wchar_t(0), eCodepage, std::endian::native != std::endian::little);
				break;
			case eCHAR_ENCODING::UNICODE_BIG_ENDIAN :
				write_string(wchar_t(0), eCodepage, std::endian::native != std::endian::big);
				break;
			case eCHAR_ENCODING::UTF8 :
				write_string(char8_t(0), eCodepage, false);
				break;
			}

			if constexpr (bWriteNewLine) {
				WriteNewLine();
			}
		};
		void WriteNewLine() requires (bSTORE) {
			CHECK_ARCHIVE_STORABLE;

			auto eCodepage = GetDefaultCodepage();
			switch (GetCodepage()) {
			case eCHAR_ENCODING::AUTO :
			case eCHAR_ENCODING::MBCS :
				WriteInts<char>("\r\n", 2, false);
				break;
			case eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN :
				WriteInts<wchar_t>(L"\r\n", 2, std::endian::native != std::endian::little);
				break;
			case eCHAR_ENCODING::UNICODE_BIG_ENDIAN :
				WriteInts<wchar_t>(L"\r\n", 2, std::endian::native != std::endian::big);
				break;
			case eCHAR_ENCODING::UTF8 :
				WriteInts<char8_t>(_U8("\r\n"), 2, false);
				break;
			}
		};

#if defined(__cpp_lib_format)
		template < typename ... Args >	void WriteString(std::string_view sv, Args&& ... args)		requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::u8string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char8_t>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::u16string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char16_t>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::u32string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char32_t>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::wstring_view sv, Args&& ... args)		requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<wchar_t>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char, true>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::u8string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char8_t, true>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::u16string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char16_t, true>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::u32string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char32_t, true>(std::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::wstring_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<wchar_t, true>(std::format(sv, std::forward<Args>(args) ...)); }
#else
		template < typename ... Args >	void WriteString(std::string_view sv, Args&& ... args)		requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::u8string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char8_t>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::u16string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char16_t>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::u32string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char32_t>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteString(std::wstring_view sv, Args&& ... args)		requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<wchar_t>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char, true>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::u8string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char8_t, true>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::u16string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char16_t, true>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::u32string_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<char32_t, true>(fmt::format(sv, std::forward<Args>(args) ...)); }
		template < typename ... Args >	void WriteStringNL(std::wstring_view sv, Args&& ... args)	requires (bSTORE) { CHECK_ARCHIVE_STORABLE; TWriteString<wchar_t, true>(fmt::format(sv, std::forward<Args>(args) ...)); }
#endif


		//---------------------------------------------------------------------
		// Read / Write Size
		template < size_t nMinItemSize = 4 > requires (bLOAD)
		size_t LoadSize() {
			static_assert(std::ranges::any_of({1, 2, 4, sizeof(size_t)}, nMinItemSize));
			CHECK_ARCHIVE_LOADABLE;
			auto& ar = *this;
			union {
				uint8_t u8;
				uint16_t u16;
				uint32_t u32;
				uint64_t u64;
			} size;
			if constexpr (nMinItemSize <= 1) {
				if (!(ar >> size.u8).m_stream)
					throw std::ios_base::failure(GTL__FUNCSIG "Stream exception");
				if (size.u8 < 0xff)
					return size.u8;
			}
			if constexpr (nMinItemSize <= 2) {
				if (!(ar >> size.u16).m_stream)
					throw std::ios_base::failure(GTL__FUNCSIG "Stream exception");
				if (size.u8 < 0xffff)
					return size.u16;
			}
			if constexpr (nMinItemSize <= 4) {
				if (!(ar >> size.u32).m_stream)
					throw std::ios_base::failure(GTL__FUNCSIG "Stream exception");

				if (size.u32 < 0xffff'ffff)
					return size.u32;
				if constexpr (sizeof(size_t) == sizeof(size.u32)) {
					throw std::ios_base::failure(GTL__FUNCSIG "Stream Size is too big");
				}
			}
			if constexpr (sizeof(size_t) >= sizeof(size.u64)) {
				if (!(ar >> size.u64).m_stream)
					throw std::ios_base::failure(GTL__FUNCSIG "Stream exception");
				return size.u64;
			}
		}
		template < size_t nMinItemSize = 4 > requires (bSTORE)
		TArchive& StoreSize(size_t size) {
			static_assert(std::ranges::any_of({1, 2, 4, sizeof(size_t)}, nMinItemSize));
			CHECK_ARCHIVE_STORABLE;
			auto& ar = *this;
			if constexpr (nMinItemSize <= 1) {
				if (size < 0xff) {
					ar << (uint8_t) size;
					return *this;
				} else {
					ar << (uint8_t) 0xff;
				}
			}
			if constexpr (nMinItemSize <= 2) {
				if (size < 0xffff) {
					ar << (uint16_t) size;
					return *this;
				} else {
					ar << (uint16_t) 0xffff;
				}
			}
			if constexpr (nMinItemSize <= 4) {
				if constexpr (sizeof(size_t) >= sizeof(uint64_t)) {
					if (size < 0xffff'ffffull) {
						ar << (uint32_t) size;
					} else {
						ar << (uint32_t) 0xffff'ffff;
						ar << (uint64_t) size;
					}
				} else {
					ar << (uint32_t) size;
				}
			}
			return *this;
		}


		//---------------------------------------------------------------------
		// Flush / Close
		void Flush() {
			m_stream.flush();
		}
		void Close() {
		}

		//-----------------------------------------------------------------------------
		// Generic Archive In / Out
		//
		//template < typename T > TArchive& operator << (TArchive& ar, const std::vector<T>& container);
		//template < typename T > TArchive& operator >> (TArchive& ar, std::vector<T>& container);
		//template < typename T > TArchive& operator & (TArchive& ar, std::vector<T>& container);


		//-------------------------
		// Container ... std::vector, std::deque
		template < typename T, gtlc::container<T> tcontainer > requires (bSTORE)
		TArchive& StoreContainer(tcontainer const& container) {
			CHECK_ARCHIVE_STORABLE;
			auto count = std::size(container);
			StoreSize(count);
			if (count) {
				constexpr bool bSwapMightBeNeeded = (bSWAP_BYTE_ORDER and (sizeof(T) > 1) and (std::is_integral_v<T> or std::is_class_v<T>));
				if constexpr (std::is_trivially_copyable_v<T> and !bSwapMightBeNeeded) {
					Write((uint8_t const*)std::data(container), std::size(container)*sizeof(T));
				} else {
					for (auto const& item : container) {
						(*this) << item;
					}
				}
			}
			return *this;
		};
		template < typename T, gtlc::container<T> tcontainer > requires (bLOAD)
		TArchive& LoadContainer(tcontainer& container) {
			CHECK_ARCHIVE_LOADABLE;
			container.clear();
			size_t size = LoadSize();
			if (size) {
				constexpr bool bSwapMightBeNeeded = (bSWAP_BYTE_ORDER and (sizeof(T) > 1) and (std::is_integral_v<T> or std::is_class_v<T>));
				if constexpr (requires (tcontainer container) { container.resize(size_t{}); }) {
					container.resize(size);
				}
				if constexpr (std::is_trivially_copyable_v<T> and !bSwapMightBeNeeded) {
					Read((uint8_t const*)std::data(container), std::size(container)*sizeof(T));
				} else {
					for (auto const& item : container) {
						(*this) << item;
					}
				}
				if constexpr (std::is_trivially_copyable_v<T>) {
					container.resize(size);
					if constexpr (bSWAP_BYTE_ORDER and std::is_integral_v<T> and (sizeof(T) > 1)) {
						if (ReadInts(container.data(), container.size()) != container.size())
							throw std::ios_base::failure(GTL__FUNCSIG "CANNOT Read Data");
					} else {
						auto len = container.size()*sizeof(T);
						if (Read(container.data(), container.size()*sizeof(T)) != len)
							throw std::ios_base::failure(GTL__FUNCSIG "CANNOT Read Data");
					}
				} else {
					if constexpr (requires (tcontainer container) { container.reserve(size); }) {
						container.reserve(size);
					}
					for (size_t i = 0; i < size; i++) {
						if constexpr (requires (tcontainer container, T a) { container.push_back(a); }) {
							container.push_back({});
							*this >> container.back();
						} else if constexpr (requires (tcontainer container, T a) { container.insert(a); }) {
							T a;
							*this >> a;
							container.insert(std::move(a));
						} else {
							static_assert(false, "CONTAINER must have push_back or insert ...");
						}
					}
				}
			}
			return *this;
		};
		template < typename T, gtlc::container<T> tcontainer > requires (bSTORE)
		TArchive& operator << (tcontainer const& data) {
			return StoreContainer(data);
		};
		template < typename T, gtlc::container<T> tcontainer > requires (bLOAD)
		TArchive& operator >> (tcontainer& data) {
			return LoadContainer(data);
		};
		template < typename T, gtlc::container<T> tcontainer >
		TArchive& operator & (CONTAINER& data) {
			if constexpr (bSTORE && bLOAD) {
				if (IsStore())
					return StoreContainer(data);
				else
					return LoadContainer(data);
			} else if constexpr (bSTORE) {
				return StoreContainer(data);
			} else if constexpr (bLOAD) {
				return LoadContainer(data);
			} else {
				//static_assert(false, "what???");
				return *this;
			}
		};
		template < gtl::is__container CONTAINER > requires (bSTORE)
		TArchive& operator & (const CONTAINER& data) {
			if constexpr (bSTORE && bLOAD) {
				if (!IsStore()) {
					throw std::ios_base::failure(GTL__FUNCSIG "NOT an storing archive!");
					return *this;
				}
			}
			return StoreContainer(data);
		};

		//-----------------------------------------------------------------------------
		// std::unique_ptr
		template < typename T > requires (bSTORE)
		TArchive& operator << (const std::unique_ptr<T>& ptr) {
			CHECK_ARCHIVE_STORABLE;
			if constexpr (gtl::is__dynamic_serializable<T>) {
				ptr->DynamicSerializeOut(*this);
			} else {
				(*this) & *ptr;
			}
			return *this;
		}
		template < typename T > requires (bLOAD)
		TArchive& operator >> (std::unique_ptr<T>& ptr) {
			CHECK_ARCHIVE_LOADABLE;
			if constexpr (gtl::is__dynamic_serializable<T>) {
				ptr = T::DynamicSerializeIn(*this);
			} else {
				if (!ptr) {
					if constexpr (has__NewObject<T>) {
						ptr = T::NewObject();
					} else {
						ptr.reset(new T);
					}
				}
				(*this) & *ptr;
			}
			return *this;
		}
		template < typename T >
		TArchive& operator & (std::unique_ptr<T>& ptr) {
			if constexpr (bSTORE && bLOAD) {
				if (IsStore())
					return *this << ptr;
				else
					return *this >> ptr;
			} else if constexpr (bSTORE) {
				return *this << ptr;
			} else if constexpr (bLOAD) {
				return *this >> ptr;
			} else {
				//static_assert(false, "what???");
				return *this;
			}
		}
		template < typename T > requires (bSTORE)
		TArchive& operator & (const std::unique_ptr<T>& ptr) {
			CHECK_ARCHIVE_STORABLE;
			return *this << ptr;
		}


		//-----------------------------------------------------------------------------
		// std::pair
		template < typename T1, typename T2 > requires (bSTORE)
		TArchive& operator << (const std::pair<T1, T2>& p) {
			CHECK_ARCHIVE_STORABLE;
			return *this & p.first & p.second;
		}
		template < typename T1, typename T2 > requires (bLOAD)
		TArchive& operator >> (std::pair<T1, T2>& p) {
			CHECK_ARCHIVE_LOADABLE;
			return *this & p.first & p.second;
		}
		template < typename T1, typename T2 >
		TArchive& operator & (std::pair<T1, T2>& p) {
			return (*this) & p.first & p.second;
		}
		template < typename T1, typename T2 > requires (bSTORE)
		TArchive& operator & (const std::pair<T1, T2>& p) {
			CHECK_ARCHIVE_STORABLE;
			return *this << (p);
		}

		//-----------------------------------------------------------------------------
		// gtl::internal::pair
		template < typename T1, typename T2 > requires (bSTORE)
		TArchive& operator << (const gtl::internal::pair<T1, T2>& p) {
			CHECK_ARCHIVE_STORABLE;
			return *this & p.first & p.second;
		}
		template < typename T1, typename T2 > requires (bLOAD)
		TArchive& operator >> (gtl::internal::pair<T1, T2>& p) {
			CHECK_ARCHIVE_LOADABLE;
			return *this & p.first & p.second;
		}
		template < typename T1, typename T2 >
		TArchive& operator & (gtl::internal::pair<T1, T2>& p) {
			return (*this) & p.first & p.second;
		}
		template < typename T1, typename T2 > requires (bSTORE)
		TArchive& operator & (const gtl::internal::pair<T1, T2>& p) {
			CHECK_ARCHIVE_STORABLE;
			return *this << (p);
		}

	public:
		//-----------------------------------------------------------------------------
		// String In / Out
		//
		template < gtlc::string_elem tchar > requires (bSTORE)
		TArchive& operator << (const std::basic_string<tchar>& str) {
			CHECK_ARCHIVE_STORABLE;

			auto& ar = *this;
			if (str.size() == 0) {
				ar << (std::uint8_t)0;
				return ar;
			}

			// Encoding
			if constexpr (std::is_same_v<tchar, char>) {
			} else if constexpr (std::is_same_v<tchar, wchar_t>) {
				// eCHAR_ENCODING::UNICODE (little / big)
				ar & (std::uint8_t)0xff;
				ar & (std::uint16_t)0xfffe;
			} else if constexpr (std::is_same_v<tchar, char8_t>) {
				ar & (std::uint8_t)0xff;
				ar & (std::uint16_t)0xfffd;
			} else {
				static_assert(false, "only MBCS, UNICODE(little endian), UTF-8 types are supported.");
				return ar;
			}

			// Size
			if (str.size() < 255) {
				// less than a byte
				ar & (std::uint8_t)str.size();
			} else if (str.size() < 0xfffd) {
				// less than ( a word - additional one for utf8 )
				ar & (std::uint8_t)0xff;
				ar & (std::uint16_t)str.size();
			} else if (str.size() < 0xffff'ffff) {
				// less than 32bit-unsigned
				ar & (std::uint8_t)0xff;
				ar & (std::uint16_t)0xffff;
				ar & (std::uint32_t)str.size();
			} else {
				// less than 64 bit-unsigned ...
				ar & (std::uint8_t)0xff;
				ar & (std::uint16_t)0xffff;
				ar & (std::uint32_t)0xffff'ffff;
				ar & (std::uint64_t)str.size();
			}

			// Text
			if (ar.IsSwapByteOrder()) {
				ar.WriteInts(str.data(), str.size());
			} else {
				std::streamsize nLen = str.size()*sizeof(tchar);
				ar.Write((const std::ostream::char_type*)str.data(), nLen);
			}
			return ar;
		}
		template < gtlc::string_elem tchar > requires (bLOAD, gtlc::is_one_of<tchar, )
		TArchive& operator >> (std::basic_string<tchar>& str) {
			CHECK_ARCHIVE_LOADABLE;

			auto& ar = *this;

			union {	// only on little-endian
				uint8_t size8;
				uint16_t size16;
				uint32_t size32;
				uint64_t size64 {0};
			} len;
			eCHAR_ENCODING eEncSource = eCHAR_ENCODING::NONE;

			// read length, encoding
			do {
				ar & len.size8;
				if (len.size8 < 0xff) {
					if (eEncSource == eCHAR_ENCODING::NONE)
						eEncSource = eCHAR_ENCODING::MBCS;
					break;
				}

				ar & len.size16;
				if (len.size16 < 0xfffd) {
					break;
				}

				if (eEncSource == eCHAR_ENCODING::NONE) {
					if (len.size16 == 0xfffe) {
						eEncSource = eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN;
					} else if (len.size16 == 0xfffd) {
						eEncSource = eCHAR_ENCODING::UTF8;
					}
					len.size16 = 0;
					continue;
				}

				ar & len.size32;
				if (len.size32 == 0xffff'ffff) {
					ar & len.size64;
					break;
				}
				break;

			} while (true);

			// read contents
			auto ReadAndConvertString = [&](auto ch) {
				std::basic_string<decltype(ch)> strSource;
				strSource.resize(len.size64);
				if (ar.IsSwapByteOrder()) {
					ar.ReadInts(strSource.data(), strSource.size());
				} else {
					ar.Read(strSource.data(), strSource.size()*sizeof(ch));
				}
				ConvCodepage_Move(str, std::move(strSource), ar.GetDefaultCodepage());
			};


			switch (eEncSource) {
			case eCHAR_ENCODING::MBCS :						ReadAndConvertString((char)0); break;
			case eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN :	ReadAndConvertString((wchar_t)0); break;
			case eCHAR_ENCODING::UTF8 :						ReadAndConvertString((char8_t)0); break;
				//case eCHAR_ENCODING::UNICODE_BIG_ENDIAN :		// to do : TEST. (NOT Tested yet)
				//	{
				//		std::basic_string<wchar_t> strSource;
				//		strSource.resize(len.size64);
				//		if constexpr (bSwapByteOrder) {
				//			ar.Read(strSource.data(), strSource.size()*sizeof(wchar_t));
				//		} else {
				//			ar.ReadInts(strSource.data(), strSource.size(), true);
				//		}
				//		ConvCodepage_Move(str, std::move(strSource), ar.GetDefaultCodepage());
				//	}
				//	break;
			}
			return *this;
		}

		template <
			typename tchar,
			ENABLE_IF(std::is_same_v<tchar, char> || std::is_same_v<tchar, wchar_t> || std::is_same_v<tchar, char8_t>)
		>
		TArchive& operator & (std::basic_string<tchar>& str) {
			if constexpr (bSTORE && bLOAD) {
				if (IsStore())
					return *this << str;
				else
					return *this >> str;
			} else if constexpr (bSTORE) {
				return *this << str;
			} else if constexpr (bLOAD) {
				return *this >> str;
			} else {
				//static_assert(false, "what???");
				return *this;
			}
		}
		template <
			typename tchar,
			ENABLE_IF(std::is_same_v<tchar, char> || std::is_same_v<tchar, wchar_t> || std::is_same_v<tchar, char8_t>)
		> requires (bSTORE)
		TArchive& operator & (const std::basic_string<tchar>& str) {
			CHECK_ARCHIVE_STORABLE;
			return *this << str;
		}

		//template < typename STREAM >
		//using Archive = TArchive<false, STREAM>;

	#undef ENABLE_IF_LOAD
	#undef CHECK_ARCHIVE_LOADABLE
	#undef CHECK_ARCHIVE_STORABLE

	};


	//-------------------------------------------------------------------------

	using XIArchive = TArchive<std::ifstream>;
	using XOArchive = TArchive<std::ofstream>;
	using XArchive = TArchive<std::fstream, true, true>;


#pragma pack(pop)
}	// namespace gtl;


#endif	 // temp
