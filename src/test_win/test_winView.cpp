
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
	ON_BN_CLICKED(IDC_TEST_LARGE_BITMAP, &CtestwinView::OnBnClickedTestLargeBitmap)
	ON_BN_CLICKED(IDC_TEST_SAVE_BMP_24BPP, &CtestwinView::OnBnClickedTestSaveBMP_24BPP)
	ON_BN_CLICKED(IDC_TEST_SAVE_BMP_1BPP, &CtestwinView::OnBnClickedTestSaveBMP_1BPP)
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
	case IDC_VIEW_BMP :
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
					palette[i] = (RGBQUAD const&)(gtl::ColorBGRA(0,0,i));
				}
			}

			gtlw::TStopWatch<char, std::chrono::duration<double>> sw;
			if (nIDCtl == IDC_VIEW1) {
				gtlw::MatToDC(img, img.size(), dc, rect, palette);
				sw.Lap("1 byte image");
			}
			else if (nIDCtl == IDC_VIEW3) {
				gtlw::MatToDC(img3, img3.size(), dc, rect);
				sw.Lap("3 byte image");
			}
			else if (nIDCtl == IDC_VIEW_BMP) {
				if (!m_bitmap.empty()) {
					cv::Mat img;
					if ((m_bitmap.cols > rect.Width()) or (m_bitmap.rows > rect.Height())) {
						double scale = std::min(rect.Width() / (double)m_bitmap.cols, rect.Height() / (double)m_bitmap.rows);
						cv::resize(m_bitmap, img, {}, scale, scale, cv::INTER_NEAREST);
					}
					else {
						img = m_bitmap;
					}
					gtlw::MatToDC(img, img.size(), dc, rect);
					sw.Lap("Bitmap");
				}
			}

		}
		return;

	}

	CFormView::OnDrawItem(nIDCtl, lpDrawItemStruct);
}


void CtestwinView::OnBnClickedTestMatTime() {
	cv::Mat img3;
	img3 = cv::Mat::zeros(1256, 1256, CV_8UC1);
	for (int row{}; row < img3.rows; row++) {
		img3.row(row) = row%256;
	}

	gtlw::TStopWatch<char, std::chrono::duration<double>> sw;
	cv::cvtColor(img3, img3, cv::COLOR_GRAY2BGR);
	sw.Lap("1256x1256 1ch -> 3ch");
}

void CtestwinView::OnBnClickedTestLargeBitmap() {
	CWaitCursor wc;

	CImage img;
	if (!img.Create(20'000, 20'000, 1))
		return ;
	std::vector<RGBQUAD> palette;
	palette.assign(2, {});
	palette.back() = (RGBQUAD&)gtl::ColorBGRA(255, 255, 255, 0);
	img.SetColorTable(0, std::size(palette), palette.data());

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
	gtlw::MatToDC(mat, mat.size(), dc, CRect(0, 0, mat.cols, mat.rows));
	dc.Detach();
	img.ReleaseDC();

	img.Save(_T("Z:\\Downloads\\a.bmp"));

	{
		cv::Mat mat = cv::imread("Z:\\Downloads\\a.bmp");
		auto s =mat.size();
	}

}


template < bool bPixelIndex, bool bLoopUnrolling = true >
void MatToBitmapFile(std::ostream& f, cv::Mat const& img, int nBPP, int width32, std::vector<uint8_t> const& pal) {
	std::vector<uint8_t> line((size_t)width32, 0);
	if constexpr (bLoopUnrolling) {
		if (nBPP == 1) {
			int ncol = img.cols / 8 * 8;
			for (int y{}; y < img.rows; y++) {
				auto* ptr = img.ptr<uint8_t>(y);
				//memset(line.data(), 0, line.size());

				int x{};
				for (; x < ncol; x += 8) {
					int col = x/8;
					if constexpr (bPixelIndex) {
						line[col] = (ptr[x+0] << 7) | (ptr[x+1] << 6) | (ptr[x+2] << 5) | (ptr[x+3] << 4) | (ptr[x+4] << 3) | (ptr[x+5] << 2) | (ptr[x+6] << 1) | (ptr[x+7]);
					}
					else {
						line[col] = (pal[ptr[x+0]] << 7) | (pal[ptr[x+1]] << 6) | (pal[ptr[x+2]] << 5) | (pal[ptr[x+3]] << 4) | (pal[ptr[x+4]] << 3) | (pal[ptr[x+5]] << 2) | (pal[ptr[x+6]] << 1) | (pal[ptr[x+7]]);
					}
				}
				int col = x/8;
				for (int shift{7}; x < img.cols; x++, shift--) {
					if constexpr (bPixelIndex) {
						line[col] = ptr[x] << shift;
					} else {
						line[col] |= pal[ptr[x]] << shift;
					}
				}

				f.write((char const*)line.data(), width32);
			}
		}
		else if (nBPP == 4) {
			int ncol = img.cols / 2 * 2;
			for (int y{}; y < img.rows; y++) {
				auto* ptr = img.ptr<uint8_t>(y);
				//memset(line.data(), 0, line.size());

				int x{};
				for (; x < ncol; x += 2) {
					int col = x/2;
					if constexpr (bPixelIndex) {
						line[col] = (ptr[x+0] << 4) | (ptr[x+1] << 0);
					}
					else {
						line[col] = (pal[ptr[x+0]] << 4) | (pal[ptr[x+1]] << 0);
					}
				}
				int col = x/2;
				for (int shift{4}; x < img.cols; x++, shift -= 4) {
					if constexpr (bPixelIndex) {
						line[col] |= ptr[x] << shift;
					} else {
						line[col] |= pal[ptr[x]] << shift;
					}
				}
				f.write((char const*)line.data(), width32);
			}
		}
		else if (nBPP == 8) {
			for (int y{}; y < img.rows; y++) {
				auto* ptr = img.ptr<uint8_t>(y);
				//memset(line.data(), 0, line.size());
				for (int x{}; x < img.cols; x++) {
					if constexpr (bPixelIndex) {
						line[x] = ptr[x];
					} else {
						line[x] = pal[ptr[x]];
					}
					f.write((char const*)line.data(), width32);
				}
			}
		}
	}
	else {
		int d = 8/nBPP;
		for (int y{}; y < img.rows; y++) {
			auto* ptr = img.ptr<uint8_t>(y);
			//memset(line.data(), 0, line.size());

			for (int x{}, shift{}; x < img.cols; x++, shift+=nBPP) {
				int col = x/d;
				if (shift >= 8)
					shift = 0;
				if constexpr (bPixelIndex) {
					line[col] |= ptr[x] << shift;
				} else {
					line[col] |= pal[ptr[x]] << shift;
				}
			}
			f.write((char const*)line.data(), width32);
		}
	}
}

/// @brief Save Image to BITMAP. Image is COLOR or GRAY level image.
/// @param path 
/// @param img : CV_8UC1 : gray scale, CV_8UC3 : color (no palette supported), for CV8UC3, palette is not used.
/// @param nBPP 
/// @param palette 
/// @param bPixelIndex if true, img value is NOT a pixel but a palette index. a full palette must be given.
/// @return 
bool SaveBitmap(std::filesystem::path const& path, cv::Mat const& img, int nBPP, std::span<RGBQUAD> palette = {}, bool bPixelIndex = false) {
#pragma pack(push, 2)
	struct BMP_FILE_HEADER {
		char sign[2]{ 'B', 'M' };
		DWORD sizeFile{};
		WORD reserved1{};
		WORD reserved2{};
		DWORD offsetData{};
	};
	struct BITMAPINFOHEADER {
		DWORD      biSize;
		LONG       biWidth;
		LONG       biHeight;
		WORD       biPlanes;
		WORD       biBitCount;
		DWORD      biCompression;
		DWORD      biSizeImage;
		LONG       biXPelsPerMeter;
		LONG       biYPelsPerMeter;
		DWORD      biClrUsed;
		DWORD      biClrImportant;
	};
#pragma pack(pop)

	if (img.empty())
		return false;

	auto type = img.type();

	auto cx = img.cols;
	auto cy = img.rows;
	if ( (cx >= 0xffff) or (cy >= 0xffff) )
		return false;
	if (cx * cy >= 0xffff'ffff - sizeof(BMP_FILE_HEADER) - sizeof(BITMAPINFOHEADER))
		return false;
	int pixel_size = (type == CV_8UC3) ? 3 : ((type == CV_8UC1) ? 1 : 0);
	if (pixel_size <= 0)
		return false;

	std::ofstream f(path, std::ios_base::binary);
	if (!f)
		return false;

	BMP_FILE_HEADER fh;
	BITMAPINFOHEADER header{};

	header.biSize = sizeof(header);
	header.biWidth = cx;
	header.biHeight = -cy;
	header.biPlanes = 1;
	header.biCompression = BI_RGB;
	header.biSizeImage = 0;//cx * cy * pixel_size;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	if (pixel_size == 3) {
		auto width32 = gtl::AdjustAlign32(cx * 3);
		fh.offsetData = sizeof(fh) + sizeof(header);
		fh.sizeFile = fh.offsetData + width32 * cy;
		header.biBitCount = 8 * pixel_size;
		header.biClrUsed = header.biClrImportant = 0;

		f.write((char const*)&fh, sizeof(fh));
		f.write((char const*)&header, sizeof(header));
		char redundant[4];
		size_t sr = width32 - (cx * 3);
		for (int y{}; y < img.rows; y++) {
			auto* ptr = img.ptr<cv::Vec3b>(y);
			f.write((char const*)ptr, 3 * img.cols);
			f.write(redundant, sr);
		}
	}
	else if (type == CV_8UC1) {
		if ( (nBPP != 1) and /*(nBPP != 2) and */(nBPP != 4) and (nBPP != 8) )
			return false;
		auto width32 = (cx * nBPP + 31) / 32;
		fh.offsetData = sizeof(fh) + sizeof(header) + (0x01ul<<nBPP)*4;
		fh.sizeFile = fh.offsetData + width32 *cy;
		header.biBitCount = nBPP;
		std::vector<RGBQUAD> paletteLocal;
		if (palette.empty()) {

		}
		header.biClrUsed = header.biClrImportant = palette.size();

		f.write((char const*)&fh, sizeof(fh));
		f.write((char const*)&header, sizeof(header));

		f.write((char const*)palette.data(), palette.size()*sizeof(palette[0]));

		std::vector<uint8_t> pal((size_t)256, 0);
		if (!bPixelIndex) {
			for (size_t i{}; i < palette.size(); i++) {
				pal[palette[i].rgbRed] = i;
			}
		}

		gtlw::CStopWatchA sw;

		if constexpr (false) {
			if (bPixelIndex)
				MatToBitmapFile<true, false>(f, img, nBPP, line, pal, width32);
			else
				MatToBitmapFile<false, false>(f, img, nBPP, line, pal, width32);

			sw.Lap("rolling {}bpp", nBPP);
		}
		else {
			// Loop Unrolling
			if (nBPP == 1) {
				int ncol = img.cols / 8 * 8;
				for (int y{}; y < img.rows; y++) {
					auto* ptr = img.ptr<uint8_t>(y);
					memset(line.data(), 0, line.size());

					int x{};
					if (bPixelIndex) {
						for (; x < ncol; x += 8) {
							int col = x/8;
							line[col] = (ptr[x+0] << 7) | (ptr[x+1] << 6) | (ptr[x+2] << 5) | (ptr[x+3] << 4) | (ptr[x+4] << 3) | (ptr[x+5] << 2) | (ptr[x+6] << 1) | (ptr[x+7]);
						}
						int col = x/8;
						for (int shift{7}; x < img.cols; x++, shift--) {
							line[col] = ptr[x] << shift;
						}
					} else {
						for (; x < ncol; x += 8) {
							int col = x/8;
							line[col] = (pal[ptr[x+0]] << 7) | (pal[ptr[x+1]] << 6) | (pal[ptr[x+2]] << 5) | (pal[ptr[x+3]] << 4) | (pal[ptr[x+4]] << 3) | (pal[ptr[x+5]] << 2) | (pal[ptr[x+6]] << 1) | (pal[ptr[x+7]]);
						}
						int col = x/8;
						for (int shift{}; x < img.cols; x++, shift++) {
							line[col] |= pal[ptr[x]] << shift;
						}
					}

					f.write((char const*)line.data(), width32);
				}
			}
			else if (nBPP == 4) {
				int ncol = img.cols / 2 * 2;
				for (int y{}; y < img.rows; y++) {
					auto* ptr = img.ptr<uint8_t>(y);
					memset(line.data(), 0, line.size());

					int x{};
					if (bPixelIndex) {
						for (; x < ncol; x += 2) {
							int col = x/2;
							line[col] = (ptr[x+0] << 4) | (ptr[x+1] << 0);
						}
						int col = x/2;
						for (int shift{4}; x < img.cols; x++, shift -= 4) {
							line[col] |= ptr[x] << shift;
						}
					} else {
						for (; x < ncol; x += 2) {
							int col = x/2;
							line[col] = (pal[ptr[x+0]] << 4) | (pal[ptr[x+1]] << 0);
						}
						int col = x/2;
						for (int shift{4}; x < img.cols; x++, shift -= 4) {
							line[col] |= pal[ptr[x]] << shift;
						}
					}
					f.write((char const*)line.data(), width32);
				}
			}
			else if (nBPP == 8) {
				if (bPixelIndex) {
					for (int y{}; y < img.rows; y++) {
						auto* ptr = img.ptr<uint8_t>(y);
						memset(line.data(), 0, line.size());
						for (int x{}; x < img.cols; x++) {
							line[x] = ptr[x];
						}
						f.write((char const*)line.data(), width32);
					}
				} else {
					for (int y{}; y < img.rows; y++) {
						auto* ptr = img.ptr<uint8_t>(y);
						memset(line.data(), 0, line.size());
						for (int x{}; x < img.cols; x++) {
							line[x] = pal[ptr[x]];
						}
						f.write((char const*)line.data(), width32);
					}
				}
			}
			sw.Lap("unrolling {}bpp", nBPP);
		}

	}

	return true;
}

void CtestwinView::OnBnClickedTestSaveBMP_24BPP() {

	CWaitCursor wc;

	static cv::Mat mat;
	if (mat.empty()) {
		mat = cv::Mat::zeros(30'000, 30'000, CV_8UC3);
		for (int row{}; row < mat.rows; row++) {
			auto v = ((row / 1'000) % 2) ? 0 : 255;

			mat.row(row) = cv::Scalar(0, 0, v);
		}
	}

	SaveBitmap(L"Z:\\Downloads\\24.bmp", mat, 24, {});

}

void CtestwinView::OnBnClickedTestSaveBMP_1BPP() {

	CWaitCursor wc;

	static cv::Mat mat;
	if (mat.empty()) {
		mat = cv::Mat::zeros(30'000, 30'000, CV_8UC1);
		for (int row{}; row < mat.rows; row++) {
			auto v = ((row / 1'000) % 2) ? 0 : 255;

			auto r = mat.row(row);
			r = v;//cv::Scalar(0, 0, v);
			r.col(0) = 255;
			r.col(r.cols-1) = 255;
		}
	}

	std::vector<RGBQUAD> palette;
	palette.push_back({});
	palette.push_back((RGBQUAD&)gtl::ColorBGRA(255, 255, 255, 0));
	SaveBitmap(L"Z:\\Downloads\\8-1bpp.bmp", mat, 1, palette);

}

void CtestwinView::OnBnClickedTestSaveBMP_nBPP() {

	CWaitCursor wc;

	static cv::Mat mat;
	if (mat.empty()) {
		mat = cv::Mat::zeros(30'000, 40'000, CV_8UC1);
		for (int row{}; row < mat.rows; row++) {
			auto v = ((row / 1'000) % 2) ? 0 : 255;

			auto r = mat.row(row);
			r = v;//cv::Scalar(0, 0, v);
			r.col(0) = 255;
			r.col(r.cols-1) = 255;
		}
	}

	//if (1) {
	//	std::vector<RGBQUAD> palette{(size_t)4, RGBQUAD{}};
	//	palette.back() = (RGBQUAD&)gtl::ColorBGRA(255, 255, 255, 0);
	//	SaveBitmap(L"Z:\\Downloads\\8-2.bmp", mat, 2, palette);
	//}

	if (1) {
		std::vector<RGBQUAD> palette{(size_t)16, RGBQUAD{}};
		palette.back() = (RGBQUAD&)gtl::ColorBGRA(255, 255, 255, 0);
		SaveBitmap(L"Z:\\Downloads\\8-4.bmp", mat, 4, palette);
	}

	if (1) {
		std::vector<RGBQUAD> palette{(size_t)256, RGBQUAD{}};
		palette.back() = (RGBQUAD&)gtl::ColorBGRA(255, 255, 255, 0);
		SaveBitmap(L"Z:\\Downloads\\8-8.bmp", mat, 8, palette);
	}
}


void CtestwinView::OnBnClickedTestLoadBMP() {
	CFileDialog dlg(true, _T(".bmp"), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, _T("All Files(*.*)|*.*||"), this);
	if (dlg.DoModal() != IDOK)
		return;

	CWaitCursor wc;

	auto buf = gtl::FileToBuffer<uint8_t>((LPCTSTR)dlg.GetPathName());
	if (!buf)
		return;

	try {
		cv::Mat img = cv::imdecode(*buf, 0);
		if (img.empty())
			return;
		m_bitmap = img;
		Invalidate(false);
	}
	catch (...) {
		MessageBox(_T("Failed"));
		return;
	}

}
