module;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CListCtrlEd : SubItem의 내용을 수정 가능하도록 만든 CListCtrl.
//
// PWH.
//
// 2009.04.13
// 2021.06.10 from Mocha
// 2021.06.24 gtlw:module
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//1. class : CListCtrlEd
//
//2. functions :
//    (1) void SetEditable(BOOL bEditable, BOOL bAutoUpdate) // 마우스 클릭으로 Edit 할 수 있도록 설정
//        - bEditable :
//            TRUE - 마우스 클릭으로 Edit 가능하도록 설정
//        - bAutoUpdate :
//            TRUE - Focus가 바뀔경우 자동으로 Text를 업데이트 함.
//
//    (2) BOOL StartEdit(const POINT& pt);    // 위치 좌표로부터 Edit 시작
//          BOOL StartEdit(int iItem, int iSubItem)    // Item 의 Edit 시작
//        - iItem : Item Index
//        - iSubItem : Sub Item Index
//
//    (3) BOOL EndEdit(BOOL bUpdate)    // Edit 종료
//        - bUpdate : TRUE일 경우 Text를 Update함. FALSE 일 경우 Update하지 않음.
//
//    (4) Call Back : LVN_BEGINLABELEDIT, LVN_ENDLABELEDIT(LVN_SETDISPINFO에서 변경) 를 ParentWnd 에 올려줌. (ClassWizard 에서 설정 가능)
//        ex)
//
//        void CParentDlg::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult) {
//            NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
//
//            CString strOldText;
//            CString strNewText;
//
//            strOldText = m_list.GetItemText(pDispInfo->item.iItem, pDispInfo->item.iSubItem);    // Original Text
//            strNewText = pDispInfo->item.pszText;                                                // New Text
//
//            SetDlgItemText(IDC_OLD_TEXT, strOldText);
//            SetDlgItemText(IDC_NEW_TEXT, strNewText);
//
//            *pResult = 0;	// pResult에 0을 return할 경우 Text Update.
//							// pResult에 0이 아닌 값을 return할 경우 Update하지 않음.
//        }
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "framework.h"

export module gtl.win_util:ListCtrlEd;


export namespace gtl::win_util {

	///----------------------------------------------------------------------------------------------------------------------------
	/// @brief CListCtrlEd
	class CListCtrlEd : public CListCtrl {
		DECLARE_DYNAMIC(CListCtrlEd)
	protected:
		BOOL m_bEditable;
		BOOL m_bAutoUpdate;
		CEdit m_edit;
		int m_iItem;
		int m_iSubItem;
	public:
		CListCtrlEd();
		virtual ~CListCtrlEd();

	public:
		void SetEditable(BOOL bEditable = TRUE, BOOL bAutoUpdate = TRUE) { m_bEditable = bEditable; m_bAutoUpdate = bAutoUpdate; }
		BOOL IsEditable() { return m_bEditable; }
		BOOL IsAutoUpdate() { return m_bAutoUpdate; }

		BOOL StartEdit(const POINT& pt);	// Local Position
		BOOL StartEdit(int iItem, int iSubItem);
		BOOL EndEdit(BOOL bUpdate = TRUE);
		BOOL IsInEditMode() const { return m_edit.m_hWnd && m_edit.IsWindowVisible(); }

	public:
		BOOL SelectItem(int rIndex = -1, BOOL bExclusive = TRUE);
		int GetSelectedItem(int iItemAfter = -1) const;

	protected:
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnEnKillFocus();
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		virtual BOOL PreTranslateMessage(MSG* pMsg);
	};

	///----------------------------------------------------------------------------------------------------------------------------
	/// @brief CMFCListCtrlEd
	class CMFCListCtrlEd : public CMFCListCtrl {
		DECLARE_DYNAMIC(CMFCListCtrlEd)
	protected:
		BOOL m_bEditable;
		BOOL m_bAutoUpdate;
		CEdit m_edit;
		int m_iItem;
		int m_iSubItem;
	public:
		CMFCListCtrlEd();
		virtual ~CMFCListCtrlEd();

	public:
		void SetEditable(BOOL bEditable = TRUE, BOOL bAutoUpdate = TRUE) { m_bEditable = bEditable; m_bAutoUpdate = bAutoUpdate; }
		BOOL IsEditable() { return m_bEditable; }
		BOOL IsAutoUpdate() { return m_bAutoUpdate; }

		BOOL StartEdit(const POINT& pt);	// Local Position
		BOOL StartEdit(int iItem, int iSubItem);
		BOOL EndEdit(BOOL bUpdate = TRUE);
		BOOL IsInEditMode() const { return m_edit.m_hWnd && m_edit.IsWindowVisible(); }

	public:
		BOOL SelectItem(int rIndex = -1, BOOL bExclusive = TRUE);
		int GetSelectedItem(int iItemAfter = -1) const;

	protected:
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnEnKillFocus();
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		virtual BOOL PreTranslateMessage(MSG* pMsg);
	};


	IMPLEMENT_DYNAMIC(CListCtrlEd, CListCtrl)

	#define IDC_EDIT 1002

	CListCtrlEd::CListCtrlEd() {
		m_bEditable = TRUE;
		m_bAutoUpdate = TRUE;

		m_iItem = -1;
		m_iSubItem = -1;
	}

	CListCtrlEd::~CListCtrlEd() {
	}

	BEGIN_MESSAGE_MAP(CListCtrlEd, CListCtrl)
		ON_WM_LBUTTONDOWN()
		ON_EN_KILLFOCUS(IDC_EDIT, OnEnKillFocus)
		ON_WM_HSCROLL()
		ON_WM_VSCROLL()
	END_MESSAGE_MAP()

	// CListCtrlEd message handlers

	BOOL CListCtrlEd::StartEdit(const POINT& pt) {
		//CHECK_MOCHA_LICENSE();

		int iItem = GetTopIndex();
		if (iItem < 0)
			return FALSE;
		int nItem = GetItemCount();
		if (GetStyle() & LVS_REPORT) {
			for ( ; iItem < nItem; iItem++) {
				CRect rect;
				GetItemRect(iItem, rect, LVIR_BOUNDS);
				if (rect.PtInRect(pt)) {
					CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
					if (!pHeaderCtrl)
						return FALSE;
					SCROLLINFO si;
					memset(&si, 0, sizeof(si));
					si.cbSize = sizeof(si);
					si.fMask = SIF_ALL;
					GetScrollInfo(SB_HORZ, &si);
					for (int iSubItem = 0; iSubItem < pHeaderCtrl->GetItemCount(); iSubItem++) {
						CRect rectSubItem;
						pHeaderCtrl->GetItemRect(iSubItem, rectSubItem);
						rectSubItem.top = rect.top;
						rectSubItem.bottom = rect.bottom;
						rectSubItem.left -= si.nPos;
						rectSubItem.right -= si.nPos;
						if (rectSubItem.PtInRect(pt))
							return StartEdit(iItem, iSubItem);
					}
					return FALSE;
				}
			}
		} else {
			for ( ; iItem < nItem; iItem++) {
				CRect rect;
				GetItemRect(iItem, rect, LVIR_LABEL);
				if (rect.PtInRect(pt)) {
					return StartEdit(iItem, 0);
				}
			}
		}
		return FALSE;
	}

	BOOL CListCtrlEd::StartEdit(int iItem, int iSubItem) {
		EndEdit(m_bAutoUpdate);

		//CHECK_MOCHA_LICENSE();

		if ( (iItem < 0) || (iItem >= GetItemCount()) || (iSubItem < 0) )
			return FALSE;
		CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
		if (!pHeaderCtrl || (iSubItem >= pHeaderCtrl->GetItemCount()) )
			return FALSE;

		// Notify
		{
			NMLVDISPINFO ldi;
			memset(&ldi, 0, sizeof(ldi));
			ldi.hdr.code = LVN_BEGINLABELEDIT;
			ldi.hdr.hwndFrom = m_hWnd;
			ldi.hdr.idFrom = GetDlgCtrlID();
			ldi.item.iItem = iItem;
			ldi.item.iSubItem = iSubItem;
			ldi.item.mask = 0;
			ldi.item.pszText = NULL;
			if (!GetParent()->SendMessage(WM_NOTIFY, ldi.hdr.idFrom, (LPARAM)&ldi) == 0) {
				return FALSE;
			}
		}

		EnsureVisible(iItem, FALSE);

		// Set State
		int iFocused = GetNextItem(-1, LVIS_FOCUSED);
		if (iFocused != iItem) {
			if (iFocused >= 0)
				SetItemState(iFocused, 0, LVIS_FOCUSED);
			SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);
		}
		if (GetStyle() & LVS_SINGLESEL) {
			int iSelected = GetNextItem(-1, LVIS_SELECTED);
			if (iSelected != iItem) {
				if (iSelected >= 0)
					SetItemState(iSelected, 0, LVIS_SELECTED);
				SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
			}
		}

		CRect rect, rectSubItem;
		DWORD dwStyle = 0;
		if (GetStyle() & LVS_REPORT) {
			GetItemRect(iItem, rect, LVIR_BOUNDS);
			pHeaderCtrl->GetItemRect(iSubItem, rectSubItem);
			rectSubItem.top = rect.top;
			rectSubItem.bottom = rect.bottom;
			SCROLLINFO si;
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(SB_HORZ, &si);
			rectSubItem.left -= si.nPos;
			rectSubItem.right -= si.nPos;
			HDITEM hi;
			memset(&hi, 0, sizeof(hi));
			hi.mask = HDI_FORMAT;
			pHeaderCtrl->GetItem(iSubItem, &hi);
			switch (hi.fmt & LVCFMT_JUSTIFYMASK) {
			case LVCFMT_LEFT : dwStyle = ES_LEFT; break;
			case LVCFMT_CENTER : dwStyle = ES_CENTER; break;
			case LVCFMT_RIGHT : dwStyle = ES_RIGHT; break;
			default :
				dwStyle = ES_LEFT;
			}
		} else {
			GetItemRect(iItem, rectSubItem, LVIR_LABEL);
			dwStyle = ES_CENTER;
		}

		//rectSubItem.InflateRect(1, 1);

		m_iItem = iItem;
		m_iSubItem = iSubItem;

		if (!m_edit.m_hWnd) {
			m_edit.Create(WS_VISIBLE|WS_CHILD|WS_BORDER|ES_AUTOHSCROLL|ES_LEFT, rectSubItem, this, IDC_EDIT);
			m_edit.SetFont(GetFont());
		}
		m_edit.ModifyStyle(ES_LEFT|ES_CENTER|ES_RIGHT, dwStyle);
		m_edit.MoveWindow(rectSubItem);
		m_edit.SetWindowText(GetItemText(iItem, iSubItem));
		m_edit.SetSel(0, -1, FALSE);
		m_edit.ShowWindow(SW_SHOW);
		m_edit.SetFocus();

		return TRUE;
	}

	BOOL CListCtrlEd::EndEdit(BOOL bUpdate) {
		if (!m_edit.m_hWnd || !m_edit.IsWindowVisible())
			return FALSE;

		if (m_edit.m_hWnd) {
			CString str;
			m_edit.GetWindowText(str);
			if ( ( (m_iItem >= 0) && (m_iItem < GetItemCount()) )
				&& (m_iSubItem >= 0) && (m_iSubItem < GetHeaderCtrl()->GetItemCount())
				)
			{
				NMLVDISPINFO ldi;
				memset(&ldi, 0, sizeof(ldi));
				ldi.hdr.code = LVN_ENDLABELEDIT;
				ldi.hdr.hwndFrom = m_hWnd;
				ldi.hdr.idFrom = GetDlgCtrlID();
				ldi.item.iItem = m_iItem;
				ldi.item.iSubItem = m_iSubItem;
				ldi.item.mask = LVIF_TEXT;
				ldi.item.pszText = (LPTSTR)(LPCTSTR)str;
				if (GetParent()->SendMessage(WM_NOTIFY, ldi.hdr.idFrom, (LPARAM)&ldi) == 0) {
					if (bUpdate)
						SetItemText(m_iItem, m_iSubItem, str);
				}
			}
		}

		m_edit.ShowWindow(SW_HIDE);
		return TRUE;
	}

	void CListCtrlEd::OnLButtonDown(UINT nFlags, CPoint point) {
		BOOL bEdit = FALSE;
		if (m_bEditable) {
			int iItem = 0;
			int iSelItem = GetSelectedItem();
			int nSelCount = GetSelectedCount();
			int nItem = 0;

			if (nSelCount <= 0) {
				iItem = 0;
				nItem = 0;
			} else if (nSelCount == 1) {
				iItem = iSelItem;
				nItem = iItem+1;
			} else {
				iItem = GetTopIndex();
				nItem = GetItemCount();
			}

			for ( ; iItem < nItem; iItem++) {
				CRect rect;
				GetItemRect(iItem, rect, LVIR_BOUNDS);
				if (rect.PtInRect(point)) {
					if ( GetItemState(iItem, LVIS_FOCUSED|LVIS_SELECTED) == (LVIS_FOCUSED|LVIS_SELECTED) )
						bEdit = TRUE;
					break;
				}
			}
		}

		CListCtrl::OnLButtonDown(nFlags, point);

		if (bEdit)
			StartEdit(point);
	}

	void CListCtrlEd::OnEnKillFocus() {
		EndEdit(m_bAutoUpdate);
	}

	void CListCtrlEd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
		CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
		EndEdit(m_bAutoUpdate);
	}

	void CListCtrlEd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
		CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		EndEdit(m_bAutoUpdate);
	}

	BOOL CListCtrlEd::PreTranslateMessage(MSG* pMsg) {
		if (m_edit.m_hWnd && m_edit.IsWindowVisible()) {
			if (pMsg->message == WM_KEYDOWN) {
				switch (pMsg->wParam) {
				case VK_ESCAPE :
					EndEdit(FALSE);
					return TRUE;
				case VK_UP :
					if (m_iItem < 1)
						return TRUE;
					EnsureVisible(m_iItem - 1, FALSE);
					StartEdit(m_iItem - 1, m_iSubItem);
					return TRUE;
				case VK_DOWN :
					if (m_iItem > GetItemCount() - 2)
						return TRUE;
					EnsureVisible(m_iItem + 1, FALSE);
					StartEdit(m_iItem + 1, m_iSubItem);
					return TRUE;

				case VK_TAB :
				case VK_LEFT :
				case VK_RIGHT :
					if (GetStyle() & LVS_REPORT) {

						int eDirection = 0;
						if ( (pMsg->wParam == VK_LEFT) || (pMsg->wParam == VK_RIGHT) ) {
							if (GetKeyState(VK_CONTROL) >= 0)
								break;
							eDirection = (pMsg->wParam == VK_LEFT) ? -1 : 1;
						}
						else if (pMsg->wParam == VK_TAB) {
							eDirection = (GetKeyState(VK_SHIFT) < 0) ? -1 : 1;
						}
						StartEdit(m_iItem, m_iSubItem + eDirection);
						return TRUE;
					}
					break;
				case VK_RETURN :
					EndEdit(m_bAutoUpdate);
					return TRUE;
				}
			}
		}
		return CListCtrl::PreTranslateMessage(pMsg);
	}

	int CListCtrlEd::GetSelectedItem(int iItemAfter) const {
		int nItem = GetItemCount();
		for (int iItem = iItemAfter+1; iItem < nItem; iItem++) {
			DWORD dwState = GetItemState(iItem, (UINT)-1);
			if (dwState & LVIS_SELECTED)
				return iItem;
		}

		return -1;
	}

	BOOL CListCtrlEd::SelectItem(int rIndex, BOOL bExclusive) {
		int nItem = GetItemCount();
		if (bExclusive) {
			for (int iItem = 0; iItem < nItem; iItem++) {
				if (iItem == rIndex)
					continue;
				DWORD dwState = GetItemState(iItem, (UINT)-1);
				if (dwState & LVIS_SELECTED) {
					SetItemState(iItem, 0, LVIS_SELECTED);
				}
			}
		}

		if ((rIndex < 0) || (rIndex >= nItem))
			return FALSE;

		return SetItemState(rIndex, LVIS_SELECTED, LVIS_SELECTED);
	}



	//=================================================================================================================================
	IMPLEMENT_DYNAMIC(CMFCListCtrlEd, CMFCListCtrl)


	#define IDC_EDIT 1002

	CMFCListCtrlEd::CMFCListCtrlEd() {
		m_bEditable = TRUE;
		m_bAutoUpdate = TRUE;

		m_iItem = -1;
		m_iSubItem = -1;
	}

	CMFCListCtrlEd::~CMFCListCtrlEd() {
	}

	BEGIN_MESSAGE_MAP(CMFCListCtrlEd, CMFCListCtrl)
		ON_WM_LBUTTONDOWN()
		ON_EN_KILLFOCUS(IDC_EDIT, OnEnKillFocus)
		ON_WM_HSCROLL()
		ON_WM_VSCROLL()
	END_MESSAGE_MAP()

	// CMFCListCtrlEd message handlers

	BOOL CMFCListCtrlEd::StartEdit(const POINT& pt) {
		//CHECK_MOCHA_LICENSE();

		int iItem = GetTopIndex();
		if (iItem < 0)
			return FALSE;
		int nItem = GetItemCount();
		if (GetStyle() & LVS_REPORT) {
			for ( ; iItem < nItem; iItem++) {
				CRect rect;
				GetItemRect(iItem, rect, LVIR_BOUNDS);
				if (rect.PtInRect(pt)) {
					auto& aHeaderCtrl = GetHeaderCtrl();
					if (!aHeaderCtrl)
						return FALSE;
					SCROLLINFO si;
					memset(&si, 0, sizeof(si));
					si.cbSize = sizeof(si);
					si.fMask = SIF_ALL;
					GetScrollInfo(SB_HORZ, &si);
					for (int iSubItem = 0; iSubItem < aHeaderCtrl.GetItemCount(); iSubItem++) {
						CRect rectSubItem;
						aHeaderCtrl.GetItemRect(iSubItem, rectSubItem);
						rectSubItem.top = rect.top;
						rectSubItem.bottom = rect.bottom;
						rectSubItem.left -= si.nPos;
						rectSubItem.right -= si.nPos;
						if (rectSubItem.PtInRect(pt))
							return StartEdit(iItem, iSubItem);
					}
					return FALSE;
				}
			}
		} else {
			for ( ; iItem < nItem; iItem++) {
				CRect rect;
				GetItemRect(iItem, rect, LVIR_LABEL);
				if (rect.PtInRect(pt)) {
					return StartEdit(iItem, 0);
				}
			}
		}
		return FALSE;
	}

	BOOL CMFCListCtrlEd::StartEdit(int iItem, int iSubItem) {
		EndEdit(m_bAutoUpdate);

		//CHECK_MOCHA_LICENSE();

		if ( (iItem < 0) || (iItem >= GetItemCount()) || (iSubItem < 0) )
			return FALSE;
		auto& aHeaderCtrl = GetHeaderCtrl();
		if (!aHeaderCtrl || (iSubItem >= aHeaderCtrl.GetItemCount()) )
			return FALSE;

		// Notify
		{
			NMLVDISPINFO ldi;
			memset(&ldi, 0, sizeof(ldi));
			ldi.hdr.code = LVN_BEGINLABELEDIT;
			ldi.hdr.hwndFrom = m_hWnd;
			ldi.hdr.idFrom = GetDlgCtrlID();
			ldi.item.iItem = iItem;
			ldi.item.iSubItem = iSubItem;
			ldi.item.mask = 0;
			ldi.item.pszText = NULL;
			if (!GetParent()->SendMessage(WM_NOTIFY, ldi.hdr.idFrom, (LPARAM)&ldi) == 0) {
				return FALSE;
			}
		}

		EnsureVisible(iItem, FALSE);

		// Set State
		int iFocused = GetNextItem(-1, LVIS_FOCUSED);
		if (iFocused != iItem) {
			if (iFocused >= 0)
				SetItemState(iFocused, 0, LVIS_FOCUSED);
			SetItemState(iItem, LVIS_FOCUSED, LVIS_FOCUSED);
		}
		if (GetStyle() & LVS_SINGLESEL) {
			int iSelected = GetNextItem(-1, LVIS_SELECTED);
			if (iSelected != iItem) {
				if (iSelected >= 0)
					SetItemState(iSelected, 0, LVIS_SELECTED);
				SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
			}
		}

		CRect rect, rectSubItem;
		DWORD dwStyle = 0;
		if (GetStyle() & LVS_REPORT) {
			GetItemRect(iItem, rect, LVIR_BOUNDS);
			aHeaderCtrl.GetItemRect(iSubItem, rectSubItem);
			rectSubItem.top = rect.top;
			rectSubItem.bottom = rect.bottom;
			SCROLLINFO si;
			memset(&si, 0, sizeof(si));
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			GetScrollInfo(SB_HORZ, &si);
			rectSubItem.left -= si.nPos;
			rectSubItem.right -= si.nPos;
			HDITEM hi;
			memset(&hi, 0, sizeof(hi));
			hi.mask = HDI_FORMAT;
			aHeaderCtrl.GetItem(iSubItem, &hi);
			switch (hi.fmt & LVCFMT_JUSTIFYMASK) {
			case LVCFMT_LEFT : dwStyle = ES_LEFT; break;
			case LVCFMT_CENTER : dwStyle = ES_CENTER; break;
			case LVCFMT_RIGHT : dwStyle = ES_RIGHT; break;
			default :
				dwStyle = ES_LEFT;
			}
		} else {
			GetItemRect(iItem, rectSubItem, LVIR_LABEL);
			dwStyle = ES_CENTER;
		}

		//rectSubItem.InflateRect(1, 1);

		m_iItem = iItem;
		m_iSubItem = iSubItem;

		if (!m_edit.m_hWnd) {
			m_edit.Create(WS_VISIBLE|WS_CHILD|WS_BORDER|ES_AUTOHSCROLL|ES_LEFT, rectSubItem, this, IDC_EDIT);
			m_edit.SetFont(GetFont());
		}
		m_edit.ModifyStyle(ES_LEFT|ES_CENTER|ES_RIGHT, dwStyle);
		m_edit.MoveWindow(rectSubItem);
		m_edit.SetWindowText(GetItemText(iItem, iSubItem));
		m_edit.SetSel(0, -1, FALSE);
		m_edit.ShowWindow(SW_SHOW);
		m_edit.SetFocus();

		return TRUE;
	}

	BOOL CMFCListCtrlEd::EndEdit(BOOL bUpdate) {
		if (!m_edit.m_hWnd || !m_edit.IsWindowVisible())
			return FALSE;

		if (m_edit.m_hWnd) {
			CString str;
			m_edit.GetWindowText(str);
			if ( ( (m_iItem >= 0) && (m_iItem < GetItemCount()) )
				&& (m_iSubItem >= 0) && (m_iSubItem < GetHeaderCtrl().GetItemCount())
				)
			{
				NMLVDISPINFO ldi;
				memset(&ldi, 0, sizeof(ldi));
				ldi.hdr.code = LVN_ENDLABELEDIT;
				ldi.hdr.hwndFrom = m_hWnd;
				ldi.hdr.idFrom = GetDlgCtrlID();
				ldi.item.iItem = m_iItem;
				ldi.item.iSubItem = m_iSubItem;
				ldi.item.mask = LVIF_TEXT;
				ldi.item.pszText = (LPTSTR)(LPCTSTR)str;
				if (GetParent()->SendMessage(WM_NOTIFY, ldi.hdr.idFrom, (LPARAM)&ldi) == 0) {
					if (bUpdate)
						SetItemText(m_iItem, m_iSubItem, str);
				}
			}
		}

		m_edit.ShowWindow(SW_HIDE);
		return TRUE;
	}

	void CMFCListCtrlEd::OnLButtonDown(UINT nFlags, CPoint point) {
		BOOL bEdit = FALSE;
		if (m_bEditable) {
			int iItem = GetTopIndex();
			int nItem = GetItemCount();
			for ( ; iItem < nItem; iItem++) {
				CRect rect;
				GetItemRect(iItem, rect, LVIR_BOUNDS);
				if (rect.PtInRect(point)) {
					if ( GetItemState(iItem, LVIS_FOCUSED|LVIS_SELECTED) == (LVIS_FOCUSED|LVIS_SELECTED) )
						bEdit = TRUE;
					break;
				}
			}
		}

		CMFCListCtrl::OnLButtonDown(nFlags, point);

		if (bEdit)
			StartEdit(point);
	}

	void CMFCListCtrlEd::OnEnKillFocus() {
		EndEdit(m_bAutoUpdate);
	}

	void CMFCListCtrlEd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
		CMFCListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
		EndEdit(m_bAutoUpdate);
	}

	void CMFCListCtrlEd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
		CMFCListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
		EndEdit(m_bAutoUpdate);
	}

	BOOL CMFCListCtrlEd::PreTranslateMessage(MSG* pMsg) {
		if (m_edit.m_hWnd && m_edit.IsWindowVisible()) {
			if (pMsg->message == WM_KEYDOWN) {
				switch (pMsg->wParam) {
				case VK_ESCAPE :
					EndEdit(FALSE);
					return TRUE;
				case VK_UP :
					if (m_iItem < 1)
						return TRUE;
					EnsureVisible(m_iItem - 1, FALSE);
					StartEdit(m_iItem - 1, m_iSubItem);
					return TRUE;
				case VK_DOWN :
					if (m_iItem > GetItemCount() - 2)
						return TRUE;
					EnsureVisible(m_iItem + 1, FALSE);
					StartEdit(m_iItem + 1, m_iSubItem);
					return TRUE;

				case VK_TAB :
				case VK_LEFT :
				case VK_RIGHT :
					if (GetStyle() & LVS_REPORT) {

						int eDirection = 0;
						if ( (pMsg->wParam == VK_LEFT) || (pMsg->wParam == VK_RIGHT) ) {
							if (GetKeyState(VK_CONTROL) >= 0)
								break;
							eDirection = (pMsg->wParam == VK_LEFT) ? -1 : 1;
						}
						else if (pMsg->wParam == VK_TAB) {
							eDirection = (GetKeyState(VK_SHIFT) < 0) ? -1 : 1;
						}
						StartEdit(m_iItem, m_iSubItem + eDirection);
						return TRUE;
					}
					break;
				case VK_RETURN :
					EndEdit(m_bAutoUpdate);
					return TRUE;
				}
			}
		}
		return CMFCListCtrl::PreTranslateMessage(pMsg);
	}

	int CMFCListCtrlEd::GetSelectedItem(int iItemAfter) const {
		int nItem = GetItemCount();
		for (int iItem = iItemAfter+1; iItem < nItem; iItem++) {
			DWORD dwState = GetItemState(iItem, (UINT)-1);
			if (dwState & LVIS_SELECTED)
				return iItem;
		}

		return -1;
	}

	BOOL CMFCListCtrlEd::SelectItem(int rIndex, BOOL bExclusive) {
		int nItem = GetItemCount();
		if (bExclusive) {
			for (int iItem = 0; iItem < nItem; iItem++) {
				if (iItem == rIndex)
					continue;
				DWORD dwState = GetItemState(iItem, (UINT)-1);
				if (dwState & LVIS_SELECTED) {
					SetItemState(iItem, 0, LVIS_SELECTED);
				}
			}
		}

		if ((rIndex < 0) || (rIndex >= nItem))
			return FALSE;

		return SetItemState(rIndex, LVIS_SELECTED, LVIS_SELECTED);
	}

}
