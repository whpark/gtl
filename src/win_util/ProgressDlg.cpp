// ProgressDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "resource.h"
#include "gtl/gtl.h"
#include "gtl/win_util/ProgressDlg.h"
#include "gtl/win_util/TMDialog.h"

// CProgressDlg dialog

namespace gtl::win_util {

	IMPLEMENT_DYNAMIC(CProgressDlg, CDialogEx)

	CProgressDlg::CProgressDlg(CWnd* pParent /*=nullptr*/)
		: CDialogEx(strIDD.c_str(), pParent) {

		m_strMessage = _T("Working...");
		m_calback = [&](int iPercent, bool bDone, bool bError)->bool{
			m_iPercent = iPercent;
			m_bError = bError;
			m_bDone = bDone;
			return !m_bUserAbort;
		};
	}

	CProgressDlg::~CProgressDlg() {
	}

	void CProgressDlg::DoDataExchange(CDataExchange* pDX) {
		CDialogEx::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_PROGRESS, m_ctrlProgress);
		DDX_Text(pDX, IDC_MESSAGE, m_strMessage);
	}

	BOOL CProgressDlg::OnInitDialog() {
		CDialogEx::OnInitDialog();

		m_tStart  = std::chrono::system_clock::now();
		SetTimer(T_UPDATE_UI, 100, nullptr);

		m_ctrlProgress.SetRange(0, 100);

		return TRUE;  // return TRUE unless you set the focus to a control
					  // EXCEPTION: OCX Property Pages should return FALSE
	}

	BEGIN_MESSAGE_MAP(CProgressDlg, CDialogEx)
		ON_WM_TIMER()
		ON_WM_CREATE()
		ON_BN_CLICKED(IDABORT, OnAbort)
	END_MESSAGE_MAP()


	// CProgressDlg message handlers

	//bool CProgressDlg::Create(CWnd* pWndParent) {
	//	return base_t::Create(strIDD.c_str(), pWndParent) != false;
	//}

	int CProgressDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) {
		if (CDialogEx::OnCreate(lpCreateStruct) == -1)
			return -1;

		return 0;
	}

	void CProgressDlg::OnTimer(UINT_PTR nIDEvent) {
		switch (nIDEvent) {
		case T_UPDATE_UI :
			if (m_ctrlProgress) {
				int iPos = m_ctrlProgress.GetPos();
				if (m_iPercent != iPos)
					m_ctrlProgress.SetPos(m_iPercent);
				auto ts = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now() - m_tStart);
				auto str = std::format(_T("{:.3f} 초, {} %"), ts.count(), m_iPercent);
				CheckAndSetDlgItemText(this, IDC_STATE, str.c_str());
			}
			if (m_bDone) {
				EndDialog(m_bUserAbort ? IDABORT : m_bError ? IDCANCEL : IDOK);
				return;
			}
			//if (m_rThread and m_rThread->) {
			//	EndDialog(IDOK);
			//}
			return;
		}

		CDialogEx::OnTimer(nIDEvent);
	}


	void CProgressDlg::OnOK() {
		return;

		//CDialogEx::OnOK();
	}


	void CProgressDlg::OnCancel() {
		m_bUserAbort = true;
		EndDialog(IDABORT);
		return;
		//CDialogEx::OnCancel();
	}


	void CProgressDlg::OnAbort() {
		m_bUserAbort = true;
		EndDialog(IDABORT);
		return ;
		//CDialogEx::OnCancel();
	}

}	// namespace gtl::win_util;
