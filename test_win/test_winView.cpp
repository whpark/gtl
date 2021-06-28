
// test_winView.cpp : implementation of the CtestwinView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "test_win.h"
#endif

#include "test_winDoc.h"
#include "test_winView.h"

#include "gtl/gtl.h"
#include "gtl/win_util/win_util.h"
#include "gtl/win_util/MatHelper.h"

#include "opencv2/opencv.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CtestwinView

IMPLEMENT_DYNCREATE(CtestwinView, CFormView)

BEGIN_MESSAGE_MAP(CtestwinView, CFormView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CtestwinView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

// CtestwinView construction/destruction

CtestwinView::CtestwinView() noexcept
	: CFormView(IDD_TEST_WIN_FORM) {
	// TODO: add construction code here

}

CtestwinView::~CtestwinView() {
}

void CtestwinView::DoDataExchange(CDataExchange* pDX) {
	CFormView::DoDataExchange(pDX);
}

BOOL CtestwinView::PreCreateWindow(CREATESTRUCT& cs) {
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CtestwinView::OnInitialUpdate() {
	CFormView::OnInitialUpdate();
	ResizeParentToFit();

}


// CtestwinView printing


void CtestwinView::OnFilePrintPreview() {
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CtestwinView::OnPreparePrinting(CPrintInfo* pInfo) {
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CtestwinView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {
	// TODO: add extra initialization before printing
}

void CtestwinView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {
	// TODO: add cleanup after printing
}

void CtestwinView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/) {
	// TODO: add customized printing code here
}

void CtestwinView::OnRButtonUp(UINT /* nFlags */, CPoint point) {
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CtestwinView::OnContextMenu(CWnd* /* pWnd */, CPoint point) {
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CtestwinView diagnostics

#ifdef _DEBUG
void CtestwinView::AssertValid() const {
	CFormView::AssertValid();
}

void CtestwinView::Dump(CDumpContext& dc) const {
	CFormView::Dump(dc);
}

CtestwinDoc* CtestwinView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CtestwinDoc)));
	return (CtestwinDoc*)m_pDocument;
}
#endif //_DEBUG


// CtestwinView message handlers


void CtestwinView::OnTimer(UINT_PTR nIDEvent) {
	// TODO: Add your message handler code here and/or call default

	CFormView::OnTimer(nIDEvent);
}


void CtestwinView::OnSize(UINT nType, int cx, int cy) {
	CFormView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}


void CtestwinView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
	switch (nIDCtl) {
	case IDC_VIEW1 :
	case IDC_VIEW3 :
		{
			CDC dc;
			dc.Attach(lpDrawItemStruct->hDC);
			gtl::CTrigger detacher([&dc]{dc.Detach();});

			CRect rect(lpDrawItemStruct->rcItem);

			dc.SelectStockObject(NULL_PEN);
			dc.SelectStockObject(WHITE_BRUSH);
			dc.Rectangle(rect);

			static cv::Mat img;
			if (img.empty()) {
				img = cv::Mat::zeros(1256, 1256, CV_8UC1);
				for (int row{}; row < img.rows; row++) {
					auto v = row%256;
					img.row(row) = cv::Scalar(v, v, v);
				}
			}

			static cv::Mat img3;
			if (img3.empty()) {
				img3 = cv::Mat::zeros(1256, 1256, CV_8UC3);
				for (int row{}; row < img3.rows; row++) {
					auto v = row%256;
					img3.row(row) = cv::Scalar(v, v, v);
				}
			}

			gtlw::TStopWatch<char, std::chrono::microseconds> sw;
			if (nIDCtl == IDC_VIEW1) {
				gtlw::MatToDC(img, img.size(), dc, rect);
				sw.Lap("1 byte image");
			}
			else {
				gtlw::MatToDC(img3, img3.size(), dc, rect);
				sw.Lap("3 byte image");
			}

		}
		return;

	}

	CFormView::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
