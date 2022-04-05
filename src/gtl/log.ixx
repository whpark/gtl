//////////////////////////////////////////////////////////////////////
//
// log.h: .ini 파일. simple log
//
// PWH
// 2019.01.09.
// 2019.07.24. QL -> GTL
//
// 2021.05.18. spdlog 쓰는게 더 좋을지도.
//
//////////////////////////////////////////////////////////////////////

module;

#include <memory>
#include <fstream>
#include <mutex>
#include <string>
#include <filesystem>

#pragma warning(push)
#pragma warning(disable: 4819)
#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/xchar.h"
#include "fmt/chrono.h"
#pragma warning(pop)

#include "gtl/_config.h"
#include "gtl/_macro.h"

export module gtl:log;

import :concepts;
import :string;
import :time;
import :mutex;
import :archive;

export namespace gtl {

	//-----------------------------------------------------------------------------
	// Simple Log
	//
	class xSimpleLog {
	public:
		using archive_out_t = gtl::TArchive<std::ofstream>;

	protected:
		mutable non_copyable_member<std::recursive_mutex> m_mutex;	// for multi-thread
		std::filesystem::path m_path;								// Current File Path
		std::ofstream m_file;										// Current File
		std::unique_ptr<archive_out_t> m_ar;						// Archive
		std::string m_strTagFilter;										// Tag Filter. 'How to use' is up to user.
																	// (if and only if m_strTagFilter and strTag is not empty), if strTag is not found on strTagFilter, no Log will be written.

	public:
		xStringW m_strName;											// Task Name
		xStringW m_fmtLogFileName{L"[Name]_{0:%m}_{0:%d}.log"};		// Format of Log File (fmt::format)
		std::filesystem::path m_folderLog;							// Folder
		eCODEPAGE m_eCharEncoding = eCODEPAGE::UTF8;				// Default Char Encoding
		std::chrono::milliseconds m_tsOld{std::chrono::hours(24)};	// 오래된 파일 삭제시 기준 시간
		bool m_bOverwriteOlderFile = true;							// 오래된 파일은 덮어 씀.
		bool m_bCloseFileAfterWrite = false;						// 로그 하나씩 쓸 때마다 파일을 열었다 닫음. 성능에 문제가 되므로 사용하면 안됨. 어쩔 수 없을때에만 사용.

	//#ifdef _DEBUG
		bool m_bTraceOut = true;
	//#endif

		std::function<xStringA  (xSimpleLog&, archive_out_t&, xSysTime, const std::string_view svTag, const std::string_view svContent)> m_funcFormatterA;			// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringW  (xSimpleLog&, archive_out_t&, xSysTime, const std::wstring_view svTag, const std::wstring_view svContent)> m_funcFormatterW;		// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringU8 (xSimpleLog&, archive_out_t&, xSysTime, const std::u8string_view svTag, const std::u8string_view svContent)> m_funcFormatterU8;		// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringU16 (xSimpleLog&, archive_out_t&, xSysTime, const std::u16string_view svTag, const std::u16string_view svContent)> m_funcFormatterU16;	// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringU32 (xSimpleLog&, archive_out_t&, xSysTime, const std::u32string_view svTag, const std::u32string_view svContent)> m_funcFormatterU32;	// 로그 파일 포맷을 바꾸고 싶을 때...

	public:
		xSimpleLog() = default;
		xSimpleLog(std::string_view pszName) : m_strName(pszName) {}
		xSimpleLog(std::wstring_view pszName) : m_strName(pszName) {}
		xSimpleLog(const xSimpleLog&) = default;
		xSimpleLog(xSimpleLog&&) = default;
		xSimpleLog& operator = (const xSimpleLog&) = default;
		xSimpleLog& operator = (xSimpleLog&&) = default;

	public:
		bool OpenFile(std::chrono::system_clock::time_point now);
		bool CloseFile();
		void Clear() {
			std::scoped_lock lock(m_mutex);

			CloseFile();

			m_strName.clear();
			m_fmtLogFileName = L"[Name]_{0:%m}_{0:%d}.log";
			m_folderLog = std::filesystem::current_path();
			m_eCharEncoding = eCODEPAGE::UTF8;
			m_tsOld = std::chrono::hours(24);
			m_bOverwriteOlderFile = true;
			m_bCloseFileAfterWrite = false;
		}
		void Flush() {
			std::scoped_lock lock(m_mutex);
			if (m_ar)
				m_ar->Flush();
		}

		void SetTagFilter(std::string_view sv) {
			std::scoped_lock lock(m_mutex);
			m_strTagFilter = sv;
		}
		std::string_view GetTagFilter() const {
			return m_strTagFilter;
		}


	public:
		template < typename tchar_t >
		void _Log(const std::string_view svTag, const std::basic_string_view<tchar_t> sv);

	public:
		// Write Log
		template < typename ... Args > void Log(std::string_view svText, Args&& ... args) { _Log<char>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::wstring_view svText, Args&& ... args) { _Log<wchar_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u8string_view svText, Args&& ... args) { _Log<char8_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u16string_view svText, Args&& ... args) { _Log<char16_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u32string_view svText, Args&& ... args) { _Log<char32_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::string_view svText, Args&& ... args) { _Log<char>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::wstring_view svText, Args&& ... args) { _Log<wchar_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u8string_view svText, Args&& ... args) { _Log<char8_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u16string_view svText, Args&& ... args) { _Log<char16_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u32string_view svText, Args&& ... args) { _Log<char32_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }

	};


	//-----------------------------------------------------------------------------
	// Simple Log Interface
	//
	template < class CLogWriter >
	class ILog {
	protected:
		mutable CLogWriter* m_pLog {};
	public:
		ILog(CLogWriter* pLog = nullptr) : m_pLog(pLog) {}

		void SetLog(CLogWriter* pLog) const { m_pLog = pLog; }
		CLogWriter* GetLog() const { return m_pLog; }

	public:
		template < typename ... Args > void Log(std::string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::wstring_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<wchar_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u8string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char8_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u16string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char16_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u32string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char32_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::wstring_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<wchar_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u8string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char8_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u16string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char16_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u32string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char32_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
	};


	bool xSimpleLog::OpenFile(std::chrono::system_clock::time_point now) {
		std::scoped_lock lock(m_mutex);

		if (m_fmtLogFileName.empty()) {
			CloseFile();
			return false;
		}

		xStringW strFilePath;

		strFilePath = (m_folderLog / m_fmtLogFileName.c_str()).c_str();
		strFilePath.Replace(L"[Name]", m_strName);

		xSysTime tNow(now);
		std::time_t t = tNow;
		std::tm tm;
		localtime_s(&tm, &t);
		strFilePath.Replace(L"%10M", fmt::format(L"{:02d}", tm.tm_min/10*10));	// 분 단위 1의 자리에서 버림 -> 10분 단위로 파일 이름 생성
		strFilePath.Replace(L"%10m", fmt::format(L"{:02d}", tm.tm_min/10*10));
		std::filesystem::path path;
		//std::vector<wchar_t> buf(std::max((std::size_t)4096, strFilePath.size()), 0);
		//auto l = std::wcsftime(buf.data(), buf.size(), strFilePath, &tm);
		//if (l > 0)
		//	path.assign(buf.data(), buf.data()+l);
		//else
		//	path = (std::wstring&)strFilePath;	// 일단 그냥 설정....
		path = tNow.Format(strFilePath);

		// Opens a file
		if ( !m_ar || !m_file.is_open() || (strFilePath.CompareNoCase(path) != 0) ) {
			CloseFile();

			// Create Directory
			std::filesystem::create_directories(path.parent_path());

			// Delete Old Files
			if (m_bOverwriteOlderFile && std::filesystem::exists(path)) {
				xSysTime tLastWrite = std::filesystem::last_write_time(path);
				xSysTime t(now);
				auto ts = t - tLastWrite;
				if (ts > m_tsOld)
					std::filesystem::remove(path);
			}

			// Open log File
			bool bWriteBOM = false;
			auto eCharEncoding = m_eCharEncoding;
			if (IsValueOneOf(m_eCharEncoding, eCODEPAGE::DEFAULT__OR_USE_MBCS_CODEPAGE))
				m_eCharEncoding = eCODEPAGE_DEFAULT<wchar_t>;
			{
				std::ifstream f(path, std::ios::_Nocreate|std::ios::binary, SH_DENYNO);
				if (f.is_open()) {
					gtl::TArchive ar(f);
					eCharEncoding = ar.ReadCodepageBOM();
				} else {
					bWriteBOM = true;
				}
			}
			m_file.open(path, std::ios_base::app|std::ios::out|std::ios::binary, SH_DENYWR);
			if (m_file.is_open()) {
				m_ar = std::make_unique<archive_out_t>(m_file);
				m_path = path;
				if (bWriteBOM || (m_file.tellp() == 0))
					m_ar->WriteCodepageBOM(eCharEncoding);
				else
					m_ar->SetCodepage(eCharEncoding);

				// Seek to the end.
				m_file.seekp(0, m_file.end);

			} else {
				throw std::runtime_error(fmt::format("{}Cannot make LOG File {}", GTL__FUNCSIG, path.string()));
			}

		}
		return true;
	}
	bool xSimpleLog::CloseFile() {
		std::scoped_lock lock(m_mutex);

		if (m_ar.get()) {
			m_ar->Flush();
			m_ar->Close();
			m_ar.reset();
		}
		if (m_file.is_open())
			m_file.close();
		m_path.clear();
		return true;
	}

	// Write Log
	template < typename tchar_t >
	void xSimpleLog::_Log(const std::string_view svTag, const std::basic_string_view<tchar_t> svText) {
		auto now = std::chrono::system_clock::now();

	#if defined(_DEBUG) and defined(_WINDOWS)
		if (m_bTraceOut) {
			if constexpr(sizeof(tchar_t) == sizeof(char)) {
				OutputDebugStringA((const char*)svText.data());
				OutputDebugStringA("\r\n");
			} else if (sizeof(tchar_t) == sizeof(wchar_t)) {
				OutputDebugStringW((wchar_t const*)svText.data());
				OutputDebugStringW(L"\r\n");
			}
		}
	#endif

		if (!OpenFile(now) || !m_ar)
			return;
		if (!m_strTagFilter.empty() && !svTag.empty()) {
			if (!m_strTagFilter.contains(svTag))
				return;
		}

		do {
			if constexpr (std::is_same_v<tchar_t, char>) {
				if (m_funcFormatterA) {
					m_funcFormatterA(*this, *m_ar, now, svTag, svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
				if (m_funcFormatterW) {
					m_funcFormatterW(*this, *m_ar, now, xStringW(svTag), svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
				if (m_funcFormatterU8) {
					m_funcFormatterU8(*this, *m_ar, now, xStringU8(svTag), svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
				if (m_funcFormatterU16) {
					m_funcFormatterU16(*this, *m_ar, now, xStringU16(svTag), svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
				if (m_funcFormatterU32) {
					m_funcFormatterU32(*this, *m_ar, now, xStringU32(svTag), svText);
					break;
				}
			} else {
				static_assert(gtlc::dependent_false_v);
			}

			time_t t = std::chrono::system_clock::to_time_t(now);
			tm st{};
		#ifdef _MSC_VER
			localtime_s(&st, &t);
		#else
			st = *std::localtime(&t);
		#endif

			auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(now - std::chrono::system_clock::from_time_t(t));
			
#define GTL__FMT_EXPAND "{:04}/{:02}/{:02}, {:02}:{:02}:{:02}.{:03} {} : "
			if constexpr (std::is_same_v<tchar_t, char>) {
				m_ar->WriteString(GTL__FMT_EXPAND, st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), svTag);
			} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
				m_ar->WriteString(TEXT_W(GTL__FMT_EXPAND), st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), xStringW(svTag));
			} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
				m_ar->WriteString(TEXT_u8(GTL__FMT_EXPAND), st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), xStringU8(svTag));
			} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
				m_ar->WriteString(TEXT_u(GTL__FMT_EXPAND), st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), xStringU16(svTag));
			} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
				m_ar->WriteString(TEXT_U(GTL__FMT_EXPAND), st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), xStringU32(svTag));
			} else {
				static_assert(gtlc::dependent_false_v);
			}
#undef GTL__FMT_EXPAND
			const tchar_t* posHead = svText.data();
			const tchar_t* posEnd = svText.data() + svText.size();
			const tchar_t* posNext{};
			for (const tchar_t* pos = posHead; pos && (pos < posEnd); pos = posNext+1) {
				if (pos != posHead) {
					pos++;
					if (m_ar->GetCodepage() == eCODEPAGE::UCS2)
						m_ar->WriteString(L"\t");
					else
						m_ar->WriteString("\t");
				}
				auto svPos = std::basic_string_view<tchar_t>(pos, posEnd);
				auto ipos = svPos.find('\n');
				posNext = (ipos == svPos.npos) ? posEnd : pos+ipos;
				size_t len = posNext-pos;

				if constexpr (std::is_same_v<tchar_t, char>) {
					m_ar->WriteString("{1:{0}}\r\n", len, pos);
				} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
					m_ar->WriteString(L"{1:{0}\r\n", len, pos);
				} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
					m_ar->WriteString(u8"{1:{0}\r\n", len, pos);
				} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
					m_ar->WriteString(u"{1:{0}\r\n", len, pos);
				} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
					m_ar->WriteString(U"{1:{0}\r\n", len, pos);
				}
			}


		} while (false);


		if (m_bCloseFileAfterWrite)
			CloseFile();
	}

	//template void xSimpleLog::_Log<char>(const std::basic_string_view<char> svMask, const std::basic_string_view<char> svText);
	//template void xSimpleLog::_Log<wchar_t>(const std::basic_string_view<wchar_t> svMask, const std::basic_string_view<wchar_t> svText);
	//template void xSimpleLog::_Log<char8_t>(const std::basic_string_view<char8_t> svMask, const std::basic_string_view<char8_t> svText);
	//template void xSimpleLog::_Log<char16_t>(const std::basic_string_view<char16_t> svMask, const std::basic_string_view<char16_t> svText);
	//template void xSimpleLog::_Log<char32_t>(const std::basic_string_view<char32_t> svMask, const std::basic_string_view<char32_t> svText);

}
