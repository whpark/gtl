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

#ifndef GTL__MFC_AS_DLL
#	define GTL__MFC_AS_DLL 1
#endif

#if (GTL__MFC_AS_DLL)
#	ifdef GTL__MFC_EXPORTS
#		define GTL__MFC_CLASS	GTL__EXPORT
#		define GTL__MFC_API		GTL__EXPORT
#		define GTL__MFC_DATA	GTL__EXPORT
#	else 
#		define GTL__MFC_CLASS	GTL__IMPORT
#		define GTL__MFC_API		GTL__IMPORT
#		define GTL__MFC_DATA	GTL__IMPORT
		GTL__IMPORT_LIB(gtl.mfc)
#	endif
#else	// GTL__MFC_AS_DLL
#	define GTL__MFC_CLASS
#	define GTL__MFC_API
#	define GTL__MFC_DATA
#endif	// GTL__MFC_AS_DLL
