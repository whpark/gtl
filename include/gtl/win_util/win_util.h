#pragma once

//////////////////////////////////////////////////////////////////////
//
// mfc_util.h: from Mocha - xUtil.h
//
// PWH
// 2000.????
// 2021.06.10 gtl.
//
///////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some xString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

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


//#include "framework.h"
#include "_lib_gtl_win_util.h"
#include "gtl/gtl.h"

namespace gtl::win_util {
#pragma pack(push, 8)	// default align. (8 bytes)

	GTL__WINUTIL_API gtl::xString GetErrorMessage(CException& e);
	GTL__WINUTIL_API gtl::xString GetErrorMessage(DWORD dwLastError);

	//-------------------------------------------------------------------------
	/// @brief StopWatch
	/// @tparam tclock 
	template < typename tchar, class ttraits = std::char_traits<tchar> >
	struct TDebugOutputStreamBuf : public std::basic_streambuf<tchar, ttraits> {
	public:
		using base_t = std::basic_streambuf<tchar, ttraits>;

		std::basic_string<tchar> str;

		virtual ~TDebugOutputStreamBuf() {
			OutputBuf();
		}
		void OutputBuf() {
			if (str.empty())
				return;
			if constexpr (gtlc::is_one_of<tchar, wchar_t, char16_t>) {
				OutputDebugStringW((wchar_t const*)str.c_str());
			} else if constexpr (gtlc::is_one_of<tchar, char, char8_t>) {
				OutputDebugStringA((char const*)str.c_str());
			} else {
				static_assert(false);
			}
		}
		virtual base_t::int_type overflow(base_t::int_type c) override {
			str += (tchar)c;
			if (c == '\n') {
				OutputBuf();
				str.clear();
			}
			return 1;//traits_type::eof();
		}
	};

	//-------------------------------------------------------------------------
	/// @brief StopWatch
	template < typename tchar, typename tresolution = std::chrono::duration<double> >
	class TStopWatch : public gtl::TStopWatch<tchar, tresolution > {
	public:
		static inline TDebugOutputStreamBuf<tchar> osbuf;
		static inline std::basic_ostream<tchar> os{&osbuf};	// !! Destruction Order ... (<- static inline)
		using base_t = gtl::TStopWatch<tchar, tresolution>;

		TStopWatch() : /*os(&osbuf),*/ base_t(os) {};
	};

	using xStopWatchA = TStopWatch<char>;
	using xStopWatchW = TStopWatch<wchar_t>;

#pragma pack(pop)
}

namespace gtlw = gtl::win_util;


#if 0

// Scan
AFX_EXT_API_MISC const char*	ScanString(const char* psz, char* pszResult, sizeXX_t& nBufferCount/* including NULL Terminator */, const char* pszDelimiter = " ", const char* pszPrefix = NULL, const char* pszEnd = " ");
AFX_EXT_API_MISC const wchar_t*	ScanString(const wchar_t* psz, wchar_t* pszResult, sizeXX_t& nBufferCount/* including NULL Terminator */, const wchar_t* pszDelimiter = L" ", const wchar_t* pszPrefix = NULL, const wchar_t* pszEnd = L" ");
AFX_EXT_API_MISC const char*	ScanString(const char* psz, xStringA& strResult, const char* pszDelimiter = " ", const char* pszPrefix = NULL, const char* pszEnd = " ");
AFX_EXT_API_MISC const wchar_t*	ScanString(const wchar_t* psz, xStringW& strResult, const wchar_t* pszDelimiter = L" ", const wchar_t* pszPrefix = NULL, const wchar_t* pszEnd = L" ");
//#define ScanStringA ScanString
//#define ScanStringW ScanString

inline BOOL CompareBoolean(BOOL b1, BOOL b2) { return ( (b1 && b2) || (!b1 && !b2) ); }
AFX_EXT_API_MISC int CompareNumberedString(LPCSTR psz1, LPCSTR psz2);
AFX_EXT_API_MISC int CompareNumberedString(LPCWSTR psz1, LPCWSTR psz2);

AFX_EXT_API_MISC void SplitPath(LPCSTR pszFullPath, xStringA& drive, xStringA& path, xStringA& title, xStringA& ext);
AFX_EXT_API_MISC void SplitPath(LPCSTR pszFullPath, xStringA& folder, xStringA& title, xStringA& ext);
AFX_EXT_API_MISC void SplitPath(LPCSTR pszFullPath, xStringA& folder, xStringA& m_name);
AFX_EXT_API_MISC void SplitPath(LPCWSTR pszFullPath, xStringW& drive, xStringW& folder, xStringW& title, xStringW& ext);
AFX_EXT_API_MISC void SplitPath(LPCWSTR pszFullPath, xStringW& folder, xStringW& title, xStringW& ext);
AFX_EXT_API_MISC void SplitPath(LPCWSTR pszFullPath, xStringW& folder, xStringW& m_name);
//#define SplitPathW SplitPath
//#define SplitPathA SplitPath
class AFX_EXT_API_MISC CPushDirectory {
private:
	xString strOldDirectory;
	xString strCurrentDirectory;
public:
	CPushDirectory(LPCTSTR pszNewDirectory) {
		::GetCurrentDirectory(1024, strOldDirectory.GetBuffer(1024));
		strOldDirectory.ReleaseBuffer();
		if (pszNewDirectory && pszNewDirectory[0]) {
			SetCurrentDirectory(pszNewDirectory);
		}
		::GetCurrentDirectory(1024, strCurrentDirectory.GetBuffer(1024));
		strCurrentDirectory.ReleaseBuffer();
	}
	~CPushDirectory() {
		if (!strOldDirectory.IsEmpty())
			SetCurrentDirectory(strOldDirectory);
	}
public:
	LPCTSTR GetOldDirectory() const { return strOldDirectory; }
	LPCTSTR GetCurrentDirectory() const { return strCurrentDirectory; }
};


AFX_EXT_API_MISC BOOL BackupMoveFile(LPCTSTR pszPath, LPCTSTR pszBackupFolder = NULL, BOOL bUseDateTime = TRUE, DWORD dwMoveFileExFlag = MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
AFX_EXT_API_MISC BOOL BackupCopyFile(LPCTSTR pszPath, LPCTSTR pszBackupFolder = NULL, BOOL bUseDateTime = TRUE, DWORD dwCopyFileExFlag = 0);

AFX_EXT_API_MISC void GetErrorText(DWORD dwLastError, xString& strMessage);

AFX_EXT_API_MISC DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

//Shell에 PathRelativePathTo(); 함수 있음.
AFX_EXT_API_MISC void GetRelativePath(LPCTSTR pszFolderBase, LPCTSTR pszFullPathTarget, xString& strPathRelative);
AFX_EXT_API_MISC void GetAbsolutePath(LPCTSTR pszFolderBase, LPCTSTR pszPathRelative, xString& strPathFull);
AFX_EXT_API_MISC xStringW GetLongPathNameUnicode(LPCWSTR szShortName);

AFX_EXT_API_MISC BOOL HexaStringToBinary(LPCSTR psz, BYTE buffer[], sizeXX_t& nBufferSize, char** pszEnd = NULL);
AFX_EXT_API_MISC BOOL HexaStringToBinary(LPCSTR psz, TBuffer<BYTE>& buf, char** pszEnd = NULL);
AFX_EXT_API_MISC BOOL HexaStringToBinary(LPCWSTR psz, BYTE buffer[], sizeXX_t& nBufferSize, wchar_t** pszEnd = NULL);
AFX_EXT_API_MISC BOOL HexaStringToBinary(LPCWSTR psz, TBuffer<BYTE>& buf, wchar_t** pszEnd = NULL);
AFX_EXT_API_MISC void MakeDataToHexForm(xStrings& strHexForm, const void* lpBuf, sizeXX_t nBufLen, int nCol = 16, BOOL bAddText = TRUE);
AFX_EXT_API_MISC void MakeDataToDecForm(xStrings& strHexForm, const void* lpBuf, sizeXX_t nBufLen, int nCol = 16, BOOL bAddText = TRUE);

AFX_EXT_API_MISC xString GetUniqueFileName(LPCTSTR path, LPCTSTR fName = _T("UniqueName??.DAT"), BOOL bCreateFile = TRUE);
AFX_EXT_API_MISC xString GetUniqueFileName(xStrings paths, LPCTSTR fName, BOOL bCreateFile = TRUE);

MOCHA_DEPRECATED AFX_EXT_API_MISC BOOL CreateIntermediateDirectory(LPCTSTR szPath);
AFX_EXT_API_MISC bool CreateSubDirectories(LPCTSTR szPath);

#ifdef _USE_SHELL_API
AFX_EXT_API_MISC LPITEMIDLIST PidlBrowse(HWND hWnd, int nCSIDL, LPTSTR pszDisplayName, LPCTSTR pszTitle = _T("Choose a folder"));
AFX_EXT_API_MISC xString BrowseFolder(HWND hWnd, int nCSIDL, LPCTSTR pszTitle = _T("Choose a folder"));
#endif

// Window/App Focus
AFX_EXT_API_MISC BOOL SetWindowFocus(HWND hWnd);
AFX_EXT_API_MISC BOOL SetWindowFocus(LPCTSTR strWindowName);

AFX_EXT_API_MISC BOOL IsEditWindow(HWND hWnd);

// Window Service Status
DWORD AFX_EXT_API_MISC QueryServiceStatus(LPCTSTR strServiceName);

// Application Control
AFX_EXT_API_MISC sizeXX_t FindApp(LPCTSTR pszFullPath, TList<DWORD>& pIDs, bool bNameOnly = false);	// returns number of process
AFX_EXT_API_MISC bool ForceTerminateApp(LPCTSTR pszFullPath, bool bNameOnly = false, bool bExcludeSelf = true);
AFX_EXT_API_MISC BOOL SetAppPriorityClass(LPCTSTR pszAppName, DWORD dwPriorityClass);

// Window OS Control
AFX_EXT_API_MISC void Restart(UINT uFlags = EWX_REBOOT|EWX_FORCE, DWORD dwReason = 0);
AFX_EXT_API_MISC BOOL SuspendSystem(BOOL bSuspended = TRUE, BOOL bForce = TRUE);

typedef struct {
	UINT Type;
	T_IP_ADDRESS ip;
	T_IP_ADDRESS subnetmask;
	T_IP_ADDRESS gateway;	 // N/A
	T_MAC_ADDRESS mac;
	xString strName;
	xString strDescription;
} T_NIC_INFO;
AFX_EXT_API_MISC BOOL GetNICInfo(std::vector<T_NIC_INFO>& nics);

//#if defined(_DEBUG) && defined(_UNICODE) && (_MSC_VER > 1200)
//	AFX_EXT_API_MISC void TRACE_U(const wchar_t* pszFormat, ...);
//	AFX_EXT_API_MISC void TRACE_U(const char* pszFormat, ...);
//#	undef TRACE
//#	define TRACE TRACE_U
//#endif

template <typename T>
BOOL SwapVectorItem(std::vector<T>& vector, uintptr_t iLeft, uintptr_t iRight) {
#ifdef _DEBUG
	if ( (iLeft < 0) || (iLeft >= vector.size()) || (iRight < 0) || (iRight >= vector.size()) )
		return FALSE;
#endif
	T c = std::move(vector[iLeft]);
	vector[iLeft] = std::move(vector[iRight]);
	vector[iRight] = std::move(c);
	return TRUE;
}
template <typename T>
BOOL ReverseVectorItem(std::vector<T>& vector) {
	if (vector.empty())
		return TRUE;
	for (uintptr_t iLeft = 0, iRight = vector.size()-1; iLeft < iRight; iLeft++, iRight--)
		SwapVectorItem(vector, iLeft, iRight);
	return TRUE;
}
template <typename T>
BOOL RotateVectorItemRight(std::vector<T>& vector, uintptr_t iLeft = 0, uintptr_t iRight = (uintptr_t)-1) {
	if (iRight >= vector.size())
		iRight = vector.size()-1;
#ifdef _DEBUG
	if ( (iLeft >= iRight) || (iRight >= vector.size()) )
		return FALSE;
#endif
	T c = std::move(vector[iRight]);
	for (uintptr_t i = iRight; i > iLeft; i--)
		vector[i] = std::move(vector[i-1]);
	vector[iLeft] = std::move(c);
	return TRUE;
}
template <typename T>
BOOL RotateVectorItemLeft(std::vector<T>& vector, uintptr_t iLeft = 0, uintptr_t iRight = (uintptr_t)-1) {
	if (iRight >= vector.size())
		iRight = vector.size()-1;
#ifdef _DEBUG
	if ( (iLeft >= iRight) || (iRight >= vector.size()) )
		return FALSE;
#endif
	T c = std::move(vector[iLeft]);
	for (uintptr_t i = iLeft; i < iRight; i++)
		vector[i] = std::move(vector[i+1]);
	vector[iRight] = std::move(c);
	return TRUE;
}

//-----------------------------------------------------------------------------
// Critical Section Helper
class AFX_EXT_CLASS_MISC CS {
protected:
	CSemaphore cs;
	CCriticalSection* pCS;
#if (_MSC_VER >= _MSC_VER_VS2013)
	std::mutex* pMX;
#endif
	BOOL bLocked;
public:
	CS(LPCTSTR m_name = NULL, LONG lInitialCount = 1, LONG lMaxCount = 1, int nTimeOut = INFINITE) : cs(lInitialCount, lMaxCount, m_name) {
		pCS = NULL;
	#if (_MSC_VER >= _MSC_VER_VS2013)
		pMX = NULL;
	#endif
		bLocked = !cs.Lock(nTimeOut);
	}
	CS(CCriticalSection* _pCS) {
		pCS = _pCS;
	#if (_MSC_VER >= _MSC_VER_VS2013)
		pMX = NULL;
	#endif
		bLocked = !pCS->Lock();
	}
#if (_MSC_VER >= _MSC_VER_VS2013)
	CS(std::mutex* _pMX) {
		pCS = NULL;
		pMX = _pMX;
		if (pMX) {
			pMX->lock();
			bLocked = TRUE;
		} else {
			bLocked = FALSE;
		}
	}
#endif
	~CS() { 
		Unlock();
	}

	BOOL IsLocked() { return bLocked; }
	BOOL Unlock() {
		if (bLocked)
			return FALSE;
		if (pCS)
			pCS->Unlock();
	#if (_MSC_VER >= _MSC_VER_VS2013)
		else if (pMX)
			pMX->unlock();
	#endif
		else
			cs.Unlock();
		bLocked = TRUE;
		return TRUE;
	}
};

template < class LockObject = CCriticalSection >
class TLock {
protected:
	LockObject* m_pLockObject;
public:
	TLock(LockObject* pLockObject) : m_pLockObject(pLockObject) {
		if (m_pLockObject)
			m_pLockObject->Lock();
	}
	~TLock() {
		if (m_pLockObject)
			m_pLockObject->Unlock();
	}
	LockObject* Detach() {
		LockObject* pLockObject = m_pLockObject;
		m_pLockObject = NULL;
		return pLockObject;
	}
};


//-----------------------------------------------------------------------------
// Reference Counter
class AFX_EXT_CLASS_MISC CRefCounter {
protected:
	std::atomic<int> mutable * m_pRefCounter{};

public:
	CRefCounter() : m_pRefCounter{new std::atomic<int>{1}} { }
	~CRefCounter() { Release(); }
	CRefCounter(const CRefCounter& counter) {
		counter.AddReferenceCounter();
		m_pRefCounter = counter.m_pRefCounter;
	}
	CRefCounter& operator = (const CRefCounter& counter) {
		counter.AddReferenceCounter();
		Release();
		m_pRefCounter = counter.m_pRefCounter;
		return *this;
	}

	void AddReferenceCounter() const {
		//if (m_pRefCounter) _InterlockedExchangeAdd((long volatile*)m_pRefCounter, 1);
		if (m_pRefCounter)
			m_pRefCounter->fetch_add(1);
	}
	void Release() {
		//if ( m_pRefCounter && _InterlockedExchangeAdd((long volatile*)m_pRefCounter, -1) == 1 ) {
		//	delete m_pRefCounter;
		//}
		//m_pRefCounter = NULL;
		if (m_pRefCounter && m_pRefCounter->fetch_sub(1) == 0) {
			delete m_pRefCounter;
			m_pRefCounter = nullptr;
		}
	}

	void Attach(const CRefCounter& counter) {
		*this = counter;
	}
	void Detach() {
		if (m_pRefCounter && m_pRefCounter->fetch_sub(1) == 0) {
			delete m_pRefCounter;
		}
		m_pRefCounter = nullptr;
	}

	BOOL IsUnique() const { return m_pRefCounter ? (*m_pRefCounter == 1) : false; }
	int GetCounter() const { return m_pRefCounter ? (int)*m_pRefCounter : 0; }
};


//-----------------------------------------------------------------------------
// Timeout
AFX_EXT_API_MISC BOOL IsTimedout(DWORD dwTickStart, DWORD dwTimeout, DWORD dwTickCurrent);
AFX_EXT_API_MISC BOOL IsTimedout(DWORD dwTickD, DWORD dwTickCurrent = GetTickCount());


//-----------------------------------------------------------------------------
// Rect Tracker with Center Cross Mark
class AFX_EXT_CLASS_MISC CRectTrackerCenterMark : public CRectTracker {
public:
	CRect m_rectExclude;
	COLORREF m_crCrossMark;
protected:
	CPoint m_ptCenterLast;
public:
	CRectTrackerCenterMark(COLORREF crCrossMark = RGB(255, 255, 255)) : CRectTracker(), m_crCrossMark(crCrossMark) {  }
	CRectTrackerCenterMark(LPCRECT lpSrcRect, UINT nStyle, COLORREF crCrossMark = RGB(255, 255, 255)) : CRectTracker(lpSrcRect, nStyle), m_crCrossMark(crCrossMark) {}

	virtual void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd);
	BOOL Track(CWnd* pWnd, CPoint point, BOOL bAllowInvert = FALSE, CWnd* pWndClipTo = NULL)	{ m_ptCenterLast.x = -1000; return __super::Track(pWnd, point, bAllowInvert, pWndClipTo); }
	BOOL TrackRubberBand(CWnd* pWnd, CPoint point, BOOL bAllowInvert = TRUE)					{ m_ptCenterLast.x = -1000; return __super::TrackRubberBand(pWnd, point, bAllowInvert); }
};


//-----------------------------------------------------------------------------
// xString       <--------->       TPoint2/3, TSize2/3, TRect2/3, T_POINT2/3, T_SIZE2/3, T_RECT2/3
//
template < class T_COORD, class = T_COORD::coord_t >
bool Text2Coord(T_COORD& coord, LPCTSTR pszText) {
	std::vector<xString> strs;
	SplitString(pszText, ',', strs, countof(coord.val));

	if (strs.size() < countof(coord.val))
		return false;
	if constexpr (std::is_integral_v<std::remove_cvref_t<decltype(T_COORD::val[0])>>) {
		for (int i = 0; i < countof(coord.val); i++)
			coord.val[i] = _round(_tcstol(strs[i], NULL, 0));
	} else {
		for (int i = 0; i < countof(coord.val); i++)
			coord.val[i] = _tcstod(strs[i], NULL);
	}

	return true;
}
template < class T_COORD, class = T_COORD::coord_t >
bool Coord2Text(const T_COORD& coord, xString& str, LPCTSTR pszFMT = nullptr) {
	if (pszFMT == NULL) {
		if constexpr (std::is_integral_v<std::remove_cvref_t<decltype(T_COORD::val[0])>>) {
			pszFMT = _T("%d");
		} else {
			pszFMT = _T("%g");
		}
	}
	str.Empty();
	for (int i = 0; i < countof(coord.val); i++) {
		if (i)
			str += _T(',');
		str += Format(pszFMT, coord.val[i]);
	}

	return true;
}

template < class T_COORD, class = T_COORD::coord_t >
T_COORD ConvText2Coord(LPCTSTR pszText) {
	T_COORD coord;
	Text2Coord(COORD, pszText);
	return coord;
}
template < class T_COORD, class = T_COORD::coord_t >
xString ConvCoord2Text(const T_COORD& coord, LPCTSTR pszFMT = nullptr) {
	xString str;
	Coord2Text(coord, str, pszFMT);
	return str;
}

#pragma pack(pop)



#endif
