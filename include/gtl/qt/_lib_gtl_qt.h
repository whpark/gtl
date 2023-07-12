#pragma once

//////////////////////////////////////////////////////////////////////
//
// _lib_gtl_wx.h : library 관련 헤더
//
// PWH
//
//////////////////////////////////////////////////////////////////////

#include "gtl/__lib.h"

#ifndef GTL__QT_AS_DLL
#	define GTL__QT_AS_DLL 1
#endif

#if (GTL__QT_AS_DLL)
#	ifdef GTL__QT_EXPORTS
#		define GTL__QT_CLASS	GTL__EXPORT
#		define GTL__QT_API		GTL__EXPORT
#		define GTL__QT_DATA		GTL__EXPORT
#	else 
#		define GTL__QT_CLASS	GTL__IMPORT
#		define GTL__QT_API		GTL__IMPORT
#		define GTL__QT_DATA		GTL__IMPORT
		GTL__IMPORT_LIB(gtl.qt)
#	endif
#else	// GTL__QT_AS_DLL
#	define GTL__QT_CLASS
#	define GTL__QT_API
#	define GTL__QT_DATA
#endif	// GTL__QT_AS_DLL
