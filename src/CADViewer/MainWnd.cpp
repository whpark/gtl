#include "pch.h"
#include "App.h"
#include "MainWnd.h"
#include "AboutDlg.h"

using namespace gtl::qt;

xMainWnd::xMainWnd(QWidget *parent) : base_t(parent) {
    ui.setupUi(this);

	auto& reg = theApp->GetReg();
	LoadWindowPosition(reg, "MainWnd", this);


	// Application Icon
	if (auto icon = std::make_unique<QIcon>(":/image/icon.ico"))
		setWindowIcon(*icon.release());

	connect(ui.actionAbout, &QAction::triggered, this, &this_t::OnAction_About);
}

xMainWnd::~xMainWnd() {
	SaveWindowPosition(theApp->GetReg(), "MainWnd", this);
}

void xMainWnd::OnAction_About(bool bChecked) {
	xAboutDlg dlg(this);
	dlg.exec();
}
