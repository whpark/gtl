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
#include "gtl/dxf/dxf_reader.h"

namespace gtl::dxf {

	std::optional<gtl::shape::xDrawing> ReadDXFShape(std::filesystem::path const& path) {
		return ReadShape(path);
	}

}	// namespace gtl::dxf
