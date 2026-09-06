#include "pch.h"
#include "App.h"

//W_OBJECT_IMPL(xApp)

std::optional<xApp> theApp;

xApp::xApp(int &argc, char **argv) : QApplication(argc, argv) {
	m_root = gtl::SetCurrentPath_ProjectFolder(L"").value_or("C:\\");


	setStyle("fusion");

	setOrganizationName(s_strOrgName);
	setOrganizationDomain(s_strOrgDomain);
	setApplicationName(s_strAppName);
	setApplicationVersion("1.0.0");
	//auto folderProject = GetDefaultProjectRoot();
	//if (!std::filesystem::exists(folderProject))
	//	std::filesystem::create_directories(folderProject);
}

xApp::~xApp() {
}

bool xApp::Init() {
	setStyle("fusion");

	m_root = std::filesystem::current_path();
	m_pathExec = QCoreApplication::applicationFilePath().toStdWString();

	m_wndMain = std::make_unique<xMainWnd>();
	m_wndMain->show();

	return true;
}

int xApp::Run() {
	return exec();
}


int main(int argc, char* argv[]) {
	// set locale utf-8 (NOT WORKING ON WINDOWS)
	std::locale l("ko_kr.utf-8");
	auto gl = std::locale::global(l);

	if (argc > 1)
		argc = 1;
	theApp.emplace(argc, argv);

	auto r = theApp->Init() ? theApp->Run() : 1;

	return r;
}
