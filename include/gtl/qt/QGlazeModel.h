#pragma once

#include <QAbstractItemModel>

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/qt/util.h"
#include "glaze/glaze.hpp"

namespace gtl::qt {

class GTL__QT_CLASS QGlazeModel : public QAbstractItemModel {
	Q_OBJECT
public:
	using this_t = QGlazeModel;
	using base_t = QAbstractItemModel;
	using json_t = glz::json_t;
	using decorator_t = std::function<std::optional<std::string>(glz::json_t const&)>;

	enum class eCOLUMN : int { key, value, n, };
	friend constexpr std::string ToStringA(eCOLUMN e) {
		switch (e) {
		case eCOLUMN::key: return "key";
		case eCOLUMN::value: return "value";
		}
		return "";
	}

protected:
	json_t m_json;
	std::map<json_t const*, json_t const*> m_mapChildToParent;
	std::map<std::string, decorator_t> m_mapDecorator;	// var name -> decorator

	size_t GetChildCount(json_t const& j) const {
		if (j.get_if<json_t::null_t>() or j.get_if<double>() or j.get_if<std::string>() or j.get_if<bool>())
			return 0;
		if (auto* p = j.get_if<json_t::array_t>())
			return p->size();
		if (auto* p = j.get_if<json_t::object_t>())
			return p->size();
		return 0;
	}
	json_t const* GetChild(json_t const& j, size_t i) const {
		if (j.get_if<json_t::null_t>() or j.get_if<double>() or j.get_if<std::string>() or j.get_if<bool>())
			return nullptr;
		if (auto* p = j.get_if<json_t::array_t>(); p and i < p->size())
			return &(*p)[i];
		if (auto* p = j.get_if<json_t::object_t>(); p and i < p->size()) {
			// map... no index access.. anyway..
			auto iter = p->begin();
			for (size_t p{}; p < i; p++) {
				iter++;
			}
			return &(iter->second);
		}
		return nullptr;
	}

public:
	QGlazeModel(QObject* parent) {
	}
	~QGlazeModel() {
	}

	bool SetJson(json_t t) {
		beginResetModel();
		m_mapChildToParent.clear();
		m_json.reset();
		endResetModel();

		beginInsertRows({}, 0, GetChildCount(t));
		m_json = std::move(t);
		BuildChildToParentMap(m_mapChildToParent, m_json);
		endInsertRows();

		return true;
	}

	void SetDecorator(std::string const& varname, decorator_t f) {
		m_mapDecorator[varname] = f;
	}
	void SetDecorator(auto const& map) {
		m_mapDecorator = map;
	}
	auto const& GetDecorator() {
		return m_mapDecorator;
	}
	void ResetDecorator() {
		m_mapDecorator.clear();
	}

protected:
	void BuildChildToParentMap(std::map<json_t const*, json_t const*>& map, json_t const& j) {
		if (j.get_if<json_t::null_t>() or j.get_if<double>() or j.get_if<std::string>() or j.get_if<bool>())
			return ;
		if (auto* p = j.get_if<json_t::array_t>()) {
			for (auto const& c : *p) {
				map[&c] = &j;
				BuildChildToParentMap(map, c);
			}
		}
		if (auto* p = j.get_if<json_t::object_t>()) {
			for (auto const& [k, c] : *p) {
				map[&c] = &j;
				BuildChildToParentMap(map, c);
			}
		}
	}

public:
	auto const& GetRootItem() const { return m_json; }

	auto const* GetItem(QModelIndex const& index) const {
		if (index == QModelIndex())
			return &m_json;
		return std::bit_cast<json_t const*>(index.constInternalPointer());
	}
	auto* GetItem(QModelIndex const& index) {
		if (index == QModelIndex())
			return &m_json;
		return std::bit_cast<json_t*>(index.constInternalPointer());
	}

	//bool hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(int row, int column, const QModelIndex& indexParent = QModelIndex()) const override {
		if (row < 0 or column < 0 or row >= rowCount(indexParent) or column >= columnCount(indexParent))
			return {};

		//std::scoped_lock lock(m_mtx);
		auto* parent = GetItem(indexParent);
		if (!parent)
			parent = &m_json;

		auto* child = GetChild(*parent, row);
		if (!child)
			return {};

		return createIndex(row, column, std::bit_cast<void*>(child));
	}
	QModelIndex parent(const QModelIndex& child) const override {
		//std::scoped_lock lock(m_mtx);
		auto* item = GetItem(child);
		if (!item)
			return {};

		if (auto iter = m_mapChildToParent.find(item); iter != m_mapChildToParent.end()) {
			auto const* parent = iter->second;
			if (!parent or parent == &m_json)
				return {};
			if (auto iterP = m_mapChildToParent.find(parent); iterP != m_mapChildToParent.end()) {
				auto const* grand_parent = iterP->second;
				auto const& s = GetChildCount(*grand_parent);
				for (size_t row{}; row < s; row++) {
					if (GetChild(*grand_parent, row) == parent)
						return createIndex(row, 0, std::bit_cast<void*>(parent));
				}
			}
		}
		return {};
	}

	//QModelIndex sibling(int row, int column, const QModelIndex &idx) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override {
		if (parent.column() > 0)
			return 0;

		if (parent.row() < 0 or !parent.constInternalPointer()) {
			return GetChildCount(m_json);
		}

		//std::scoped_lock lock(m_mtx);
		if (auto* item = GetItem(parent)) {
			return GetChildCount(*item);
		}

		return 0;
	}
	int columnCount(const QModelIndex& parent = QModelIndex()) const override {
		if (parent.column() > 0)
			return 0;
		return std::to_underlying(eCOLUMN::n);
	}
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const override {
		if (auto const* item = GetItem(parent))
			return GetChildCount(*item) > 0;
		return false;
	}

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
		using namespace gtl::qt;
		auto* item = GetItem(index);
		if (!item)
			return {};
		if (role == Qt::DisplayRole) {
			//std::scoped_lock lock(m_mtx);
			switch (index.column()) {
			case std::to_underlying(eCOLUMN::key):
				{
					auto indexParent = parent(index);
					auto* p = GetItem(indexParent);
					if (auto* arr = p->get_if<json_t::array_t>()) {
						auto dist = std::distance(&((*arr)[0]), item);
						return (int)dist+1;
					}
					if (auto* m = p->get_if<json_t::object_t>()) {
						for (auto iter = m->begin(); iter != m->end(); ++iter) {
							if (&iter->second == item) {
								return ToQString(iter->first);
							}
						}
					}
					return "";
				}
				break;
			case std::to_underlying(eCOLUMN::value):
				if (auto const* p = item->get_if<json_t::null_t>())
					return "null";
				if (!m_mapDecorator.empty()) {
					// get variable name
					std::string name;
					for (QModelIndex iCurrent = index, iParent = parent(index);
						iCurrent != QModelIndex();
						iCurrent = iParent, iParent = parent(iCurrent))
					{
						auto* p = GetItem(iParent);
						auto* m = p->get_if<json_t::object_t>();
						if (!m) continue;

						if (auto pos = std::find_if(m->begin(), m->end(),
							[item](auto const& pair) { return &pair.second == item; }); pos != m->end())
						{
							name = pos->first;
							break;
						}
					}
					// if decorator exists
					if (auto iter = m_mapDecorator.find(name); iter != m_mapDecorator.end()) {
						auto func = iter->second;
						if (auto str = func(*item))
							return ToQString(*str);
					}
				}
				if (auto const* p = item->get_if<double>())
					return *p;
				if (auto const* p = item->get_if<std::string>())
					return ToQString(*p);
				if (auto const* p = item->get_if<bool>())
					return *p;
				break;
			}
		}
		if (role == Qt::TextAlignmentRole) {
			switch (index.column()) {
			case std::to_underlying(eCOLUMN::value):
				//if (auto const* p = item->get_if<json_t::null_t>())
				//	return Qt::AlignLeft;
				if (auto const* p = item->get_if<double>())
					return Qt::AlignRight;
				//if (auto const* p = item->get_if<std::string>())
				//	return Qt::AlignLeft;
				//if (auto const* p = item->get_if<bool>())
				//	return Qt::AlignLeft;
			}
		}
		return {};
	}
	//bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
		if (role == Qt::DisplayRole) {
			switch (section) {
			case 0: return "key";
			case 1: return "value";
			}
		}
		return {};
	}
	//bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

	void Clear() {
		beginResetModel();
		m_mapChildToParent.clear();
		m_json.reset();
		endResetModel();
	}
	//bool InsertLeaf(std::unique_ptr<sDuplicatedItem> item) {
	//	if (!item or item->parent != &m_top)
	//		return false;
	//	auto s = m_top.children.size();
	//	auto i = index(-1, -1);
	//	{
	//		std::scoped_lock lock(m_mtx);
	//		beginInsertRows(i, s, s+1);
	//		m_top.children.push_back(std::move(item));
	//		endInsertRows();
	//	}

	//	return true;
	//}

};

}	// namespace gtl::qt
