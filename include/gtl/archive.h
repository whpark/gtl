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

#if 1

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
		bool bSTORE = std::is_base_of_v<std::basic_ostream<typename tstream::char_type>, tstream>,
		bool bLOAD = std::is_base_of_v<std::basic_istream<typename tstream::char_type>, tstream>//,
		//size_t PROCESSING_BUFFER_SIZE = 4096
	>
	class TArchive {

	#define CHECK_ARCHIVE_LOADABLE\
		if constexpr (bSTORE && bLOAD) { if (!IsLoading()) { throw std::ios_base::failure(GTL__FUNCSIG "NOT an archive for loading!"); } }
	#define CHECK_ARCHIVE_STORABLE\
		if constexpr (bSTORE && bLOAD) { if (!IsStoring()) { throw std::ios_base::failure(GTL__FUNCSIG "NOT an archive for storing!"); } }


	protected:
		tstream& stream_;
		eCODEPAGE eCodepage_{ eCODEPAGE::UTF8 };
	public:
		bool const bStore_;

	public:
		TArchive() = delete;
		TArchive(const TArchive&) = delete;
		TArchive(TArchive&&) = default;
		explicit TArchive(tstream& stream) requires ((bSTORE && !bLOAD) || (!bSTORE && bLOAD))
			: stream_{ stream }, bStore_{ bSTORE }
		{
		}

		TArchive(tstream& stream, bool bStore) requires (bSTORE && bLOAD)
			: stream_{ stream }, bStore_{ bStore }
		{
		}

	public:
		/// @brief Set/Get Codepage.
		eCODEPAGE SetCodepage(eCODEPAGE eCodepage) { return std::exchange(eCodepage_, eCodepage); }
		eCODEPAGE GetCodepage() const { return eCodepage_; }

		// 
		constexpr bool IsSwapByteOrder() const { return bSWAP_BYTE_ORDER; }	// 'Get'SwapByteOrder  라고 이름을 바꾸면 안됨..... 헷갈림

		/// @brief tells its loading or storing.
		/// @return 
		bool IsStoring() const {
			if constexpr (bSTORE && bLOAD) {
				return bStore_;
			}
			else {
				return bSTORE;
			}
		}
		bool IsLoading() const {
			if constexpr (bSTORE && bLOAD) {
				return !bStore_;
			}
			else {
				return bLOAD;
			}
		}

		using is_storing_only_t = std::bool_constant<bSTORE && !bLOAD>;
		using is_loading_only_t = std::bool_constant<!bSTORE && bLOAD>;
		using is_storing_t = std::bool_constant<bSTORE>;
		using is_loading_t = std::bool_constant<bLOAD>;


	public:
		/// @brief Write byte buffer
		inline void Write(void const* data, std::streamsize size) requires (bSTORE) {
			CHECK_ARCHIVE_STORABLE;
			stream_.write((std::ostream::char_type const*)data, size);
		}
		/// @brief Write an object.
		/// @param TYPE trivially_copyable objects. (when bSWAP_BYTE_ORDER is true, only integral or floating point value and their array  can be serialized.)
		template < gtlc::trivially_copyable TYPE > requires (bSTORE)
		inline void Write(TYPE const& v) {
			CHECK_ARCHIVE_STORABLE;
			if constexpr (bSWAP_BYTE_ORDER) {
				if constexpr (gtlc::is_array<TYPE>) {
					if constexpr (std::is_integral_v < std::remove_cvref_t<decltype(TYPE{}[0]) >> ) {
						this->WriteInts(std::data(v), std::size(v));
					}
					else if constexpr (std::is_floating_point_v < std::remove_cvref_t<decltype(TYPE{}[0]) >> ) {
						this->Write(std::data(v), sizeof(v));
					}
					else {
						static_assert(false, "Object Type cannot be Serialized directly.");
					}
				}
				else if constexpr (std::is_integral_v<TYPE>) {
					this->WriteInts(&v, 1);
				}
				else if constexpr (std::is_floating_point_v<TYPE>) {
					this->Write(&v, sizeof(v));
				}
				else {
					static_assert(false, "Object Type cannot be Serialized directly.");
				}
			} else {
				Write(&v, sizeof(v));
			}
			return *this;
		}


	public:
		
		/// @brief Read byte buffer
		template < typename T > requires (bLOAD)
		inline std::streamsize Read(void* data, std::streamsize size) {
			CHECK_ARCHIVE_LOADABLE;
			auto pos0 = stream_.tellg();
			if (stream_.read((std::istream::char_type*)data, size)) {
#ifdef _DEBUG
				auto pos1 = stream_.tellg();
				auto nRead = pos1 - pos0;
				return nRead;
#else
				return (m_stream.tellg() - pos0);
#endif
			}
			return -1;
		}
		/// @brief Write an object.
		/// @param TYPE trivially_copyable objects. (when bSWAP_BYTE_ORDER is true, only integral or floating point value and their array  can be serialized.)
		template < gtlc::trivially_copyable TYPE > requires (bLOAD)
		inline std::streamsize Read(TYPE& v) {
			CHECK_ARCHIVE_LOADABLE;
			if constexpr (bSWAP_BYTE_ORDER) {
				if constexpr (gtlc::is_array<TYPE>) {
					if constexpr (std::is_integral_v < std::remove_cvref_t<decltype(TYPE{}[0]) >> ) {
						this->ReadInts(std::data(v), std::size(v));
					}
					else if constexpr (std::is_floating_point_v < std::remove_cvref_t<decltype(TYPE{}[0]) >> ) {
						this->Read(std::data(v), sizeof(v));
					}
					else {
						static_assert(false, "Object Type cannot be Serialized directly.");
					}
				}
				else if constexpr (std::is_integral_v<TYPE>) {
					this->ReadInts(&v, 1);
				}
				else if constexpr (std::is_floating_point_v<TYPE>) {
					this->Read(&v, sizeof(v));
				}
				else {
					static_assert(false, "Object Type cannot be Serialized directly.");
				}
			} else {
				Read(&v, sizeof(v));
			}
			return *this;
		}


	protected:
		/// @brief Write Ints Swaps byte-order if (bSWAP_BYTE_ORDER)
		/// @param container 
		/// @param nCount 
		template < std::integral T_INT, int PROCESSING_BUFFER_SIZE = 4096 > requires (bSTORE)
		void WriteInts(T_INT const* data, std::streamsize nCount) {
			CHECK_ARCHIVE_STORABLE;
			if constexpr (bSWAP_BYTE_ORDER and (sizeof(T_INT) > 1)) {
				WriteIntsSwapByte<T_INT, PROCESSING_BUFFER_SIZE>(data, nCount);
			}
			else {
				Write(data, nCount*sizeof(T_INT));
			}
		}
		// .... 사용법 헷갈림. 삭제.
		//template < std::integral T_INT, gtlc::contiguous_container<T_INT> tcontainer > requires (bSTORE)
		//inline void WriteInts(tcontainer const& container) {
		//	WriteInts(std::data(container), std::size(container));
		//}

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
		// .... 사용법 헷갈림. 삭제.
		//template < std::integral T_INT, gtlc::contiguous_container<T_INT> tcontainer > requires (bLOAD)
		//inline std::streamsize ReadInts(tcontainer& container) {
		//	return ReadInts(std::data(container), std::size(container));
		//}


		/// @brief Write Ints Swaps byte-order if (bSWAP_BYTE_ORDER)
		/// @param container 
		/// @param nCount 
		template < std::integral T_INT, int PROCESSING_BUFFER_SIZE = 4096 > requires (bSTORE)
		void WriteIntsSwapByte(T_INT const* data, std::streamsize nCount) {
			CHECK_ARCHIVE_STORABLE;
			if constexpr (sizeof(T_INT) > 1) {
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


	public:

		//---------------------------------------------------------------------
		// operator >>, <<, &
		template < gtlc::trivially_copyable TYPE > requires (bSTORE)
		TArchive& operator << (TYPE const& v) {
			Write(v);
			return *this;
		};
		template < gtlc::trivially_copyable TYPE > requires (bLOAD)
		TArchive& operator >> (TYPE& v) {
			Read(v);
			return *this;
		};
		template < typename TYPE >
		TArchive& operator & (TYPE& v) {
			if constexpr (bSTORE && bLOAD) {
				if (IsStoring())
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
		template < typename TYPE > requires (bSTORE)
		TArchive& operator & (TYPE const& v) {
			if constexpr (bSTORE and bLOAD) {
				if (!IsStoring())
					throw std::ios_base::failure(GTL__FUNCSIG "Stream is NOT for storing.");
			}
			return *this << v;
		};


		//---------------------------------------------------------------------

		/// @brief Write BOM
		void WriteCodepageBOM(eCODEPAGE eCodepage) requires (bSTORE) {
			CHECK_ARCHIVE_STORABLE;
			if (eCodepage == eCODEPAGE::DEFAULT) {
				eCodepage = eCODEPAGE::UTF8;
			}
			std::string_view sv = GetCodepageBOM(eCodepage);
			if (sv.size() > 0)
				Write(sv.data(), sv.size());
			SetCodepage(eCodepage);
		};

		/// @brief Read BOM
		eCODEPAGE ReadCodepageBOM(eCODEPAGE eDefaultCodepage = eCODEPAGE::UTF8) requires (bLOAD) {
			CHECK_ARCHIVE_LOADABLE;
			unsigned char c = 0;

			auto peek = [](tstream& stream, std::string_view sv) -> bool {
				std::ifstream s;
				auto pos = stream.tellg();
				bool bMatch {true};
				for (auto c : sv) {
					char ci{};
					if (!stream.get(ci)) {
						if (stream.eof())
							stream.clear(std::ios_base::eofbit);
						bMatch = false;
						break;
					}
					if (ci != c) {
						bMatch = false;
						break;
					}
				}
				if (bMatch)
					return true;
				stream.seekg(pos);
				return false;
			};

			constexpr static std::array<std::pair<std::string_view, eCODEPAGE>, 5> const codepages{{
				{GetCodepageBOM(eCODEPAGE::UTF32BE), eCODEPAGE::UTF32BE},	// in BOM Length order ...
				{GetCodepageBOM(eCODEPAGE::UTF32LE), eCODEPAGE::UTF32LE},
				{GetCodepageBOM(eCODEPAGE::UTF16BE), eCODEPAGE::UTF16BE},
				{GetCodepageBOM(eCODEPAGE::UTF16LE), eCODEPAGE::UTF16LE},
				{GetCodepageBOM(eCODEPAGE::UTF8), eCODEPAGE::UTF8},
			}};

			for (auto const& [sv, codepage] : codepages) {
				if (peek(stream_, sv)) {
					SetCodepage(codepage);
					return codepage;
				}
			}

			SetCodepage(eDefaultCodepage);

			return eDefaultCodepage;
		};

	protected:
		template < typename tchar > requires (bLOAD)
		std::optional<std::basic_string<tchar>> GetLine(tchar cDelimiter, tchar cDelimiter2 = 0) {
			std::basic_string<tchar> str;

			static_assert(gtlc::is_one_of<tstream::char_type, char, char8_t>);

			if (!stream_)
				return {};

			for (tchar c{}; stream_.read((char*)&c, sizeof(c)); ) {
				if (c == cDelimiter)
					break;
				str += c;
			}

			if (str.empty() && stream_.eof())
				return {};

			if (cDelimiter2 and !str.empty() and *str.rbegin() == cDelimiter2)
				str.resize(str.size()-1);

			return str;
		}

		template < eCODEPAGE eCodepage, typename tchar > requires (bLOAD)
		inline std::optional<std::basic_string<tchar>> ReadLine(tchar cDelimiter = '\n', bool bTrimCR = true) {
			using tchar_codepage = typename char_type_from<eCodepage>::char_type;
			if constexpr (std::is_same_v<std::remove_cvref_t<tchar>, tchar_codepage>
				or (std::is_same_v<std::remove_cvref_t<tchar>, wchar_t> and (sizeof(tchar) == sizeof(tchar_codepage)))
				)
			{
				bool bSwapStreamByteOrder = (sizeof(tchar) >= 2) and (eCodepage == eCODEPAGE_OTHER_ENDIAN<tchar>);
				bool bSwapByteOrder = bSwapStreamByteOrder xor bSWAP_BYTE_ORDER;
				if (bSwapByteOrder) [[unlikely]] {
					if (auto r = GetLine<tchar>(GetByteSwap<tchar>(cDelimiter), bTrimCR ? GetByteSwap<tchar>('\r') : tchar{}); r) {
						for (auto& c : r.value())
							ByteSwap(c);
						return r;
					}
					else {
						return {};
					}
				}
				else [[likely]] {
					return GetLine<tchar>(cDelimiter, bTrimCR ? (tchar)'\r' : tchar{});
				}
			}
			else {
				if (auto r = ReadLine<eCodepage, tchar_codepage>((tchar_codepage)cDelimiter, bTrimCR); r) {
					S_CODEPAGE_OPTION option{
						.from = std::is_same_v<tchar_codepage, char> ? eCodepage_ : eCODEPAGE::DEFAULT,
						.to = std::is_same_v<tchar, char> ? eMBCS_Codepage_g : eCODEPAGE::DEFAULT
					};
					return ((TString<tchar_codepage>&)*r).ToString<tchar>(option);
				}
				else {
					return {};
				}
			}
		}

	public:
		/// @brief Read / Write String
		template < typename tchar > requires (bLOAD)
		std::optional<std::basic_string<tchar>> ReadLine(tchar cDelimiter = '\n', bool bTrimCR = true) {
			CHECK_ARCHIVE_LOADABLE;

			switch (eCodepage_) {

			default :
			case eCODEPAGE::DEFAULT :
				return ReadLine<eCODEPAGE::DEFAULT, tchar>(cDelimiter, bTrimCR);
				break;

			case eCODEPAGE::UTF8 :
				return ReadLine<eCODEPAGE::UTF8, tchar>(cDelimiter, bTrimCR);
				break;

			case eCODEPAGE::UTF16LE :
				return ReadLine<eCODEPAGE::UTF16LE, tchar>(cDelimiter, bTrimCR);
				break;
			case eCODEPAGE::UTF16BE :
				return ReadLine<eCODEPAGE::UTF16BE, tchar>(cDelimiter, bTrimCR);
				break;

			case eCODEPAGE::UTF32LE :
				return ReadLine<eCODEPAGE::UTF32LE, tchar>(cDelimiter, bTrimCR);
				break;
			case eCODEPAGE::UTF32BE :
				return ReadLine<eCODEPAGE::UTF32BE, tchar>(cDelimiter, bTrimCR);
				break;

			}
		};

		inline std::optional<std::string>		ReadLineA(char cDelimiter = '\n', bool bTrimCR = true) requires (bLOAD) { return ReadLine<char>(cDelimiter, bTrimCR); }
		inline std::optional<std::u8string>		ReadLineU8(char8_t cDelimiter = u8'\n', bool bTrimCR = true) requires (bLOAD) { return ReadLine<char8_t>(cDelimiter, bTrimCR); }
		inline std::optional<std::u16string>	ReadLineU16(char16_t cDelimiter = u'\n', bool bTrimCR = true) requires (bLOAD) { return ReadLine<char16_t>(cDelimiter, bTrimCR); }
		inline std::optional<std::u32string>	ReadLineU32(char32_t cDelimiter = U'\n', bool bTrimCR = true) requires (bLOAD) { return ReadLine<char32_t>(cDelimiter, bTrimCR); }
		inline std::optional<std::wstring>		ReadLineW(wchar_t cDelimiter = L'\n', bool bTrimCR = true) requires (bLOAD) { return ReadLine<wchar_t>(cDelimiter, bTrimCR); }

		// todo : 2020.12.15.

		///// @brief Read / Write String
		//template < typename tchar, bool bWriteNewLine = false > requires (bSTORE)
		//void TWriteString(std::basic_string_view<tchar> sv) {
		//	CHECK_ARCHIVE_STORABLE;

		//	auto write_string = [&] <typename tchar_other> (eCODEPAGE eCodepage, bool bForceSwapByteOrder) {
		//		std::streamsize nWritten {};
		//		if constexpr (std::is_same_v<std::remove_cvref_t<tchar>, std::remove_cvref_t<tchar_other>>) {
		//			WriteInts(sv.data(), sv.size(), bForceSwapByteOrder);
		//		} else {
		//			std::basic_string<decltype(ch)> strDest;
		//			ConvCodepage(strDest, sv, eCodepage);
		//			WriteInts(strDest.data(), strDest.size(), bForceSwapByteOrder);
		//		}
		//	};

		//	bool bResult = false;
		//	auto eCodepage = GetCodepage();
		//	switch (GetCodepage()) {
		//	case eCHAR_ENCODING::AUTO :
		//	case eCHAR_ENCODING::MBCS :
		//		write_string<char>(eCodepage, false);
		//		break;
		//	case eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN :
		//		write_string<char16_t>(eCodepage, std::endian::native != std::endian::little);
		//		break;
		//	case eCHAR_ENCODING::UNICODE_BIG_ENDIAN :
		//		write_string<char16_t>(eCodepage, std::endian::native != std::endian::big);
		//		break;
		//	case eCHAR_ENCODING::UTF8 :
		//		write_string<char8_t>(eCodepage, false);
		//		break;
		//	}

		//	if constexpr (bWriteNewLine) {
		//		WriteNewLine();
		//	}
		//};
		//void WriteNewLine() requires (bSTORE) {
		//	CHECK_ARCHIVE_STORABLE;

		//	auto eCodepage = GetCodepage();
		//	switch (GetCodepage()) {
		//	case eCHAR_ENCODING::AUTO :
		//	case eCHAR_ENCODING::MBCS :
		//		WriteInts<char>("\r\n", 2, false);
		//		break;
		//	case eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN :
		//		WriteInts<wchar_t>(L"\r\n", 2, std::endian::native != std::endian::little);
		//		break;
		//	case eCHAR_ENCODING::UNICODE_BIG_ENDIAN :
		//		WriteInts<wchar_t>(L"\r\n", 2, std::endian::native != std::endian::big);
		//		break;
		//	case eCHAR_ENCODING::UTF8 :
		//		WriteInts<char8_t>(_U8("\r\n"), 2, false);
		//		break;
		//	}
		//};

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
			stream_.flush();
		}
		void Close() {
		}

		//-----------------------------------------------------------------------------
		// Generic Archive In / Out
		//
		//template < typename T > TArchive& operator << (TArchive& ar, const std::vector<T>& container);
		//template < typename T > TArchive& operator >> (TArchive& ar, std::vector<T>& container);
		//template < typename T > TArchive& operator & (TArchive& ar, std::vector<T>& container);

		////-------------------------
		//// Container ... std::vector, std::deque
		//template < typename T, gtlc::container<T> tcontainer > requires (bSTORE)
		//TArchive& StoreContainer(tcontainer const& container) {
		//	CHECK_ARCHIVE_STORABLE;
		//	auto count = std::size(container);
		//	StoreSize(count);
		//	if (count) {
		//		constexpr bool bSwapMightBeNeeded = (bSWAP_BYTE_ORDER and (sizeof(T) > 1) and (std::is_integral_v<T> or std::is_class_v<T>));
		//		if constexpr (std::is_trivially_copyable_v<T> and !bSwapMightBeNeeded) {
		//			Write((uint8_t const*)std::data(container), std::size(container)*sizeof(T));
		//		} else {
		//			for (auto const& item : container) {
		//				(*this) << item;
		//			}
		//		}
		//	}
		//	return *this;
		//};
		//template < typename T, gtlc::container<T> tcontainer > requires (bLOAD)
		//TArchive& LoadContainer(tcontainer& container) {
		//	CHECK_ARCHIVE_LOADABLE;
		//	container.clear();
		//	size_t size = LoadSize();
		//	if (size) {
		//		constexpr bool bSwapMightBeNeeded = (bSWAP_BYTE_ORDER and (sizeof(T) > 1) and (std::is_integral_v<T> or std::is_class_v<T>));
		//		if constexpr (requires (tcontainer container) { container.resize(size_t{}); }) {
		//			container.resize(size);
		//		}
		//		if constexpr (std::is_trivially_copyable_v<T> and !bSwapMightBeNeeded) {
		//			Read((uint8_t const*)std::data(container), std::size(container)*sizeof(T));
		//		} else {
		//			for (auto const& item : container) {
		//				(*this) << item;
		//			}
		//		}
		//		if constexpr (std::is_trivially_copyable_v<T>) {
		//			container.resize(size);
		//			if constexpr (bSWAP_BYTE_ORDER and std::is_integral_v<T> and (sizeof(T) > 1)) {
		//				if (ReadInts(container.data(), container.size()) != container.size())
		//					throw std::ios_base::failure(GTL__FUNCSIG "CANNOT Read Data");
		//			} else {
		//				auto len = container.size()*sizeof(T);
		//				if (Read(container.data(), container.size()*sizeof(T)) != len)
		//					throw std::ios_base::failure(GTL__FUNCSIG "CANNOT Read Data");
		//			}
		//		} else {
		//			if constexpr (requires (tcontainer container) { container.reserve(size); }) {
		//				container.reserve(size);
		//			}
		//			for (size_t i = 0; i < size; i++) {
		//				if constexpr (requires (tcontainer container, T a) { container.push_back(a); }) {
		//					container.push_back({});
		//					*this >> container.back();
		//				} else if constexpr (requires (tcontainer container, T a) { container.insert(a); }) {
		//					T a;
		//					*this >> a;
		//					container.insert(std::move(a));
		//				} else {
		//					static_assert(false, "CONTAINER must have push_back or insert ...");
		//				}
		//			}
		//		}
		//	}
		//	return *this;
		//};

		//template < typename T, gtlc::container<T> tcontainer > requires (bSTORE)
		//TArchive& operator << (tcontainer const& data) {
		//	return StoreContainer(data);
		//};
		//template < typename T, gtlc::container<T> tcontainer > requires (bLOAD)
		//TArchive& operator >> (tcontainer& data) {
		//	return LoadContainer(data);
		//};
		//template < typename T, gtlc::container<T> tcontainer >
		//TArchive& operator & (CONTAINER& data) {
		//	if constexpr (bSTORE && bLOAD) {
		//		if (IsStoring())
		//			return StoreContainer(data);
		//		else
		//			return LoadContainer(data);
		//	} else if constexpr (bSTORE) {
		//		return StoreContainer(data);
		//	} else if constexpr (bLOAD) {
		//		return LoadContainer(data);
		//	} else {
		//		//static_assert(false, "what???");
		//		return *this;
		//	}
		//};
		//template < gtl::is__container CONTAINER > requires (bSTORE)
		//TArchive& operator & (const CONTAINER& data) {
		//	if constexpr (bSTORE && bLOAD) {
		//		if (!IsStoring()) {
		//			throw std::ios_base::failure(GTL__FUNCSIG "NOT an storing archive!");
		//			return *this;
		//		}
		//	}
		//	return StoreContainer(data);
		//};

		////-----------------------------------------------------------------------------
		//// std::unique_ptr
		//template < typename T > requires (bSTORE)
		//TArchive& operator << (const std::unique_ptr<T>& ptr) {
		//	CHECK_ARCHIVE_STORABLE;
		//	if constexpr (gtlc::is_dynamic_serializable<T>) {
		//		ptr->DynamicSerializeOut(*this);
		//	} else {
		//		(*this) & *ptr;
		//	}
		//	return *this;
		//}
		//template < typename T > requires (bLOAD)
		//TArchive& operator >> (std::unique_ptr<T>& ptr) {
		//	CHECK_ARCHIVE_LOADABLE;
		//	if constexpr (gtlc::is_dynamic_serializable<T>) {
		//		ptr = T::DynamicSerializeIn(*this);
		//	} else {
		//		if (!ptr) {
		//			if constexpr (has__NewObject<T>) {
		//				ptr = T::NewObject();
		//			} else {
		//				ptr.reset(new T);
		//			}
		//		}
		//		(*this) & *ptr;
		//	}
		//	return *this;
		//}
		//template < typename T >
		//TArchive& operator & (std::unique_ptr<T>& ptr) {
		//	if constexpr (bSTORE && bLOAD) {
		//		if (IsStoring())
		//			return *this << ptr;
		//		else
		//			return *this >> ptr;
		//	} else if constexpr (bSTORE) {
		//		return *this << ptr;
		//	} else if constexpr (bLOAD) {
		//		return *this >> ptr;
		//	} else {
		//		//static_assert(false, "what???");
		//		return *this;
		//	}
		//}
		//template < typename T > requires (bSTORE)
		//TArchive& operator & (const std::unique_ptr<T>& ptr) {
		//	CHECK_ARCHIVE_STORABLE;
		//	return *this << ptr;
		//}


		//-----------------------------------------------------------------------------
		// std::pair
		template < typename T1, typename T2 > requires (bSTORE)
		TArchive& operator << (const std::pair<T1, T2>& p) {
			CHECK_ARCHIVE_STORABLE;
			return *this << p.first << p.second;
		}
		template < typename T1, typename T2 > requires (bLOAD)
		TArchive& operator >> (std::pair<T1, T2>& p) {
			CHECK_ARCHIVE_LOADABLE;
			return *this >> p.first >> p.second;
		}
		template < typename T1, typename T2 >
		TArchive& operator & (std::pair<T1, T2>& p) {
			return (*this) & p.first & p.second;
		}
		template < typename T1, typename T2 > requires (bSTORE)
		TArchive& operator & (std::pair<T1, T2> const& p) {
			CHECK_ARCHIVE_STORABLE;
			return *this << (p);
		}

		////-----------------------------------------------------------------------------
		//// gtl::internal::pair
		//template < typename T1, typename T2 > requires (bSTORE)
		//TArchive& operator << (gtl::internal::pair<T1, T2> const& p) {
		//	CHECK_ARCHIVE_STORABLE;
		//	return *this << p.first << p.second;
		//}
		//template < typename T1, typename T2 > requires (bLOAD)
		//TArchive& operator >> (gtl::internal::pair<T1, T2>& p) {
		//	CHECK_ARCHIVE_LOADABLE;
		//	return *this >> p.first >> p.second;
		//}
		//template < typename T1, typename T2 >
		//TArchive& operator & (gtl::internal::pair<T1, T2>& p) {
		//	return (*this) & p.first & p.second;
		//}
		//template < typename T1, typename T2 > requires (bSTORE)
		//TArchive& operator & (gtl::internal::pair<T1, T2> const& p) {
		//	CHECK_ARCHIVE_STORABLE;
		//	return *this << (p);
		//}

	public:
		////-----------------------------------------------------------------------------
		//// String In / Out
		////
		//template < gtlc::string_elem tchar > requires (bSTORE)
		//TArchive& operator << (const std::basic_string<tchar>& str) {
		//	CHECK_ARCHIVE_STORABLE;

		//	auto& ar = *this;
		//	if (str.size() == 0) {
		//		ar << (std::uint8_t)0;
		//		return ar;
		//	}

		//	// Encoding
		//	if constexpr (std::is_same_v<tchar, char>) {
		//	} else if constexpr (std::is_same_v<tchar, wchar_t>) {
		//		// eCHAR_ENCODING::UNICODE (little / big)
		//		ar & (std::uint8_t)0xff;
		//		ar & (std::uint16_t)0xfffe;
		//	} else if constexpr (std::is_same_v<tchar, char8_t>) {
		//		ar & (std::uint8_t)0xff;
		//		ar & (std::uint16_t)0xfffd;
		//	} else {
		//		static_assert(false, "only MBCS, UNICODE(little endian), UTF-8 types are supported.");
		//		return ar;
		//	}

		//	// Size
		//	if (str.size() < 255) {
		//		// less than a byte
		//		ar & (std::uint8_t)str.size();
		//	} else if (str.size() < 0xfffd) {
		//		// less than ( a word - additional one for utf8 )
		//		ar & (std::uint8_t)0xff;
		//		ar & (std::uint16_t)str.size();
		//	} else if (str.size() < 0xffff'ffff) {
		//		// less than 32bit-unsigned
		//		ar & (std::uint8_t)0xff;
		//		ar & (std::uint16_t)0xffff;
		//		ar & (std::uint32_t)str.size();
		//	} else {
		//		// less than 64 bit-unsigned ...
		//		ar & (std::uint8_t)0xff;
		//		ar & (std::uint16_t)0xffff;
		//		ar & (std::uint32_t)0xffff'ffff;
		//		ar & (std::uint64_t)str.size();
		//	}

		//	// Text
		//	if (ar.IsSwapByteOrder()) {
		//		ar.WriteInts(str.data(), str.size());
		//	} else {
		//		std::streamsize nLen = str.size()*sizeof(tchar);
		//		ar.Write((const std::ostream::char_type*)str.data(), nLen);
		//	}
		//	return ar;
		//}
		//template < gtlc::string_elem tchar > requires (bLOAD, gtlc::is_one_of<tchar, )
		//TArchive& operator >> (std::basic_string<tchar>& str) {
		//	CHECK_ARCHIVE_LOADABLE;

		//	auto& ar = *this;

		//	union {	// only on little-endian
		//		uint8_t size8;
		//		uint16_t size16;
		//		uint32_t size32;
		//		uint64_t size64 {0};
		//	} len;
		//	eCHAR_ENCODING eEncSource = eCHAR_ENCODING::NONE;

		//	// read length, encoding
		//	do {
		//		ar & len.size8;
		//		if (len.size8 < 0xff) {
		//			if (eEncSource == eCHAR_ENCODING::NONE)
		//				eEncSource = eCHAR_ENCODING::MBCS;
		//			break;
		//		}

		//		ar & len.size16;
		//		if (len.size16 < 0xfffd) {
		//			break;
		//		}

		//		if (eEncSource == eCHAR_ENCODING::NONE) {
		//			if (len.size16 == 0xfffe) {
		//				eEncSource = eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN;
		//			} else if (len.size16 == 0xfffd) {
		//				eEncSource = eCHAR_ENCODING::UTF8;
		//			}
		//			len.size16 = 0;
		//			continue;
		//		}

		//		ar & len.size32;
		//		if (len.size32 == 0xffff'ffff) {
		//			ar & len.size64;
		//			break;
		//		}
		//		break;

		//	} while (true);

		//	// read contents
		//	auto ReadAndConvertString = [&](auto ch) {
		//		std::basic_string<decltype(ch)> strSource;
		//		strSource.resize(len.size64);
		//		if (ar.IsSwapByteOrder()) {
		//			ar.ReadInts(strSource.data(), strSource.size());
		//		} else {
		//			ar.Read(strSource.data(), strSource.size()*sizeof(ch));
		//		}
		//		ConvCodepage_Move(str, std::move(strSource), ar.GetCodepage());
		//	};


		//	switch (eEncSource) {
		//	case eCHAR_ENCODING::MBCS :						ReadAndConvertString((char)0); break;
		//	case eCHAR_ENCODING::UNICODE_LITTLE_ENDIAN :	ReadAndConvertString((wchar_t)0); break;
		//	case eCHAR_ENCODING::UTF8 :						ReadAndConvertString((char8_t)0); break;
		//		//case eCHAR_ENCODING::UNICODE_BIG_ENDIAN :		// to do : TEST. (NOT Tested yet)
		//		//	{
		//		//		std::basic_string<wchar_t> strSource;
		//		//		strSource.resize(len.size64);
		//		//		if constexpr (bSwapByteOrder) {
		//		//			ar.Read(strSource.data(), strSource.size()*sizeof(wchar_t));
		//		//		} else {
		//		//			ar.ReadInts(strSource.data(), strSource.size(), true);
		//		//		}
		//		//		ConvCodepage_Move(str, std::move(strSource), ar.GetCodepage());
		//		//	}
		//		//	break;
		//	}
		//	return *this;
		//}

		//template <
		//	typename tchar,
		//	ENABLE_IF(std::is_same_v<tchar, char> || std::is_same_v<tchar, wchar_t> || std::is_same_v<tchar, char8_t>)
		//>
		//TArchive& operator & (std::basic_string<tchar>& str) {
		//	if constexpr (bSTORE && bLOAD) {
		//		if (IsStoring())
		//			return *this << str;
		//		else
		//			return *this >> str;
		//	} else if constexpr (bSTORE) {
		//		return *this << str;
		//	} else if constexpr (bLOAD) {
		//		return *this >> str;
		//	} else {
		//		//static_assert(false, "what???");
		//		return *this;
		//	}
		//}
		//template <
		//	typename tchar,
		//	ENABLE_IF(std::is_same_v<tchar, char> || std::is_same_v<tchar, wchar_t> || std::is_same_v<tchar, char8_t>)
		//> requires (bSTORE)
		//TArchive& operator & (const std::basic_string<tchar>& str) {
		//	CHECK_ARCHIVE_STORABLE;
		//	return *this << str;
		//}

		//template < typename STREAM >
		//using Archive = TArchive<false, STREAM>;

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
