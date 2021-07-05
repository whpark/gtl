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

#ifndef GTL__WINUTIL_AS_DLL
#	define GTL__WINUTIL_AS_DLL 1
#endif

#if (GTL__WINUTIL_AS_DLL)
#	ifdef GTL__WINUTIL_EXPORTS
#		define GTL__WINUTIL_CLASS	GTL__EXPORT
#		define GTL__WINUTIL_API		GTL__EXPORT
#		define GTL__WINUTIL_DATA		GTL__EXPORT
#	else 
#		define GTL__WINUTIL_CLASS	GTL__IMPORT
#		define GTL__WINUTIL_API		GTL__IMPORT
#		define GTL__WINUTIL_DATA		GTL__IMPORT
		GTL__IMPORT_LIB(win_util)
#	endif
#else	// GTL__WINUTIL_AS_DLL
#	define GTL__WINUTIL_CLASS
#	define GTL__WINUTIL_API
#	define GTL__WINUTIL_DATA
#endif	// GTL__WINUTIL_AS_DLL
