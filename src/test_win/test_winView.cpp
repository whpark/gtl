
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
	ON_BN_CLICKED(IDC_TEST_MAT_TIME, &CtestwinView::OnBnClickedTestMatTime)
	ON_BN_CLICKED(IDC_TEST_SAVE_CIMAGE, &CtestwinView::OnBnClickedTestSaveCImage)
	ON_BN_CLICKED(IDC_TEST_SAVE_BMP_1BPP, &CtestwinView::OnBnClickedTestSaveBMP_1BPP)
	ON_BN_CLICKED(IDC_TEST_SAVE_BMP_4BPP, &CtestwinView::OnBnClickedTestSaveBMP_4BPP)
	ON_BN_CLICKED(IDC_TEST_SAVE_BMP_8BPP, &CtestwinView::OnBnClickedTestSaveBMP_8BPP)
	ON_BN_CLICKED(IDC_TEST_SAVE_BMP_24BPP, &CtestwinView::OnBnClickedTestSaveBMP_24BPP)
	ON_BN_CLICKED(IDC_TEST_SAVE_BMP_nBPP, &CtestwinView::OnBnClickedTestSaveBMP_nBPP)
	ON_BN_CLICKED(IDC_TEST_LOAD_BMP, &CtestwinView::OnBnClickedTestLoadBMP)
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

	auto strFolder = theApp.GetProfileString(_T("misc"), _T("WorkingFolder"), _T("\\Temp\\"));
	SetDlgItemText(IDC_FOLDER_WORKING, strFolder);

	auto strMatSize = theApp.GetProfileString(_T("misc"), _T("MatSize"), _T("20000,20000"));
	SetDlgItemText(IDC_MAT_SIZE, strMatSize);
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
void CtestwinView::AssertValid() const { CFormView::AssertValid(); }
void CtestwinView::Dump(CDumpContext& dc) const { CFormView::Dump(dc); }
CtestwinDoc* CtestwinView::GetDocument() const { ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CtestwinDoc))); return (CtestwinDoc*)m_pDocument; }
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
	case IDC_VIEW_BMP :
		{
			CDC dc;
			dc.Attach(lpDrawItemStruct->hDC);
			gtl::xTrigger detacher([&dc]{dc.Detach();});

			CRect rect(lpDrawItemStruct->rcItem);

			dc.SelectStockObject(NULL_PEN);
			dc.SelectStockObject(WHITE_BRUSH);
			dc.Rectangle(rect);

			static cv::Mat img;
			if (img.empty()) {
				img = cv::Mat::zeros(1'200, 120, CV_8UC1);
				for (int row{}; row < img.rows; row++) {
					auto v = row;
					if (row < 100)
						v = 0;
					else if (row < 200)
						v = 1;
					else if (row < 300)
						v = 254;
					else if (row < 400)
						v = 255;
					else 
						v = row % 256;
					img.row(row) = v;
				}
			}

			static cv::Mat img3;
			if (img3.empty()) {
				img3 = cv::Mat::zeros(1'200, 120, CV_8UC3);
				for (int row{}; row < img3.rows; row++) {
					auto v = row%256;
					img3.row(row) = cv::Scalar(v, v, v);
				}
			}

			static std::vector<RGBQUAD> palette;
			if (palette.empty()) {
				palette.assign(256, {});
				for (size_t i{}; i < 256; i++) {
					palette[i] = (RGBQUAD const&)(gtl::ColorBGRA(0,0,(uint8_t)i));
				}
			}

			//gtlw::TStopWatch<char, std::chrono::duration<double>> sw;
			if (nIDCtl == IDC_VIEW1) {
				gtlw::MatToDC(img, img.size(), dc, rect, palette);
				//sw.Lap("1 byte image");
			}
			else if (nIDCtl == IDC_VIEW3) {
				gtlw::MatToDC(img3, img3.size(), dc, rect);
				//sw.Lap("3 byte image");
			}
			else if (nIDCtl == IDC_VIEW_BMP) {
				if (!m_bitmap.empty()) {
					cv::Mat img;
					if ((m_bitmap.cols > rect.Width()) or (m_bitmap.rows > rect.Height())) {
						double scale = std::min(rect.Width() / (double)m_bitmap.cols, rect.Height() / (double)m_bitmap.rows);
						cv::resize(m_bitmap, img, {}, scale, scale, cv::INTER_AREA);
					}
					else {
						img = m_bitmap;
					}
					gtlw::MatToDC(img, img.size(), dc, rect);
					//sw.Lap("Bitmap");
				}
			}
		}
		return;

	}

	CFormView::OnDrawItem(nIDCtl, lpDrawItemStruct);
}


void CtestwinView::OnBnClickedTestMatTime() {
	auto size = GetMatSize();
	if (size.width < 1'000) size.width = 1'000;
	if (size.height < 1'000) size.height = 1'000;

	cv::Mat img3;
	img3 = cv::Mat::zeros(size, CV_8UC1);
	for (int row{}; row < img3.rows; row++) {
		img3.row(row) = row%256;
	}

	gtlw::xStopWatch sw;
	cv::cvtColor(img3, img3, cv::COLOR_GRAY2BGR);
	sw.Lap("1256x1256 1ch -> 3ch");
}

void CtestwinView::OnBnClickedTestSaveCImage() {
	CWaitCursor wc;

	CImage img;
	auto size = GetMatSize();
	if (size.width < 1'000) size.width = 1'000;
	if (size.height < 1'000) size.height = 1'000;

	if (!img.Create(size.width, size.height, 1))
		return ;
	std::vector<RGBQUAD> palette;
	palette.assign(2, {});
	palette.back() = (RGBQUAD&)gtl::ColorBGRA(255, 255, 255, 0);
	img.SetColorTable(0, (UINT)std::size(palette), palette.data());

	gtlw::xStopWatch sw;

	CDC dc;
	dc.Attach(img.GetDC());

	static cv::Mat mat;
	if (mat.empty()) {
		mat = cv::Mat::zeros(img.GetHeight(), img.GetWidth(), CV_8UC1);
		for (int row{}; row < mat.rows; row++) {
			auto v = ((row/1'000)%2) ? 0 : 255;

			mat.row(row) = v;
		}
	}
	sw.Lap("Prepare");
	gtlw::MatToDC(mat, mat.size(), dc, CRect(0, 0, mat.cols, mat.rows));
	sw.Lap("MatToDC");
	dc.Detach();
	img.ReleaseDC();

	auto folder = GetWorkingFolder();

	img.Save((folder / _T("a.bmp")).c_str());
	sw.Lap("Save as a.bmp. {}x{} 1 bpp", img.GetWidth(), img.GetHeight());

	{
		cv::Mat mat = cv::imread((folder / "\\a.bmp").string());
		auto s = mat.size();
		sw.Lap("imread {}x{}", mat.cols, mat.rows);
	}
}

cv::Mat CtestwinView::CreateSampleImage(cv::Size size, int type, cv::Scalar cr1, cv::Scalar cr2) {
	static cv::Mat mat;
	if ( (mat.size() == size) and (mat.type() == type) )
		return mat;

	gtlw::xStopWatch sw;
	mat = cv::Mat::zeros(size, type);
	for (int row{}; row < mat.rows; row++) {
		auto v = ((row / 1'000) % 2) ? cr1 : cr2;

		auto r = mat.row(row);
		r = v;//cv::Scalar(0, 0, v);
		r.col(0) = cr2;
		r.col(1) = cr1;
		r.col(r.cols - 2) = cr1;
		r.col(r.cols - 1) = cr2;

		cv::Rect rect(row % r.cols/*사선*/, 0, mat.cols / 5, 1);
		rect = gtl::GetSafeROI(rect, r.size());
		if (!rect.empty())
			r(rect) = ~r(rect);
	}
	sw.Lap("ImageCreated rows = {}, cols = {}", mat.rows, mat.cols);
	return mat;
}

bool CtestwinView::WriteSampleImage(std::filesystem::path const& folder, cv::Mat const& mat, int nBPP) {
	gtlw::xStopWatch sw;

	std::vector<gtl::color_bgra_t> palette{ (nBPP <= 8) ? (size_t)(0x01 << nBPP) : (size_t)0, gtl::color_bgra_t{} };
	if (!palette.empty())
		palette.back() = gtl::ColorBGRA(255, 255, 255, 0);

	auto path = folder / std::format(L"{}x{}-{}bpp.bmp", mat.cols, mat.rows, nBPP);
	gtl::xSize2i pelsPerMeter{gtl::Round(360*1'000/25.4), gtl::Round(360*1'000/25.4)};
	bool bResult = gtlw::SaveBitmapMatProgress(path, mat, nBPP, pelsPerMeter, palette, false);

	sw.Lap("{}rows = {}, cols = {} {}bpp", bResult ? "" : "FAILED ", mat.rows, mat.cols, nBPP);

	return bResult;
};


void CtestwinView::TestSaveBMP(int nBPP) {
	CWaitCursor wc;

	auto size = GetMatSize();
	if (size.width < 1'000) size.width = 1'000;
	if (size.height < 1'000) size.height = 1'000;

	gtlw::xStopWatch sw;

	cv::Mat mat = CreateSampleImage(size, nBPP <= 8 ? CV_8UC1 : CV_8UC3);

	auto folder = GetWorkingFolder();

	if (!WriteSampleImage(folder, mat, nBPP)) {
		return;
	}
}

void CtestwinView::OnBnClickedTestSaveBMP_1BPP()  { TestSaveBMP( 1); }
void CtestwinView::OnBnClickedTestSaveBMP_4BPP()  { TestSaveBMP( 4); }
void CtestwinView::OnBnClickedTestSaveBMP_8BPP()  { TestSaveBMP( 8); }
void CtestwinView::OnBnClickedTestSaveBMP_24BPP() { TestSaveBMP(24); }

void CtestwinView::OnBnClickedTestSaveBMP_nBPP() {
	TestSaveBMP(1);
	TestSaveBMP(4);
	TestSaveBMP(8);
}


void CtestwinView::OnBnClickedTestLoadBMP() {
	CFileDialog dlg(true, _T(".bmp"), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, _T("All Files(*.*)|*.*||"), this);
	if (dlg.DoModal() != IDOK)
		return;
	std::filesystem::path path = (LPCTSTR)dlg.GetPathName();

	CWaitCursor wc;

	gtlw::xStopWatch sw;

	cv::Mat img;
	//img = gtl::LoadImageMat(path);
	//sw.Lap("LoadImageMat");
	if (img.empty()) {
		if (m_bitmap.cols* m_bitmap.rows >= 20'000*20'000)
			m_bitmap.release();
		
		gtl::xSize2i pelsPerMeter;
		img = gtlw::LoadBitmapMatProgress(path, pelsPerMeter);

		sw.Lap("LoadBitmapMat -- ");
	}
	if (img.empty()) {
		MessageBox(_T("Failed"));
		return;
	}

	if (img.channels() == 4)
		cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);

	m_bitmap = img;
	Invalidate(false);

}
