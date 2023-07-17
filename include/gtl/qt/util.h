#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/gtl.h"

#include <QApplication>
#include <QCheckBox>
#include <QlineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSettings>

namespace gtl::qt {

	class xWaitCursor {
	public:
		xWaitCursor() {
			QApplication::setOverrideCursor(Qt::WaitCursor);
		}
		~xWaitCursor() {
			QApplication::restoreOverrideCursor();
		}
	};

	GTL__QT_API QImage::Format GetImageFormatType(int type);

	//=============================================================================================================================
	// Dynamic Data Exchange

	#define GTL__QT_UPDATE_WIDGET_VALUE(widget_t, value_t, getter, setter)	\
	inline void UpdateWidgetValue(bool bSaveAndValidate, widget_t* w, value_t& value) {	\
		if (bSaveAndValidate) value = w->getter(); else w->setter(value); \
	}

	#define GTL__QT_UPDATE_WIDGET_STRING(widget_t, getter, setter)	\
		GTL__QT_UPDATE_WIDGET_VALUE(widget_t, QString, getter, setter) \
	template < gtlc::string_elem tchar_t > \
	inline void UpdateWidgetValue(bool bSaveAndValidate, widget_t* w, std::basic_string<tchar_t>& value) {	\
		if (bSaveAndValidate) value = ToString(w->getter()); else w->setter(ToQString(value)); \
	}

	#define GTL__QT_UPDATE_WIDGET_ARITHMETIC(widget_t, str_getter, str_setter) \
	template < gtlc::arithmetic value_t>\
	inline void UpdateWidgetValue(bool bSaveAndValidate, widget_t* w, value_t& value, int base = 0,\
		std::string_view fmt = "{}") {\
		if (bSaveAndValidate) value = gtl::qt::ToArithmeticValue<value_t>(w->str_getter());\
		else w->str_setter(std::vformat(fmt, std::make_format_args(value)).c_str());\
	}

	GTL__QT_UPDATE_WIDGET_VALUE(QCheckBox, bool, isChecked, setChecked);
	GTL__QT_UPDATE_WIDGET_STRING(QLineEdit, text, setText);
	GTL__QT_UPDATE_WIDGET_ARITHMETIC(QLineEdit, text, setText);
	GTL__QT_UPDATE_WIDGET_STRING(QPlainTextEdit, toPlainText, setPlainText);
	GTL__QT_UPDATE_WIDGET_VALUE(QSpinBox, int, value, setValue);
	GTL__QT_UPDATE_WIDGET_VALUE(QDoubleSpinBox, double, value, setValue);
	GTL__QT_UPDATE_WIDGET_VALUE(QComboBox, int, currentIndex, setCurrentIndex);
	GTL__QT_UPDATE_WIDGET_STRING(QComboBox, currentText, setCurrentText);

	// enum
	template < typename value_t > requires std::is_enum_v<value_t>
	inline void UpdateWidgetValue(bool bSaveAndValidate, QComboBox* w, value_t& value) {
		if (bSaveAndValidate)
			value = (value_t)w->currentIndex();
		else
			w->setCurrentIndex(std::to_underlying(value));
	}

	// std::duration
	template < typename trep_t, typename tperiod_t, typename widget >
	inline void UpdateWidgetValue(bool bSaveAndValidate, widget* w, std::chrono::duration<trep_t, tperiod_t>& value) {
		using duration_t = std::chrono::duration<trep_t, tperiod_t>;
		if (bSaveAndValidate) {
			value = duration_t{(trep_t)w->value()};
		}
		else {
			w->setValue(value.count());
		}
	}

	#undef GTL__QT_UPDATE_WIDGET_STRING
	#undef GTL__QT_UPDATE_WIDGET_VALUE


	//=============================================================================================================================
	// Window Position
	GTL__QT_API bool SaveWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd);
	GTL__QT_API bool LoadWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd);

} // namespace gtl::qt
