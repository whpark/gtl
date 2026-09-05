#pragma once

#include "gtl/_config.h"
#include "gtl/__lib.h"

#ifndef GTL__DWG_AS_DLL
#	define GTL__DWG_AS_DLL 1
#endif

#if GTL__DWG_AS_DLL
#	ifdef GTL__DWG_EXPORTS
#		define GTL__DWG_CLASS GTL__EXPORT
#		define GTL__DWG_API GTL__EXPORT
#	else
#		define GTL__DWG_CLASS GTL__IMPORT
#		define GTL__DWG_API GTL__IMPORT
		GTL__IMPORT_LIB(gtl.dwg)
#	endif
#else
#	define GTL__DWG_CLASS
#	define GTL__DWG_API
#endif
