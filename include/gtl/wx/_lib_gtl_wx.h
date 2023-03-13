#pragma once

//////////////////////////////////////////////////////////////////////
//
// _lib_gtl_wx.h : library 관련 헤더
//
// PWH
//
//////////////////////////////////////////////////////////////////////

#include "gtl/__lib.h"

#ifndef GTL__WX_AS_DLL
#	define GTL__WX_AS_DLL 1
#endif

#if (GTL__WX_AS_DLL)
#	ifdef GTL__WX_EXPORTS
#		define GTL__WX_CLASS	GTL__EXPORT
#		define GTL__WX_API		GTL__EXPORT
#		define GTL__WX_DATA		GTL__EXPORT
#	else 
#		define GTL__WX_CLASS	GTL__IMPORT
#		define GTL__WX_API		GTL__IMPORT
#		define GTL__WX_DATA		GTL__IMPORT
		GTL__IMPORT_LIB(wx)
#	endif
#else	// GTL__WX_AS_DLL
#	define GTL__WX_CLASS
#	define GTL__WX_API
#	define GTL__WX_DATA
#endif	// GTL__WX_AS_DLL
