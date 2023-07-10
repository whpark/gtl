#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
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


	GTL__QT_API QImage::Format GetImageFormatType(int type);

	//=================================================================================================================================
	// helper inline functions
	inline QString ToQString(std::string const& str) {
		return QString::fromStdString(str);
	}

	inline QString ToQString(std::wstring const& str) {
		return QString::fromStdWString(str);
	}

	template < typename T > requires 
		requires (T t) { {ToString(t)} -> std::convertible_to<std::string>; }
	or  requires (T t) { {ToString(t)} -> std::convertible_to<std::wstring>; }
	inline QString ToQString(T const& t) {
		return ToQString(ToString(t));
	}

	inline std::string ToString(QString const& str) {
		return str.toStdString();
	}

	inline std::wstring ToWString(QString const& str) {
		return str.toStdWString();
	}

	inline std::string ToString(QColor cr) {
		return ToString(cr.name());
	}

	inline std::wstring ToWString(QColor cr) {
		return ToWString(cr.name());
	}

} // namespace gtl::qt
