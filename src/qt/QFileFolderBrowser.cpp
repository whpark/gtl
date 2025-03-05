#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/QFileFolderBrowser.h"

namespace gtl::qt {

	QFileFolderBrowser::QFileFolderBrowser(QWidget* parent) : QWidget(parent) {
		ui.setupUi(this);

		connect(ui.btnBrowse, &QToolButton::clicked, this, [this]() {
			QString path;
			QString pathCurrent = ui.cmbPath->currentText();
			if (pathCurrent.isEmpty())
				pathCurrent = QDir::homePath();
			if (m_bFileBrowser)
				path = QFileDialog::getOpenFileName(this, "Select File", pathCurrent);
			else
				path = QFileDialog::getExistingDirectory(this, "Select Folder", pathCurrent);

			if (!path.isEmpty()) {
				if (ui.cmbPath->findText(path, Qt::MatchExactly) < 0)
					ui.cmbPath->addItem(path);
				ui.cmbPath->setCurrentText(path);

				SigPathBrowsed(path);
			}
		});

		connect(ui.cmbPath, &QComboBoxEnter::currentTextChanged, this, [this](const QString& text) {
			emit SigPathChanged(text);
		});
		connect(ui.cmbPath, &QComboBoxEnter::currentIndexChanged, this, [this](int index) {
			emit SigPathBrowsed(ui.cmbPath->itemText(index));
		});
		connect(ui.cmbPath, &QComboBoxEnter::editTextChanged, this, [this](const QString& text) {
			emit SigPathChanged(text);
		});
		connect(ui.cmbPath, &QComboBoxEnter::SigEnterPressed, this, [this] {
			emit SigPathBrowsed(ui.cmbPath->currentText());
		});
	}

	QFileFolderBrowser::~QFileFolderBrowser() {
	}

};	// namespace biscuit::qt

