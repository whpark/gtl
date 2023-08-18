#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include <QCompleter>

namespace gtl::qt { class GTL__QT_CLASS QPathCompleter ; }

class gtl::qt::QPathCompleter : public QCompleter {
	Q_OBJECT
public:
	using this_t = QPathCompleter;
	using base_t = QCompleter;

protected:
	QFileSystemModel m_model;

public:
	QPathCompleter(QObject *parent);
	QPathCompleter(QObject *parent, std::filesystem::path const& root, bool bCaseSensitive = false);
	~QPathCompleter();

	bool Init(std::filesystem::path const& root, bool bCaseSensitive = false);
};
