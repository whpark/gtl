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

#include "__lib.h"

#ifndef _GTL_AS_DLL
#	define _GTL_AS_DLL 1
#endif

#if (_GTL_AS_DLL)
#	ifdef GTL_EXPORTS
#		define GTL_CLASS		GTL_EXPORT
#		define GTL_API			GTL_EXPORT
#		define GTL_DATA			GTL_EXPORT
#	else 
#		define GTL_CLASS		GTL_IMPORT
#		define GTL_API			GTL_IMPORT
#		define GTL_DATA			GTL_IMPORT
#	endif
#else	// _GTL_AS_DLL
#	define GTL_CLASS
#	define GTL_API
#	define GTL_DATA
#endif	// _GTL_AS_DLL

#if (_GTL_AS_DLL) && !defined(GTL_EXPORTS)
	GTL_IMPORT_LIB(gtl)
#endif
