#pragma once

//////////////////////////////////////////////////////////////////////
//
// sections.h : DXF sections (HEADER/CLASSES/TABLES/BLOCKS/ENTITIES/...)
//
// PWH
// 2026-09-02 ported from biscuit.dxf (module :sections) to gtl.dxf (header)
//
//////////////////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "gtl/container_map.h"
#include "gtl/misc.h"

#include "gtl/dxf/group.h"
#include "gtl/dxf/aliases.h"
#include "gtl/dxf/stream.h"
#include "gtl/dxf/entities.h"

namespace gtl::dxf {

	//=============================================================================================================================

	template < typename TSection >
	bool ReadSection(TSection& section, group_iter_t& iter) {
		if (!section.InitSection())
			return false;

		for (; iter; ) {
			static sGroup const groupEndSection{ 0, "ENDSEC"s};
			auto const& r = *iter;
			if (r == groupEndSection) {
				iter++;
				return true;
			}
			auto iterP = iter;
			if (!section.ReadSectionItem(iter))
				return false;
		}
		return false;
	}

	//=============================================================================================================================
	// Header section
	// NOTE : gtl::TContainerMap takes the container as its FIRST template argument. (biscuit::TContainerMap takes it last)
	using variable_map_t = gtl::TContainerMap<std::vector, string_t, std::vector<sGroup>>;
	//-----------------------------------------------------------------------------------------------------------------------------
	class xSectionHead {
	public:
		using this_t = xSectionHead;

		variable_map_t m_mapVariables;
	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			m_mapVariables.clear();
			return true;
		}

		//template < typename group_iter_t >
		bool ReadSectionItem(group_iter_t& iter) {
			auto const& r = *iter;
			if (r.eCode != eGROUP_CODE::variable)
				return false;
			auto str = r.GetValue<string_t>();
			if (!str)
				return false;
			auto& sub = m_mapVariables[*str];
			for (iter++; iter; iter++) {
				auto const& item = *iter;
				if ((item.eCode == eGROUP_CODE::entity) or (item.eCode == eGROUP_CODE::variable))
					break;
				sub.push_back(item);
			}
			return true;
		}
	};

	//=============================================================================================================================
	// Classes Section
	struct sClass {
		using this_t = sClass;
		gcv<  1> name;
		gcv<  2> cppClassName;
		gcv<  3> appName;
		gcv< 90> flags{};
		gcv< 91> count{};
		gcv<280> proxy{};
		gcv<281> entity{};

		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	using classes_t = std::vector<sClass>;

	//-----------------------------------------------------------------------------------------------------------------------------
	class xSectionClasses {
	public:
		using this_t = xSectionClasses;

		classes_t m_classes;
	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			m_classes.clear();
			return true;
		}
		bool ReadSectionItem(group_iter_t& iter) {
			auto const& r = *iter;
			static const sGroup groupClass{ 0, "CLASS"s };
			if (r != groupClass)
				return false;
			if (!++iter)
				return false;
			m_classes.emplace_back();
			auto& aClass = m_classes.back();
			if (!entities::ReadFieldMembers(aClass, iter))
				return false;
			return true;
		}
	};

	//=============================================================================================================================

	struct sTableHeader {
		using this_t = sTableHeader;
		gcv<  2> tableType;
		gcv<  5> handle{};
		std::vector<entities::sControl> controls;	// 360
		gcv<330> pOwnerObject;		// Soft-pointer ID/handle to owner object (optional)
		entities::sAcDbSymbolTable tableSymbol;
		entities::sAcDbDimStyleTable tableDimStyle;
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	struct sTableItemHeader {
		using this_t = sTableItemHeader;
		gcv<  0> entityType;		// APPID, BLOCK_RECORD, DIMSTYLE, LAYER, LTYPE, STYLE, UCS, VIEW, VPORT
		gcv<  5> handle;
		gcv<105> handleDimStyle;
		std::vector<entities::sControl> controls;
		gcv<330> pOwnerObject;				// Soft-pointer ID/handle to owner object (optional)
		gcv<100> markerEnd{"AcDbSymbolTableRecord"s};

		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	template < typename T, xStringLiteral name >
	class TTableEntry {
	public:
		using this_t = TTableEntry;
		static inline std::string const s_tableName = name.str;
	public:
		sTableHeader header;
		std::vector<std::pair<sTableItemHeader, T>> items;

		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		void Init() {
			header = sTableHeader{};
			items.clear();
		}

		bool ReadTableEntry(group_iter_t& iter) {
			auto const& r = *iter;
			if (r.eCode != header.tableType.eCode or r.value != s_tableName)
				return false;
			if (!entities::ReadFieldMembers(header, iter))
				return false;
			for (; iter; ) {
				if (iter->eCode != eGROUP_CODE::entity)
					return false;	// not a table entry
				if (iter->value == "ENDTAB"s) {
					iter++;
					return true;
				}
				items.emplace_back();
				auto& itemHeader = items.back().first;
				if (!entities::ReadFieldMembers(itemHeader, iter))
					return false;
				T& item = items.back().second;
				if (!entities::ReadFieldMembers(item, iter))
					return false;
			}
			return false;		// NOT 'true'
		}
	};

	//-----------------------------------------------------------------------------------------------------------------------------
	class xSectionTables {
	public:
		using this_t = xSectionTables;

		TTableEntry<entities::sAcDbRegAppTableRecord, "APPID"> m_appIDs;
		TTableEntry<entities::sAcDbBlockTableRecord, "BLOCK_RECORD"> m_blockRecords;
		TTableEntry<entities::sAcDbDimStyleTableRecord, "DIMSTYLE"> m_dimStyles;
		TTableEntry<entities::sAcDbLayerTableRecord, "LAYER"> m_layers;
		TTableEntry<entities::sAcDbLinetypeTableRecord, "LTYPE"> m_lineTypes;
		TTableEntry<entities::sAcDbTextStyleTableRecord, "STYLE"> m_styles;
		TTableEntry<entities::sAcDbUCSTableRecord, "UCS"> m_ucs;
		TTableEntry<entities::sAcDbViewTableRecord, "VIEW"> m_views;
		TTableEntry<entities::sAcDbViewportTableRecord, "VPORT"> m_vports;

	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			m_appIDs.Init();
			m_blockRecords.Init();
			m_dimStyles.Init();
			m_layers.Init();
			m_lineTypes.Init();
			m_styles.Init();
			m_ucs.Init();
			m_views.Init();
			m_vports.Init();
			return true;
		}
		//template < typename group_iter_t >
		bool ReadSectionItem(group_iter_t& iter) {
			auto const& r = *iter;
			static const sGroup groupTable{ 0, "TABLE"s };
			if (r != groupTable)
				return false;
			if (!++iter)
				return false;

			if (iter->eCode != eGROUP_CODE::table_entry)
				return false;
			auto entry = iter->GetValue<string_t>().value_or(""s);
			if (entry == "APPID"s) {
				if (!m_appIDs.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "BLOCK_RECORD"s) {
				if (!m_blockRecords.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "DIMSTYLE"s) {
				if (!m_dimStyles.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "LAYER"s) {
				if (!m_layers.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "LTYPE"s) {
				if (!m_lineTypes.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "STYLE"s) {
				if (!m_styles.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "UCS"s) {
				if (!m_ucs.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "VIEW"s) {
				if (!m_views.ReadTableEntry(iter))
					return false;
			}
			else if (entry == "VPORT"s) {
				if (!m_vports.ReadTableEntry(iter))
					return false;
			}

			return true;
		}
	};

	//=============================================================================================================================
	struct sBlockStart {
		using this_t = sBlockStart;
		gcv<  5> handle;
		std::vector<entities::sControl> controls;	//
		gcv<330> pOwnerObject;		// Soft-pointer ID/handle to owner object
		entities::sAcDbEntity entity;
		entities::sAcDbBlockBegin block;

		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	struct sBlockEnd {
		using this_t = sBlockEnd;

		gcv<  5> handle;
		std::vector<entities::sControl> controls;
		gcv<330> pOwnerObject;	// Soft-pointer ID/handle to owner object
		entities::sAcDbEntity entity;
		entities::sAcDbBlockEnd blockEnd;	 //gcv<100> markerBlockEnd{"AcDbBlockEnd"s};

		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);
	};

	class xBlock {
	public:
		using this_t = xBlock;
	public:
		sBlockStart header;
		sBlockEnd tail;

		entities::entities_t entities;

		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);
	};
	class xSectionBlocks {
	public:
		using this_t = xSectionBlocks;
		std::vector<xBlock> m_blocks;
	//protected:
	//	binary_t hExit;

	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			m_blocks.clear();
			return true;
		}

		bool ReadSectionItem(group_iter_t& iter) {
			auto const& r = *iter;
			static sGroup const groupBlock{ 0, "BLOCK"s };
			if (r != groupBlock)
				return false;
			if (!++iter)
				return false;

			m_blocks.emplace_back();
			auto& block = m_blocks.back();
			if (!entities::ReadFieldMembers(block.header, iter))
				return false;

			while (iter and iter->eCode == eGROUP_CODE::entity) {
				string_t const* entity_name = std::get_if<string_t>(&iter->value);
				if (!entity_name)
					return false;
				if (!++iter)
					return false;
				if (*entity_name == "ENDBLK"s) {
					return entities::ReadFieldMembers(block.tail, iter);	// normal exits here.
				}
				auto entity = entities::xEntity::CreateEntity(*entity_name);
				if (!entity->Read(iter))
					return false;
				block.entities.push_back(std::move(entity));
			}
			return false;	// NOT true
		}
	};

	//=============================================================================================================================
	class xSectionEntities {
	public:
		using this_t = xSectionEntities;
		entities::entities_t m_entities;

	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			m_entities.clear();
			return true;
		}

		//template < typename group_iter_t >
		bool ReadSectionItem(group_iter_t& iter) {
			auto const& r = *iter;
			auto const* entity_name = std::get_if<string_t>(&r.value);
			if (r.eCode != eGROUP_CODE::entity or !entity_name)
				return false;
			iter++;
			if (auto entity = entities::xEntity::CreateEntity(*entity_name))
				m_entities.push_back(std::move(entity));
			else
				return false;

			auto& entity = *m_entities.back();
			return entity.Read(iter);
		}
	};

	//=============================================================================================================================
	class xSectionObjects {
	public:
		using this_t = xSectionObjects;
	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			return true;
		}

		//template < typename group_iter_t >
		bool ReadSectionItem(group_iter_t& iter) {
			auto const& r = *iter;
			for (; iter; iter++) {
				static sGroup const groupObjectEnd{ 0, "ENDSEC"s };
				if (*iter == groupObjectEnd) {
					return true;
				}
			}
			return false;
		}
	};

	//=============================================================================================================================
	class xSectionThumbnailImage {
	public:
		using this_t = xSectionThumbnailImage;
	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			return true;
		}
		//template < typename group_iter_t >
		bool ReadSectionItem(group_iter_t& iter) {
			for (; iter; iter++) {
				static sGroup const groupThumbnailImageEnd{ 0, "ENDSEC"s };
				if (*iter == groupThumbnailImageEnd) {
					return true;
				}
			}
			return false;
		}
	};

	//=============================================================================================================================
	class xSectionACDSDATA {
	public:
		using this_t = xSectionACDSDATA;
	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			return true;
		}
		//template < typename group_iter_t >
		bool ReadSectionItem(group_iter_t& iter) {
			for (; iter; iter++) {
				static sGroup const groupACDSDATAEnd{ 0, "ENDSEC"s };
				if (*iter == groupACDSDATAEnd) {
					return true;
				}
			}
			return false;
		}
	};


	//=============================================================================================================================
	class xSectionUnknown {
	public:
		using this_t = xSectionUnknown;
	public:
		GTL__DXF_DEFINE_SPACESHIP_OPERATOR(this_t);

		bool InitSection() {
			return true;
		}

		//template < typename group_iter_t >
		bool ReadSectionItem(group_iter_t& iter) {
			auto const& r = *iter;
			for (; iter; iter++) {
				static sGroup const groupObjectEnd{ 0, "ENDSEC"s };
				if (*iter == groupObjectEnd) {
					return true;
				}
			}
			return false;
		}
	};

}	// namespace gtl::dxf
