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

#pragma once


	
#if 1

#include "gtl/_default.h"

//#include "gtl/_pre_lib_util.h"
#include "gtl/time.h"
#include "gtl/archive.h"


namespace gtl {
#pragma pack(push, 8)


	//-----------------------------------------------------------------------------
	// Simple Log
	//
	class GTL_CLASS CSimpleLog {
	public:
		using archive_out_t = gtl::TArchive<std::ofstream>;

	protected:
		mutable non_copyable_member<std::recursive_mutex> m_mutex;	// for multi-thread
		std::filesystem::path m_path;								// Current File Path
		std::ofstream m_file;										// Current File
		std::unique_ptr<archive_out_t> m_ar;						// Archive
		CString m_strTagFilter;										// Tag Filter. 'How to use' is up to user.
																	// (if and only if m_strTagFilter and strTag is not empty), if strTag is not found on strTagFilter, no Log will be written.

	public:
		CStringW m_strName;											// Task Name
		CStringW m_fmtLogFileName{L"[Name]_{0:%m}_{0:%d}.log"};		// Format of Log File (fmt::format)
		std::filesystem::path m_folderLog;							// Folder
		eCODEPAGE m_eCharEncoding = eCODEPAGE::UTF8;				// Default Char Encoding
		std::chrono::milliseconds m_tsOld{std::chrono::hours(24)};	// 오래된 파일 삭제시 기준 시간
		bool m_bOverwriteOlderFile = true;							// 오래된 파일은 덮어 씀.
		bool m_bCloseFileAfterWrite = false;						// 로그 하나씩 쓸 때마다 파일을 열었다 닫음. 성능에 문제가 되므로 사용하면 안됨. 어쩔 수 없을때에만 사용.

	#ifdef _DEBUG
		bool m_bTraceOut = true;
	#endif

		std::function<int(int, int, int)> m_fun;

		std::function<CStringA  (CSimpleLog&, archive_out_t&, CSysTime, const std::string_view svTag, const std::string_view svContent)> m_funcFormatterA;			// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<CStringW  (CSimpleLog&, archive_out_t&, CSysTime, const std::wstring_view svTag, const std::wstring_view svContent)> m_funcFormatterW;		// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<CStringU8 (CSimpleLog&, archive_out_t&, CSysTime, const std::u8string_view svTag, const std::u8string_view svContent)> m_funcFormatterU8;		// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<CStringU16 (CSimpleLog&, archive_out_t&, CSysTime, const std::u16string_view svTag, const std::u16string_view svContent)> m_funcFormatterU16;	// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<CStringU32 (CSimpleLog&, archive_out_t&, CSysTime, const std::u32string_view svTag, const std::u32string_view svContent)> m_funcFormatterU32;	// 로그 파일 포맷을 바꾸고 싶을 때...

	public:
		CSimpleLog() = default;
		CSimpleLog(std::string_view pszName) : m_strName(pszName) {}
		CSimpleLog(std::wstring_view pszName) : m_strName(pszName) {}
		CSimpleLog(const CSimpleLog&) = default;
		CSimpleLog(CSimpleLog&&) = default;
		CSimpleLog& operator = (const CSimpleLog&) = default;
		CSimpleLog& operator = (CSimpleLog&&) = default;

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
		std::wstring_view GetTagFilter() const {
			return m_strTagFilter;
		}


	public:
		template < typename tchar_t >
		void _Log(const std::basic_string_view<tchar_t> svTag, const std::basic_string_view<tchar_t> sv);

	public:
		// Write Log
		template < typename ... Args > void Log(std::string_view svText, Args&& ... args) { _Log<char>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::wstring_view svText, Args&& ... args) { _Log<wchar_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::u8string_view svText, Args&& ... args) { _Log<char8_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::u16string_view svText, Args&& ... args) { _Log<char16_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::u32string_view svText, Args&& ... args) { _Log<char32_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::string_view svText, Args&& ... args) { _Log<char>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::wstring_view svTag, const std::wstring_view svText, Args&& ... args) { _Log<wchar_t>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::u8string_view svTag, const std::u8string_view svText, Args&& ... args) { _Log<char8_t>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::u16string_view svTag, const std::u8string_view svText, Args&& ... args) { _Log<char16_t>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::u32string_view svTag, const std::u8string_view svText, Args&& ... args) { _Log<char32_t>(svTag, fmt::format(svText, args...)); }

	};


	//-----------------------------------------------------------------------------
	// Simple Log Interface
	//
	template < class CLogWriter >
	class ILog {
	protected:
		mutable CLogWriter* m_pLog = nullptr;
	public:
		ILog(CLogWriter* pLog = nullptr) : m_pLog(pLog) {}

		void SetLog(CLogWriter* pLog) const { m_pLog = pLog; }
		CLogWriter* GetLog() const { return m_pLog; }

	public:
		template < typename ... Args > void Log(std::string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::wstring_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<wchar_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::u8string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char8_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::u16string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char16_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void Log(std::u32string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char32_t>(nullptr, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::wstring_view svTag, const std::wstring_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<wchar_t>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::u8string_view svTag, const std::u8string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char8_t>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::u16string_view svTag, const std::u8string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char16_t>(svTag, fmt::format(svText, args...)); }
		template < typename ... Args > void LogTag(const std::u32string_view svTag, const std::u8string_view svText, Args&& ... args) { if (m_pLog) m_pLog->_Log<char32_t>(svTag, fmt::format(svText, args...)); }
	};


#pragma pack(pop)
}	// namespace gtl


#endif	//
