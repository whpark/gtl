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

#ifndef GTL__AS_DLL
#	define GTL__AS_DLL 1
#endif

#if (GTL__AS_DLL)
#	ifdef GTL__EXPORTS
#		define GTL__CLASS		GTL__EXPORT
#		define GTL__API			GTL__EXPORT
#		define GTL__DATA		GTL__EXPORT
#	else 
#		define GTL__CLASS		GTL__IMPORT
#		define GTL__API			GTL__IMPORT
#		define GTL__DATA		GTL__IMPORT
		GTL__IMPORT_LIB(gtl)
#	endif
#else	// GTL__AS_DLL
#	define GTL__CLASS
#	define GTL__API
#	define GTL__DATA
#endif	// GTL__AS_DLL
