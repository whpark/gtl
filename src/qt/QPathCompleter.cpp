#include "pch.h"
#include "gtl/qt/QPathCompleter.h"
#include "gtl/qt/util.h"

namespace gtl::qt {

QPathCompleter::QPathCompleter(QObject* parent) : QCompleter(parent), m_model(parent) {
}

QPathCompleter::QPathCompleter(QObject* parent, std::filesystem::path const& root, bool bCaseSensitive) : QPathCompleter(parent) {
	Init(root, bCaseSensitive);
}

QPathCompleter::~QPathCompleter() {
}

bool QPathCompleter::Init(std::filesystem::path const& root, bool bCaseSensitive) {
	m_model.setRootPath(ToQString(root));
	setModel(&m_model);

	setCompletionMode(QCompleter::PopupCompletion);
	setCaseSensitivity(bCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

	return true;
}

}	// namespace gtl::qt
