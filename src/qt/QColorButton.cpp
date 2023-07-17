#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/QColorButton.h"

namespace gtl::qt {

QColorButton::QColorButton(QWidget* parent) : QPushButton(parent) {
	auto p = qApp->palette();
	//SetMainColor(p.button().color(), p.buttonText().color());
}

void QColorButton::SetMainColor(QColor colorBackground, std::optional<QColor> colorForeground) {
	m_crBackground = colorBackground;
	if (colorForeground) {
		m_crForeground = *colorForeground;
	}
	else {
		auto p = qApp->palette();
		m_crForeground = p.buttonText().color();
	}

	QPalette pal = palette();
	pal.setColor(QPalette::ColorRole::Button, m_crBackground);
	pal.setColor(QPalette::ColorRole::ButtonText, m_crForeground);
	setPalette(pal);
}

} // namespace gtl::qt
