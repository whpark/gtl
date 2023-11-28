#pragma once

#include "gtl/gtl.h"
#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/qt/qt.h"

#include <QApplication>
#include <QCheckBox>
#include <QGroupBox>
#include <QlineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSettings>

namespace gtl::qt {

	//=============================================================================================================================
	/// @brief Wait Cursor
	class xWaitCursor {
	protected:
		inline thread_local static int m_nCount;
	public:
		xWaitCursor() {
			m_nCount++;	// 걸 때마다 호출. 처음 걸었어도 다른 곳에서 풀었을 수 있으므로, 무조건 호출 함.
			QApplication::setOverrideCursor(Qt::WaitCursor);
		}
		~xWaitCursor() {
			if (--m_nCount == 0)	// 끄는거는, 0이 되었을 때만.
				QApplication::restoreOverrideCursor();
		}
	};

	//=============================================================================================================================
	/// @brief QCustomEventFilter
	/// @details filters out events in m_lstEventTypesToFilter
	class QCustomEventFilter  : public QObject {
		//Q_OBJECT
	public:
		using this_t = QCustomEventFilter;
		using base_t = QObject;

	public:
		std::vector<QEvent::Type> m_lstEventTypesToFilter;

		QCustomEventFilter(QObject* parent, std::initializer_list<QEvent::Type> lst) : QObject(parent), m_lstEventTypesToFilter(std::move(lst)) { }
		~QCustomEventFilter() {}

		bool eventFilter(QObject* obj, QEvent* event) override {
			if (auto iter = std::ranges::find(m_lstEventTypesToFilter, event->type());
				iter != m_lstEventTypesToFilter.end())
			{
				return true;
			}
			return QObject::eventFilter(obj, event);
		}

	};


	/// @brief cv::Mat::type() -> QImage::Format
	/// @param cv::Mat::type() (CV8UC1 ...)
	/// @return  QImage::Format
	GTL__QT_API QImage::Format GetImageFormatType(int cv_type);

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
		if (bSaveAndValidate) value = ToTString<tchar_t>(w->getter()); else w->setter(ToQString(value)); \
	}\
	inline void UpdateWidgetValue(bool bSaveAndValidate, widget_t* w, std::filesystem::path& path) {	\
		if (bSaveAndValidate) path = ToWString(w->getter()); else w->setter(ToQString(path)); \
	}

	#define GTL__QT_UPDATE_WIDGET_ARITHMETIC(widget_t, str_getter, str_setter) \
	template < gtlc::arithmetic value_t>\
	inline void UpdateWidgetValue(bool bSaveAndValidate, widget_t* w, value_t& value, int base = 0,\
		std::string_view fmt = "{}") {\
		if (bSaveAndValidate) value = gtl::qt::ToArithmeticValue<value_t>(w->str_getter());\
		else w->str_setter(std::vformat(fmt, std::make_format_args(value)).c_str());\
	}

	GTL__QT_UPDATE_WIDGET_VALUE(QCheckBox, bool, isChecked, setChecked);
	//GTL__QT_UPDATE_WIDGET_VALUE(QCheckBox, int, isChecked, setChecked);
	GTL__QT_UPDATE_WIDGET_VALUE(QGroupBox, bool, isChecked, setChecked);
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
	template < typename trep_t, typename tperiod_t, typename twidget_t > requires (requires (twidget_t* w) {
		{ w->value() } -> std::convertible_to<trep_t>;
		w->setValue(trep_t{});
	})
	inline void UpdateWidgetValue(bool bSaveAndValidate, twidget_t* w, std::chrono::duration<trep_t, tperiod_t>& value) {
		using duration_t = std::chrono::duration<trep_t, tperiod_t>;
		if (bSaveAndValidate) {
			value = duration_t{(trep_t)w->value()};
		}
		else {
			w->setValue(value.count());
		}
	}
	template < typename trep_t, typename tperiod_t, typename twidget_t > requires (requires (twidget_t* w) {
		{ w->text() } -> std::convertible_to<QString>;
		w->setText(QString{});
	})
	inline void UpdateWidgetValue(bool bSaveAndValidate, twidget_t* w, std::chrono::duration<trep_t, tperiod_t>& value) {
		using duration_t = std::chrono::duration<trep_t, tperiod_t>;
		if constexpr (std::is_floating_point_v<trep_t>) {
			if (bSaveAndValidate) {
				value = duration_t{ToArithmeticValue<trep_t>(w->text())};
			}
			else {
				w->setText(ToQString(std::format("{:g}", value.count())));
			}
		}
		else if constexpr (std::is_integral_v<trep_t>) {
			if (bSaveAndValidate) {
				value = duration_t{ToArithmeticValue<trep_t>(w->text())};
			}
			else {
				w->setText(ToQString(std::format("{}", value.count())));
			}
		}
	}

	#undef GTL__QT_UPDATE_WIDGET_STRING
	#undef GTL__QT_UPDATE_WIDGET_VALUE


	//=============================================================================================================================
	// Window Position
	GTL__QT_API bool SaveWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd);
	GTL__QT_API bool LoadWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd);

} // namespace gtl::qt
