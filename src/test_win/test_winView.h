
// test_winView.h : interface of the CtestwinView class
//

#pragma once


class CtestwinView : public CFormView {
protected: // create from serialization only
	CtestwinView() noexcept;
	DECLARE_DYNCREATE(CtestwinView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_WIN_FORM };
#endif

// Attributes
public:
	CtestwinDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CtestwinView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	cv::Mat m_bitmap;

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnBnClickedTestMatTime();
	afx_msg void OnBnClickedTestLargeBitmap();
	afx_msg void OnBnClickedTestSaveBMP_24BPP();
	afx_msg void OnBnClickedTestSaveBMP_1BPP();
	afx_msg void OnBnClickedTestSaveBMP_nBPP();
	afx_msg void OnBnClickedTestLoadBMP();
};

#ifndef _DEBUG  // debug version in test_winView.cpp
inline CtestwinDoc* CtestwinView::GetDocument() const    {
	return reinterpret_cast<CtestwinDoc*>(m_pDocument);
}
#endif

