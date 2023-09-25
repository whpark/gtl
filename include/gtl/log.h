﻿//////////////////////////////////////////////////////////////////////
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
#include "_lib_gtl.h"
#include "gtl/time.h"
#include "gtl/archive.h"


namespace gtl {
#pragma pack(push, 8)


	//-----------------------------------------------------------------------------
	// Simple Log
	//
	class GTL__CLASS xSimpleLog {
	public:
		using archive_out_t = gtl::TArchive<std::ofstream>;

	protected:
		mutable TCopyTransparent<std::recursive_mutex> m_mutex;	// for multi-thread
		std::filesystem::path m_path;								// Current File Path
		std::ofstream m_file;										// Current File
		std::unique_ptr<archive_out_t> m_ar;						// Archive
		std::string m_strTagFilter;									// Tag Filter. 'How to use' is up to user.
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

		//std::function<int(int, int, int)> m_fun;

		std::function<xStringA  (xSimpleLog&, archive_out_t&, xSysTime, const std::string_view svTag, const std::string_view svContent)> m_funcFormatterA;			// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringW  (xSimpleLog&, archive_out_t&, xSysTime, const std::string_view svTag, const std::wstring_view svContent)> m_funcFormatterW;		// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringU8 (xSimpleLog&, archive_out_t&, xSysTime, const std::string_view svTag, const std::u8string_view svContent)> m_funcFormatterU8;		// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringU16 (xSimpleLog&, archive_out_t&, xSysTime, const std::string_view svTag, const std::u16string_view svContent)> m_funcFormatterU16;	// 로그 파일 포맷을 바꾸고 싶을 때...
		std::function<xStringU32 (xSimpleLog&, archive_out_t&, xSysTime, const std::string_view svTag, const std::u32string_view svContent)> m_funcFormatterU32;	// 로그 파일 포맷을 바꾸고 싶을 때...

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
		void _Log(std::string_view svTag, std::basic_string<tchar_t>&& str);

	public:
		// Write Log
		template < typename ... targs> constexpr void Log(gtl::internal::tformat_string<char, targs...> const& fmt, targs&& ... args)		{ return _Log<char>("", fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void Log(gtl::internal::tformat_string<wchar_t, targs...> const& fmt, targs&& ... args)	{ return _Log<wchar_t>("", fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void Log(gtl::internal::tformat_string<char8_t, targs...> const& fmt, targs&& ... args)	{ return _Log<char8_t>("", fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void Log(gtl::internal::tformat_string<char16_t, targs...> const& fmt, targs&& ... args)	{ return _Log<char16_t>("", fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void Log(gtl::internal::tformat_string<char32_t, targs...> const& fmt, targs&& ... args)	{ return _Log<char32_t>("", fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void LogTag(std::string_view const& svTag, gtl::internal::tformat_string<char, targs...> const& fmt, targs&& ... args)		{ return _Log<char>(svTag, fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void LogTag(std::string_view const& svTag, gtl::internal::tformat_string<wchar_t, targs...> const& fmt, targs&& ... args)	{ return _Log<wchar_t>(svTag, fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void LogTag(std::string_view const& svTag, gtl::internal::tformat_string<char8_t, targs...> const& fmt, targs&& ... args)	{ return _Log<char8_t>(svTag, fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void LogTag(std::string_view const& svTag, gtl::internal::tformat_string<char16_t, targs...> const& fmt, targs&& ... args)	{ return _Log<char16_t>(svTag, fmt::format(fmt, std::forward<targs>(args)...)); }
		template < typename ... targs> constexpr void LogTag(std::string_view const& svTag, gtl::internal::tformat_string<char32_t, targs...> const& fmt, targs&& ... args)	{ return _Log<char32_t>(svTag, fmt::format(fmt, std::forward<targs>(args)...)); }

	//#if 1
	//	template < typename ... Args > void Log(std::string_view fmt, Args&& ... args) {
	//		_Log({}, std::vformat(fmt, std::make_format_args(args...)));
	//	}
	//	template < typename ... Args > void Log(std::wstring_view fmt, Args&& ... args) {
	//		_Log({}, std::vformat(fmt, std::make_wformat_args(args...)));
	//	}

	//	template < typename ... Args > void LogTag(std::string_view svTag, std::string_view fmt, Args&& ... args) {
	//		_Log(svTag, std::vformat(fmt, std::make_format_args(args...)));
	//	}
	//	template < typename ... Args > void LogTag(std::string_view svTag, std::wstring_view fmt, Args&& ... args) {
	//		_Log(svTag, std::vformat(fmt, std::make_wformat_args(args...)));
	//	}
	//#else
	//	template < typename ... Args > void Log(std::string_view svText, Args&& ... args) { _Log<char>({}, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void Log(std::wstring_view svText, Args&& ... args) { _Log<wchar_t>({}, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void Log(std::u8string_view svText, Args&& ... args) { _Log<char8_t>({}, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void Log(std::u16string_view svText, Args&& ... args) { _Log<char16_t>({}, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void Log(std::u32string_view svText, Args&& ... args) { _Log<char32_t>({}, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void LogTag(const std::string_view svTag, const std::string_view svText, Args&& ... args) { _Log<char>(svTag, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void LogTag(const std::string_view svTag, const std::wstring_view svText, Args&& ... args) { _Log<wchar_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void LogTag(const std::string_view svTag, const std::u8string_view svText, Args&& ... args) { _Log<char8_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void LogTag(const std::string_view svTag, const std::u16string_view svText, Args&& ... args) { _Log<char16_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
	//	template < typename ... Args > void LogTag(const std::string_view svTag, const std::u32string_view svText, Args&& ... args) { _Log<char32_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
	//#endif

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
	#if 0
		template < typename SFMT, typename ... Args > void Log(SFMT const& fmt, Args&& ... args) {
			if (m_pLog) m_pLog->_Log({}, std::format(fmt, std::forward<Args>(args)...));
		}
		template < typename SFMT, typename ... Args > void LogTag(const std::string_view svTag, SFMT const& fmt, Args&& ... args) {
			if (m_pLog) m_pLog->_Log(svTag, std::format(fmt, std::forward<Args>(args)...));
		}
	#else
		template < typename ... Args > void Log(std::string_view svText, Args&& ... args) 
			{ if (m_pLog) m_pLog->_Log<char>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::wstring_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<wchar_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u8string_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<char8_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u16string_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<char16_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void Log(std::u32string_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<char32_t>({}, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::string_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<char>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::wstring_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<wchar_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u8string_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<char8_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u16string_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<char16_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
		template < typename ... Args > void LogTag(const std::string_view svTag, const std::u32string_view svText, Args&& ... args)
			{ if (m_pLog) m_pLog->_Log<char32_t>(svTag, std::format(svText, std::forward<Args>(args)...)); }
	#endif
	};



	// Write Log
	template < typename tchar_t >
	void xSimpleLog::_Log(std::string_view svTag, std::basic_string<tchar_t>&& str) {
		auto now = std::chrono::system_clock::now();

	#if defined(_DEBUG) and defined(_WINDOWS)
		if (m_bTraceOut) {
			if constexpr (std::is_same_v<tchar_t, char>) {
				OutputDebugStringA(str.c_str());
				OutputDebugStringA("\r\n");
			}
			else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
				OutputDebugStringW(str.c_str());
				OutputDebugStringW(L"\r\n");
			}
			else {
				OutputDebugStringW(xStringW(str));
				OutputDebugStringW(L"\r\n");
			}
		}
	#endif

		if (!OpenFile(now) || !m_ar)
			return;
		if (!m_strTagFilter.empty() && !svTag.empty() &&
		#if 0	// TEMP
			!m_strTagFilter.contains(svTag)
		#else
			(m_strTagFilter.find(svTag) != m_strTagFilter.npos)
		#endif
			)
		{
			return;
		}

		do {
			if constexpr (std::is_same_v<tchar_t, char>) {
				if (m_funcFormatterA) {
					m_funcFormatterA(*this, *m_ar, now, svTag, str);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
				if (m_funcFormatterW) {
					m_funcFormatterW(*this, *m_ar, now, svTag, str);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
				if (m_funcFormatterU8) {
					m_funcFormatterU8(*this, *m_ar, now, svTag, str);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
				if (m_funcFormatterU16) {
					m_funcFormatterU16(*this, *m_ar, now, svTag, str);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
				if (m_funcFormatterU32) {
					m_funcFormatterU32(*this, *m_ar, now, svTag, str);
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
				m_ar->WriteString(TEXT_u8(GTL__FMT_EXPAND), st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), (std::u8string_view&)svTag);
			} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
				m_ar->WriteString(TEXT_u(GTL__FMT_EXPAND), st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), xStringU16(svTag));
			} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
				m_ar->WriteString(TEXT_U(GTL__FMT_EXPAND), st.tm_year, st.tm_mon, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, msec.count(), xStringU32(svTag));
			} else {
				static_assert(gtlc::dependent_false_v);
			}
#undef GTL__FMT_EXPAND
			const tchar_t* posHead = str.data();
			const tchar_t* posEnd = str.data() + str.size();
			const tchar_t* posNext {};
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
					m_ar->WriteString(L"{1:{0}}\r\n", len, pos);
				} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
					m_ar->WriteString(u8"{1:{0}}\r\n", len, pos);
				} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
					m_ar->WriteString(u"{1:{0}}\r\n", len, pos);
				} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
					m_ar->WriteString(U"{1:{0}}\r\n", len, pos);
				}
			}


		} while (false);


		if (m_bCloseFileAfterWrite)
			CloseFile();
	}

	//template void xSimpleLog::_Log<char>(const std::basic_string_view<char> svTag, const std::basic_string_view<char> svText);
	//template void xSimpleLog::_Log<wchar_t>(const std::basic_string_view<wchar_t> svTag, const std::basic_string_view<wchar_t> svText);
	//template void xSimpleLog::_Log<char8_t>(const std::basic_string_view<char8_t> svTag, const std::basic_string_view<char8_t> svText);
	//template void xSimpleLog::_Log<char16_t>(const std::basic_string_view<char16_t> svTag, const std::basic_string_view<char16_t> svText);
	//template void xSimpleLog::_Log<char32_t>(const std::basic_string_view<char32_t> svTag, const std::basic_string_view<char32_t> svText);

#pragma pack(pop)
}	// namespace gtl


#endif	//
