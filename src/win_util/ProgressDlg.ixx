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

#include <thread>
#include <format>
#include <chrono>

#include "framework.h"
#define FMT_HEADER_ONLY
#include "fmt/format.h"

#include "Resource.h"

export module gtlw:ProgressDlg;
import gtl;
import :misc;
import :TMDialog;


export namespace gtl::win_util {

	// CProgressDlg dialog
	class CProgressDlg : public CDialogEx {
		DECLARE_DYNAMIC(CProgressDlg)
	public:
		using base_t = CDialogEx;
		static inline std::wstring const strIDD {L"GTL__WINUTIL_DLG_PROGRESS"};
		static inline DLGTEMPLATE m_template{WS_VISIBLE, 0, 0, 0, 0, 300, 120};
	public:
		CProgressDlg(CWnd* pParent = nullptr);   // standard constructor
		virtual ~CProgressDlg();

	// Dialog Data
	//#ifdef AFX_DESIGN_TIME
	//	enum { IDD = IDD_PROGRESS };
	//#endif
	public:
		std::chrono::system_clock::time_point m_tStart;
		gtl::callback_progress_t m_calback;
		CString m_strMessage;
		int m_iPercent{};
		bool m_bUserAbort{};
		bool m_bDone{};
		bool m_bError{};

		std::unique_ptr<std::jthread> m_rThreadWorker;

	protected:
		CFont m_font;
		CStatic m_ctrlMessage;
		CProgressCtrl m_ctrlProgress;
		CStatic m_ctrlState;
		CButton m_btnCancel;
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		virtual BOOL OnInitDialog();

		DECLARE_MESSAGE_MAP()
	public:
		enum eTIMER { T_UPDATE_UI = 1064, };
		afx_msg void OnTimer(UINT_PTR nIDEvent);
		virtual void OnOK();
		virtual void OnCancel();
		void OnAbort();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	};


	IMPLEMENT_DYNAMIC(CProgressDlg, CDialogEx)

	CProgressDlg::CProgressDlg(CWnd* pParent /*=nullptr*/)
		: CDialogEx(/*strIDD.c_str()*/nullptr, pParent) {

		m_strMessage = _T("Working...");
		m_lpDialogTemplate = &m_template;
		m_font.CreatePointFont(100, _T("Segoe UI"));
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
		//DDX_Control(pDX, IDC_PROGRESS, m_ctrlProgress);
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
		lpCreateStruct->style |= WS_THICKFRAME;
		if (CDialogEx::OnCreate(lpCreateStruct) == -1)
			return -1;
		CenterWindow();
		CRect rectClient;
		GetClientRect(rectClient);
		CRect rect(rectClient);
		int const nHeight = 30;
		int const nMargin = 2;

		SetWindowText(_T("Progress..."));

		SetFont(&m_font);

		// MessageBox
		rect.DeflateRect(10, 10, 10, 10);
		rect.bottom = rect.top+nHeight;
		m_ctrlMessage.Create(m_strMessage, WS_CHILD | WS_VISIBLE | /*WS_BORDER | */SS_CENTER | SS_CENTERIMAGE, rect, this, IDC_MESSAGE);
		//m_ctrlMessage.SetFont(&m_font);

		// Progress
		rect.MoveToY(rect.bottom+nMargin);
		rect.bottom = rect.top+nHeight;
		m_ctrlProgress.Create(WS_CHILD|WS_VISIBLE/*|WS_BORDER*/, rect, this, IDC_PROGRESS);

		// State
		rect.MoveToY(rect.bottom+nMargin);
		rect.DeflateRect(rect.Width()/4, 0);
		m_ctrlState.Create(_T("state"), WS_CHILD | WS_VISIBLE | /*WS_BORDER | */SS_CENTER | SS_CENTERIMAGE, rect, this, IDC_STATE);
		//m_ctrlState.SetFont(&m_font);

		// Cancel Button
		rect.right = rect.left + 70;
		rect.MoveToXY(rectClient.right-rect.Width()-nMargin*2, rectClient.bottom-nHeight-nMargin);
		m_btnCancel.Create(_T("Cancel"), WS_CHILD | WS_VISIBLE | /*WS_BORDER | */BS_PUSHBUTTON, rect, this, IDCANCEL);
		//m_btnCancel.SetFont(&m_font);

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
		return;
		//CDialogEx::OnCancel();
	}


	void CProgressDlg::OnAbort() {
		m_bUserAbort = true;
		return ;
		//CDialogEx::OnCancel();
	}

}	// namespace gtl::win_util;
