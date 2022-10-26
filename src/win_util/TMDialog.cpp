// MDialog.cpp : implementation file
//

#include "pch.h"
#include "gtl/win_util/TMDialog.h"

namespace gtl::win_util {

	// CMDialog dialog

	//BEGIN_MESSAGE_MAP(CMDialogEx, CDialogEx)
	//	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	//	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	//	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	//	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	//END_MESSAGE_MAP()


	CEdit* GetFocusedEdit(CWnd* pWnd) {
		if (!pWnd || !pWnd->m_hWnd)
			return nullptr;
		CWnd* pWndFocused = pWnd->GetFocus();
		if (!pWndFocused)
			return nullptr;
		if (pWndFocused->IsKindOf(RUNTIME_CLASS(CEdit)))
			return (CEdit*)pWndFocused;
		TCHAR szClassName[64];
		::GetClassName(pWndFocused->m_hWnd, szClassName, (int)std::size(szClassName));
		if (_tcscmp(szClassName, _T("Edit")) == 0)
			return (CEdit*)pWndFocused;

		return nullptr;
	}
	bool OnEditCopy(CWnd* pWnd) {
		CEdit* pEdit = GetFocusedEdit(pWnd);
		if (!pEdit)
			return false;
		pEdit->Copy();
		return true;
	}
	bool OnEditCut(CWnd* pWnd) {
		CEdit* pEdit = GetFocusedEdit(pWnd);
		if (!pEdit)
			return false;
		pEdit->Cut();
		return true;
	}
	bool OnEditPaste(CWnd* pWnd) {
		CEdit* pEdit = GetFocusedEdit(pWnd);
		if (!pEdit)
			return false;
		pEdit->Paste();
		return true;
	}
	bool OnEditUndo(CWnd* pWnd) {
		CEdit* pEdit = GetFocusedEdit(pWnd);
		if (!pEdit)
			return false;
		pEdit->Undo();
		return true;
	}

	int32_t GetDlgItemInt32(CWnd* pWnd, int idc, int eRadix) {
		if (!pWnd || !pWnd->m_hWnd)
			return 0;

		CString str;
		pWnd->GetDlgItemText(idc, str);
		return _tcstol(str, nullptr, eRadix);
	}

	void SetDlgItemInt32(CWnd* pWnd, int idc, int32_t iValue, std::wstring_view fmt) {
		if (!pWnd || !pWnd->m_hWnd)
			return;

		if (fmt.empty())
			fmt = L"{}";
		auto str = fmt::format(fmt::runtime(fmt), iValue);
		CheckAndSetDlgItemText(pWnd, idc, str);
	}

	int64_t GetDlgItemInt64(CWnd* pWnd, int idc, int eRadix) {
		if (!pWnd || !pWnd->m_hWnd)
			return 0;

		CString str;
		pWnd->GetDlgItemText(idc, str);
		return _tcstoi64(str, nullptr, eRadix);
	}

	void SetDlgItemInt64(CWnd* pWnd, int idc, int64_t iValue, std::wstring_view fmt) {
		if (!pWnd || !pWnd->m_hWnd)
			return;

		if (fmt.empty())
			fmt = L"{}";
		auto str = fmt::format(fmt::runtime(fmt), iValue);
		CheckAndSetDlgItemText(pWnd, idc, str);
	}

	double GetDlgItemDouble(CWnd* pWnd, int idc) {
		if (!pWnd || !pWnd->m_hWnd)
			return 0.0;

		CString str;
		pWnd->GetDlgItemText(idc, str);
		return _ttof(str);
	}

	void SetDlgItemDouble(CWnd* pWnd, int idc, double dValue, std::wstring_view fmt) {
		if (!pWnd || !pWnd->m_hWnd)
			return;

		if (fmt.empty())
			fmt = L"{:.5f}";
		auto str = fmt::format(fmt::runtime(fmt), dValue);
		CheckAndSetDlgItemText(pWnd, idc, str);
	}

	void DDX_Double(CDataExchange* pDX, int idc, double& dValue, std::wstring_view fmt) {
		if (!pDX || !pDX->m_pDlgWnd)
			return;
		if (pDX->m_bSaveAndValidate) {
			dValue = GetDlgItemDouble(pDX->m_pDlgWnd, idc);
		} else {
			SetDlgItemDouble(pDX->m_pDlgWnd, idc, dValue, fmt);
		}
	}

	void DDX_DoubleScaled(CDataExchange* pDX, int idc, double& value, double scaleToView, std::wstring_view fmt) {
		if (!pDX || !pDX->m_pDlgWnd)
			return;
		double dScaledValue = value * scaleToView;
		gtlw::DDX_Double(pDX, idc, dScaledValue, fmt);
		if (pDX->m_bSaveAndValidate) {
			value = dScaledValue / scaleToView;
		}
	}

	void DDX_CheckBool(CDataExchange* pDX, int idc, bool& value) {
		if (!pDX || !pDX->m_pDlgWnd)
			return;
		BOOL bCheck = value;
		DDX_Check(pDX, idc, bCheck);
		if (pDX->m_bSaveAndValidate) {
			value = bCheck ? true : false;
		}
	}

	//WARNING_PUSH_AND_DISABLE(4996)

	template < typename tchar > requires gtlc::string_elem<tchar>
	void DDX_String(CDataExchange* pDX, int idc, std::basic_string<tchar>& str) {
		if (pDX->m_bSaveAndValidate) {
			auto strValue = GetDlgItemString(pDX->m_pDlgWnd, idc);

			if constexpr (std::is_same_v<tchar, TCHAR>) {
				str = (LPCTSTR)strValue;
			}
			else {
				str = ToString<tchar>({(LPCTSTR)strValue, (size_t)strValue.GetLength()});
			}
		}
		else {
			if constexpr (std::is_same_v<tchar, TCHAR>) {
				::SetDlgItemText(pDX->m_pDlgWnd, idc, str.c_str());
			} else {
				::SetDlgItemText(pDX->m_pDlgWnd, idc, ToString<TCHAR>(str).c_str());
			}
		}
	}

	//WARNING_POP()

	bool CheckAndSetDlgItemText(CWnd* pWnd, int idc, std::wstring const& str, CString* pStrOrigin) {
		if (!pWnd)
			return false;
		auto strOrg = GetDlgItemString(pWnd, idc);
		bool bChanged = ((LPCTSTR)strOrg != str);
		if (bChanged)
			pWnd->SetDlgItemText(idc, str.c_str());
		if (pStrOrigin)
			*pStrOrigin = strOrg;
		return bChanged;
	}

	CString GetDlgItemString(CWnd* pWnd, int idc) {
		CString str;
		if (!pWnd)
			return str;
		pWnd->GetDlgItemText(idc, str);
		return str;
	}


}	// namespace gtl::win_util
