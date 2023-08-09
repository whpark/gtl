// win.cpp : Defines the initialization routines for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "gtl/win/win.h"

//#include <Iphlpapi.h>
//#pragma comment(lib, "IPHLPAPI.lib")

//#include <PSApi.h>
//#include <WinSvc.h>
//
//#pragma comment(lib, "PSApi.lib")

WARNING_DISABLE(4018)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace gtl::win {

	xString GetErrorMessage(DWORD dwLastError) {
		xString str;
		LPVOID lpMsgBuf;
		FormatMessageW( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwLastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPWSTR) &lpMsgBuf,
			0,
			NULL 
		);

		str = (LPCWSTR)lpMsgBuf;
		str.TrimRight();

		// Free the buffer.
		LocalFree( lpMsgBuf );

		return str;
	}

}	// namespace gtl::win

