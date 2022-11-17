#pragma once

//////////////////////////////////////////////////////////////////////
//
// ___lib.h : library 관련 설정
//
// PWH
// 2018.12.05.
// 2019.07.24. QL -> GTL
// 2020.11.12.
// 2021.07.05. new naming convention
//
//////////////////////////////////////////////////////////////////////


#define GTL__LIBRARY_2020			// 그냥.


//-----------------------------------------------------------------------------
// 32 / 64 bit. DON't USE _X64 flag.  use  :  if constexpr (sizeof(size_t) == sizeof(uint64_t))
#if defined(_M_X64) || defined(_M_IA64) || defined(_M_AMD64) || defined(_WIN64)
#	define _X64 1
#else
#	undef _X64	// NOT _X64 0
#endif


//-----------------------------------------------------------------------------
// _MSC_VER
// Visual C++ 4.x (4.0)          1000
// Visual C++ 5    (5.0)         1100
// Visual C++ 6    (6.0)         1200
// Visual C++ .NET (7.0)         1300
// Visual C++ .NET 2003 (7.1)    1310
// Visual C++ .NET 2005 (8.0)    1400
// Visual C++ .NET 2008 (9.0)    1500
// Visual C++ 2010 (10.0)        1600
// Visual C++ 2012 (11.0)        1700
// Visual C++ 2013 (12.0)        1800
// Visual C++ 2015 (14.0)        1900
// Visual C++ 2017 (14.1)        1910
#define _MSC_VER_VS4		1000
#define _MSC_VER_VS5		1100
#define _MSC_VER_VS6		1200
#define _MSC_VER_VS2005		1400
#define _MSC_VER_VS2010		1600
#define _MSC_VER_VS2013		1800
#define _MSC_VER_VS2015		1900
#define _MSC_VER_VS2017		1910
#define _MSC_VER_VS2019		1920
#define _MSC_VER_VS2022		1930


//-----------------------------------------------------------------------------
// Compiler Directives

#if (1)
#	define WARNING_DISABLE(id)				__pragma(warning( disable : id ))
#	define WARNING_PUSH()					__pragma(warning( push ))
#	define WARNING_PUSH_AND_DISABLE(id)		__pragma(warning( push )) __pragma(warning( disable : id ))
#	define WARNING_POP()					__pragma(warning(pop))
#else
#	define WARNING_DISABLE(id)
#	define WARNING_PUSH()
#	define WARNING_PUSH_AND_DISABLE(id)
#	define WARNING_POP()
#endif


#ifndef GTL__DEPRECATED
#	ifdef _DEBUG
#		define GTL__DEPRECATED __declspec(deprecated)
#	else
#		define GTL__DEPRECATED
#	endif
#endif


//-----------------------------------------------------------------------------
// library
#ifndef GTL__EXPORT
#	define GTL__EXPORT __declspec(dllexport)
#endif
#ifndef GTL__IMPORT
#	define GTL__IMPORT __declspec(dllimport)
#endif


//-----------------------------------------------------------------------------
// library name
#if (_MSC_VER >= _MSC_VER_VS2022)
#	define GTL__PLATFORM_TOOLSET_VER_ "v143"
#elif (_MSC_VER >= _MSC_VER_VS2019)
#	define GTL__PLATFORM_TOOLSET_VER_ "v142"
#elif (_MSC_VER >= _MSC_VER_VS2017)
#	define GTL__PLATFORM_TOOLSET_VER_ "v141"
#elif (_MSC_VER >= _MSC_VER_VS2015)
#	define GTL__PLATFORM_TOOLSET_VER_ "v140"
#elif (_MSC_VER >= _MSC_VER_VS2013)
#	define GTL__PLATFORM_TOOLSET_VER_ "v120"
#elif (_MSC_VER >= _MSC_VER_VS2010)
#	define GTL__PLATFORM_TOOLSET_VER_ "v100"
#else
#	define GTL__PLATFORM_TOOLSET_VER_
#endif

#if _X64
#	define GTL__PLATFORM_MACHINE_ "x64"
#else
#	define GTL__PLATFORM_MACHINE_ "x32"
#endif
#ifdef _DEBUG
#	define GTL__PLATFORM_CONFIGURE_ "d"
#else
#	define GTL__PLATFORM_CONFIGURE_ "r"
#endif
#ifdef _UNICODE
#	define GTL__PLATFORM_CHARSET_ ""
#else
#	define GTL__PLATFORM_CHARSET_ "A"
#endif

#define GTL__IMPORT_LIB(name) \
	__pragma(comment(lib, #name "." GTL__PLATFORM_MACHINE_ GTL__PLATFORM_CHARSET_ GTL__PLATFORM_CONFIGURE_ ".lib" ))
#define GTL__IMPORT_LIB__TOOLSET(name) \
	__pragma(comment(lib, #name "." GTL__PLATFORM_TOOLSET_VER_ "." GTL__PLATFORM_MACHINE_  GTL__PLATFORM_CHARSET_ GTL__PLATFORM_CONFIGURE_ ".lib" ))

//-----------------------------------------------------------------------------

