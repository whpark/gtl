#pragma once

//////////////////////////////////////////////////////////////////////
//
// _lib_gtl_dxf.h : library 관련 헤더
//
// PWH
// 2026-09-02 biscuit.dxf -> gtl.dxf
//
//////////////////////////////////////////////////////////////////////

#include "gtl/_config.h"
#include "gtl/__lib.h"

// NOTE : gtl/dxf/*.h is a header-only implementation. the DLL only provides the
//        light-weight entry point declared in gtl/dxf/dxf_reader.h.
#ifndef GTL__DXF_AS_DLL
#	define GTL__DXF_AS_DLL 1
#endif

#if (GTL__DXF_AS_DLL)
#	ifdef GTL__DXF_EXPORTS
#		define GTL__DXF_CLASS		GTL__EXPORT
#		define GTL__DXF_API			GTL__EXPORT
#		define GTL__DXF_DATA		GTL__EXPORT
#	else
#		define GTL__DXF_CLASS		GTL__IMPORT
#		define GTL__DXF_API			GTL__IMPORT
#		define GTL__DXF_DATA		GTL__IMPORT
		GTL__IMPORT_LIB(gtl.dxf)
#	endif
#else	// GTL__DXF_AS_DLL
#	define GTL__DXF_CLASS
#	define GTL__DXF_API
#	define GTL__DXF_DATA
#endif	// GTL__DXF_AS_DLL
