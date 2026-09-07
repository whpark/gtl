#pragma once

//////////////////////////////////////////////////////////////////////
//
// dxf.h : DXF reader. (gtl.dxf)
//
// PWH
// 2026-09-02 ported from biscuit.dxf (module biscuit.dxf) to gtl.dxf (header)
//
//	usage :
//		if (auto drawing = gtl::dxf::ReadShape(path))
//			...
//
//////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>

#include "fmt/format.h"

#include "gtl/dxf/_lib_gtl_dxf.h"
#include "gtl/dxf/group.h"
#include "gtl/dxf/aliases.h"
#include "gtl/dxf/stream.h"
#include "gtl/dxf/entities.h"
#include "gtl/dxf/sections.h"
#include "gtl/shape/shape.h"

namespace gtl::dxf {

	using namespace std::literals;
	using namespace gtl::literals;

	class GTL__DXF_CLASS xDXF :
		public xSectionHead,
		public xSectionClasses,
		public xSectionTables,
		public xSectionBlocks,
		public xSectionEntities,
		public xSectionObjects,
		public xSectionThumbnailImage,
		public xSectionACDSDATA
	{
	public:
		using this_t = xDXF;
	protected:
		groups_t m_groups;

	public:
		auto const& GetGroups() const { return m_groups; }

	public:
		bool ReadDXF(std::filesystem::path const& path);
	};

	/// @brief a string value of a HEADER section variable. ex) GetHeaderString(dxf, "$ACADVER")
	GTL__DXF_API string_t GetHeaderString(xSectionHead const& header, string_t const& name);
	GTL__DXF_API gtl::shape::xDrawing ToShape(xDXF const& dxf);
	GTL__DXF_API std::optional<gtl::shape::xDrawing> ReadDXFShape(std::filesystem::path const& path);

}	// namespace gtl::dxf
