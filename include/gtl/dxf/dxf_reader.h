#pragma once

//////////////////////////////////////////////////////////////////////
//
// dxf_reader.h : light-weight entry point of gtl.dxf
//
// PWH
// 2026-09-02
//
//	gtl/dxf/dxf.h is a header-only implementation (a lot of templates).
//	include THIS header instead if all you need is 'dxf file -> gtl::shape::xDrawing'.
//	(link : gtl.dxf)
//
//////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <optional>

#include "gtl/dxf/_lib_gtl_dxf.h"

// NOTE : gtl/iconv_wrapper.h (pulled in by gtl/shape/shape.h) uses gtl::xStringLiteral
//        without including gtl/misc.h, so misc.h must come first.
#include "gtl/misc.h"
#include "gtl/shape/shape.h"

namespace gtl::dxf {

	/// @brief reads a dxf file (ascii / binary / binary pre-R14) and builds a drawing.
	/// @return std::nullopt if the file could not be read.
	GTL__DXF_API std::optional<gtl::shape::xDrawing> ReadDXFShape(std::filesystem::path const& path);

}	// namespace gtl::dxf
