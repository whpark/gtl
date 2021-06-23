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

#ifndef _GTL_WINUTIL_AS_DLL
#	define _GTL_WINUTIL_AS_DLL 1
#endif

#if (_GTL_WINUTIL_AS_DLL)
#	ifdef GTL_WINUTIL_EXPORTS
#		define GTL_WINUTIL_CLASS	GTL_EXPORT
#		define GTL_WINUTIL_API		GTL_EXPORT
#		define GTL_WINUTIL_DATA		GTL_EXPORT
#	else 
#		define GTL_WINUTIL_CLASS	GTL_IMPORT
#		define GTL_WINUTIL_API		GTL_IMPORT
#		define GTL_WINUTIL_DATA		GTL_IMPORT
		GTL_IMPORT_LIB(win_util)
#	endif
#else	// _GTL_WINUTIL_AS_DLL
#	define GTL_WINUTIL_CLASS
#	define GTL_WINUTIL_API
#	define GTL_WINUTIL_DATA
#endif	// _GTL_WINUTIL_AS_DLL
