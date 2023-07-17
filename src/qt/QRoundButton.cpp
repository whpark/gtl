#include "pch.h"

#include "gtl/qt/qt.h"
#include "gtl/qt/QRoundButton.h"

namespace gtl::qt {

QRoundButton::QRoundButton(QWidget* parent) : QColorButton(parent) {
	auto p = qApp->palette();
//	SetMainColor(p.button().color(), p.buttonText().color());
}

void QRoundButton::SetMainColor(QColor colorBackground, std::optional<QColor> colorForeground) {
	base_t::SetMainColor(colorBackground, colorForeground);

	static constexpr auto const style {R"xxx(
QPushButton{{
  color: {1};
  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 {0}, stop:1 {2});
  border: 1px solid {3};
  border-radius: 5px;
  line-height: 20px;
  padding: 2px 6px;
  position: relative;
  text-align: center;
  text-decoration: none;
  vertical-align: middle;
  white-space: nowrap;
}}

QPushButton:hover {{
  background-color: {2};
}}

QPushButton:focus {{
  border: 2px solid {2};
}}

QPushButton:disabled {{
  background-color: {3};
  border-color: rgba(27, 31, 35, .1);
  color: rgba(255, 255, 255, .8);
}}

QPushButton:pressed {{
  background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 {4}, stop:1 {2});
}}
)xxx"};

	auto c1 = m_crBackground.darker(120);
	auto c2 = m_crBackground.lighter(120);
	auto c3 = c1.darker(120);
	auto str = fmt::format(style, ToString(m_crBackground), ToString(m_crForeground), ToString(c1), ToString(c2), ToString(c3));
	setStyleSheet(ToQString(str));

}

}	// namespace gtl::qt
