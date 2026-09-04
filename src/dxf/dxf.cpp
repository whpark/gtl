//////////////////////////////////////////////////////////////////////
//
// dxf.cpp : gtl.dxf
//
// PWH
// 2026-09-02 biscuit.dxf -> gtl.dxf
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "gtl/dxf/dxf.h"

namespace gtl::dxf {

	string_t GetHeaderString(xSectionHead const& header, string_t const& name) {
		auto const& vars = header.m_mapVariables;			// const -> TContainerMap::find() does NOT insert
		if (auto iter = vars.find(name); iter != vars.end()) {
			for (auto const& group : iter->second) {
				if (auto v = group.GetValue<string_t>())
					return *v;
			}
		}
		return {};
	}

	gtl::shape::xDrawing ToShape(xDXF const& dxf) {
		auto const& header = static_cast<xSectionHead const&>(dxf);
		auto const eCodepage = detail::CodepageOfDXF(GetHeaderString(header, "$ACADVER"), GetHeaderString(header, "$DWGCODEPAGE"));
		return detail::ConvertToShape(
			static_cast<xSectionTables const&>(dxf),
			static_cast<xSectionBlocks const&>(dxf),
			static_cast<xSectionEntities const&>(dxf),
			eCodepage);
	}

	std::optional<gtl::shape::xDrawing> ReadDXFShape(std::filesystem::path const& path) {
		xDXF dxf;
		if (!dxf.ReadDXF(path))
			return std::nullopt;
		return ToShape(dxf);
	}

	bool xDXF::ReadDXF(std::filesystem::path const& path) {
		if (auto g = ReadGroups(path); g)
			m_groups = std::move(*g);
		else
			return false;

		// TopMost. Read SECTIONs
		auto& groups = m_groups;

		// NOTE : biscuit uses TContainerMap here. gtl::TContainerMap::find() inserts a new item
		//        when the key is missing, so a plain std::map is used instead.
		std::map<std::string, std::function<bool(this_t& self, group_iter_t& iter)>> mapReader;
		mapReader["HEADER"]			= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionHead&>(self), iter); };
		mapReader["CLASSES"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionClasses&>(self), iter); };
		mapReader["TABLES"]			= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionTables&>(self), iter); };
		mapReader["BLOCKS"]			= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionBlocks&>(self), iter); };
		mapReader["ENTITIES"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionEntities&>(self), iter); };
		mapReader["OBJECTS"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionObjects&>(self), iter); };
		mapReader["THUMBNAILIMAGE"] = [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionThumbnailImage&>(self), iter); };
		mapReader["ACDSDATA"]		= [](this_t& self, group_iter_t& iter) { return ReadSection(static_cast<xSectionACDSDATA&>(self), iter); };

		for (group_iter_t iter{groups}; iter; ) {
			// Read Section Mark
			{
				static sGroup const groupEOF{ 0, "EOF"s };
				static sGroup const groupSectionStart{ 0, "SECTION"s };
				auto const& r = *iter;
				if (r == groupEOF)
					break;
				if (r != groupSectionStart)
					return false;
				if (!++iter)
					break;
			}

			// Read Section Content
			auto const& group = *iter;
			auto section = group.GetValue<string_t>();
			if (group.eCode != 2 or !section) {
				auto pos = iter - groups.cbegin()+1;
				fmt::println("ReadDXF: invalid section name, pos: {}(rec:{}), group: {}", 2*pos, pos, *iter);
				return false;
			}
			if (auto p = mapReader.find(*section); p != mapReader.end() and p->second) {
				auto& reader = p->second;
				auto pos = iter - groups.cbegin()+1;
				if (!++iter or !reader(*this, iter)) {
					auto pos = iter - groups.cbegin()+1;
					fmt::println("ReadDXF Section - Error, pos: {}(rec:{}), group: {}", 2*pos, pos, iter ? *iter : sGroup{});
					return false;
				}
				mapReader.erase(p);
			}
			else {
				auto pos = iter - groups.cbegin()+1;
				fmt::println("ReadDXF Section - Unknown section: {}, pos: {}(rec:{})", *section, 2*pos, pos);
				xSectionUnknown section;
				if (!++iter or !ReadSection(section, iter)) {
					auto pos = iter - groups.cbegin()+1;
					fmt::println("ReadDXF Section - Error, pos: {}(rec:{}), group: {}", 2*pos, pos, iter ? *iter : sGroup{});
					return false;
				}
			}
		}
		//return mapReader.size() <= 1;
		return mapReader.find("ENTITIES") == mapReader.end();
	}


}	// namespace gtl::dxf
