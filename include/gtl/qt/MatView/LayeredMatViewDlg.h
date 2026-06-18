#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/qt/MatView/LayeredMatView.h"

#include <QWidget>
#include <QDialog>

namespace gtl::qt {

class GTL__QT_CLASS xLayeredMatViewDlg : public QDialog {
	Q_OBJECT
public:
	using this_t = xLayeredMatViewDlg;
	using base_t = QDialog;

protected:
	QVBoxLayout* verticalLayout;
protected:
	xLayeredMatView* m_view{};
public:
	xLayeredMatViewDlg(QWidget* parent = nullptr);
	~xLayeredMatViewDlg();

	xLayeredMatView& GetView() { return *m_view; }

protected:
	void setupUi(QDialog* LayeredMatViewDlgClass); // setupUi
	void retranslateUi(QDialog* MatViewDlgClass); // retranslateUi

};

} // namespace gtl::qt
