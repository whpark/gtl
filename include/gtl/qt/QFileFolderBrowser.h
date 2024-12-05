#pragma once

#include "gtl/qt/_lib_gtl_qt.h"

#include <QWidget>
#include <QFileDialog>
#include <QKeyEvent>

//#include "ui_QFileFolderBrowser.h"
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>


namespace gtl::qt {

	class GTL__QT_CLASS QComboBoxEnter : public QComboBox {
		Q_OBJECT
		using this_t = QComboBoxEnter;
		using base_t = QComboBox;

	public:
		using base_t::base_t;

	signals:
		void SigEnterPressed();

	public:
		void keyPressEvent(QKeyEvent* event) override {
			if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
				SigEnterPressed();
			}
			base_t::keyPressEvent(event);
		}

	};

}

QT_BEGIN_NAMESPACE
class Ui_QFileFolderBrowserClass {
public:
	QHBoxLayout* horizontalLayout{};
	gtl::qt::QComboBoxEnter* cmbPath{};
	QToolButton* btnBrowse{};

	void setupUi(QWidget* QFileFolderBrowserClass) {
		if (QFileFolderBrowserClass->objectName().isEmpty())
			QFileFolderBrowserClass->setObjectName("QFileFolderBrowserClass");
		QFileFolderBrowserClass->resize(402, 25);
		horizontalLayout = new QHBoxLayout(QFileFolderBrowserClass);
		horizontalLayout->setSpacing(3);
		horizontalLayout->setObjectName("horizontalLayout");
		horizontalLayout->setContentsMargins(0, 0, 0, 0);
		cmbPath = new gtl::qt::QComboBoxEnter(QFileFolderBrowserClass);
		cmbPath->setObjectName("cmbPath");
		cmbPath->setEditable(true);

		horizontalLayout->addWidget(cmbPath);

		btnBrowse = new QToolButton(QFileFolderBrowserClass);
		btnBrowse->setObjectName("btnBrowse");

		horizontalLayout->addWidget(btnBrowse);

		horizontalLayout->setStretch(0, 1);

		retranslateUi(QFileFolderBrowserClass);

		QMetaObject::connectSlotsByName(QFileFolderBrowserClass);
	} // setupUi

	void retranslateUi(QWidget* QFileFolderBrowserClass) {
		QFileFolderBrowserClass->setWindowTitle(QCoreApplication::translate("QFileFolderBrowserClass", "QFileFolderBrowser", nullptr));
		btnBrowse->setText(QCoreApplication::translate("QFileFolderBrowserClass", "...", nullptr));
	} // retranslateUi

};

namespace Ui {
	class QFileFolderBrowserClass : public Ui_QFileFolderBrowserClass {};
} // namespace Ui

QT_END_NAMESPACE

namespace gtl::qt {

	class GTL__QT_CLASS QFileFolderBrowser : public QWidget {
		Q_OBJECT

	public:
		bool m_bFileBrowser{ true };

	public:
		QFileFolderBrowser(QWidget* parent = nullptr);
		~QFileFolderBrowser();

	signals:
		void SigModeChanged(bool bFileBrowse);
	signals:
		void SigPathBrowsed(QString const& path);
	signals:
		void SigPathChanged(QString const& path);

	public:
		auto& GetComboBox() { return *ui.cmbPath; }
		std::filesystem::path GetPath() {
			return ui.cmbPath->currentText().toStdWString();
		}
		void SetPath(const std::filesystem::path& path) {
			ui.cmbPath->setCurrentText(QString::fromStdWString(path.wstring()));
		}

	private:
		Ui::QFileFolderBrowserClass ui;

	public:
		Q_PROPERTY(bool bFileBrowse MEMBER m_bFileBrowser NOTIFY SigModeChanged);
	};

};	// namespace biscuit::qt

