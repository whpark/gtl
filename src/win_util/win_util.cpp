// win_util.cpp : Defines the initialization routines for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "gtl/win_util/win_util.h"

#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#include <Iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include <PSApi.h>
#include <WinSvc.h>

#pragma comment(lib, "PSApi.lib")

WARNING_DISABLE(4018)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace gtl::win_util {

	CString GetErrorMessage(CException& e) {
		CString str;
		e.GetErrorMessage(str.GetBuffer(1024), 1024);
		str.ReleaseBuffer();
		str.TrimRight();
		return str;
	}

	CString GetErrorMessage(DWORD dwLastError) {
		CString str;
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwLastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

		str = (LPCTSTR)lpMsgBuf;
		str.TrimRight();

		// Free the buffer.
		LocalFree( lpMsgBuf );

		return str;
	}

}

#if 0

template <typename TCHAR, class TSTRING>
intXX_t TSplitString(const TSTRING& strSRC, TCHAR cDelimiter, TList<TSTRING>& strs) {
	TCHAR szDelimiter[2];
	szDelimiter[0] = cDelimiter;
	szDelimiter[1] = 0;
	return TSplitString<TCHAR, TSTRING>(strSRC, szDelimiter, strs);
}
template <typename TCHAR, class TSTRING>
intXX_t TFindOneOf(const TCHAR* pszSource, const TCHAR* pszDelimiters) {
	for (const TCHAR* psz = pszSource; psz && *psz; psz++) {
		for (const TCHAR* pszDelimiter = pszDelimiters; *pszDelimiter; pszDelimiter++) {
			if (*pszDelimiter == *psz)
				return (intXX_t)(psz - pszSource);
		}
	}
	return -1;
}
template <typename TCHAR, class TSTRING>
intXX_t TSplitString(TSTRING strSRC, const TCHAR* pszDelimiters, TList<TSTRING>& strs) {
	strs.DeleteAll();
	if (strSRC.IsEmpty())
		return 0;
	const TCHAR* pszSRC = strSRC;
	while (TRUE) {
		intXX_t index = TFindOneOf<TCHAR, TSTRING>(pszSRC, pszDelimiters);
		if (index < 0) {
			strs.Push(new TSTRING(pszSRC));
			break;
		} else if (index == 0) {
			strs.Push(new TSTRING);
		} else {
			TSTRING str;
			WARNING_PUSH_AND_DISABLE(4244)
				TCHAR* psz = str.GetBufferSetLength(index);
			WARNING_POP()
				memcpy(psz, pszSRC, index*sizeof(TCHAR));
			str.ReleaseBuffer();
			strs.Push(new TSTRING(str));
		}
		pszSRC += index+1;
	}
	return strs.size();
}
intXX_t SplitString(CStringA strSRC, LPCSTR szDelimiters, CStringsA& strs)					{ return TSplitString<char, CStringA>(strSRC, szDelimiters, strs); }
intXX_t SplitString(const CStringA& strSRC, char cDelimiter, CStringsA& strs)					{ return TSplitString<char, CStringA>(strSRC, cDelimiter, strs); }
intXX_t SplitString(CStringW strSRC, LPCWSTR szDelimiters, CStringsW& strs)					{ return TSplitString<wchar_t, CStringW>(strSRC, szDelimiters, strs); }
intXX_t SplitString(const CStringW& strSRC, wchar_t cDelimiter, CStringsW& strs)				{ return TSplitString<wchar_t, CStringW>(strSRC, cDelimiter, strs); }

template <typename TCHAR, class TSTRING>
intXX_t TSplitString(const TSTRING& strSRC, TCHAR cDelimiter, std::vector<TSTRING>& strs, int nInitialSize) {
	TCHAR szDelimiter[2];
	szDelimiter[0] = cDelimiter;
	szDelimiter[1] = 0;
	return TSplitString<TCHAR, TSTRING>(strSRC, szDelimiter, strs, nInitialSize);
}
template <typename TCHAR, class TSTRING>
intXX_t TSplitString(TSTRING strSRC, const TCHAR* pszDelimiters, std::vector<TSTRING>& strs, int nInitialSize) {
	strs.clear();
	if (nInitialSize > 0)
		strs.reserve(nInitialSize);
	if (strSRC.IsEmpty())
		return 0;
	const TCHAR* pszSRC = strSRC;
	while (TRUE) {
		intXX_t index = TFindOneOf<TCHAR, TSTRING>(pszSRC, pszDelimiters);
		if (index < 0) {
			strs.push_back(pszSRC);
			break;
		} else if (index == 0) {
			strs.push_back(TSTRING());
		} else {
			TSTRING str;
			WARNING_PUSH_AND_DISABLE(4244)
				TCHAR* psz = str.GetBufferSetLength(index);
			WARNING_POP()
				memcpy(psz, pszSRC, index*sizeof(TCHAR));
			str.ReleaseBuffer();
			strs.push_back(str);
		}
		pszSRC += index+1;
	}
	return strs.size();
}
intXX_t SplitString(CStringA strSRC, LPCSTR szDelimiters, std::vector<CStringA>& strs, int nInitialSize)			{ return TSplitString<char, CStringA>(strSRC, szDelimiters, strs, nInitialSize); }
intXX_t SplitString(const CStringA& strSRC, char cDelimiter, std::vector<CStringA>& strs, int nInitialSize)		{ return TSplitString<char, CStringA>(strSRC, cDelimiter, strs, nInitialSize); }
intXX_t SplitString(CStringW strSRC, LPCWSTR szDelimiters, std::vector<CStringW>& strs, int nInitialSize)			{ return TSplitString<wchar_t, CStringW>(strSRC, szDelimiters, strs, nInitialSize); }
intXX_t SplitString(const CStringW& strSRC, wchar_t cDelimiter, std::vector<CStringW>& strs, int nInitialSize)	{ return TSplitString<wchar_t, CStringW>(strSRC, cDelimiter, strs, nInitialSize); }

template <typename TCHAR, class TSTRING>
BOOL TFilterString(const TCHAR* szSRC, const TCHAR* szFilter, TList<TSTRING>& strParams, BOOL bCaseSensitive) {
	for (; *szSRC && *szFilter; szSRC++, szFilter++) {
		switch (*szFilter) {
		case '?' :
			{
				TList<TSTRING> strParamsNew;
				if (TFilterString<TCHAR, TSTRING>(szSRC+1, szFilter+1, strParamsNew, bCaseSensitive)) {
					strParams.Push(new TSTRING(*szSRC));
					strParams += strParamsNew;
					return TRUE;
				}
			}
			break;

		case '*' :
			{
				TSTRING str = szSRC;
				for (int nPos = 0; nPos <= str.GetLength(); nPos++) {
					TList<TSTRING> strParamsNew;
					if (TFilterString<TCHAR, TSTRING>(szSRC+nPos, szFilter+1, strParamsNew, bCaseSensitive)) {
						strParams.Push(new TSTRING(str.Left(nPos)));
						strParams += strParamsNew;
						return TRUE;
					}
				}
				szSRC--;
			}
			break;

		default :
			if (bCaseSensitive) {
				if (*szSRC != *szFilter)
					return FALSE;
			} else {
				if (_totupper(*szSRC) != _totupper(*szFilter))
					return FALSE;
			}
			break;
		}
	}

	return !*szSRC && !*szFilter;	// both reached to end
}
BOOL FilterString(LPCSTR szSRC, LPCSTR szFilter, CStringsA& strParams, BOOL bCaseSensitive)	{
	return TFilterString<char, CStringA>(szSRC, szFilter, strParams, bCaseSensitive);
}
BOOL FilterString(LPCWSTR szSRC, LPCWSTR szFilter, CStringsW& strParams, BOOL bCaseSensitive) {
	return TFilterString<wchar_t, CStringW>(szSRC, szFilter, strParams, bCaseSensitive);
}

template <typename TCHAR>
BOOL TranslateEscapeCharactersT(const TCHAR* szSRC, TCHAR* pszResult, sizeXX_t& nSize, TCHAR** ppszEnd, TCHAR cAdditionalTerminating) {
	sizeXX_t nCount = 0;
	const TCHAR* psz;
	// first, check and count number of bytes to be translated into
	for (psz = szSRC; *psz && (*psz != cAdditionalTerminating); psz++) {
		if (*psz == _T('\\')) {
			psz++;
			if (*psz == 0)
				return FALSE;

			if (*psz == _T('x')) {				// Hexa ASCII Code
				if (_istxdigit(psz[1]) && _istxdigit(psz[2])) {
					nCount++;
					psz += 2;
				} else {
					return FALSE;
				}
			} else if (*psz == _T('u')) {				// Hexa ASCII Code
				if (_istxdigit(psz[1]) && _istxdigit(psz[2]) && _istxdigit(psz[3]) && _istxdigit(psz[4])) {
					nCount++;
					psz += 4;
				} else {
					return FALSE;
				}
			} else if (__isodigit(psz[0])) {		// Octal ASCII Code
				TCHAR szDigits[4];
				szDigits[0] = psz[0];
				szDigits[1] = psz[1];
				szDigits[2] = psz[2];
				szDigits[3] = 0;
				TCHAR* pszEnd = NULL;
				if (sizeof(TCHAR) == sizeof(wchar_t))
					(BYTE)wcstoul((const wchar_t*)szDigits, (wchar_t**)&pszEnd, 8);
				else 
					(BYTE)strtoul((const char*)szDigits, (char**)&pszEnd, 8);
				if (pszEnd) {
					psz += pszEnd-szDigits;
					psz--;
				} else
					return FALSE;
				nCount++;
			} else {
				nCount++;
			}
		} else {
			nCount++;
		}
	}

	if (!pszResult) {
		nSize = nCount;
		return TRUE;
	}

	if (nSize < nCount)
		return FALSE;

	pszResult[nCount] = 0;
	TCHAR szDigits[8];
	for (psz = szSRC; *psz && (*psz != cAdditionalTerminating); psz++) {
		if (*psz == _T('\\')) {
			psz++;

			if (*psz == _T('x')) {				// Hexa ASCII Code
				szDigits[0] = psz[1];
				szDigits[1] = psz[2];
				szDigits[2] = 0;
				if (sizeof(TCHAR) == sizeof(wchar_t))
					*pszResult++ = (BYTE)wcstoul((const wchar_t*)szDigits, NULL, 16);
				else 
					*pszResult++ = (BYTE)strtoul((const char*)szDigits, NULL, 16);
				psz += 2;
			} else if (*psz == _T('u')) {				// Hexa ASCII Code
				szDigits[0] = psz[1];
				szDigits[1] = psz[2];
				szDigits[2] = psz[3];
				szDigits[3] = psz[4];
				szDigits[4] = 0;
				if (sizeof(TCHAR) == sizeof(wchar_t))
					*pszResult++ = (TCHAR)wcstoul((const wchar_t*)szDigits, NULL, 16);
				else 
					*pszResult++ = (TCHAR)strtoul((const char*)szDigits, NULL, 16);
				psz += 4;
			} else if (__isodigit(psz[0])) {		// Octal ASCII Code
				TCHAR szDigits[4];
				szDigits[0] = psz[0];
				szDigits[1] = psz[1];
				szDigits[2] = psz[2];
				szDigits[3] = 0;
				TCHAR* pszEnd = NULL;
				if (sizeof(TCHAR) == sizeof(wchar_t))
					*pszResult++ = (BYTE)wcstoul((const wchar_t*)szDigits, (wchar_t**)&pszEnd, 8);
				else 
					*pszResult++ = (BYTE)strtoul((const char*)szDigits, (char**)&pszEnd, 8);
				if (pszEnd) {
					psz += pszEnd-szDigits;
					psz--;
				} else
					return FALSE;

			} else {
				const static struct {
					TCHAR cEscape;
					TCHAR cValue;
				} escapes[] = {
					{ _T('a'), _T('\a') },
					{ _T('b'), _T('\b') },
					{ _T('f'), _T('\f') },
					{ _T('n'), _T('\n') },
					{ _T('r'), _T('\r') },
					{ _T('t'), _T('\t') },
					{ _T('v'), _T('\v') },
				};

				TCHAR cValue = *psz;
				for (uintXX_t i = 0; i < countof(escapes); i++) {
					if (escapes[i].cEscape != *psz)
						continue;
					cValue = escapes[i].cValue;
					break;
				}
				*pszResult++ = cValue;
			}
		} else {
			*pszResult++ = *psz;
		}
	}

	if (ppszEnd)
		*ppszEnd = (TCHAR*)psz;

	return TRUE;
}

template <typename TCHAR, class STRING>
BOOL TranslateEscapeCharactersT(const TCHAR* szSRC, STRING& strResult, TCHAR** ppszEnd, TCHAR cAdditionalTerminating) {
	sizeXX_t nSize = 0;
	BOOL bResult = FALSE;
	bResult = TranslateEscapeCharactersT<TCHAR>(szSRC, NULL, nSize, ppszEnd, cAdditionalTerminating);
	STRING _strResult;
	if (nSize > 0) {
		WARNING_PUSH_AND_DISABLE(4267)
			bResult = TranslateEscapeCharactersT<TCHAR>(szSRC, (TCHAR*)_strResult.GetBuffer(nSize), nSize, ppszEnd, cAdditionalTerminating);
		WARNING_POP()
			_strResult.ReleaseBuffer();
		nSize = 0;
	}
	strResult = _strResult;
	return bResult;
}

BOOL TranslateEscapeCharacters(const char* szSRC, char* pszResult, sizeXX_t& nSize/*exclude null-terminating*/, char** ppszEnd, char cAdditionalTerminating) {
	return TranslateEscapeCharactersT(szSRC, pszResult, nSize, ppszEnd, cAdditionalTerminating);
}
BOOL TranslateEscapeCharacters(const wchar_t* szSRC, wchar_t* pszResult, sizeXX_t& nSize/*exclude null-terminating*/, wchar_t** ppszEnd, wchar_t cAdditionalTerminating) {
	return TranslateEscapeCharactersT(szSRC, pszResult, nSize, ppszEnd, cAdditionalTerminating);
}
BOOL TranslateEscapeCharacters(const char* szSRC, CStringA& strResult, char** ppszEnd, char cAdditionalTerminating) {
	return TranslateEscapeCharactersT(szSRC, strResult, ppszEnd, cAdditionalTerminating);
}
BOOL TranslateEscapeCharacters(const wchar_t* szSRC, CStringW& strResult, wchar_t** ppszEnd, wchar_t cAdditionalTerminating) {
	return TranslateEscapeCharactersT(szSRC, strResult, ppszEnd, cAdditionalTerminating);
}


template < typename T >
CString DtoA(T number, LPCTSTR format) {
	CString buf, result;
	buf.Format(format, number);
	BOOL bMinus = FALSE;
	if (buf.GetLength() && (buf.GetAt(0) == '-')) {
		buf = buf.Mid(1);
		bMinus = TRUE;
	}
	int index = buf.Find('.');
	if (index < 0) index = buf.GetLength();
	CString str = buf.Left(index);
	int len = str.GetLength();
	for (int i = 0; i < len; i++) {
		result += str.GetAt(i);
		if ( !((len - i-1) % 3) && (len != i+1) ) result += ',';
	}
	result = result + buf.Mid(index, buf.GetLength()-index);
	if (bMinus)
		result = _T("-") + result;
	return result;
}
CString DtoA(const double number, LPCTSTR format) {
	return DtoA<double>(number, format);
}
CString DtoA(int32_t number) {
	return DtoA<int32_t>(number, _T("%I32d"));
}
CString DtoA(uint32_t number) {
	return DtoA<uint32_t>(number, _T("%I32u"));
}
CString DtoA(DWORD number) {
	return DtoA<DWORD>(number, _T("%I32u"));
}
CString DtoA(int64_t number) {
	return DtoA<int64_t>(number, _T("%I64d"));
}
CString DtoA(uint64_t number) {
	return DtoA<uint64_t>(number, _T("%I64u"));
}

//CStringA Format(LPCSTR fmt, ... ) {
//	va_list arglist;
//	va_start(arglist, fmt);
//
//	CStringA str;
//	str.FormatV(fmt, arglist);
//
//	va_end(arglist);
//	return str;
//}
//
//CStringW Format(LPCWSTR fmt, ... ) {
//	va_list arglist;
//	va_start(arglist, fmt);
//
//	CStringW str;
//	str.FormatV(fmt, arglist);
//
//	va_end(arglist);
//	return str;
//}

CString FormatDate(const LPCTSTR szDate) {
	CString str;
	if (szDate && (_tcslen(szDate) >= 8)) {
		str.Format(_T("%.4s/%.2s/%.2s"), szDate, szDate+4, szDate+6);
	}
	return str;
}

CString FormatTime(const LPCTSTR szTime) {
	CString str;
	if (szTime) {
		if (_tcslen(szTime) >= 6)
			str.Format(_T("%.2s:%.2s:%.2s"), szTime, szTime+2, szTime+4);
		else if (_tcslen(szTime) >= 4)
			str.Format(_T("%.2s:%.2s"), szTime, szTime+2);
	}
	return str;
}

CString GetErrorMessage(CException& e) {
	CString str;
	e.GetErrorMessage(str.GetBuffer(1024), 1024);
	str.ReleaseBuffer();
	str.TrimRight();
	return str;
}

CString GetErrorMessage(DWORD dwLastError) {
	CString str;
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);

	str = (LPCTSTR)lpMsgBuf;
	str.TrimRight();

	// Free the buffer.
	LocalFree( lpMsgBuf );

	return str;
}

// Scan
template < typename TCHAR >
const TCHAR* ScanString(const TCHAR* psz, TCHAR* pszResult, sizeXX_t& nBufferCount/* including NULL Terminator */, const TCHAR* pszDelimiter = NULL/*Skipping Charater Set*/, const TCHAR* pszPrefix = NULL/*Condition*/, const TCHAR* pszEnd = NULL/*Terminating*/) {
	if (!psz)
		return NULL;
	if (nBufferCount && pszResult)
		*pszResult = NULL;

	// Skip Spaces
	while (*psz && tszsearch(pszDelimiter, *psz))
		psz++;

	// Skip Prefix
	if (pszPrefix) {
		for (; *psz && *pszPrefix; psz++, pszPrefix++)
			if (*psz != *pszPrefix)
				return NULL;
	}

	// Skip Spaces
	while (*psz && tszsearch(pszDelimiter, *psz))
		psz++;

	// Set Starting Point
	const TCHAR* pszStart = psz;

	// Find End
	while (*psz && !tszsearch<TCHAR>(pszEnd, *psz))
		psz++;

	// Packing
	intXX_t nLength = (intXX_t)(psz - pszStart);
	if (nLength) {
		WARNING_PUSH_AND_DISABLE(4018)
			if (pszResult && (nLength+1 <= nBufferCount)) {
				// Copy Result ...
				memmove(pszResult, pszStart, sizeof(TCHAR)*nLength);
				pszResult[nLength] = 0;
			} else  {
				nBufferCount = nLength+1;
			}
		WARNING_POP()
	} else {
		return NULL;
	}

	return psz;
}
template < typename CHAR, class STRING >
const CHAR* ScanStringStr(const CHAR* psz, STRING& strResult, const CHAR* pszDelimiter = NULL, const CHAR* pszPrefix = NULL, const CHAR* pszEnd = NULL) {
	strResult.Empty();
	const CHAR* pszResult = NULL;
	sizeXX_t nBufferCount = 0;
	pszResult = ScanString<CHAR>(psz, NULL, nBufferCount, pszDelimiter, pszPrefix, pszEnd);
	if (pszResult && (nBufferCount > 0)) {
		WARNING_PUSH_AND_DISABLE(4267)
			pszResult = ScanString<CHAR>(psz, strResult.GetBuffer(nBufferCount), nBufferCount, pszDelimiter, pszPrefix, pszEnd);
		WARNING_POP()
			strResult.ReleaseBuffer();
	}
	return pszResult;
}

const char* ScanString(const char* psz, char* pszResult, sizeXX_t& nBufferCount/* including NULL Terminator */, const char* pszDelimiter, const char* pszPrefix, const char* pszEnd) {
	return ScanString<char>(psz, pszResult, nBufferCount, pszDelimiter, pszPrefix, pszEnd);
}
const wchar_t* ScanString(const wchar_t* psz, wchar_t* pszResult, sizeXX_t& nBufferCount/* including NULL Terminator */, const wchar_t* pszDelimiter, const wchar_t* pszPrefix, const wchar_t* pszEnd) {
	return ScanString<wchar_t>(psz, pszResult, nBufferCount, pszDelimiter, pszPrefix, pszEnd);
}
const char* ScanString(const char* psz, CStringA& strResult, const char* pszDelimiter, const char* pszPrefix, const char* pszEnd) {
	return ScanStringStr(psz, strResult, pszDelimiter, pszPrefix, pszEnd);
}
const wchar_t* ScanString(const wchar_t* psz, CStringW& strResult, const wchar_t* pszDelimiter, const wchar_t* pszPrefix, const wchar_t* pszEnd) {
	return ScanStringStr(psz, strResult, pszDelimiter, pszPrefix, pszEnd);
}

int CompareNumberedString(LPCSTR psz1, LPCSTR psz2) { return tdszcmp(psz1, psz2); }
int CompareNumberedString(LPCWSTR psz1, LPCWSTR psz2) { return tdszcmp(psz1, psz2); }

void SplitPath(LPCSTR pszFullPath, CStringA& drive, CStringA& folder, CStringA& title, CStringA& ext) {
	drive.Empty(); folder.Empty(); title.Empty(); ext.Empty();
	if (!pszFullPath)
		return;
	_splitpath_s(pszFullPath, drive.GetBuffer(30), 30, folder.GetBuffer(1024), 1024, title.GetBuffer(256), 256, ext.GetBuffer(256), 256);

	drive.ReleaseBuffer(); folder.ReleaseBuffer(); title.ReleaseBuffer(); ext.ReleaseBuffer();
}

void SplitPath(LPCSTR pszFullPath, CStringA& folder, CStringA& title, CStringA& ext) {
	CStringA drive;
	SplitPath(pszFullPath, drive, folder, title, ext);
	drive.ReleaseBuffer();

	folder = drive + folder;
}

void SplitPath(LPCSTR pszFullPath, CStringA& folder, CStringA& name) {
	CStringA drive, title, ext;

	SplitPath(pszFullPath, drive, folder, title, ext);
	drive.ReleaseBuffer();

	folder = drive + folder;
	name = title + ext;
}

void SplitPath(LPCWSTR pszFullPath, CStringW& drive, CStringW& folder, CStringW& title, CStringW& ext) {
	drive.Empty(); folder.Empty(); title.Empty(); ext.Empty();
	if (!pszFullPath)
		return;
	_wsplitpath_s(pszFullPath, drive.GetBuffer(30), 30, folder.GetBuffer(1024), 1024, title.GetBuffer(256), 256, ext.GetBuffer(256), 256);

	drive.ReleaseBuffer(); folder.ReleaseBuffer(); title.ReleaseBuffer(); ext.ReleaseBuffer();
}

void SplitPath(LPCWSTR pszFullPath, CStringW& folder, CStringW& title, CStringW& ext) {
	CStringW drive;
	SplitPath(pszFullPath, drive, folder, title, ext);
	drive.ReleaseBuffer();

	folder = drive + folder;
}

void SplitPath(LPCWSTR pszFullPath, CStringW& folder, CStringW& name) {
	CStringW drive, title, ext;

	SplitPath(pszFullPath, drive, folder, title, ext);
	drive.ReleaseBuffer();

	folder = drive + folder;
	name = title + ext;
}

BOOL BackupFile(BOOL bMove, LPCTSTR pszPath, LPCTSTR pszBackupFolder, BOOL bUseDateTime, DWORD dwFlag) {
	CString strPath(pszPath);
	CString strBackupFolder(pszBackupFolder);

	if (strPath.IsEmpty())
		return FALSE;

	if (strBackupFolder.IsEmpty()) {
		CString strName;
		SplitPath(strPath, strBackupFolder, strName);
	}
	if (!strBackupFolder.IsEmpty()) {
		TCHAR c = strBackupFolder.GetAt(strBackupFolder.GetLength()-1);
		if ( (c != '\\') || (c != '/') )
			strBackupFolder += _T("\\");
	}
	CreateSubDirectories(strBackupFolder);

	CFileFind ff;
	BOOL bFound = ff.FindFile(strPath);
	if (!bFound)
		return FALSE;
	int nBackup = 0;
	while (bFound) {
		bFound = ff.FindNextFile();
		if (ff.IsDirectory()) {
			continue;
		}
		CString strTarget;
		CString strFolder, strTitle, strExt;
		SplitPath(ff.GetFilePath(), strFolder, strTitle, strExt);
		if (bUseDateTime) {
			CTime t;
			ff.GetLastWriteTime(t);
			strTitle += t.Format(_T(".%Y%m%d_%H%M%S"));
		}
		if (bMove) {
			if (MoveFileEx(ff.GetFilePath(), strBackupFolder + strTitle + strExt, dwFlag))
				nBackup++;
		} else {
			if (CopyFileEx(ff.GetFilePath(), strBackupFolder + strTitle + strExt, NULL, NULL, NULL, dwFlag))
				nBackup++;
		}
	}
	return nBackup ? TRUE : FALSE;
}

BOOL BackupMoveFile(LPCTSTR pszPath, LPCTSTR pszBackupFolder, BOOL bUseDateTime, DWORD dwMoveFileExFlag) {
	return BackupFile(TRUE, pszPath, pszBackupFolder, bUseDateTime, dwMoveFileExFlag);
}
BOOL BackupCopyFile(LPCTSTR pszPath, LPCTSTR pszBackupFolder, BOOL bUseDateTime, DWORD dwCopyFileExFlag) {
	return BackupFile(FALSE, pszPath, pszBackupFolder, bUseDateTime, dwCopyFileExFlag);
}

// From MSDN
#include <lmerr.h>
void GetErrorText(DWORD dwLastError, CString& strMessage) {
	HMODULE hModule = NULL; // default to system source
	LPSTR MessageBuffer;
	DWORD dwBufferLength;

	DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_FROM_SYSTEM ;

	//
	// If dwLastError is in the network range, 
	//  load the message source.
	//

	if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
		hModule = LoadLibraryEx(
			TEXT("netmsg.dll"),
			NULL,
			LOAD_LIBRARY_AS_DATAFILE
		);

		if(hModule != NULL)
			dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
	}

	//
	// Call FormatMessage() to allow for message 
	//  text to be acquired from the system 
	//  or from the supplied module handle.
	//

	if(dwBufferLength = FormatMessageA(
		dwFormatFlags,
		hModule, // module to get message from (NULL == system)
		dwLastError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
		(LPSTR) &MessageBuffer,
		0,
		NULL
	))
	{
		//DWORD dwBytesWritten;

		////
		//// Output message string on stderr.
		////


		//WriteFile(
		//	GetStdHandle(STD_ERROR_HANDLE),
		//	MessageBuffer,
		//	dwBufferLength,
		//	&dwBytesWritten,
		//	NULL
		//	);

		strMessage = MessageBuffer;

		//
		// Free the buffer allocated by the system.
		//
		LocalFree(MessageBuffer);
	}

	//
	// If we loaded a message source, unload it.
	//
	if(hModule != NULL)
		FreeLibrary(hModule);
}

DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb) {
	/*
	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)&ar;
	es.pfnCallback = EditStreamCallback;
	rec.StreamIn(SF_TEXT, es);
	*/
	//WARNING_PUSH_AND_DISABLE(4312)
	CArchive* pAR = (CArchive*)dwCookie;
	//WARNING_POP()
	CFile* pFile = pAR->GetFile();
	if (pAR->IsStoring()) {
		pFile->Write(pbBuff, cb);
		*pcb = cb;
	} else {
		*pcb = pFile->Read(pbBuff, cb);
	}
	return 0;
}

//BOOL FilterFileR(LPCTSTR szFilter, LPCTSTR szFName) {
//	while (*szFName) {
//		switch (*szFilter) {
//		case 0 :
//			return !*szFName;
//		case '*' :
//			szFilter++;
//			do {
//				while (*szFName && (*szFilter != *szFName)) szFName++;
//				if (FilterFileR(szFilter, szFName))
//					return TRUE;
//				if (*szFName)
//					szFName++;
//			} while (*szFName);
//			break;
//		default :
//			if (*szFilter != *szFName)
//				return FALSE;
//		case '?' :
//			szFilter++;
//			szFName++;
//			break;
//		}
//	}
//
//	return (szFilter[0] == szFName[0]);
//}

BOOL FilterFile(LPCTSTR pszFilter, LPCTSTR pszName, BOOL bCaseSensitive) {
	if (!pszFilter || !pszFilter[0] || !pszName || !pszName[0])
		return TRUE;
	//if (!bCaseSensitive) {
	//	CString strFilter(pszFilter), strName(pszName);
	//	strFilter.MakeLower();
	//	strName.MakeLower();
	//	return FilterFileR(strFilter, strName);
	//}
	//return FilterFileR(pszFilter, pszName);
	CStrings strs;
	return FilterString(pszName, pszFilter, strs, bCaseSensitive);
}

BOOL FilterFileMultiFilter(LPCTSTR pszFilters, LPCTSTR pszName, BOOL bCaseSensitive, TCHAR cSpliter) {
	if (!pszFilters || !pszFilters[0] || !pszName || !pszName[0])
		return TRUE;

	//if (bCaseSensitive) {
	CStrings strsFilter;
	SplitString(pszFilters, cSpliter, strsFilter);
	for (int i = 0; i < strsFilter.N(); i++) {
		CStrings strsTemp;
		if (FilterString(pszName, strsFilter[i], strsTemp, bCaseSensitive))
			return TRUE;
	}
	//} else {
	//	CString strFilters(pszFilters), strName(pszName);
	//	strFilters.MakeLower();
	//	strName.MakeLower();
	//	CStrings strsFilter;
	//	SplitString(strFilters, cSpliter, strsFilter);
	//	for (int i = 0; i < strsFilter.N(); i++) {
	//		if (FilterFileR(strsFilter[i], strName))
	//			return TRUE;
	//	}
	//}

	return FALSE;
}

void GetRelativePath(LPCTSTR pszFolderBase, LPCTSTR pszFullPathTarget, CString& strPathRelative) {
	//PathRelativePathTo();

	strPathRelative.Empty();
	std::vector<CString> strPathsBase;
	std::vector<CString> strPathsTarget;

	CString strFolderBase(pszFolderBase), strFullPathTarget(pszFullPathTarget);

	strFolderBase.Replace(_T('/'), _T('\\'));
	strFullPathTarget.Replace(_T('/'), _T('\\'));
	//strFolderBase.TrimRight(_T('\\'));
	//strFullPathTarget.TrimRight(_T('\\'));
	while ( strFolderBase.Replace(_T("\\\\"), _T("\\")) );
	while ( strFullPathTarget.Replace(_T("\\\\"), _T("\\")) );

	SplitString(strFolderBase, _T('\\'), strPathsBase);
	SplitString(strFullPathTarget, _T('\\'), strPathsTarget);

	intXX_t n = _min(strPathsBase.size(), strPathsTarget.size());
	intXX_t i = 0;
	for (i = 0; i < n; i++) {
		if (strPathsBase[i].CompareNoCase(strPathsTarget[i]) != 0)
			break;
	}

	if (i == 0) {
		strPathRelative = strFullPathTarget;
		return ;
	}

	for (intXX_t iParent = i+1; iParent < strPathsBase.size(); iParent++) {
		//if (!strPathsBase[iParent].IsEmpty())
		strPathRelative += _T("..\\");
	}

	for (; i < strPathsTarget.size(); i++) {
		if (!strPathsTarget[i].IsEmpty())
			strPathRelative += strPathsTarget[i] + _T("\\");
	}

	if (strFullPathTarget.GetLength() && (strFullPathTarget.Right(1) != _T("\\")))
		strPathRelative.TrimRight(_T("\\"));

	if (strPathRelative.IsEmpty())
		strPathRelative = _T(".\\");
}

void GetAbsolutePath(LPCTSTR pszFolderBase, LPCTSTR pszPathRelative, CString& strPathFull) {
	if (tszncmp(pszPathRelative, _T("\\\\"), 2) == 0) {	// in case '\\.\' or '\\?\'
		strPathFull = pszPathRelative;
		return;
	}

	CString strFolderBase(pszFolderBase), strPathRelative(pszPathRelative);
	if (strFolderBase.IsEmpty()) {
		GetCurrentDirectory(1024, strFolderBase.GetBuffer(1024));
		strFolderBase.ReleaseBuffer();
	}
	if (strPathRelative.IsEmpty()) {
		strPathFull = strFolderBase;
		return;
	}

	strFolderBase.Replace(_T('/'), _T('\\'));
	strPathRelative.Replace(_T('/'), _T('\\'));
	//strFolderBase.TrimRight(_T('\\'));
	//strPathRelative.TrimRight(_T('\\'));
	while ( strFolderBase.Replace(_T("\\\\"), _T("\\")) > 0 );
	while ( strPathRelative.Replace(_T("\\\\"), _T("\\")) > 0 );

	if (strFolderBase && strFolderBase.GetAt(strFolderBase.GetLength()-1) != '\\')
		strFolderBase += "\\";

	CString strDrive, strFolder, strTitle, strExt;
	SplitPath(strPathRelative, strDrive, strFolder, strTitle, strExt);

	if (!strDrive.IsEmpty()) {
		strPathFull = strPathRelative;
		return;
	}

	if (tszncmp<TCHAR>(strFolder, _T("\\"), 1) == 0) {
		strPathFull = strDrive + strFolder;
		return;
	}

	strPathFull = strFolderBase + strPathRelative;

	if ( (strPathFull.Find(_T("..")) >= 0) || (strPathFull.Find(_T("\\.")) >= 0) || (strPathFull.Find(_T(".\\")) >= 0)) {
		CStrings strs;
		SplitString(strPathFull, '\\', strs);

		for (int i = 1; i < strs.N(); i++) {
			if (strs[i] == _T("..")) {
				strs.Delete(i--);
				strs.Delete(i--);
				continue;
			}
			if (strs[i] == _T(".")) {
				strs.Delete(i--);
				continue;
			}
		}

		strPathFull.Empty();
		for (int i = 0; i < strs.N(); i++) {
			if (i) strPathFull += _T("\\");
			strPathFull += strs[i];
		}
	}

}

CStringW GetLongPathNameUnicode(const wchar_t* szShortName) {
	const int BUFFER_LENGTH = 4096;
	CStringW strResult;
	CStringW strSource;
	strSource = szShortName;

	wchar_t* psz = strResult.GetBuffer(BUFFER_LENGTH);

	if (!GetLongPathNameW(strSource, psz, BUFFER_LENGTH)) {
		CStringW strPath;
		CStringW strPathTemp;

		CStringW strName;
		int iStart = 0;
		for (int i = 0; strName = strSource.Tokenize(L"\\/", iStart), !strName.IsEmpty(); i++) {
			if (i)
				strPathTemp += _T('\\');
			CFileFind ff;
			CString str;
			if (GetLongPathNameW(strPathTemp + strName, psz, BUFFER_LENGTH)) {
				strPathTemp = psz;
			} else {
				BOOL bFound = ff.FindFile(CString(strPathTemp + L"*.*"));
				while (bFound) {
					bFound = ff.FindNextFile();
					if (ff.IsDots())
						continue;
					CStringA strA;
					strA = ff.GetFileName();
					if (strA.CompareNoCase((CStringA)strName) != 0)
						continue;
					strPathTemp += ff.GetFileName();
					break;
				}
			}
		}
	}
	strResult.ReleaseBuffer();

	return strResult;
}

template < typename TCHAR, typename TSTRING >
BOOL HexaStringToBinaryT(const TCHAR* psz, BYTE buffer[], sizeXX_t& nBufferSize, TCHAR** ppszEnd) {
	// Calc Length needed.
	sizeXX_t nLen = 0;
	const TCHAR* pos = psz;
	for ( ; *pos; pos++) {
		TCHAR c = *pos;
		if (__isspace(c)) {
			if (nLen % 2)
				nLen++;
			continue;
		}

		unsigned int b = 0;
		if ( (c >= '0') && (c <= '9') ) {
			b = c - '0';
		} else if ( (c >= 'A') && (c <= 'F') ) {
			b = c - 'A' + 0x0A;
		} else if ( (c >= 'a') && (c <= 'f') ) {
			b = c - 'a' + 0x0A;
			//} else if ( (pos[0] == '\'') && pos[1] && (pos[2] == '\'') ) {
			//	if (nLen%2)
			//		nLen++;
			//	pos+=2;
			//	nLen+=2;
			//	continue;
		} else if (pos[0] == '\"') {
			if (nLen%2)
				nLen++;

			// 아래와 같이 그대로 해야 함. Unicode -> MBCS 변환시 글자 수 달라질 가능성 있음.
			TSTRING strTemp;
			if (TranslateEscapeCharacters(pos+1, strTemp, (TCHAR**)&pos, _T('\"'))) {
				if (sizeof(TCHAR) == sizeof(char)) {
					nLen += strTemp.GetLength()*2;
				} else if (sizeof(TCHAR) == sizeof(wchar_t)) {
					CStringA strA(strTemp);
					nLen += strA.GetLength()*2;
				}
				continue;
			}
			// roll back
			break;
		} else
			break;

		sizeXX_t i = nLen/2;
		nLen++;
	}
	if (nLen % 2)
		nLen++;
	nLen = nLen/2;

	// Check Size.
	if (!buffer || nBufferSize < nLen) {
		nBufferSize = nLen;
		return 0;
	}

	// Translate
	nLen = 0;
	memset(buffer, 0, nBufferSize);
	for (pos = psz; *pos && (nLen/2 < nBufferSize); pos++) {
		TCHAR c = *pos;
		if (__isspace(c)) {
			if (nLen % 2)
				nLen++;
			continue;
		}

		unsigned int b = 0;
		if ( (c >= '0') && (c <= '9') ) {
			b = c - '0';
		} else if ( (c >= 'A') && (c <= 'F') ) {
			b = c - 'A' + 0x0A;
		} else if ( (c >= 'a') && (c <= 'f') ) {
			b = c - 'a' + 0x0A;
			//} else if ( (pos[0] == '\'') && pos[1] && (pos[2] == '\'') ) {
			//	if (nLen%2)
			//		nLen++;
			//	buffer[nLen/2] = (BYTE)pos[1];
			//	pos+=2;
			//	nLen+=2;
			//	continue;
		} else if (pos[0] == '\"') {
			if (nLen%2)
				nLen++;

			TSTRING strTemp;
			if (TranslateEscapeCharacters(pos+1, strTemp, (TCHAR**)&pos, _T('\"'))) {
				if (sizeof(TCHAR) == sizeof(char)) {
					if (strTemp.GetLength() <= nBufferSize-nLen/2) {
						memmove(buffer + nLen/2, strTemp, strTemp.GetLength());
						nLen += strTemp.GetLength()*2;
						continue;
					}
				} else if (sizeof(TCHAR) == sizeof(wchar_t)) {
					CStringA strA(strTemp);
					if (strA.GetLength() <= nBufferSize-nLen/2) {
						memmove(buffer + nLen/2, strA, strA.GetLength());
						nLen += strA.GetLength()*2;
						continue;
					}
				}
			}
			// roll back
			break;
		} else
			break;

		sizeXX_t i = nLen/2;
		buffer[i] <<= 4;
		buffer[i] |= (BYTE)b;
		nLen++;
	}
	if (nLen % 2)
		nLen++;

	// End Position
	if (ppszEnd)
		*ppszEnd = (TCHAR*)pos;

	return nLen/2;
}

template < typename TCHAR, typename TSTRING >
BOOL HexaStringToBinaryT(const TCHAR* psz, TBuffer<BYTE>& buf, TCHAR** ppszEnd) {
	sizeXX_t nSize = 0;
	HexaStringToBinaryT<TCHAR, TSTRING>(psz, NULL, nSize, ppszEnd);
	buf.SetSize(nSize);
	if (nSize <= 0)
		return 0;

	return HexaStringToBinaryT<TCHAR, TSTRING>(psz, buf, nSize, ppszEnd);
}

BOOL HexaStringToBinary(LPCSTR psz, BYTE buffer[], sizeXX_t& nBufferSize, char** ppszEnd)		{ return HexaStringToBinaryT<char, CStringA>(psz, buffer, nBufferSize, ppszEnd); }
BOOL HexaStringToBinary(LPCSTR psz, TBuffer<BYTE>& buf, char** ppszEnd)							{ return HexaStringToBinaryT<char, CStringA>(psz, buf, ppszEnd); }
BOOL HexaStringToBinary(LPCWSTR psz, BYTE buffer[], sizeXX_t& nBufferSize, wchar_t** ppszEnd)	{ return HexaStringToBinaryT<wchar_t, CStringW>(psz, buffer, nBufferSize, ppszEnd); }
BOOL HexaStringToBinary(LPCWSTR psz, TBuffer<BYTE>& buf, wchar_t** ppszEnd)						{ return HexaStringToBinaryT<wchar_t, CStringW>(psz, buf, ppszEnd); }

void MakeDataToHexForm(CStrings& strHexForms, const void* lpBuf, sizeXX_t nBufLen, int nCol, BOOL bAddText) {
	int iCurrent = 0;
	char* buf = (char*)lpBuf;
	strHexForms.DeleteAll();
	while (iCurrent < nBufLen) {
		char hex[512];
		char* pHex = hex;

		int iCol = nCol;
		int iHeader = iCurrent;
		for (iCol = 0; (iCurrent < nBufLen) && (iCol < nCol); iCurrent++, iCol++) {
			*pHex = ((buf[iCurrent] & 0xF0) >> 4) + '0';
			if (*pHex > '9') *pHex += 'A' - 0x0A - '0';
			pHex++;

			*pHex = (buf[iCurrent] & 0x0F) + '0';
			if (*pHex > '9') *pHex += 'A' - 0x0A - '0';
			pHex++;

			*pHex++ = ' ';

			if ( (nCol > 2) && (iCol+1 != nCol) && !((iCol+1) % (nCol/2)) )
				*pHex++ = ' ';
		}
		for ( ; iCol < nCol; iCol++) {
			static char spc[] = { '.', '.', ' ' };
			memmove(pHex, spc, sizeof(spc));
			pHex += sizeof(spc);

			if ( (nCol > 2) && (iCol+1 != nCol) && !((iCol+1) % (nCol/2)) )
				*pHex++ = ' ';
		}
		if (bAddText) {
			int i = iHeader;
			memmove(pHex, " | ", 3);
			pHex += 3;
			for (iCol = 0; (i < nBufLen) && (iCol < nCol); i++, iCol++) {
				if ( (buf[i] > 0) && isprint(buf[i]) )
					*pHex++ = buf[i];
				else
					*pHex++ = '.';
			}
		}
		*pHex = 0;
		strHexForms.Push(new CString(hex));
	}
}

void MakeDataToDecForm(CStrings& strHexForms, const void* lpBuf, sizeXX_t nBufLen, int nCol, BOOL bAddText) {
	int iCurrent = 0;
	char* buf = (char*)lpBuf;
	strHexForms.DeleteAll();
	while (iCurrent < nBufLen) {
		char hex[512];
		char* pHex = hex;

		int iCol = nCol;
		int iHeader = iCurrent;
		for (iCol = 0; (iCurrent < nBufLen) && (iCol < nCol); iCurrent++, iCol++) {
			pHex += sprintf_s(pHex, sizeof(hex)-(pHex-hex), "%3d", ((BYTE*)buf)[iCurrent]);

			*pHex++ = ' ';

			if ( (nCol > 2) && (iCol+1 != nCol) && !((iCol+1) % (nCol/2)) )
				*pHex++ = ' ';
		}
		for ( ; iCol < nCol; iCol++) {
			static char spc[] = { '.', '.', '.', ' ' };
			memmove(pHex, spc, sizeof(spc));
			pHex += sizeof(spc);

			if ( (nCol > 2) && (iCol+1 != nCol) && !((iCol+1) % (nCol/2)) )
				*pHex++ = ' ';
		}
		if (bAddText) {
			int i = iHeader;
			memmove(pHex, " | ", 3);
			pHex += 3;
			for (iCol = 0; (i < nBufLen) && (iCol < nCol); i++, iCol++) {
				if ( (buf[i] > 0) && isprint(buf[i]) )
					*pHex++ = buf[i];
				else
					*pHex++ = '.';
			}
		}
		*pHex = 0;
		strHexForms.Push(new CString(hex));
	}
}

CString GetUniqueFileName(LPCTSTR path, LPCTSTR fName, BOOL bCreateFile) {
	if (_tcslen(path) == 0)
		return CString();

	CStrings paths;
	paths.Push(new CString(path));
	return GetUniqueFileName(paths, fName, bCreateFile);
}

CString GetUniqueFileName(CStrings paths, LPCTSTR fName, BOOL bCreateFile) {
	if (paths.IsEmpty())
		return CString();

	CString strFName;
	int iSeq = 0;

	// Find position and number of '?'
	CString strFilter;
	strFilter = fName;
	const TCHAR* buf = fName;
	int nQt = 0;
	int iQt = 0;
	for (; *buf; buf++) {
		if (*buf == '?') {
			nQt ++;
		} else {
			if (nQt)
				break;
			iQt ++;
		}
	}
	if (!nQt)
		return strFilter;

	CString fmt = strFilter.Left(iQt) + Format(_T("%%0%dd"), nQt) + strFilter.Mid(iQt+nQt);

	CS cs(_T("GetUniqueFileName"));

	int i;
	for (i = 0; i < paths.N(); i++) {
		CString path = paths[i];
		if (!path.IsEmpty()) {
			//TCHAR cSplitter[] = {'/', '\\'};
			TCHAR cRight = path.Right(1).GetAt(0);
			if ( (cRight != '/') && (cRight != '\\') )
				path += '\\';
		}
		paths[i] = path;
	}

	for (i = 0; i < paths.N(); i++) {
		CFileFind ff;
		BOOL bFound = ff.FindFile(paths[i] + strFilter);
		while (bFound) {
			bFound = ff.FindNextFile();
			CString str = ff.GetFileName();
			if (str.GetLength() < iQt + nQt)
				continue;
			int temp = _ttoi(str.Mid(iQt, nQt));
			if (iSeq <= temp)
				iSeq = temp+1;
		}
	}

	strFName = Format(fmt, iSeq);

	for (i = 0; i < paths.N(); i++) {
		CFileFind ff;
		if (ff.FindFile(paths[i] + strFName)) {
			return CString();
		}
	}

	if (bCreateFile) {
		CFile f;
		CFileException e;
		if (!f.Open(paths[0] + strFName, CFile::modeCreate, &e)) {
			TRACE(GetErrorMessage(e) + EOL);
			return strFName;
		}
		f.Close();
	}

	return strFName;
}

BOOL CreateIntermediateDirectory(LPCTSTR szPath) {
	return CreateSubDirectories(szPath);
}
bool CreateSubDirectories(LPCTSTR szPath) {
	if (!szPath || !_tcslen(szPath))
		return false;

	CStrings strs;
	CString strPath = szPath;
	strPath.Replace('/', '\\');

	SplitString(strPath, '\\', strs);
	strPath.Empty();
	CFileFind ff;
	for (int i = 0; i < strs.N(); i++) {
		if (strs[i].IsEmpty())
			continue;
		bool bFound = false;
		strPath += strs[i];
	#if 0
		if (!i) {
			bFound = ff.FindFile(strPath + "\\*.*");
		} else {
			bFound = ff.FindFile(strPath);
		}
	#else
		bFound = ff.FindFile(strPath) ? true : false;
	#endif
		if (bFound) {
			ff.FindNextFile();
			if (!ff.IsDirectory())
				return false;
		} else {
			CreateDirectory(strPath, NULL);
			//if ( !CreateDirectory(strPath, NULL) && (GetLastError() != ERROR_FILE_EXISTS) )
			//	return FALSE;
		}
		strPath += '\\';
	}
	return true;
}

#ifdef _USE_SHELL_API
#include "shlobj.h"
LPITEMIDLIST PidlBrowse(HWND hWnd, int nCSIDL, LPTSTR pszDisplayName, LPCTSTR pszTitle) {
	LPITEMIDLIST pidlRoot = NULL;
	LPITEMIDLIST pidlSelected = NULL;
	BROWSEINFO bi = {0};
	LPMALLOC pMalloc = NULL;

	SHGetMalloc(&pMalloc);

	if(nCSIDL) {
		//SHGetFolderLocation(hWnd, nCSIDL, NULL, NULL, &pidlRoot);
	} else {
		pidlRoot = NULL;
	}

	bi.hwndOwner = hWnd;
	bi.pidlRoot = pidlRoot;
	bi.pszDisplayName = pszDisplayName;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = 0;

	pidlSelected = SHBrowseForFolder(&bi);

	if(pidlRoot) {
		pMalloc->Free(pidlRoot);
	}
	pMalloc->Release();
	return pidlSelected;
}

CString BrowseFolder(HWND hWnd, int nCSIDL, LPCTSTR pszTitle) {
	CString strFolder;

	TCHAR* buf = strFolder.GetBuffer(2048);
	LPITEMIDLIST pidlSelected = PidlBrowse(hWnd, nCSIDL, buf, pszTitle);

	if (pidlSelected != NULL) {
		BOOL bret = SHGetPathFromIDList(pidlSelected, buf);
		if (bret) {
			strFolder.Format(_T("%s\\"), buf); 
		}
	}

	strFolder.ReleaseBuffer();

	return strFolder;
}
#endif

BOOL SetWindowFocus(HWND hWnd) {
	return ::SetForegroundWindow(hWnd);
}

BOOL SetWindowFocus(LPCTSTR strWindowName) {
	HWND hWnd = FindWindow(NULL, strWindowName);
	if (hWnd)
		return SetForegroundWindow(hWnd);
	return FALSE;
}

BOOL IsEditWindow(HWND hWnd) {
	TCHAR szClassName[1024];
	::GetClassName(hWnd, szClassName, sizeof(szClassName));

	return _tcscmp(szClassName, _T("Edit")) == 0;
}

DWORD QueryServiceStatus(LPCTSTR strServiceName) {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS|GENERIC_READ);
	if (!hSCManager) {
		TRACE(_T("Open SCManager Failed!\r\n"));
		return SERVICE_STOPPED;
	}
	SC_HANDLE hSCService = OpenService(hSCManager, strServiceName, STANDARD_RIGHTS_REQUIRED|GENERIC_READ|SERVICE_QUERY_STATUS);
	if (!hSCService) {
		CloseServiceHandle(hSCManager);
		TRACE(_T("Open Service Failed!\r\n"));
		return SERVICE_STOPPED;
	}
	SERVICE_STATUS ss;
	memset(&ss, 0, sizeof(ss));
	DWORD dwResult;
	if (QueryServiceStatus(hSCService, &ss)) {
		dwResult = ss.dwCurrentState;
	} else
		dwResult = SERVICE_STOPPED;

	CloseServiceHandle(hSCService);
	CloseServiceHandle(hSCManager);

	return dwResult;
}

sizeXX_t FindApp(LPCTSTR pszFullPath, TList<DWORD>& pIDs, bool bNameOnly) {
	//DWORD dwProcessID = 0;
	std::vector<DWORD> dwProcessIDs;
	dwProcessIDs.assign(4096, 0);
	DWORD size = 0;
	unsigned int nProcess;
	EnumProcesses(dwProcessIDs.data(), dwProcessIDs.size()*sizeof(dwProcessIDs[0]), &size);
	nProcess = size/sizeof(dwProcessIDs[0]);

	TCHAR buf[1024];
	for (unsigned int i = 0; i < nProcess; i++) {
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION/*PROCESS_QUERY_INFORMATION|PROCESS_VM_READ*/, FALSE, dwProcessIDs[i]);
		if (!hProcess)
			continue;
	#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)	// GetModuleFileNameEx () : 모든 Process 를 다 가지고 오지 못함.
		DWORD dwSize = countof(buf);
		if (!QueryFullProcessImageName(hProcess, 0, buf, &dwSize))
			continue;
	#else
		//<< XP
		if (!GetModuleFileNameEx(hProcess, NULL, buf, countof(buf)))
			continue;
		//if (!GetProcessImageFileName(hProcess, buf, countof(buf)))
		//	continue;
		//GetFullPathName(temp, countof(buf), buf, NULL);
		//>>
	#endif

		CloseHandle(hProcess);
		if (size) {
			CString str;
			if (bNameOnly) {
				CString folder, name;
				SplitPath(buf, folder, name);
				str = name;
			} else {
				str = buf;
			}
			if (str.CompareNoCase(pszFullPath) == 0) {
				pIDs.Push(new DWORD(dwProcessIDs[i]));
			}
		}
	}

	return pIDs.N();
}

bool ForceTerminateApp(LPCTSTR pszFullPath, bool bNameOnly, bool bExcludeSelf) {
	CWaitCursor wc;
	bool bExist = false;
	DWORD dwSelf = GetCurrentProcessId();
	bool bKillSelf = false;

	TList<DWORD> pIDs;
	FindApp(pszFullPath, pIDs, bNameOnly);
	for (DWORD dwProcessID : pIDs) {
		if (dwProcessID == dwSelf) {
			bKillSelf = !bExcludeSelf;
			continue;
		}

		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, dwProcessID);
		if (!hProcess) {
			//TRACE1("OpenProcess(%s) 실패\r\n", pszFullPath);
			continue;
		}

		bExist = TRUE;

		BOOL bResult = ::TerminateProcess(hProcess, 0);
		if (bResult) {
			//TRACE1("%s를 종료시킴\r\n", pszFullPath);
		} else {
			//TRACE1("%s를 종료시키지 못했습니다.\r\n", pszFullPath);
		}
	}
	if (bKillSelf) {
		TerminateProcess(GetCurrentProcess(), 0);
	}

	return TRUE;
}

BOOL SetAppPriorityClass(LPCTSTR pszAppName, DWORD dwPriorityClass) {
	TList<DWORD> pIDs;
	FindApp(pszAppName, pIDs, TRUE);
	if (!pIDs.N())
		return FALSE;
	for (int i = 0; i < pIDs.N(); i++) {
		HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pIDs[i]);
		if (!hProcess || !SetPriorityClass(hProcess, dwPriorityClass))
			return FALSE;
	}
	return TRUE;
}

void Restart(UINT uFlags, DWORD dwReason) {
	HANDLE hProcess = ::GetCurrentProcess();

	HANDLE hToken;
	OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken);

	LUID luid;
	memset(&luid, 0, sizeof(luid));
	::LookupPrivilegeValue(NULL, _T("SeShutdownPrivilege"), &luid);

	TOKEN_PRIVILEGES tkp;
	LUID_AND_ATTRIBUTES d;
	d.Attributes = 0;
	TOKEN_PRIVILEGES tkpTemp;
	LUID_AND_ATTRIBUTES d2;
	d2.Attributes = 0;
	DWORD dTemp;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkpTemp), &tkpTemp, &dTemp);
	ExitWindowsEx(uFlags, dwReason);
}

BOOL SuspendSystem(BOOL bSuspended, BOOL bForce) {
	HANDLE hProcess = ::GetCurrentProcess();

	HANDLE hToken;
	OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken);

	LUID luid;
	memset(&luid, 0, sizeof(luid));
	::LookupPrivilegeValue(NULL, _T("SeShutdownPrivilege"), &luid);

	TOKEN_PRIVILEGES tkp;
	LUID_AND_ATTRIBUTES d;
	d.Attributes = 0;
	TOKEN_PRIVILEGES tkpTemp;
	LUID_AND_ATTRIBUTES d2;
	d2.Attributes = 0;
	DWORD dTemp;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkpTemp), &tkpTemp, &dTemp);

	SetSystemPowerState(bSuspended, bForce);

	return TRUE;
}

//#if defined(_DEBUG) && defined(_UNICODE) && (_MSC_VER > 1200)
//void TRACE_U(const wchar_t* pszFormat, ...) {
//	va_list arglist;
//	va_start(arglist, pszFormat);
//
//	wchar_t wszDest[4096];
//	char szDest[8192];
//	ZeroVar(wszDest);
//	ZeroVar(szDest);
//#if (_MSC_VER > 1200)
//	vswprintf(wszDest, countof(wszDest), pszFormat, arglist);
//#else
//	vswprintf(wszDest, pszFormat, arglist);
//#endif
//
//	BOOL bUsedDefaultChar = FALSE;
//	if (WideCharToMultiByte(CP_ACP, 0, wszDest, -1, szDest, (int)sizeof(szDest), " ", &bUsedDefaultChar))
//		ATLTRACE("%s", szDest);
//	else
//		ATLTRACE(L"%s", wszDest);
//
//	va_end(arglist);
//}
//
//void TRACE_U(const char* pszFormat, ...) {
//	va_list arglist;
//	va_start(arglist, pszFormat);
//
//	char szDest[8192];
//	ZeroVar(szDest);
//#if (_MSC_VER > 1200)
//	vsprintf_s(szDest, pszFormat, arglist);
//#else
//	vsprintf(szDest, pszFormat, arglist);
//#endif
//	ATLTRACE(szDest);
//
//	va_end(arglist);
//}
//#endif	// defined(_DEBUG) && defined(_UNICODE)

BOOL IsTimedout(DWORD dwTickStart, DWORD dwTimeout, DWORD dwTickCurrent) {
	if (dwTimeout == INFINITE)
		return FALSE;

	return (dwTickCurrent - dwTickStart) > dwTimeout;

	////DWORD dwTickD = dwTickStart + dwTimeout;
	//return IsTimedout(dwTickStart + dwTimeout, dwTickCurrent);
}

BOOL IsTimedout(DWORD dwTickD, DWORD dwTickCurrent) {
	return (
		(dwTickD == dwTickCurrent)
		|| ( (dwTickD < dwTickCurrent) && ((dwTickCurrent - dwTickD) < MAXLONG) )
		|| ( (dwTickD > dwTickCurrent) && ((dwTickD - dwTickCurrent) > MAXLONG) )
		);
}


BOOL GetNICInfo(std::vector<T_NIC_INFO>& nics) {
	nics.clear();
	DWORD Err = 0;

	PFIXED_INFO pFixedInfo = NULL;
	DWORD FixedInfoSize = 0;

	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	DWORD AdapterInfoSize = 0;
	PIP_ADDR_STRING pAddrStr = NULL;

	//
	// Get the main IP configuration information for this machine using a FIXED_INFO structure
	//
	if ((Err = GetNetworkParams(NULL, &FixedInfoSize)) != 0) {
		if (Err != ERROR_BUFFER_OVERFLOW) {
			TRACE("GetNetworkParams sizing failed with error %d\n", Err);
			return FALSE;
		}
	}

	// Allocate memory from sizing information
	if ((pFixedInfo = (PFIXED_INFO) GlobalAlloc(GPTR, FixedInfoSize)) == NULL) {
		TRACE("Memory allocation error\n");
		return FALSE;
	}

	if ((Err = GetNetworkParams(pFixedInfo, &FixedInfoSize)) == 0) {
		//TRACE("\tHost Name . . . . . . . . . : %s\n", pFixedInfo->HostName);
		//TRACE("\tDNS Servers . . . . . . . . : %s\n", pFixedInfo->DnsServerList.IpAddress.String);
		//pAddrStr = pFixedInfo->DnsServerList.Next;
		//while(pAddrStr) {
		//	TRACE("%52s\n", pAddrStr->IpAddress.String);
		//	pAddrStr = pAddrStr->Next;
		//}

		//TRACE("\tNode Type . . . . . . . . . : ");
		//switch (pFixedInfo->NodeType)
		//{
		//case 1:
		//	TRACE("%s\n", "Broadcast");
		//	break;
		//case 2:
		//	TRACE("%s\n", "Peer to peer");
		//	break;
		//case 4:
		//	TRACE("%s\n", "Mixed");
		//	break;
		//case 8:
		//	TRACE("%s\n", "Hybrid");
		//	break;
		//default:
		//	TRACE("\n");
		//}

		//TRACE("\tNetBIOS Scope ID. . . . . . : %s\n", pFixedInfo->ScopeId);
		//TRACE("\tIP Routing Enabled. . . . . : %s\n", (pFixedInfo->EnableRouting ? "yes" : "no"));
		//TRACE("\tWINS Proxy Enabled. . . . . : %s\n", (pFixedInfo->EnableProxy ? "yes" : "no"));
		//TRACE("\tNetBIOS Resolution Uses DNS : %s\n", (pFixedInfo->EnableDns ? "yes" : "no"));
	} else {
		TRACE("GetNetworkParams failed with error %d\n", Err);
		return FALSE;
	}

	//
	// Enumerate all of the adapter specific information using the IP_ADAPTER_INFO structure.
	// Note:  IP_ADAPTER_INFO contains a linked list of adapter entries.
	//
	AdapterInfoSize = 0;
	if ((Err = GetAdaptersInfo(NULL, &AdapterInfoSize)) != 0) {
		if (Err != ERROR_BUFFER_OVERFLOW) {
			TRACE("GetAdaptersInfo sizing failed with error %d\n", Err);
			return FALSE;
		}
	}

	// Allocate memory from sizing information
	if ((pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc(GPTR, AdapterInfoSize)) == NULL) {
		TRACE("Memory allocation error\n");
		return FALSE;
	}

	// Get actual adapter information
	if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0) {
		TRACE("GetAdaptersInfo failed with error %d\n", Err);
		return FALSE;
	}

	nics.reserve(10);
	for (PIP_ADAPTER_INFO pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next) {
		T_NIC_INFO nic;
		nic.Type = pAdapt->Type;

		switch (pAdapt->Type) {
		case MIB_IF_TYPE_ETHERNET:
			TRACE("Ethernet adapter\n");
			break;
		case MIB_IF_TYPE_TOKENRING:
			TRACE("Token Ring adapter\n");
			break;
		case MIB_IF_TYPE_FDDI:
			TRACE("FDDI adapter\n");
			break;
		case MIB_IF_TYPE_PPP:
			TRACE("PPP adapter\n");
			break;
		case MIB_IF_TYPE_LOOPBACK:
			TRACE("Loopback adapter\n");
			break;
		case MIB_IF_TYPE_SLIP:
			TRACE("Slip adapter\n");
			break;
		case IF_TYPE_IEEE80211:
			TRACE("IEEE 802.11 wireless\n");
			break;
		case MIB_IF_TYPE_OTHER:
		default:
			TRACE("Other adapter\n");
			//continue;
			break;
		}
		//TRACE("%s:\n\n", pAdapt->AdapterName);
		nic.strName = pAdapt->AdapterName;

		//TRACE("\tDescription . . . . . . . . : %s\n", pAdapt->Description); 
		nic.strDescription = pAdapt->Description;

		//TRACE("\tPhysical Address. . . . . . : ");
		for (UINT i=0; i<_min(countof(nic.mac.addr), pAdapt->AddressLength); i++) {
			//if (i == (pAdapt->AddressLength - 1))
			//	TRACE("%.2X\n",(int)pAdapt->Address[i]);
			//else
			//	TRACE("%.2X-",(int)pAdapt->Address[i]);

			nic.mac.addr[i] = pAdapt->Address[i];
			//strMAC += FormatA("%.02x", (int)pAdapt->Address[i]);
		}

		//TRACE("\tDHCP Enabled. . . . . . . . : %s\n", (pAdapt->DhcpEnabled ? "yes" : "no"));

		pAddrStr = &(pAdapt->IpAddressList);
		CString strIP, strMask;
		while (pAddrStr) {
			if (!strIP.IsEmpty()) {
				strIP.Empty();
				strMask.Empty();
			}
			strIP += pAddrStr->IpAddress.String;
			strMask += pAddrStr->IpMask.String;
			//TRACE("\tIP Address. . . . . . . . . : %s\n", pAddrStr->IpAddress.String);
			//TRACE("\tSubnet Mask . . . . . . . . : %s\n", pAddrStr->IpMask.String);

			break;	// 한 개만 적용. 가끔 가다 2~3개씩 들어오는 경우도 있음.

			pAddrStr = pAddrStr->Next;
		}
		nic.ip.SetFromString((LPCTSTR)strIP);
		nic.subnetmask.SetFromString((LPCTSTR)strMask);

		//TRACE("\tDefault Gateway . . . . . . : %s\n", pAdapt->GatewayList.IpAddress.String);
		//pAddrStr = pAdapt->GatewayList.Next;
		//while(pAddrStr) {
		//	TRACE("%52s\n", pAddrStr->IpAddress.String);
		//	pAddrStr = pAddrStr->Next;
		//}

		//TRACE("\tDHCP Server . . . . . . . . : %s\n", pAdapt->DhcpServer.IpAddress.String);
		//TRACE("\tPrimary WINS Server . . . . : %s\n", pAdapt->PrimaryWinsServer.IpAddress.String);
		//TRACE("\tSecondary WINS Server . . . : %s\n", pAdapt->SecondaryWinsServer.IpAddress.String);

		//struct tm *newtime;

		//// Display coordinated universal time - GMT 
		//newtime = gmtime(&pAdapt->LeaseObtained);
		//if (newtime)
		//	TRACE( "\tLease Obtained. . . . . . . : %s", asctime( newtime ) );
		//else
		//	TRACE( "\tLease Obtained. . . . . . . " );

		//newtime = gmtime(&pAdapt->LeaseExpires);
		//if (newtime)
		//	TRACE( "\tLease Expires . . . . . . . : %s", asctime( newtime ) );
		//else
		//	TRACE( "\tLease Expires . . . . . . . " );

		nics.push_back(nic);

		//if (!strMAC.IsEmpty()) {
		//	nics.push_back(T_IP_MAC_ADDRESS(ip, mac));
		//	strNICName.Replace(_T(':'), _T('_'));
		//	addrs().strNICName = strNICName;
		//	addrs().strMAC = strMAC;
		//	addrs().strIP = strIP;
		//}
	}

	return TRUE;

}


void CRectTrackerCenterMark::DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd) {
	__super::DrawTrackerRect(lpRect, pWndClipTo, pDC, pWnd);
	if (lpRect && pDC) {
		CRect rect(*lpRect);
		// convert to client coordinates
		if (pWndClipTo) {
			pWnd->ClientToScreen(rect);
			pWndClipTo->ScreenToClient(rect);
		}

		CRect rectExclude(m_rectExclude);
		if (!rectExclude.IsRectEmpty()) {
			if (pWndClipTo) {
				pWnd->ClientToScreen(rectExclude);
				pWndClipTo->ScreenToClient(rectExclude);
			}
			pDC->ExcludeClipRect(rectExclude);
		}

		const int nSize = 30;

		int eOldMode = pDC->SetROP2(R2_XORPEN);
		CPen pen(PS_SOLID, 1, m_crCrossMark);
		CPen* pOldPen = pDC->SelectObject(&pen);

		CPoint ptCenter;
		if (m_bErase) {
			if (m_ptCenterLast.x >= 0) {
				ptCenter = m_ptCenterLast;
				pDC->MoveTo(ptCenter.x - nSize/2, ptCenter.y);
				pDC->LineTo(ptCenter.x + nSize/2, ptCenter.y);
				pDC->MoveTo(ptCenter.x, ptCenter.y - nSize/2);
				pDC->LineTo(ptCenter.x, ptCenter.y + nSize/2);
			}
		} else {
			ptCenter = rect.CenterPoint();
			pDC->MoveTo(ptCenter.x - nSize/2, ptCenter.y);
			pDC->LineTo(ptCenter.x + nSize/2, ptCenter.y);
			pDC->MoveTo(ptCenter.x, ptCenter.y - nSize/2);
			pDC->LineTo(ptCenter.x, ptCenter.y + nSize/2);

			m_ptCenterLast = ptCenter;
		}

		pDC->SelectObject(pOldPen);
		pDC->SetROP2(eOldMode);
	}
}

#endif
