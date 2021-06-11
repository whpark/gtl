/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TMDialog : Modal/Modeless Dialog
//
// PWH.
//
// 2021.06.10 from Mocha
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/win_util/win_util.h"

#include <afxdialogex.h>

namespace gtl::win_util {

	// TMDialog dialog

	GTL_WINUTIL_API int32_t		GetDlgItemInt32(CWnd* pWnd, int idc, int eRadix = 0);
	GTL_WINUTIL_API void		SetDlgItemInt32(CWnd* pWnd, int idc, int32_t iValue, LPCTSTR pszFMT = _T("%d"));
	GTL_WINUTIL_API int64_t		GetDlgItemInt64(CWnd* pWnd, int idc, int eRadix = 0);
	GTL_WINUTIL_API void		SetDlgItemInt64(CWnd* pWnd, int idc, int64_t iValue, LPCTSTR pszFMT = _T("%I64d"));
	GTL_WINUTIL_API double		GetDlgItemDouble(CWnd* pWnd, int idc);
	GTL_WINUTIL_API void		SetDlgItemDouble(CWnd* pWnd, int idc, double dValue, LPCTSTR pszFMT = _T("%.5f"));
	GTL_WINUTIL_API void		DDX_Double(CDataExchange* pDX, int nIDC, double& value, LPCTSTR pszFMT = _T("%.5f"));
	GTL_WINUTIL_API void		DDX_String(CDataExchange* pDX, int nIDC, char* sz, size_t nSize);
	GTL_WINUTIL_API void		DDX_String(CDataExchange* pDX, int nIDC, wchar_t* sz, size_t nSize);
	GTL_WINUTIL_API bool		CheckAndSetDlgItemText(CWnd* pWnd, int idc, LPCTSTR pszText, ::CString* pStrOrigin = nullptr);	// returns true if changed
	GTL_WINUTIL_API ::CString	GetDlgItemText(CWnd* pWnd, int idc);

	template < typename T_COORD, class = T_COORD::coord_t >
	void DDX_Coord(CDataExchange* pDX, int nIDC, T_COORD& coord, LPCTSTR pszFMT = _T("%.3f")) {
		if (!pDX)
			return;
		::CString str;
		if (pDX->m_bSaveAndValidate) {
			::DDX_Text(pDX, nIDC, str);
			Text2Coord(coord, str);
		} else {
			::CString str;
			Coord2Text(coord, str, pszFMT);
			::DDX_Text(pDX, nIDC, str);
		}
	}
	template < typename T_COORD, class = T_COORD::coord_t >
	T_COORD GetDlgItemCoord(CWnd* pWnd, int idc) {
		T_COORD coord;
		::CString str = GetDlgItemText(pWnd, idc);
		Text2Coord(coord, str);
		return coord;
	}
	template < typename T_COORD, class = T_COORD::coord_t >
	void SetDlgItemCoord(CWnd* pWnd, int idc, T_COORD& coord, LPCTSTR pszFMT = nullptr) {
		::CString str;
		Coord2Text(coord, str, pszFMT);
		SetDlgItemText(pWnd->GetSafeHwnd(), idc, str);
	}

	//=============================================================================
	// Subclassing

	template < class CCtrl, class Creator >
	bool SubclassItem(CWnd* pWndOwner, int idc, CCtrl& ctrl, Creator creator, bool bCheckDynamicLayout = true) {

		if (!pWndOwner)
			return false;

		CWnd* pWnd = pWndOwner->GetDlgItem(idc);
		if (!pWnd)
			return false;

		//CMFCDynamicLayout* pDynamicLayout = nullptr;
		//if (bCheckDynamicLayout && pDynamicLayout) {
		//	pDynamicLayout = pWndOwner->GetDynamicLayout();
		//	if (!pDynamicLayout->HasItem(pWnd->m_hWnd))
		//		bCheckDynamicLayout = false;
		//}

		CRect rect;
		pWnd->GetWindowRect(rect);
		pWndOwner->ScreenToClient(rect);
		CWnd* pWndPrev = pWndOwner->GetNextDlgTabItem(pWnd, TRUE);
		pWnd->DestroyWindow();

		creator();
		ctrl.MoveWindow(rect);

		if (pWndPrev)
			ctrl.SetWindowPos(pWndPrev, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

		//if (bCheckDynamicLayout && pDynamicLayout) {
		//	pDynamicLayout->AddItem
		//}

		return true;
	};
	template < class CCtrl >
	bool SubclassItemWnd(CWnd* pWndOwner, int idc, CCtrl& ctrl, DWORD dwStyle = WS_CHILD|WS_VISIBLE|WS_BORDER /*|WS_TABSTOP|WS_VSCROLL|WS_HSCROLL*/, bool bCheckDynamicLayout = true) {
		return SubclassItem(pWndOwner, idc, ctrl, [&]() { ctrl.Create(nullptr, nullptr, dwStyle, CRect(), pWndOwner, idc); }, bCheckDynamicLayout);
	}

	//=============================================================================
	// Clipboard

	GTL_WINUTIL_API CEdit* GetFocusedEdit(CWnd* pWnd);
	GTL_WINUTIL_API BOOL OnEditCopy(CWnd* pWnd);
	GTL_WINUTIL_API BOOL OnEditCut(CWnd* pWnd);
	GTL_WINUTIL_API BOOL OnEditPaste(CWnd* pWnd);
	GTL_WINUTIL_API BOOL OnEditUndo(CWnd* pWnd);


	//=============================================================================
	//

	inline BOOL OnCmdMsg_EditFunction(CWnd* pWnd, UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) {
		if (nCode == CN_COMMAND) {
			if (pExtra) {
				switch (nID) {
				case ID_EDIT_COPY :
				case ID_EDIT_CUT :
				case ID_EDIT_PASTE :
				case ID_EDIT_UNDO :
					if (GetFocusedEdit(pWnd))
						return TRUE;
					break;
				}
			} else {
				switch (nID) {
				case ID_EDIT_COPY :		if (OnEditCopy(pWnd)) return TRUE; break;
				case ID_EDIT_CUT :		if (OnEditCut(pWnd)) return TRUE; break;
				case ID_EDIT_PASTE :	if (OnEditPaste(pWnd)) return TRUE;  break;
				case ID_EDIT_UNDO :		if (OnEditUndo(pWnd)) return TRUE; break;
				}
			}
		}
		return FALSE;
	}


	#define INCLUDE_CLIPBOARD_FUNC()\
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) {\
		if (OnCmdMsg_EditFunction(this, nID, nCode, pExtra, pHandlerInfo))\
			return TRUE;\
		return __super::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);\
	}\


	template <class WNDCLASS>
	class TWndEditFunc : public WNDCLASS {
	public:
		template <typename ... A>
		TWndEditFunc<WNDCLASS>(A... a) : WNDCLASS(a...) {}

		virtual ~TWndEditFunc<WNDCLASS>() { }

		INCLUDE_CLIPBOARD_FUNC();
	};

	//=============================================================================
	//
	class GTL_WINUTIL_CLASS CMDialogEx : public CDialogEx {
	protected:
		CWnd* m_pWndParent;
		BOOL m_bModal;

	public:
		CMDialogEx(int eIDD, CWnd* pParent = NULL) : m_eIDD(eIDD), CDialogEx(eIDD, pParent) {
			m_pWndParent = pParent;
			m_bModal = TRUE;
		}
		CMDialogEx(LPCTSTR pszIDD, CWnd* pParent = NULL) : m_eIDD(0), m_strIDD(pszIDD), CDialogEx(pszIDD, pParent) {
			m_pWndParent = pParent;
			m_bModal = TRUE;
		}
		virtual ~CMDialogEx() { }

		int const m_eIDD;
		::CString const m_strIDD;

	public:
		BOOL IsModal() const { return m_bModal; }

	public:
		virtual BOOL Create(CWnd* pParent) {
			m_bModal = FALSE;
			m_pWndParent = pParent;

			if (m_eIDD == 0)
				return CDialogEx::Create(m_strIDD, pParent);
			return CDialogEx::Create(m_eIDD, pParent);
		}
		virtual void OnOK() {
			UpdateData(TRUE);
			SyncData(TRUE);
			if (m_bModal) {
				CDialogEx::OnOK();
			} else {
				if (IsPopupWindow())
					ShowWindow(SW_HIDE);
			}
		}
		virtual void OnCancel() {
			if (m_bModal) {
				__super::OnCancel();
			} else {
				//SyncData(FALSE);
				//UpdateData(FALSE);
				DWORD dwStyle = GetStyle();
				if (IsPopupWindow())
					ShowWindow(SW_HIDE);
			}
		}
		bool IsPopupWindow() const {
			DWORD dwStyle = GetStyle();
			return ( dwStyle & (WS_POPUP|WS_DLGFRAME|WS_THICKFRAME/*|WS_CAPTION|WS_SYSMENU*/) ) ? true : false;
		}
		CWnd* GetParent() { return m_pWndParent; }

		CEdit* GetFocusedEdit() { return gtl::win_util::GetFocusedEdit(this); }

		CMFCDynamicLayout* CreateDynamicLayout() {
			CMFCDynamicLayout* pDynamicLayout = GetDynamicLayout();
			if (!pDynamicLayout) {
				//EnableDynamicLayout(true);
				//pDynamicLayout = GetDynamicLayout();
				//if (!pDynamicLayout) {
					pDynamicLayout = new CMFCDynamicLayout;
					pDynamicLayout->Create(this);
					CRect rect;
					GetClientRect(rect);
					pDynamicLayout->SetMinSize(CSize(rect.Width(), rect.Height()));
					m_pDynamicLayout = pDynamicLayout;
				//}
			}
			return pDynamicLayout;
		}

	public:
		virtual BOOL SyncData(BOOL bStore) { return TRUE; }

		int32_t GetDlgItemInt32 (int idc, int eRadix = 0)								{ return gtl::win_util::GetDlgItemInt32(this, idc, eRadix); }
		void    SetDlgItemInt32 (int idc, int32_t iValue, LPCTSTR pszFMT = _T("%I32d"))	{ gtl::win_util::SetDlgItemInt32(this, idc, iValue, pszFMT); }
		int64_t GetDlgItemInt64 (int idc, int eRadix = 0)								{ return gtl::win_util::GetDlgItemInt64(this, idc, eRadix); }
		void    SetDlgItemInt64 (int idc, int64_t iValue, LPCTSTR pszFMT = _T("%I64d"))	{ gtl::win_util::SetDlgItemInt64(this, idc, iValue, pszFMT); }
		double  GetDlgItemDouble(int idc)												{ return gtl::win_util::GetDlgItemDouble(this, idc); }
		void    SetDlgItemDouble(int idc, double dValue, LPCTSTR pszFMT = _T("%.5f"))	{ gtl::win_util::SetDlgItemDouble(this, idc, dValue, pszFMT); }

		template < typename T_COORD, class = T_COORD::coord_t >
		T_COORD GetDlgItemCoord(int idc) {
			T_COORD coord;
			::CString str = GetDlgItemText(idc);
			Text2Coord(coord, str);
			return coord;
		}
		template < typename T_COORD, class = T_COORD::coord_t >
		T_COORD GetDlgItemCoord(int idc, T_COORD& coord) {
			::CString str = GetDlgItemText(idc);
			Text2Coord(coord, str);
			return coord;
		}
		template < typename T_COORD, class = T_COORD::coord_t >
		void SetDlgItemCoord(int idc, T_COORD& coord, LPCTSTR pszFMT = nullptr) {
			::CString str;
			Coord2Text(coord, str, pszFMT);
			CheckAndSetDlgItemText(idc, str);
		}

		bool CheckAndSetDlgItemText(int idc, LPCTSTR pszText, ::CString* pStrOrigin = nullptr) {
			return gtl::win_util::CheckAndSetDlgItemText(this, idc, pszText, pStrOrigin);
		}
		using CDialogEx::GetDlgItemText;
		//int GetDlgItemText(int idc, CString& str) { return __super::GetDlgItemText(idc, str); }
		//int GetDlgItemText(int idc, LPTSTR lpStr, int nMaxCount ) { return __super::GetDlgItemText(idc, lpStr, nMaxCount); }
		::CString GetDlgItemText(int idc) { return gtl::win_util::GetDlgItemText(this, idc); }

	public:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnEditCopy();
		afx_msg void OnEditCut();
		afx_msg void OnEditPaste();
		afx_msg void OnEditUndo();

	};

	//=============================================================================
	//
	template < class DIALOG >
	class TMDialog : public DIALOG {
	protected:
		CWnd* m_pWndParent;
		BOOL m_bModal;

	public:
		TMDialog(int eIDD, CWnd* pParent = NULL) : m_eIDD(eIDD), DIALOG(eIDD, pParent) {
			m_pWndParent = pParent;
			m_bModal = TRUE;
		}
		TMDialog(LPCTSTR pszIDD, CWnd* pParent = NULL) : m_eIDD(0), m_strIDD(pszIDD), CDialogEx(pszIDD, pParent) {
			m_pWndParent = pParent;
			m_bModal = TRUE;
		}
		virtual ~TMDialog() { }

		int const m_eIDD;
		::CString const m_strIDD;

	public:
		BOOL IsModal() const { return m_bModal; }

	public:
		virtual BOOL Create(CWnd* pParent) {
			m_bModal = FALSE;
			m_pWndParent = pParent;

			if (m_eIDD == 0)
				return DIALOG::Create(m_strIDD, pParent);
			return DIALOG::Create(m_eIDD, pParent);
		}
		virtual void OnOK() {
			this->UpdateData(TRUE);
			SyncData(TRUE);
			if (m_bModal) {
				DIALOG::OnOK();
			} else {
				if (IsPopupWindow())
					this->ShowWindow(SW_HIDE);
			}
		}
		virtual void OnCancel() {
			if (m_bModal) {
				__super::OnCancel();
			} else {
				//SyncData(FALSE);
				//UpdateData(FALSE);
				if (this->IsPopupWindow())
					this->ShowWindow(SW_HIDE);
			}
		}
		bool IsPopupWindow() const {
			DWORD dwStyle = this->GetStyle();
			return ( dwStyle & (WS_POPUP|WS_DLGFRAME|WS_THICKFRAME|WS_CAPTION|WS_SYSMENU) ) ? true : false;
		}
		CWnd* GetParent() { return m_pWndParent; }

		CEdit* GetFocusedEdit() { return ::GetFocusedEdit(this); }

		CMFCDynamicLayout* CreateDynamicLayout() {
			CMFCDynamicLayout* pDynamicLayout = this->GetDynamicLayout();
			if (!pDynamicLayout) {
				//EnableDynamicLayout(true);
				//pDynamicLayout = GetDynamicLayout();
				//if (!pDynamicLayout) {
				pDynamicLayout = new CMFCDynamicLayout;
				pDynamicLayout->Create(this);
				CRect rect;
				this->GetClientRect(rect);
				pDynamicLayout->SetMinSize(CSize(rect.Width(), rect.Height()));
				this->m_pDynamicLayout = pDynamicLayout;
				//}
			}
			return pDynamicLayout;
		}

	public:
		virtual BOOL SyncData(BOOL bStore) { return TRUE; }

		int32_t GetDlgItemInt32 (int idc, int eRadix = 0)								{ return gtl::win_util::GetDlgItemInt32(this, idc, eRadix); }
		void    SetDlgItemInt32 (int idc, int32_t iValue, LPCTSTR pszFMT = _T("%I32d"))	{ gtl::win_util::SetDlgItemInt32(this, idc, iValue, pszFMT); }
		int64_t GetDlgItemInt64 (int idc, int eRadix = 0)								{ return gtl::win_util::GetDlgItemInt64(this, idc, eRadix); }
		void    SetDlgItemInt64 (int idc, int64_t iValue, LPCTSTR pszFMT = _T("%I64d"))	{ gtl::win_util::SetDlgItemInt64(this, idc, iValue, pszFMT); }
		double  GetDlgItemDouble(int idc)												{ return gtl::win_util::GetDlgItemDouble(this, idc); }
		void    SetDlgItemDouble(int idc, double dValue, LPCTSTR pszFMT = _T("%.5f"))	{ gtl::win_util::SetDlgItemDouble(this, idc, dValue, pszFMT); }

		template < typename T_COORD, class = T_COORD::coord_t >
		T_COORD GetDlgItemCoord(int idc) {
			T_COORD coord;
			::CString str = GetDlgItemText(idc);
			Text2Coord(coord, str);
			return coord;
		}
		template < typename T_COORD, class = T_COORD::coord_t >
		T_COORD GetDlgItemCoord(int idc, T_COORD& coord) {
			::CString str = GetDlgItemText(idc);
			Text2Coord(coord, str);
			return coord;
		}
		template < typename T_COORD, class = T_COORD::coord_t >
		void SetDlgItemCoord(int idc, T_COORD& coord, LPCTSTR pszFMT = nullptr) {
			::CString str;
			Coord2Text(coord, str, pszFMT);
			CheckAndSetDlgItemText(idc, str);
		}

		bool CheckAndSetDlgItemText(int idc, LPCTSTR pszText, ::CString* pStrOrigin = nullptr) {
			return gtl::win_util::CheckAndSetDlgItemText(this, idc, pszText, pStrOrigin);
		}
		using DIALOG::GetDlgItemText;
		//int GetDlgItemText(int idc, CString& str) { return __super::GetDlgItemText(idc, str); }
		//int GetDlgItemText(int idc, LPTSTR lpStr, int nMaxCount ) { return __super::GetDlgItemText(idc, lpStr, nMaxCount); }
		::CString GetDlgItemText(int idc) { return gtl::win_util::GetDlgItemText(this, idc); }

	public:
		DECLARE_MESSAGE_MAP()
		void OnEditCopy()  { gtl::win_util::OnEditCopy(this); }
		void OnEditCut()   { gtl::win_util::OnEditCut(this); }
		void OnEditPaste() { gtl::win_util::OnEditPaste(this);  }
		void OnEditUndo()  { gtl::win_util::OnEditUndo(this);  }

	};

	typedef TMDialog<CDialog> CMDialog;



}	// namespace gtl::win_util
