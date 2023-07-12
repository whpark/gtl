#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/QColorButton.h"

namespace gtl::qt {

QColorButton::QColorButton(QWidget* parent) : QPushButton(parent) {
	//auto t0 = std::chrono::high_resolution_clock::now();
	auto p = qApp->palette();
	//auto t1 = std::chrono::high_resolution_clock::now();
	SetMainColor(p.button().color(), p.buttonText().color());
	//Log("QColorButton Time : {}", std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0));
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
