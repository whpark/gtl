#include "pch.h"

#include "App.h"
#include "AboutDlg.h"

using namespace gtl::qt;

xAboutDlg::xAboutDlg(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);

	QString strContent =
R"xxx(

# **CADViewer** is built on

- openCV ( <https://opencv.org> )

  (license : [https://opencv.org/license/ )](https://opencv.org/license/)

<br/>

- Qt 6 ( <https://www.qt.io/> )

  (license : <https://www.qt.io/licensing/open-source-lgpl-obligations> )

  The Qt Toolkit is Copyright (C) The Qt Company Ltd.

  Qt is licensed under terms of the GNU LGPLv3,

<br/>

- fmtlib ( <https://fmt.dev/> )

  (license : MIT license. <https://github.com/fmtlib/fmt?tab=MIT-1-ov-file> )

<br/>

- ctre ( <https://compile-time.re/> )

  (license : Apache License 2.0. <https://github.com/hanickadot/compile-time-regular-expressions?tab=Apache-2.0-1-ov-file#readme> )

<br/>

- glaze ( <https://github.com/stephenberry/glaze> )

  (license : MIT license. <https://github.com/stephenberry/glaze?tab=MIT-1-ov-file#readme> )
<br/>

- gtl ( <https://github.com/whpark/gtl> )
<br/>

)xxx";

	ui.textEdit->setMarkdown(strContent);

	QString strBuildDate;
	try {
		std::istringstream strDate(__DATE__);
		std::string month;
		int day{}, year{};
		strDate >> month;
		strDate >> day;
		strDate >> year;
		static std::vector<std::string> const months { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", };
		auto pos = std::find(months.begin(), months.end(), month);
		auto m = std::distance(months.begin(), pos) + 1;
		strBuildDate = ToQString(std::format("{}-{:02d}-{:02d}", year, (int)m, day));
	} catch (...) {
		strBuildDate = __DATE__;
	}

	ui.groupBoxAbout->setTitle(strBuildDate);

	connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &xAboutDlg::accept);
}

xAboutDlg::~xAboutDlg() {
}
