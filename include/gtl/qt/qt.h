#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/gtl.h"

#include <QApplication>

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

	inline auto ToCoord(QPoint pt) { return xPoint2i(pt.x(), pt.y()); }
	inline auto ToCoord(QPointF pt) { return xPoint2d(pt.x(), pt.y()); }
	inline auto ToCoord(QSize s) { return xSize2i(s.width(), s.height()); }
	inline auto ToCoord(QSizeF s) { return xSize2d(s.width(), s.height()); }
	inline auto ToCoord(QRect rect) { return xRect2i(rect.left(), rect.top(), rect.right(), rect.bottom()); }
	inline auto ToCoord(QRectF rect) { return xRect2d(rect.left(), rect.top(), rect.right(), rect.bottom()); }

	inline auto ToQCoord(xPoint2i pt) { return QPoint (pt.x, pt.y); }
	inline auto ToQCoord(xPoint2d pt) { return QPointF(pt.x, pt.y); }
	inline auto ToQCoord(xSize2i s) { return QSize (s.cx, s.cy); }
	inline auto ToQCoord(xSize2d s) { return QSizeF(s.cx, s.cy); }
	inline auto ToQCoord(xRect2i rect) { return QRect (rect.left, rect.top, rect.Width(), rect.Height()); }
	inline auto ToQCoord(xRect2d rect) { return QRectF(rect.left, rect.top, rect.Width(), rect.Height()); }

	inline QPoint Floor(QPointF pt) {
		return QPoint{ (int)pt.x(), (int)pt.y() };
	}
	inline QRect Floor(QRectF rect) {
		return QRect{ Floor(rect.topLeft()), Floor(rect.bottomRight()) };
	}

	GTL__QT_API QImage::Format GetImageFormatType(int type);

	//=================================================================================================================================
	// helper inline functions
	inline QString ToQString(std::string const& str) { return QString::fromStdString(str);}
	inline QString ToQString(std::wstring const& str) { return QString::fromStdWString(str); }
	inline QString ToQString(std::u8string const& str) { return QString::fromUtf8(str.c_str(), str.size()); }
	inline QString ToQString(std::u16string const& str) { return QString::fromStdU16String(str); }
	inline QString ToQString(std::u32string const& str) { return QString::fromStdU32String(str); }

	template < typename T > requires 
		requires (T t) { {ToString(t)} -> std::convertible_to<std::string>; }
	or  requires (T t) { {ToString(t)} -> std::convertible_to<std::wstring>; }
	inline QString ToQString(T const& t) {
		return ToQString(ToString(t));
	}

	inline std::string ToString(QString const& str) { return str.toStdString(); }
	inline std::wstring ToWString(QString const& str) { return str.toStdWString(); }

	inline std::string ToString(QColor cr) { return ToString(cr.name()); }
	inline std::wstring ToWString(QColor cr) { return ToWString(cr.name()); }

	template < gtlc::string_elem tchar_t >
	std::basic_string<tchar_t> ToTString(QString const& str) {
		if constexpr (sizeof(tchar_t) == sizeof(char)) {
			return ToString(str);
		}
		else if constexpr (std::is_same_v<tchar_t, wchar_t>) {
			return ToWString(str);
		}
		else if constexpr (std::is_same_v<tchar_t, char16_t>) {
			return str.toStdU16String();
		}
		else if constexpr (std::is_same_v<tchar_t, char32_t>) {
			return str.toStdU32String();
		}
		else {
			static_assert(gtlc::dependent_false_v, "Not supported");
		}
	}

	template < gtlc::arithmetic value_t >
	value_t ToArithmeticValue(QString const& str, int base = 0, std::from_chars_result* result = {}) {
		return gtl::ToArithmeticValue<value_t>(str.toStdString(), base, result);
	}


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
	GTL__QT_UPDATE_WIDGET_VALUE(QDoubleSpinBox, double, value, setValue);
	GTL__QT_UPDATE_WIDGET_VALUE(QSpinBox, int, value, setValue);
	GTL__QT_UPDATE_WIDGET_VALUE(QComboBox, int, currentIndex, setCurrentIndex);
	GTL__QT_UPDATE_WIDGET_STRING(QComboBox, currentText, setCurrentText);


	template < typename value_t > requires std::is_enum_v<value_t>
	inline void UpdateWidgetValue(bool bSaveAndValidate, QComboBox* w, value_t& value) {
		if (bSaveAndValidate)
			value = (value_t)w->currentIndex();
		else
			w->setCurrentIndex(std::to_underlying(value));
	}

#undef GTL__QT_UPDATE_WIDGET_STRING
#undef GTL__QT_UPDATE_WIDGET_VALUE

} // namespace gtl::qt
