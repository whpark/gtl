#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "QColorButton.h"

namespace gtl::qt { class GTL__QT_CLASS QRoundButton; }

class gtl::qt::QRoundButton : public gtl::qt::QColorButton {
	Q_OBJECT
public:
	using this_t = QRoundButton;
	using base_t = QColorButton;

public:
	using base_t::base_t;
	QRoundButton(QWidget* parent = nullptr);

public:
	auto GetMainColor() const { return m_crBackground; }
	void SetMainColor(QColor colorBackground, std::optional<QColor> colorForeground = {});	// don't make it virtual. this function is called in constructor.

};
