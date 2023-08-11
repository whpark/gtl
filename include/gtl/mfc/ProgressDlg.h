#pragma once

// CProgressDlg dialog

#include "gtl/mfc/_lib_gtl_mfc.h"
#include <chrono>

namespace gtl::win::inline mfc {
#pragma pack(push, 8)

	class GTL__MFC_CLASS CProgressDlg : public CDialogEx {
		DECLARE_DYNAMIC(CProgressDlg)
	public:
		using base_t = CDialogEx;
		static inline std::wstring const strIDD {L"GTL__MFC_DLG_PROGRESS"};

	public:
		CProgressDlg(CWnd* pParent = nullptr);   // standard constructor
		virtual ~CProgressDlg();

	// Dialog Data
	//#ifdef AFX_DESIGN_TIME
	//	enum { IDD = IDD_PROGRESS };
	//#endif
	public:
		std::chrono::system_clock::time_point m_tStart;
		gtl::callback_progress_t m_callback;
		CString m_strMessage;
		int m_iPercent{};
		bool m_bUserAbort{};
		bool m_bDone{};
		bool m_bError{};

		std::unique_ptr<std::jthread> m_rThreadWorker;

	protected:
		CProgressCtrl m_ctrlProgress;
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


#pragma pack(pop)
}	// namespace gtl::win::inline mfc
