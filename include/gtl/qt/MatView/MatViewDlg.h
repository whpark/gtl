#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/qt/MatView/MatView.h"

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
	using this_t = xMatViewDlg;
	using base_t = QDialog;

protected:
	xMatView* m_view{};
public:
	xMatViewDlg(QWidget* parent = nullptr);
	~xMatViewDlg();

	xMatView& GetView() { return *m_view; }

private:
	std::unique_ptr<Ui::MatViewDlgClass> ui;
};
