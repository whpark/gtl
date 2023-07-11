#pragma once

#include "gtl/qt/_lib_gtl_qt.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MatViewDlgClass; };
QT_END_NAMESPACE

namespace gtl::qt {
	class GTL__QT_CLASS xMatViewDlg;
}

class gtl::qt::xMatViewDlg : public QDialog {
	Q_OBJECT

public:
	xMatViewDlg(QWidget* parent = nullptr);
	~xMatViewDlg();

private:
	std::unique_ptr<Ui::MatViewDlgClass> ui;
};
