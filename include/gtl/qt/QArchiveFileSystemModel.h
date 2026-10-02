#pragma once

//////////////////////////////////////////////////////////////////////
//
// QArchiveFileSystemModel.h : QFileSystemModel + archive (zip, 7z) contents as virtual nodes.
//
// PWH
// 2026-10-03
//
//  - archive file (gtl::IsArchiveFile()) has an expand arrow (assumed to have children) until loaded.
//  - archive is loaded (gtl::ListArchive()) on fetchMore() (expanding) or index(path).
//  - virtual nodes (entries in archive) follow filter() : QDir::Dirs/AllDirs -> folders, QDir::Files -> files (+nameFilters)
//  - path of virtual node : "D:/folder/a.zip/sub/img.png"
//  - DO NOT call QFileSystemModel's non-virtual functions (filePath, fileInfo, ...) with a virtual index.
//    use the ones (hiding them) in this class.
//  - set filter() / nameFilters() before use. (virtual nodes are filtered when archive is loaded)
//
//////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>
#include "gtl/qt/_lib_gtl_qt.h"
#include <QFileSystemModel>
#include <QPersistentModelIndex>
#include <QDateTime>

namespace gtl::qt { class GTL__QT_CLASS QArchiveFileSystemModel; }

class gtl::qt::QArchiveFileSystemModel : public QFileSystemModel {
	Q_OBJECT
public:
	using this_t = QArchiveFileSystemModel;
	using base_t = QFileSystemModel;

protected:
	struct sArchive;
	struct sNode {
		sArchive* archive{};
		sNode* parent{};				// nullptr for archive root
		int row{};						// row in parent (visible), or index in parent->children if hidden
		bool bDir{};
		QString name;
		std::filesystem::path path;		// path in archive (generic)
		uint64_t size{};
		QDateTime tLastWrite;
		std::vector<std::unique_ptr<sNode>> children;	// sorted. folders first.
		std::vector<sNode*> visible;	// filtered children
	};
	struct sArchive {
		QPersistentModelIndex index;	// archive file (column 0)
		QString key;					// base_t::filePath()
		std::filesystem::path path;
		bool bLoaded{};
		qint64 sizeFile{};
		QDateTime tLastModified;
		QString error;
		sNode root;
	};

	std::map<QString, std::unique_ptr<sArchive>> m_archives;	// key : base_t::filePath()
	std::unordered_set<void const*> m_nodes;	// all virtual nodes (except archive root)
	std::vector<std::unique_ptr<sArchive>> m_trash;		// freed later (next event loop)

public:
	QArchiveFileSystemModel(QObject* parent = nullptr);
	~QArchiveFileSystemModel();

public:
	// QAbstractItemModel
	QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const override;
	QModelIndex parent(QModelIndex const& child) const override;
	int rowCount(QModelIndex const& parent = QModelIndex()) const override;
	bool hasChildren(QModelIndex const& parent = QModelIndex()) const override;
	bool canFetchMore(QModelIndex const& parent) const override;
	void fetchMore(QModelIndex const& parent) override;
	QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
	bool setData(QModelIndex const& index, QVariant const& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(QModelIndex const& index) const override;
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
	QMimeData* mimeData(QModelIndexList const& indexes) const override;
	bool dropMimeData(QMimeData const* data, Qt::DropAction action, int row, int column, QModelIndex const& parent) override;

public:
	// hiding QFileSystemModel's (non-virtual)

	/// @brief path : real path or virtual path ("D:/a.zip/sub/img.png"). loads archive if needed.
	QModelIndex index(QString const& path, int column = 0) const;
	/// @brief virtual path -> watches archive's folder. returns index of path.
	QModelIndex setRootPath(QString const& path);
	QString filePath(QModelIndex const& index) const;
	QString fileName(QModelIndex const& index) const;
	bool isDir(QModelIndex const& index) const;
	qint64 size(QModelIndex const& index) const;
	QDateTime lastModified(QModelIndex const& index) const;
	QFileInfo fileInfo(QModelIndex const& index) const;	// empty for virtual node

public:
	/// @brief index of an entry in archive
	bool IsVirtual(QModelIndex const& index) const;
	/// @brief index of an archive file
	bool IsArchive(QModelIndex const& index) const;
	/// @brief {archive file, path in archive} for archive file or virtual node. (path in archive is empty for archive file)
	std::optional<std::pair<std::filesystem::path, std::filesystem::path>> GetArchivePath(QModelIndex const& index) const;
	/// @brief loads (or reloads if bReload) archive. index : archive file.
	bool LoadArchive(QModelIndex const& index, bool bReload = false);

signals:
	void archiveLoadFailed(QString const& path, QString const& message);

protected:
	sNode* GetNode(QModelIndex const& index) const;
	sArchive* FindArchive(QModelIndex const& index) const;	// index : archive file. (base index)
	QModelIndex CreateIndex(sNode* node, int column) const;
	void UnloadArchive(sArchive& archive);
	void Trash(std::unique_ptr<sArchive> archive);

	void OnDataChanged(QModelIndex const& topLeft, QModelIndex const& bottomRight);
	void OnRowsRemoved();
	void OnModelReset();
};
