#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CListCtrlEd : SubItem의 내용을 수정 가능하도록 만든 CListCtrl.
//
// PWH.
//
// 2009.04.13
// 2021.06.10 from Mocha
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
//            xString strOldText;
//            xString strNewText;
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

#include "_lib_gtl_win_util.h"
#include "gtl/win_util/win_util.h"

#include <afxlistctrl.h>

namespace gtl::win_util {
#pragma pack(push, 8)

	///----------------------------------------------------------------------------------------------------------------------------
	/// @brief CListCtrlEd
	class GTL__WINUTIL_CLASS CListCtrlEd : public CListCtrl {
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
	class GTL__WINUTIL_CLASS CMFCListCtrlEd : public CMFCListCtrl {
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


#pragma pack(pop)
}

