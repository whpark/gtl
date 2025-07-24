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

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some xString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxbutton.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#include <afxdialogex.h>

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include <ATLImage.h>

