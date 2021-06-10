// MDialog.cpp : implementation file
//

#include "pch.h"
#include "gtl/win_util/TMDialog.h"

// CMDialog dialog

BEGIN_MESSAGE_MAP(CMDialogEx, CDialogEx)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
END_MESSAGE_MAP()


CEdit* GetFocusedEdit(CWnd* pWnd) {
	if (!pWnd || !pWnd->m_hWnd)
		return NULL;
	CWnd* pWndFocused = pWnd->GetFocus();
	if (!pWndFocused)
		return NULL;
	if (pWndFocused->IsKindOf(RUNTIME_CLASS(CEdit)))
		return (CEdit*)pWndFocused;
	TCHAR szClassName[64];
	::GetClassName(pWndFocused->m_hWnd, szClassName, std::size(szClassName));
	if (_tcscmp(szClassName, _T("Edit")) == 0)
		return (CEdit*)pWndFocused;

	return NULL;
}
BOOL OnEditCopy(CWnd* pWnd) {
	CEdit* pEdit = GetFocusedEdit(pWnd);
	if (!pEdit)
		return FALSE;
	pEdit->Copy();
	return TRUE;
}
BOOL OnEditCut(CWnd* pWnd) {
	CEdit* pEdit = GetFocusedEdit(pWnd);
	if (!pEdit)
		return FALSE;
	pEdit->Cut();
	return TRUE;
}
BOOL OnEditPaste(CWnd* pWnd) {
	CEdit* pEdit = GetFocusedEdit(pWnd);
	if (!pEdit)
		return FALSE;
	pEdit->Paste();
	return TRUE;
}
BOOL OnEditUndo(CWnd* pWnd) {
	CEdit* pEdit = GetFocusedEdit(pWnd);
	if (!pEdit)
		return FALSE;
	pEdit->Undo();
	return TRUE;
}

int32_t GetDlgItemInt32(CWnd* pWnd, int idc, int eRadix) {
	if (!pWnd || !pWnd->m_hWnd)
		return 0;

	CString str;
	pWnd->GetDlgItemText(idc, str);
	return _tcstol(str, NULL, eRadix);
}

void SetDlgItemInt32(CWnd* pWnd, int idc, int32_t iValue, LPCTSTR pszFMT) {
	if (!pWnd || !pWnd->m_hWnd)
		return;

	CString str;
	CString strFMT = pszFMT;
	if (strFMT.IsEmpty())
		strFMT = _T("%I32d");
	str.Format(strFMT, iValue);
	CheckAndSetDlgItemText(pWnd, idc, str);
}

int64_t GetDlgItemInt64(CWnd* pWnd, int idc, int eRadix) {
	if (!pWnd || !pWnd->m_hWnd)
		return 0;

	CString str;
	pWnd->GetDlgItemText(idc, str);
	return _tcstoi64(str, NULL, eRadix);
}

void SetDlgItemInt64(CWnd* pWnd, int idc, int64_t iValue, LPCTSTR pszFMT) {
	if (!pWnd || !pWnd->m_hWnd)
		return;

	CString str;
	CString strFMT = pszFMT;
	if (strFMT.IsEmpty())
		strFMT = _T("%I64d");
	str.Format(strFMT, iValue);
	CheckAndSetDlgItemText(pWnd, idc, str);
}

double GetDlgItemDouble(CWnd* pWnd, int idc) {
	if (!pWnd || !pWnd->m_hWnd)
		return 0.0;

	CString str;
	pWnd->GetDlgItemText(idc, str);
	return _ttof(str);
}

void SetDlgItemDouble(CWnd* pWnd, int idc, double dValue, LPCTSTR pszFMT) {
	if (!pWnd || !pWnd->m_hWnd)
		return;

	CString str;
	CString strFMT = pszFMT;
	if (strFMT.IsEmpty())
		strFMT = _T("%.5f");
	str.Format(strFMT, dValue);
	CheckAndSetDlgItemText(pWnd, idc, str);
}

void DDX_Double(CDataExchange* pDX, int idc, double& dValue, LPCTSTR pszFMT) {
	if (!pDX || !pDX->m_pDlgWnd)
		return;
	if (pDX->m_bSaveAndValidate) {
		dValue = GetDlgItemDouble(pDX->m_pDlgWnd, idc);
	} else {
		SetDlgItemDouble(pDX->m_pDlgWnd, idc, dValue, pszFMT);
	}
}

//WARNING_PUSH_AND_DISABLE(4996)

void DDX_String(CDataExchange* pDX, int idc, char* sz, size_t nSize) {
	CString str(sz);
	DDX_Text(pDX, idc, str);
	if (pDX->m_bSaveAndValidate) {

		#ifdef _UNICODE
		#	define STR CStringA(str)
		#else
		#	define STR str
		#endif

		if (nSize == (size_t)-1) {
			//strcpy(sz, STR);
		} else if (nSize > 0) {
			strncpy_s(sz, nSize, STR, nSize);
		}

		#undef STR
	}
}
void DDX_String(CDataExchange* pDX, int idc, wchar_t* sz, size_t nSize) {
	CString str(sz);
	DDX_Text(pDX, idc, str);
	if (pDX->m_bSaveAndValidate) {

		#ifdef _UNICODE
		#	define STR str
		#else
		#	define STR CStringW(str)
		#endif

		if (nSize == (size_t)-1) {
			//wcscpy(sz, CStringW(str));
		} else if (nSize > 0) {
			wcsncpy_s(sz, nSize, CStringW(str), nSize);
		}

		#undef STR
	}
}

//WARNING_POP()

bool CheckAndSetDlgItemText(CWnd* pWnd, int idc, LPCTSTR pszText, CString* pStrOrigin) {
	CString str;
	if (!pWnd)
		return false;
	pWnd->GetDlgItemText(idc, str);
	bool bChanged = (str != pszText);
	if (bChanged)
		pWnd->SetDlgItemText(idc, pszText);
	if (pStrOrigin)
		*pStrOrigin = str;
	return bChanged;
}

CString	GetDlgItemText(CWnd* pWnd, int idc) {
	CString str;
	if (!pWnd)
		return str;
	pWnd->GetDlgItemText(idc, str);
	return str;
}

void CMDialogEx::OnEditCopy()  { ::OnEditCopy(this); }
void CMDialogEx::OnEditCut()   { ::OnEditCut(this); }
void CMDialogEx::OnEditPaste() { ::OnEditPaste(this);  }
void CMDialogEx::OnEditUndo()  { ::OnEditUndo(this);  }
