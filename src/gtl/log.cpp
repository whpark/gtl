#include "pch.h"

//////////////////////////////////////////////////////////////////////
//
// log.cpp: .ini 파일. simple log
//
// PWH
// 2019.01.09.
// 2019.07.24. QL -> GTL
// 
//
//////////////////////////////////////////////////////////////////////

#if 1

#include "gtl/log.h"
//#include "gtl/filefind.h"

namespace gtl {


	bool CSimpleLog::OpenFile(std::chrono::system_clock::time_point now) {
		std::scoped_lock lock(m_mutex);

		if (m_fmtLogFileName.empty()) {
			CloseFile();
			return false;
		}

		CStringW strFilePath;

		strFilePath = (m_folderLog / m_fmtLogFileName.c_str()).c_str();
		strFilePath.Replace(L"[Name]", m_strName);

		CSysTime tNow(now);
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
				CSysTime tLastWrite = std::filesystem::last_write_time(path);
				CSysTime t(now);
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
				throw std::runtime_error(fmt::format(GTL__FUNCSIG "Cannot make LOG File {}", path.string()));
			}

		}
		return true;
	}
	bool CSimpleLog::CloseFile() {
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
	void CSimpleLog::_Log(const std::basic_string_view<tchar_t> svMask, const std::basic_string_view<tchar_t> svText) {
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
		if (!m_strTagFilter.empty() && !svMask.empty()) {
			if constexpr (std::is_same_v<tchar_t, CString::value_type>) {
				if (!m_strTagFilter.find(svMask))
					return;
			} else {
				if (!m_strTagFilter.find(CString(svMask)))
					return;
			}
		}

		do {
			if constexpr (std::is_same_v<tchar_t, char>) {
				if (m_funcFormatterA) {
					m_funcFormatterA(*this, *m_ar, now, svMask, svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
				if (m_funcFormatterW) {
					m_funcFormatterW(*this, *m_ar, now, svMask, svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
				if (m_funcFormatterU8) {
					m_funcFormatterU8(*this, *m_ar, now, svMask, svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
				if (m_funcFormatterU16) {
					m_funcFormatterU16(*this, *m_ar, now, svMask, svText);
					break;
				}
			} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
				if (m_funcFormatterU32) {
					m_funcFormatterU32(*this, *m_ar, now, svMask, svText);
					break;
				}
			} else {
				static_assert(false);
			}

			auto st = CSysTime(now).GetLocalSystemTime();

			//auto GetFMT_Header = []() -> const tchar_t* {
			//	if constexpr (std::is_same_v<tchar_t, char>) {
			//		return "%04d/%02d/%02d, %02d:%02d:%02d.%03d [%.*s] :";
			//	} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
			//		return L"%04d/%02d/%02d, %02d:%02d:%02d.%03d [%.*s] :";
			//	} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
			//		return _U8("%04d/%02d/%02d, %02d:%02d:%02d.%03d [%.*s] :");
			//	} else {
			//		static_assert(false);
			//	}
			//};

			if constexpr (std::is_same_v<tchar_t, char>) {
				m_ar->WriteString("{:04}/{:02}/{:02}, {:02}:{:02}:{:02}.{:03} {8:{7}} : ",
									st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
									svMask.size(), svMask);
			} else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
				m_ar->WriteString(TEXT_W("{:04}/{:02}/{:02}, {:02}:{:02}:{:02}.{:03} {8:{7}} : "),
								  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
								  svMask.size(), svMask);
			} else if constexpr (std::is_same_v<tchar_t, char8_t>) {
				m_ar->WriteString(TEXT_u8("{:04}/{:02}/{:02}, {:02}:{:02}:{:02}.{:03} {8:{7}} : "),
								  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
								  svMask.size(), svMask);
			} else if constexpr (std::is_same_v<tchar_t, char16_t>) {
				m_ar->WriteString(TEXT_u("{:04}/{:02}/{:02}, {:02}:{:02}:{:02}.{:03} {8:{7}} : "),
								  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
								  svMask.size(), svMask);
			} else if constexpr (std::is_same_v<tchar_t, char32_t>) {
				m_ar->WriteString(TEXT_U("{:04}/{:02}/{:02}, {:02}:{:02}:{:02}.{:03} {8:{7}} : "),
								  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
								  svMask.size(), svMask);
			} else {
				static_assert(false);
			}

			const tchar_t* posHead = svText.data();
			const tchar_t* posEnd = svText.data() + svText.size();
			const tchar_t* posNext = nullptr;
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
				}
			}


		} while (false);


		if (m_bCloseFileAfterWrite)
			CloseFile();
	}

	template void CSimpleLog::_Log<char>(const std::basic_string_view<char> svMask, const std::basic_string_view<char> svText);
	template void CSimpleLog::_Log<wchar_t>(const std::basic_string_view<wchar_t> svMask, const std::basic_string_view<wchar_t> svText);
	template void CSimpleLog::_Log<char8_t>(const std::basic_string_view<char8_t> svMask, const std::basic_string_view<char8_t> svText);
	template void CSimpleLog::_Log<char16_t>(const std::basic_string_view<char16_t> svMask, const std::basic_string_view<char16_t> svText);
	template void CSimpleLog::_Log<char32_t>(const std::basic_string_view<char32_t> svMask, const std::basic_string_view<char32_t> svText);

}

#endif
