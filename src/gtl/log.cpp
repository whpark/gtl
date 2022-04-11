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
		// 분 단위 1의 자리에서 버림 -> 10분 단위로 파일 이름 생성
		//strFilePath.Replace(L"%10M", fmt::format(L"{:02d}", tm.tm_min/10*10));
		//strFilePath.Replace(L"%10m", fmt::format(L"{:02d}", tm.tm_min/10*10));
		std::wregex reMin(LR"xxx(%([0-9]{1,2})[mM])xxx");
		for (std::wsmatch result; std::regex_search(strFilePath, result, reMin); ) {
			auto const& r = result[1];
			int minute = std::stoi(r.str());
			strFilePath.replace(result.position(), result.length(), std::format(L"{:02d}", tm.tm_min/minute*minute));
		}
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

}

#endif
