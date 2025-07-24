#pragma once

//////////////////////////////////////////////////////////////////////
//
// _lib_win.h : library 관련 헤더
//
// PWH
// 2023-08-09. win_util 에서 MFC 제거하고 Windows api 관련 루틴만 사용
//
//////////////////////////////////////////////////////////////////////

#include "gtl/__lib.h"

#ifndef GTL__WIN_AS_DLL
#	define GTL__WIN_AS_DLL 1
#endif

#if (GTL__WIN_AS_DLL)
#	ifdef GTL__WIN_EXPORTS
#		define GTL__WIN_CLASS	GTL__EXPORT
#		define GTL__WIN_API		GTL__EXPORT
#		define GTL__WIN_DATA	GTL__EXPORT
#	else 
#		define GTL__WIN_CLASS	GTL__IMPORT
#		define GTL__WIN_API		GTL__IMPORT
#		define GTL__WIN_DATA	GTL__IMPORT
		GTL__IMPORT_LIB(gtl.win)
#	endif
#else	// GTL__WIN_AS_DLL
#	define GTL__WIN_CLASS
#	define GTL__WIN_API
#	define GTL__WIN_DATA
#endif	// GTL__WIN_AS_DLL


#if (!GTL__USE_WINDOWS_API)
#	ifndef NOMINMAX
#		define NOMINMAX	// disable Windows::min/max
#	endif
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers
#	endif 
#	pragma warning(push)
#	pragma warning(disable: 5104 5105)
#	include <windows.h>
#	pragma warning(pop)
#endif
