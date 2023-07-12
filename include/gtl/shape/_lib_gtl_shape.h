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

#ifndef GTL__SHAPE_AS_DLL
#	define GTL__SHAPE_AS_DLL 1
#endif

#if (GTL__SHAPE_AS_DLL)
#	ifdef GTL__SHAPE_EXPORTS
#		define GTL__SHAPE_CLASS		GTL__EXPORT
#		define GTL__SHAPE_API		GTL__EXPORT
#		define GTL__SHAPE_DATA		GTL__EXPORT
#	else 
#		define GTL__SHAPE_CLASS		GTL__IMPORT
#		define GTL__SHAPE_API		GTL__IMPORT
#		define GTL__SHAPE_DATA		GTL__IMPORT
		GTL__IMPORT_LIB(gtl.shape)
#	endif
#else	// GTL__SHAPE_AS_DLL
#	define GTL__SHAPE_CLASS
#	define GTL__SHAPE_API
#	define GTL__SHAPE_DATA
#endif	// GTL__SHAPE_AS_DLL
