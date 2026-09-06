#pragma once

#include "MainWnd.h"

//---------------------------------------------------------------------------------------------------------------------------------
class xApp : public QApplication {
	Q_OBJECT
public:
	using this_t = xApp;
	using base_t = QApplication;

protected:
	inline static QString const s_strOrgName{"Biscuit-lab"};
	inline static QString const s_strOrgDomain{"biscuit-lab.com"};
	inline static QString const s_strAppName{"CADViewer"};
	QSettings m_reg { s_strOrgName, s_strAppName };

	std::unique_ptr<xMainWnd> m_wndMain;
	std::filesystem::path m_root;
	std::filesystem::path m_pathExec;

public:
	xApp(int &argc, char **argv/*, int flag = ApplicationFlags*/);
	~xApp();

	bool Init();
	int Run();

	std::filesystem::path const& GetRoot() const { return m_root; }
	std::filesystem::path const& GetPathExec() const { return m_pathExec; }
	std::filesystem::path const& GetDefaultProjectRoot() const {
		//auto str = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		//static std::filesystem::path root = [] { return std::filesystem::path(str.toStdWString()) / L"TestApp"; }();
		static std::filesystem::path root = m_root.root_path() / s_strAppName.toStdWString();
		return root;
	}

public:
	QSettings& GetReg() { return m_reg; };
	xMainWnd& GetMainWnd() { return *m_wndMain; }
};

//---------------------------------------------------------------------------------------------------------------------------------
extern std::optional<xApp> theApp;

