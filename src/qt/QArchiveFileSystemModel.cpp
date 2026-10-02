#include "pch.h"
#include "gtl/qt/QArchiveFileSystemModel.h"
#include "gtl/qt/util.h"
#include "gtl/archive.h"
#include <QCollator>
#include <QMimeData>

//////////////////////////////////////////////////////////////////////
//
// QArchiveFileSystemModel.cpp
//
// PWH
// 2026-10-03
//
// QFileSystemModel assumes every index points to its own (private) node.
//  -> every virtual function that can receive an index is overridden here,
//  -> sort() parks virtual persistent indexes on their archive index while QFileSystemModel::sort() remaps persistent indexes.
//
//////////////////////////////////////////////////////////////////////

namespace gtl::qt {

namespace fs = std::filesystem;

namespace {

	constexpr int s_columnParked = 0x10000;	// sort() : column marker for parked virtual indexes

	struct sFilter {
		bool bDirs{}, bFiles{};
		std::vector<QRegularExpression> names;

		bool Accept(bool bDir, QString const& name) const {
			if (bDir)
				return bDirs;
			if (!bFiles)
				return false;
			if (names.empty())
				return true;
			return std::ranges::any_of(names, [&](auto const& re) { return re.match(name).hasMatch(); });
		}
	};

	QString FolderTypeName() {
	#ifdef Q_OS_WIN
		return QCoreApplication::translate("QAbstractFileIconProvider", "File Folder", "Match Windows Explorer");
	#else
		return QCoreApplication::translate("QAbstractFileIconProvider", "Folder", "All other platforms");
	#endif
	}

	QDateTime ToQDateTime(std::chrono::system_clock::time_point t) {
		if (t == std::chrono::system_clock::time_point{})
			return {};
		return QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count());
	}

}	// anonymous namespace

QArchiveFileSystemModel::QArchiveFileSystemModel(QObject* parent) : base_t(parent) {
	connect(this, &QAbstractItemModel::dataChanged, this, &this_t::OnDataChanged);
	connect(this, &QAbstractItemModel::rowsRemoved, this, &this_t::OnRowsRemoved);
	connect(this, &QAbstractItemModel::modelReset, this, &this_t::OnModelReset);
}

QArchiveFileSystemModel::~QArchiveFileSystemModel() {
}

//-----------------------------------------------------------------------------
// helpers

auto QArchiveFileSystemModel::GetNode(QModelIndex const& index) const -> sNode* {
	if (!index.isValid() or index.model() != this)
		return nullptr;
	auto* p = index.internalPointer();
	if (!m_nodes.contains(p))
		return nullptr;
	return static_cast<sNode*>(p);
}

bool QArchiveFileSystemModel::IsVirtual(QModelIndex const& index) const {
	return GetNode(index) != nullptr;
}

bool QArchiveFileSystemModel::IsArchive(QModelIndex const& index) const {
	if (!index.isValid() or index.model() != this or GetNode(index))
		return false;
	if (base_t::isDir(index))
		return false;
	return gtl::IsArchiveFile(ToWString(base_t::fileName(index)));
}

auto QArchiveFileSystemModel::FindArchive(QModelIndex const& index) const -> sArchive* {
	if (m_archives.empty() or !IsArchive(index))
		return nullptr;
	auto iter = m_archives.find(base_t::filePath(index));
	return iter == m_archives.end() ? nullptr : iter->second.get();
}

QModelIndex QArchiveFileSystemModel::CreateIndex(sNode* node, int column) const {
	if (!node)
		return {};
	if (!node->parent)	// archive root
		return column == 0 ? QModelIndex(node->archive->index) : node->archive->index.sibling(node->archive->index.row(), column);
	return createIndex(node->row, column, node);
}

std::optional<std::pair<std::filesystem::path, std::filesystem::path>> QArchiveFileSystemModel::GetArchivePath(QModelIndex const& index) const {
	if (auto* node = GetNode(index))
		return std::pair{node->archive->path, node->path};
	if (IsArchive(index))
		return std::pair{fs::path(ToWString(base_t::filePath(index))), fs::path{}};
	return std::nullopt;
}

bool QArchiveFileSystemModel::LoadArchive(QModelIndex const& index0, bool bReload) {
	if (!IsArchive(index0))
		return false;
	QModelIndex const index = index0.siblingAtColumn(0);
	auto const key = base_t::filePath(index);
	auto& rArchive = m_archives[key];
	if (rArchive and rArchive->bLoaded) {
		if (!bReload)
			return rArchive->error.isEmpty();
		UnloadArchive(*rArchive);
	}
	if (!rArchive) {
		rArchive = std::make_unique<sArchive>();
		rArchive->key = key;
		rArchive->path = ToWString(key);
		rArchive->root.archive = rArchive.get();
		rArchive->root.bDir = true;
	}
	auto& archive = *rArchive;
	archive.index = index;
	archive.sizeFile = base_t::size(index);
	archive.tLastModified = base_t::lastModified(index);
	archive.error.clear();

	std::expected<std::vector<sArchiveEntry>, std::string> entries;
	{
		xWaitCursor wc;
		entries = gtl::ListArchive(archive.path);
	}
	if (!entries) {
		archive.bLoaded = true;
		archive.error = ToQString(entries.error());
		emit dataChanged(index, index);	// no more expand arrow
		emit archiveLoadFailed(key, archive.error);
		return false;
	}

	// filter
	sFilter filter;
	{
		auto const f = this->filter();
		filter.bDirs = (f & (QDir::Dirs | QDir::AllDirs)) != 0;
		filter.bFiles = (f & QDir::Files) != 0;
		if (!nameFilterDisables()) {
			for (auto const& name : nameFilters())
				filter.names.emplace_back(QRegularExpression::wildcardToRegularExpression(name), QRegularExpression::CaseInsensitiveOption);
		}
	}

	// build tree
	std::map<std::wstring, sNode*> dirs;
	auto GetDir = [&](this auto&& self, fs::path const& path) -> sNode* {
		if (path.empty())
			return &archive.root;
		auto key = path.generic_wstring();
		if (auto iter = dirs.find(key); iter != dirs.end())
			return iter->second;
		auto* parent = self(path.parent_path());
		auto node = std::make_unique<sNode>();
		node->archive = &archive;
		node->parent = parent;
		node->bDir = true;
		node->name = ToQString(path.filename().wstring());
		node->path = path;
		auto* p = parent->children.emplace_back(std::move(node)).get();
		dirs[key] = p;
		return p;
	};
	for (auto const& entry : *entries) {
		if (entry.bDir) {
			auto* node = GetDir(entry.path);
			node->tLastWrite = ToQDateTime(entry.tLastWrite);
			continue;
		}
		auto* parent = GetDir(entry.path.parent_path());
		auto node = std::make_unique<sNode>();
		node->archive = &archive;
		node->parent = parent;
		node->name = ToQString(entry.path.filename().wstring());
		node->path = entry.path;
		node->size = entry.size;
		node->tLastWrite = ToQDateTime(entry.tLastWrite);
		parent->children.push_back(std::move(node));
	}

	// sort, filter
	QCollator collator;
	collator.setNumericMode(true);
	collator.setCaseSensitivity(Qt::CaseInsensitive);
	auto Arrange = [&](this auto&& self, sNode& node) -> void {
		std::ranges::stable_sort(node.children, [&](auto const& a, auto const& b) {
			if (a->bDir != b->bDir)
				return a->bDir;
			return collator.compare(a->name, b->name) < 0;
		});
		node.visible.clear();
		for (int i{}; auto& child : node.children) {
			m_nodes.insert(child.get());
			child->row = i++;	// for hidden node
			if (filter.Accept(child->bDir, child->name)) {
				child->row = (int)node.visible.size();
				node.visible.push_back(child.get());
			}
			if (child->bDir)
				self(*child);
		}
	};
	Arrange(archive.root);

	// report
	if (auto n = (int)archive.root.visible.size(); n > 0) {
		beginInsertRows(index, 0, n-1);
		archive.bLoaded = true;
		endInsertRows();
	}
	else {
		archive.bLoaded = true;
	}
	emit dataChanged(index, index);	// updates expand arrow
	return true;
}

void QArchiveFileSystemModel::UnloadArchive(sArchive& archive) {
	if (!archive.bLoaded)
		return;

	// nodes are freed later. (indexes may still be referenced in this event loop)
	auto husk = std::make_unique<sArchive>();
	QModelIndex const index = archive.index;
	if (int n = (int)archive.root.visible.size(); n > 0 and index.isValid()) {
		beginRemoveRows(index, 0, n-1);
		husk->root.children = std::move(archive.root.children);
		archive.root.children.clear();
		archive.root.visible.clear();
		archive.bLoaded = false;
		endRemoveRows();
	}
	else {
		husk->root.children = std::move(archive.root.children);
		archive.root.children.clear();
		archive.root.visible.clear();
		archive.bLoaded = false;
	}
	archive.error.clear();
	Trash(std::move(husk));
	if (index.isValid())
		emit dataChanged(index, index);
}

void QArchiveFileSystemModel::Trash(std::unique_ptr<sArchive> archive) {
	if (!archive)
		return;
	bool const bFirst = m_trash.empty();
	m_trash.push_back(std::move(archive));
	if (!bFirst)
		return;
	QTimer::singleShot(0, this, [this]() {
		auto Forget = [this](this auto&& self, sNode& node) -> void {
			for (auto& child : node.children) {
				m_nodes.erase(child.get());
				self(*child);
			}
		};
		for (auto& archive : m_trash)
			Forget(archive->root);
		m_trash.clear();
	});
}

//-----------------------------------------------------------------------------
// QAbstractItemModel

QModelIndex QArchiveFileSystemModel::index(int row, int column, QModelIndex const& parent) const {
	sNode const* node = GetNode(parent);
	if (!node) {
		if (auto* archive = FindArchive(parent); archive and archive->bLoaded)
			node = &archive->root;
	}
	if (!node)
		return base_t::index(row, column, parent);
	if (parent.column() != 0 or row < 0 or row >= (int)node->visible.size() or column < 0 or column >= columnCount(parent))
		return {};
	return createIndex(row, column, node->visible[row]);
}

QModelIndex QArchiveFileSystemModel::parent(QModelIndex const& child) const {
	auto* node = GetNode(child);
	if (!node)
		return base_t::parent(child);
	auto* parent = node->parent;
	if (!parent)
		return {};
	return CreateIndex(parent, 0);
}

int QArchiveFileSystemModel::rowCount(QModelIndex const& parent) const {
	if (parent.column() > 0)
		return 0;
	if (auto* node = GetNode(parent))
		return (int)node->visible.size();
	if (auto* archive = FindArchive(parent))
		return archive->bLoaded ? (int)archive->root.visible.size() : 0;
	return base_t::rowCount(parent);
}

bool QArchiveFileSystemModel::hasChildren(QModelIndex const& parent) const {
	if (parent.column() > 0)
		return false;
	if (auto* node = GetNode(parent))
		return node->bDir and !node->visible.empty();
	if (IsArchive(parent)) {
		auto* archive = FindArchive(parent);
		return !archive or !archive->bLoaded or !archive->root.visible.empty();	// not loaded yet : assume it has children
	}
	return base_t::hasChildren(parent);
}

bool QArchiveFileSystemModel::canFetchMore(QModelIndex const& parent) const {
	if (GetNode(parent))
		return false;
	if (IsArchive(parent)) {
		auto* archive = FindArchive(parent);
		return !archive or !archive->bLoaded;
	}
	return base_t::canFetchMore(parent);
}

void QArchiveFileSystemModel::fetchMore(QModelIndex const& parent) {
	if (GetNode(parent))
		return;
	if (IsArchive(parent)) {
		LoadArchive(parent);
		return;
	}
	base_t::fetchMore(parent);
}

QVariant QArchiveFileSystemModel::data(QModelIndex const& index, int role) const {
	auto* node = GetNode(index);
	if (!node)
		return base_t::data(index, role);

	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
		switch (index.column()) {
		case 0: return node->name;
		case 1: return node->bDir ? QString() : QLocale::system().formattedDataSize((qint64)node->size);
		case 2:
			if (node->bDir)
				return FolderTypeName();
			if (auto* ip = iconProvider())
				return ip->type(QFileInfo(node->name));
			return {};
		case 3: return node->tLastWrite.isValid() ? QLocale::system().toString(node->tLastWrite, QLocale::ShortFormat) : QString();
		}
		break;
	case Qt::DecorationRole:
		if (index.column() == 0) {
			if (auto* ip = iconProvider()) {
				if (node->bDir)
					return ip->icon(QAbstractFileIconProvider::Folder);
				// icon by extension. (falls back to generic file icon)
				if (auto icon = ip->icon(QFileInfo(filePath(index))); !icon.isNull())
					return icon;
				return ip->icon(QAbstractFileIconProvider::File);
			}
		}
		break;
	case Qt::TextAlignmentRole:
		if (index.column() == 1)
			return QVariant(Qt::AlignTrailing | Qt::AlignVCenter);
		break;
	case FilePathRole:
		return filePath(index);
	case FileNameRole:
		return node->name;
	}
	return {};
}

bool QArchiveFileSystemModel::setData(QModelIndex const& index, QVariant const& value, int role) {
	if (GetNode(index))
		return false;	// read only
	return base_t::setData(index, value, role);
}

Qt::ItemFlags QArchiveFileSystemModel::flags(QModelIndex const& index) const {
	if (auto* node = GetNode(index)) {
		Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		if (!node->bDir)
			f |= Qt::ItemNeverHasChildren;
		return f;
	}
	auto f = base_t::flags(index);
	if (IsArchive(index))
		f &= ~Qt::ItemNeverHasChildren;
	return f;
}

void QArchiveFileSystemModel::sort(int column, Qt::SortOrder order) {
	auto HasVirtual = [this]() {
		for (auto const& idx : persistentIndexList()) {
			if (GetNode(idx))
				return true;
		}
		return false;
	};
	if (!HasVirtual()) {
		base_t::sort(column, order);
		return;
	}

	// QFileSystemModel::sort() casts every persistent index to its own node. park virtual ones on their archive index.
	emit layoutAboutToBeChanged({}, QAbstractItemModel::VerticalSortHint);

	QModelIndexList parked, from, to;
	for (auto const& idx : persistentIndexList()) {
		auto* node = GetNode(idx);
		if (!node)
			continue;
		QModelIndex const idxArchive = node->archive->index;
		from.push_back(idx);
		to.push_back(idxArchive.isValid()
			? createIndex(idxArchive.row(), s_columnParked + (int)parked.size(), idxArchive.internalPointer())
			: QModelIndex{});
		parked.push_back(idx);
	}
	changePersistentIndexList(from, to);

	{
		QSignalBlocker blocker(this);
		base_t::sort(column, order);
	}

	from.clear();
	to.clear();
	for (auto const& idx : persistentIndexList()) {
		if (idx.column() < s_columnParked or GetNode(idx))
			continue;
		if (auto i = idx.column() - s_columnParked; i < parked.size()) {
			from.push_back(idx);
			to.push_back(parked[i]);
		}
	}
	changePersistentIndexList(from, to);

	emit layoutChanged({}, QAbstractItemModel::VerticalSortHint);
}

QMimeData* QArchiveFileSystemModel::mimeData(QModelIndexList const& indexes) const {
	QModelIndexList real;
	for (auto const& idx : indexes) {
		if (!GetNode(idx))
			real.push_back(idx);
	}
	return real.isEmpty() ? nullptr : base_t::mimeData(real);
}

bool QArchiveFileSystemModel::dropMimeData(QMimeData const* data, Qt::DropAction action, int row, int column, QModelIndex const& parent) {
	if (GetNode(parent) or IsArchive(parent))
		return false;
	return base_t::dropMimeData(data, action, row, column, parent);
}

//-----------------------------------------------------------------------------
// QFileSystemModel (hiding)

QModelIndex QArchiveFileSystemModel::index(QString const& path, int column) const {
	auto split = gtl::SplitArchivePath(fs::path(ToWString(path)));
	if (!split)
		return base_t::index(path, column);

	auto const idxArchive = base_t::index(ToQString(split->first), 0);
	if (!IsArchive(idxArchive))
		return {};
	if (split->second.empty())
		return idxArchive.siblingAtColumn(column);

	// loads archive. (QFileSystemModel::index(path) also fetches)
	if (!const_cast<this_t*>(this)->LoadArchive(idxArchive))
		return {};
	auto* archive = FindArchive(idxArchive);
	if (!archive)
		return {};
	sNode* node = &archive->root;
	for (auto const& name : split->second) {
		auto iter = std::ranges::find_if(node->children, [&](auto const& child) { return child->path.filename() == name; });
		if (iter == node->children.end())
			return {};
		node = iter->get();
	}
	return CreateIndex(node, column);
}

QModelIndex QArchiveFileSystemModel::setRootPath(QString const& path) {
	auto split = gtl::SplitArchivePath(fs::path(ToWString(path)));
	if (!split)
		return base_t::setRootPath(path);
	base_t::setRootPath(ToQString(split->first.parent_path()));
	return index(path);
}

QString QArchiveFileSystemModel::filePath(QModelIndex const& index) const {
	if (auto* node = GetNode(index))
		return node->archive->key + u'/' + ToQString(node->path.generic_wstring());
	return base_t::filePath(index);
}

QString QArchiveFileSystemModel::fileName(QModelIndex const& index) const {
	if (auto* node = GetNode(index))
		return node->name;
	return base_t::fileName(index);
}

bool QArchiveFileSystemModel::isDir(QModelIndex const& index) const {
	if (auto* node = GetNode(index))
		return node->bDir;
	return base_t::isDir(index);
}

qint64 QArchiveFileSystemModel::size(QModelIndex const& index) const {
	if (auto* node = GetNode(index))
		return node->bDir ? 0 : (qint64)node->size;
	return base_t::size(index);
}

QDateTime QArchiveFileSystemModel::lastModified(QModelIndex const& index) const {
	if (auto* node = GetNode(index))
		return node->tLastWrite;
	return base_t::lastModified(index);
}

QFileInfo QArchiveFileSystemModel::fileInfo(QModelIndex const& index) const {
	if (GetNode(index))
		return {};
	return base_t::fileInfo(index);
}

//-----------------------------------------------------------------------------
// slots

void QArchiveFileSystemModel::OnDataChanged(QModelIndex const& topLeft, QModelIndex const& bottomRight) {
	if (m_archives.empty() or GetNode(topLeft))
		return;
	// archive file changed -> reload (later. not in the middle of QFileSystemModel's update)
	QStringList keys;
	auto const parent = topLeft.parent();
	for (auto const& [key, archive] : m_archives) {
		QModelIndex const idx = archive->index;
		if (!archive->bLoaded or !idx.isValid() or idx.parent() != parent or idx.row() < topLeft.row() or idx.row() > bottomRight.row())
			continue;
		if (base_t::size(idx) != archive->sizeFile or base_t::lastModified(idx) != archive->tLastModified)
			keys.push_back(key);
	}
	if (keys.isEmpty())
		return;
	QTimer::singleShot(0, this, [this, keys]() {
		for (auto const& key : keys) {
			auto iter = m_archives.find(key);
			if (iter == m_archives.end() or !iter->second->bLoaded)
				continue;
			QModelIndex const idx = iter->second->index;
			if (idx.isValid())
				LoadArchive(idx, true);
		}
	});
}

void QArchiveFileSystemModel::OnRowsRemoved() {
	// archive file removed (persistent indexes under it are already invalidated)
	for (auto iter = m_archives.begin(); iter != m_archives.end(); ) {
		if (!iter->second->index.isValid()) {
			Trash(std::move(iter->second));
			iter = m_archives.erase(iter);
		}
		else
			iter++;
	}
}

void QArchiveFileSystemModel::OnModelReset() {
	for (auto& [key, archive] : m_archives)
		Trash(std::move(archive));
	m_archives.clear();
}

}	// namespace gtl::qt
