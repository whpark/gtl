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

	GTL__WINUTIL_API int32_t	GetDlgItemInt32(CWnd* pWnd, int idc, int eRadix = 0);
	GTL__WINUTIL_API void		SetDlgItemInt32(CWnd* pWnd, int idc, int32_t iValue, std::wstring_view fmt = L"{}");
	GTL__WINUTIL_API int64_t	GetDlgItemInt64(CWnd* pWnd, int idc, int eRadix = 0);
	GTL__WINUTIL_API void		SetDlgItemInt64(CWnd* pWnd, int idc, int64_t iValue, std::wstring_view fmt = L"{}");
	GTL__WINUTIL_API double		GetDlgItemDouble(CWnd* pWnd, int idc);
	GTL__WINUTIL_API void		SetDlgItemDouble(CWnd* pWnd, int idc, double dValue, std::wstring_view fmt = _T("{:.5f}"));
	GTL__WINUTIL_API void		DDX_Double(CDataExchange* pDX, int idc, double& value, std::wstring_view fmt = _T("{:.5f}"));
	GTL__WINUTIL_API void		DDX_DoubleScaled(CDataExchange* pDX, int idc, double& value, double scaleToView, std::wstring_view fmt = _T("{:.5f}"));
	GTL__WINUTIL_API void		DDX_CheckBool(CDataExchange* pDX, int idc, bool& value);
	template < typename tchar > requires gtlc::string_elem<tchar>
	void		DDX_String(CDataExchange* pDX, int idc, std::basic_string<tchar>& str);
	GTL__WINUTIL_API bool		CheckAndSetDlgItemText(CWnd* pWnd, int idc, std::wstring const& sv, CString* pStrOrigin = nullptr);	// returns true if changed
	GTL__WINUTIL_API CString	GetDlgItemString(CWnd* pWnd, int idc);

	template < typename T_COORD > requires gtlc::coord<T_COORD>
	void DDX_Coord(CDataExchange* pDX, int idc, T_COORD& coord, fmt::basic_format_string<xString::value_type, typename T_COORD::value_type> fmt = {}) {
		if (!pDX)
			return;
		CString str;
		if (pDX->m_bSaveAndValidate) {
			::DDX_Text(pDX, idc, str);
			coord = FromString<T_COORD, CString::XCHAR>((LPCTSTR)str);
		} else {
			CString str;
			str = ToString(coord, fmt).c_str();
			::DDX_Text(pDX, idc, str);
		}
	}
	template < typename T_COORD > requires gtlc::coord<T_COORD>
	T_COORD GetDlgItemCoord(CWnd* pWnd, int idc) {
		T_COORD coord;
		auto str = GetDlgItemString(pWnd, idc);
		coord = FromString<T_COORD>(std::wstring_view(str));
		return coord;
	}
	template < typename T_COORD > requires gtlc::coord<T_COORD>
	void SetDlgItemCoord(CWnd* pWnd, int idc, T_COORD& coord, fmt::basic_format_string<CString::XCHAR, typename T_COORD::value_type> fmt = {}) {
		std::wstring str = ToString<wchar_t>(coord, fmt).c_str();
		SetDlgItemText(pWnd->GetSafeHwnd(), idc, str.c_str());
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
		CWnd* pWndPrev = pWndOwner->GetNextDlgTabItem(pWnd, true);
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

	GTL__WINUTIL_API CEdit* GetFocusedEdit(CWnd* pWnd);
	GTL__WINUTIL_API bool OnEditCopy(CWnd* pWnd);
	GTL__WINUTIL_API bool OnEditCut(CWnd* pWnd);
	GTL__WINUTIL_API bool OnEditPaste(CWnd* pWnd);
	GTL__WINUTIL_API bool OnEditUndo(CWnd* pWnd);


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
				case ID_EDIT_COPY :		if (OnEditCopy(pWnd)) return true; break;
				case ID_EDIT_CUT :		if (OnEditCut(pWnd)) return true; break;
				case ID_EDIT_PASTE :	if (OnEditPaste(pWnd)) return true;  break;
				case ID_EDIT_UNDO :		if (OnEditUndo(pWnd)) return true; break;
				}
			}
		}
		return false;
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


	//=============================================================================
	//
	template < class DIALOG >
	class TMDialog : public DIALOG {
	public:
		using base_t = DIALOG;
	protected:
		CWnd* m_pWndParent;
		BOOL m_bModal;

	public:
		TMDialog(int eIDD, CWnd* pParent = nullptr) : m_eIDD(eIDD), base_t(eIDD, pParent) {
			m_pWndParent = pParent;
			m_bModal = true;
		}
		TMDialog(LPCTSTR pszIDD, CWnd* pParent = nullptr) : m_eIDD(0), m_strIDD(pszIDD), base_t(pszIDD, pParent) {
			m_pWndParent = pParent;
			m_bModal = true;
		}
		virtual ~TMDialog() { }

		int const m_eIDD;
		CString const m_strIDD;

	public:
		BOOL IsModal() const { return m_bModal; }

	public:
		virtual BOOL Create(CWnd* pParent) {
			m_bModal = false;
			m_pWndParent = pParent;

			if (m_eIDD == 0)
				return base_t::Create(m_strIDD, pParent);
			return base_t::Create(m_eIDD, pParent);
		}
		virtual void OnOK() override {
			this->UpdateData(true);
			SyncData(true);
			if (m_bModal) {
				base_t::OnOK();
			} else {
				if (IsPopupWindow())
					this->ShowWindow(SW_HIDE);
			}
		}
		virtual void OnCancel() {
			if (m_bModal) {
				__super::OnCancel();
			} else {
				//SyncData(false);
				//UpdateData(false);
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
		virtual BOOL SyncData(BOOL bStore) { return true; }

		int32_t GetDlgItemInt32 (int idc, int eRadix = 0)								{ return gtl::win_util::GetDlgItemInt32(this, idc, eRadix); }
		void    SetDlgItemInt32 (int idc, int32_t iValue, std::wstring_view fmt = _T("{}"))	{ gtl::win_util::SetDlgItemInt32(this, idc, iValue, fmt); }
		int64_t GetDlgItemInt64 (int idc, int eRadix = 0)								{ return gtl::win_util::GetDlgItemInt64(this, idc, eRadix); }
		void    SetDlgItemInt64 (int idc, int64_t iValue, std::wstring_view fmt = _T("{}"))	{ gtl::win_util::SetDlgItemInt64(this, idc, iValue, fmt); }
		double  GetDlgItemDouble(int idc)												{ return gtl::win_util::GetDlgItemDouble(this, idc); }
		void    SetDlgItemDouble(int idc, double dValue, std::wstring_view fmt = _T("{:.5f}"))	{ gtl::win_util::SetDlgItemDouble(this, idc, dValue, fmt); }

		template < typename T_COORD, class = T_COORD::coord_t >
		T_COORD GetDlgItemCoord(int idc) {
			T_COORD coord = FromString<T_COORD, xString::value_type>(GetDlgItemString(idc));
			return coord;
		}
		template < typename T_COORD, class = T_COORD::coord_t >
		T_COORD GetDlgItemCoord(int idc, T_COORD& coord) {
			coord = FromString<T_COORD, xString::value_type>(GetDlgItemString(idc));
			return coord;
		}
		template < typename T_COORD, class = T_COORD::coord_t >
		void SetDlgItemCoord(int idc, T_COORD& coord, fmt::basic_format_string<CString::XCHAR, typename T_COORD::value_type> fmt = {}) {
			CString str;
			str = ToString(coord, fmt).c_str();
			CheckAndSetDlgItemText(idc, str);
		}

		bool CheckAndSetDlgItemText(int idc, LPCTSTR pszText, CString* pStrOrigin = nullptr) {
			return gtl::win_util::CheckAndSetDlgItemText(this, idc, pszText, pStrOrigin);
		}
		using base_t::GetDlgItemText;
		//int GetDlgItemText(int idc, CString& str) { return __super::GetDlgItemText(idc, str); }
		//int GetDlgItemText(int idc, LPTSTR lpStr, int nMaxCount ) { return __super::GetDlgItemText(idc, lpStr, nMaxCount); }
		CString GetDlgItemString(int idc) { return gtl::win_util::GetDlgItemString(this, idc); }

	public:
		//DECLARE_MESSAGE_MAP()
		void OnEditCopy()  { gtl::win_util::OnEditCopy(this); }
		void OnEditCut()   { gtl::win_util::OnEditCut(this); }
		void OnEditPaste() { gtl::win_util::OnEditPaste(this);  }
		void OnEditUndo()  { gtl::win_util::OnEditUndo(this);  }

		AFX_MSGMAP const* GetMessageMap() const override
			{ return GetThisMessageMap(); }
		static AFX_MSGMAP const* PASCAL GetThisMessageMap() {
		#pragma warning(push)
		#pragma warning(disable: 4640) /* message maps can only be called by single threaded message pump */
			static const AFX_MSGMAP_ENTRY _messageEntries[] = {
				ON_COMMAND(ID_EDIT_COPY, &OnEditCopy)
				ON_COMMAND(ID_EDIT_CUT, &OnEditCut)
				ON_COMMAND(ID_EDIT_PASTE, &OnEditPaste)
				ON_COMMAND(ID_EDIT_UNDO, &OnEditUndo)
				{0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 },
			};
		#pragma warning(pop)
			static const AFX_MSGMAP messageMap =
				{ &base_t::GetThisMessageMap, &_messageEntries[0] };
			return &messageMap;
		}
	};


	using CMDialog = TMDialog<CDialog>;
	using CMDialogEx = TMDialog<CDialogEx>;


}	// namespace gtl::win_util
