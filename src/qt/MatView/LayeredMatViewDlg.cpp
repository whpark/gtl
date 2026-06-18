#include "pch.h"

#include "gtl/qt/MatView/LayeredMatViewDlg.h"

namespace gtl::qt {

xLayeredMatViewDlg::xLayeredMatViewDlg(QWidget* parent) : QDialog(parent) {
	setupUi(this);
}
xLayeredMatViewDlg::~xLayeredMatViewDlg() {
}

void xLayeredMatViewDlg::setupUi(QDialog* LayeredMatViewDlgClass) {
	if (LayeredMatViewDlgClass->objectName().isEmpty())
		LayeredMatViewDlgClass->setObjectName("LayeredMatViewDlgClass");
	LayeredMatViewDlgClass->resize(393, 176);
	LayeredMatViewDlgClass->setWindowTitle(QString::fromUtf8("MatView"));
	verticalLayout = new QVBoxLayout(LayeredMatViewDlgClass);
	verticalLayout->setSpacing(0);
	verticalLayout->setObjectName("verticalLayout");
	verticalLayout->setContentsMargins(0, 0, 0, 0);
	auto* view = new gtl::qt::xLayeredMatView(LayeredMatViewDlgClass);
	view->setObjectName("view");
	view->setFocusPolicy(Qt::StrongFocus);

	verticalLayout->addWidget(view);

	retranslateUi(LayeredMatViewDlgClass);

	QMetaObject::connectSlotsByName(LayeredMatViewDlgClass);

	m_view = view;
}

void xLayeredMatViewDlg::retranslateUi(QDialog* MatViewDlgClass) {
	(void)MatViewDlgClass;
}

} // namespace gtl::qt

