/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TMDialog : Modal/Modeless Dialog
//
// PWH.
//
// 2021.06.10 from Mocha
// 2021.06.24 module
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module;

#include "framework.h"
#define FMT_HEADER_ONLY
#include "fmt/format.h"

export module gtlw:TMDialog;
import gtl;
import :misc;

export namespace gtl::win_util {

	// TMDialog dialog

	int32_t		GetDlgItemInt32(CWnd* pWnd, int idc, int eRadix = 0);
	void		SetDlgItemInt32(CWnd* pWnd, int idc, int32_t iValue, std::wstring_view fmt = L"{}");
	int64_t		GetDlgItemInt64(CWnd* pWnd, int idc, int eRadix = 0);
	void		SetDlgItemInt64(CWnd* pWnd, int idc, int64_t iValue, std::wstring_view fmt = L"{}");
	double		GetDlgItemDouble(CWnd* pWnd, int idc);
	void		SetDlgItemDouble(CWnd* pWnd, int idc, double dValue, std::wstring_view fmt = _T("{:.5f}"));
	void		DDX_Double(CDataExchange* pDX, int idc, double& value, std::wstring_view fmt = _T("{:.5f}"));
	template < typename tchar > requires gtlc::string_elem<tchar>
	void		DDX_String(CDataExchange* pDX, int idc, std::basic_string<tchar>& str);
	bool		CheckAndSetDlgItemText(CWnd* pWnd, int idc, std::wstring const& sv, std::wstring* pStrOrigin = nullptr);	// returns true if changed
	::CString GetDlgItemText(CWnd* pWnd, int idc);

	template < typename T_COORD > requires gtlc::coord<T_COORD>
	void DDX_Coord(CDataExchange* pDX, int idc, T_COORD& coord, std::basic_string_view<::CString::XCHAR> fmt = {}) {
		if (!pDX)
			return;
		::CString str;
		if (pDX->m_bSaveAndValidate) {
			::DDX_Text(pDX, idc, str);
			coord = FromString<T_COORD, ::CString::XCHAR>((LPCTSTR)str);
		} else {
			::CString str;
			str = ToString(coord, fmt).c_str();
			::DDX_Text(pDX, idc, str);
		}
	}
	template < typename T_COORD > requires gtlc::coord<T_COORD>
	T_COORD GetDlgItemCoord(CWnd* pWnd, int idc) {
		T_COORD coord;
		auto str = GetDlgItemText(pWnd, idc);
		coord = FromString<T_COORD>(std::wstring_view(str));
		return coord;
	}
	template < typename T_COORD > requires gtlc::coord<T_COORD>
	void SetDlgItemCoord(CWnd* pWnd, int idc, T_COORD& coord, std::basic_string_view<::CString::XCHAR> fmt = {}) {
		std::wstring str = ToString<wchar_t>(coord, fmt);
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

		::CString str;
		pWnd->GetDlgItemText(idc, str);
		return _tcstol(str, nullptr, eRadix);
	}

	void SetDlgItemInt32(CWnd* pWnd, int idc, int32_t iValue, std::wstring_view fmt) {
		if (!pWnd || !pWnd->m_hWnd)
			return;

		if (fmt.empty())
			fmt = L"{}";
		auto str = fmt::format(fmt, iValue);
		CheckAndSetDlgItemText(pWnd, idc, str);
	}

	int64_t GetDlgItemInt64(CWnd* pWnd, int idc, int eRadix) {
		if (!pWnd || !pWnd->m_hWnd)
			return 0;

		::CString str;
		pWnd->GetDlgItemText(idc, str);
		return _tcstoi64(str, nullptr, eRadix);
	}

	void SetDlgItemInt64(CWnd* pWnd, int idc, int64_t iValue, std::wstring_view fmt) {
		if (!pWnd || !pWnd->m_hWnd)
			return;

		if (fmt.empty())
			fmt = L"{}";
		auto str = fmt::format(fmt, iValue);
		CheckAndSetDlgItemText(pWnd, idc, str);
	}

	double GetDlgItemDouble(CWnd* pWnd, int idc) {
		if (!pWnd || !pWnd->m_hWnd)
			return 0.0;

		::CString str;
		pWnd->GetDlgItemText(idc, str);
		return _ttof(str);
	}

	void SetDlgItemDouble(CWnd* pWnd, int idc, double dValue, std::wstring_view fmt) {
		if (!pWnd || !pWnd->m_hWnd)
			return;

		if (fmt.empty())
			fmt = L"{:.5f}";
		auto str = fmt::format(fmt, dValue);
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

	//WARNING_PUSH_AND_DISABLE(4996)

	template < typename tchar > requires gtlc::string_elem<tchar>
	void DDX_String(CDataExchange* pDX, int idc, std::basic_string<tchar>& str) {
		if (pDX->m_bSaveAndValidate) {
			auto strValue = GetDlgItemText(pDX->m_pDlgWnd, idc);

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

	bool CheckAndSetDlgItemText(CWnd* pWnd, int idc, std::wstring const& str, std::wstring* pStrOrigin) {
		if (!pWnd)
			return false;
		auto strOrg = GetDlgItemText(pWnd, idc);
		bool bChanged = ((LPCTSTR)strOrg != str);
		if (bChanged)
			pWnd->SetDlgItemText(idc, str.c_str());
		if (pStrOrigin)
			*pStrOrigin = (LPCTSTR)strOrg;
		return bChanged;
	}

	::CString GetDlgItemText(CWnd* pWnd, int idc) {
		::CString str;
		if (!pWnd)
			return str;
		pWnd->GetDlgItemText(idc, str);
		return str;
	}


	template <class WNDCLASS>
	class TWndEditFunc : public WNDCLASS {
	public:
		using base_t = WNDCLASS;
		template <typename ... A>
		TWndEditFunc<WNDCLASS>(A&&... a) : WNDCLASS(std::forward<A>(a)...) {}
		virtual ~TWndEditFunc<WNDCLASS>() { }

		virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override {
			if (nCode == CN_COMMAND) {
				if (pExtra) {
					switch (nID) {
					case ID_EDIT_COPY :
					case ID_EDIT_CUT :
					case ID_EDIT_PASTE :
					case ID_EDIT_UNDO :
						if (GetFocusedEdit(this))
							return true;
						break;
					}
				} else {
					switch (nID) {
					case ID_EDIT_COPY :		if (OnEditCopy(this)) return true; break;
					case ID_EDIT_CUT :		if (OnEditCut(this)) return true; break;
					case ID_EDIT_PASTE :	if (OnEditPaste(this)) return true;  break;
					case ID_EDIT_UNDO :		if (OnEditUndo(this)) return true; break;
					}
				}
			}
			return base_t::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}
	};


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
		::CString const m_strIDD;

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
		void SetDlgItemCoord(int idc, T_COORD& coord, std::wstring_view fmt = {}) {
			::CString str;
			Coord2Text(coord, str, fmt);
			CheckAndSetDlgItemText(idc, str);
		}

		bool CheckAndSetDlgItemText(int idc, LPCTSTR pszText, ::CString* pStrOrigin = nullptr) {
			return gtl::win_util::CheckAndSetDlgItemText(this, idc, pszText, pStrOrigin);
		}
		using base_t::GetDlgItemText;
		//int GetDlgItemText(int idc, CString& str) { return __super::GetDlgItemText(idc, str); }
		//int GetDlgItemText(int idc, LPTSTR lpStr, int nMaxCount ) { return __super::GetDlgItemText(idc, lpStr, nMaxCount); }
		::CString GetDlgItemText(int idc) { return gtl::win_util::GetDlgItemText(this, idc); }

	public:
		//DECLARE_MESSAGE_MAP()
		void OnEditCopy()  { gtl::win_util::OnEditCopy(this); }
		void OnEditCut()   { gtl::win_util::OnEditCut(this); }
		void OnEditPaste() { gtl::win_util::OnEditPaste(this);  }
		void OnEditUndo()  { gtl::win_util::OnEditUndo(this);  }

		const AFX_MSGMAP* GetMessageMap() const override
			{ return GetThisMessageMap(); }
		const AFX_MSGMAP* PASCAL GetThisMessageMap() {
		#pragma warning(push)
		#pragma warning(disable: 4640) /* message maps can only be called by single threaded message pump */
			static const AFX_MSGMAP_ENTRY _messageEntries[] = {
				ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
				ON_COMMAND(ID_EDIT_CUT, OnEditCut)
				ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
				ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
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
