#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QPushButton>

namespace gtl::qt { class GTL__QT_CLASS QColorButton; }

class gtl::qt::QColorButton : public QPushButton {
	Q_OBJECT
public:
	using this_t = QColorButton;
	using base_t = QPushButton;

protected:
	QColor m_crBackground;
	QColor m_crForeground;
public:
	using base_t::base_t;
	QColorButton(QWidget* parent = nullptr);
	~QColorButton() {}

public:
	auto GetMainColor() const { return m_crBackground; }
	auto GetMainColorText() const { return m_crForeground; }
	virtual void SetMainColor(QColor colorBackground, std::optional<QColor> colorForeground = {});

};
