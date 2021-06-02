#pragma once

//////////////////////////////////////////////////////////////////////
//
// _lib_gtl.h : library 관련 헤더
//
// PWH
// 2018.12.02.
// 2019.07.24. QL -> GTL
//
//////////////////////////////////////////////////////////////////////

#include "gtl/__lib.h"

#ifndef _GTL_SHAPE_AS_DLL
#	define _GTL_SHAPE_AS_DLL 1
#endif

#if (_GTL_SHAPE_AS_DLL)
#	ifdef GTL_SHAPE_EXPORTS
#		define GTL_SHAPE_CLASS		GTL_EXPORT
#		define GTL_SHAPE_API		GTL_EXPORT
#		define GTL_SHAPE_DATA		GTL_EXPORT
#	else 
#		define GTL_SHAPE_CLASS		GTL_IMPORT
#		define GTL_SHAPE_API		GTL_IMPORT
#		define GTL_SHAPE_DATA		GTL_IMPORT
		GTL_IMPORT_LIB(gtl)
#	endif
#else	// _GTL_SHAPE_AS_DLL
#	define GTL_SHAPE_CLASS
#	define GTL_SHAPE_API
#	define GTL_SHAPE_DATA
#endif	// _GTL_SHAPE_AS_DLL
