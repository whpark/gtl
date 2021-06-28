/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MatHelper.h : cv::Mat, CDC,...
//
// PWH.
//
// 2021.06.11 from Mocha xMathUtil
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module;

#include "opencv2/opencv.hpp"
#include "framework.h"

export module gtlw:MatHelper;
import gtl;

namespace gtl::win_util {

	//-----------------------------------------------------------------------------
	// Mat to DC
	bool MatToDC(cv::Mat const& _img, cv::Size const& sizeView, CDC& dc, CRect const& rect, CBitmap const& mask);
	bool MatToDC(cv::Mat const& img, cv::Size const& sizeEffective, CDC& dc, CRect const& rectTarget);
	bool MatToDCTransparent(cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, COLORREF crTransparent);
	bool MatToDCAlphaBlend(cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, BLENDFUNCTION blend);

	bool MatToDC(cv::Mat const& _img, cv::Size const& sizeView, CDC& dc, CRect const& rect, CBitmap const& mask) {
		if (!mask.m_hObject)
			return MatToDC(_img, sizeView, dc, rect);

		cv::Mat img;
		if (_img.type() == CV_8UC3)
			img = _img;
		else if (_img.type() == CV_8UC1)
			ConvertColor(_img, img, cv::COLOR_GRAY2BGR);
		else
			return false;

		int cx = AdjustAlign32(img.cols);
		int cy = img.rows;

		try {
			int nChannel = 3;
			CRect rectSrc(0, 0, sizeView.width, sizeView.height);
			CRect rectDst(rect);
			CalcViewPosition(sizeView, rect, rectSrc, rectDst);

			CImage img2;
			img2.Create(cx, cy, 24, BI_RGB);
			CImage img3;
			img3.Create(cx, cy, 24, BI_RGB);
			int nBufferPitch = cx*nChannel;
			for (int y = 0; y < img.rows; y++)
				memcpy(img2.GetPixelAddress(0, y), img.ptr<cv::Vec3b>(y), nBufferPitch);
			img2.MaskBlt(img3.GetDC(), POINT(), mask, MAKEROP4(SRCCOPY, SRCAND));
			img3.ReleaseDC();
			img3.MaskBlt(dc, rectDst, CPoint(0, 0), mask, CPoint(0, 0), MAKEROP4(SRCCOPY, SRCPAINT));

		} catch (...) {
			TRACE( (GTL__FUNCSIG " - Unknown Error...").c_str() );
			return false;
		}
		return true;
	}


	bool MatToDC(cv::Mat const& img, cv::Size const& sizeEffective, CDC& dc, CRect const& rectTarget) {

		auto const type = img.type();

		int pixel_size = (type == CV_8UC3) ? 3 : ((type == CV_8UC1) ? 1 : 0);
		if (pixel_size <= 0)
			return false;

		uint8_t const* pImage = nullptr;
		int cx = gtl::AdjustAlign32(img.cols);
		int cy = img.rows;

		try {

			if (cx*pixel_size != img.step) {
				thread_local static std::vector<BYTE> buf;
				int nBufferPitch = cx *pixel_size;
				int row_size = img.cols * pixel_size;
				auto size = nBufferPitch * img.rows;
				if (buf.size() != size) {
					buf.assign(size, 0);
				}
				pImage = buf.data();
				auto * pos = buf.data();
				for (int y = 0; y < img.rows; y++) {
					memcpy(pos, img.ptr(y), row_size);
					pos += nBufferPitch;
				}
			} else {
				pImage = img.ptr<BYTE>(0);
			}

			struct BMP {
				bool bPaletteInitialized{false};
				BITMAPINFO bmpInfo{};
				RGBQUAD dummy[256-1]{};
			};
			thread_local static std::unique_ptr<BMP> bmp = std::make_unique<BMP>();
			if (!bmp->bPaletteInitialized && pixel_size == 1) {
				bmp->bPaletteInitialized = true;
				for (uint32_t i = 0; i < 256; i++) {
					(gtl::color_bgra_t&)bmp->bmpInfo.bmiColors[i] = ColorBGRA(i, i, i);
				}
			}
			BITMAPINFO& bmpInfo = bmp->bmpInfo;
			bmpInfo.bmiHeader.biSize = sizeof(bmpInfo);
			bmpInfo.bmiHeader.biWidth = cx;
			bmpInfo.bmiHeader.biHeight = -cy;
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = 8*pixel_size;
			bmpInfo.bmiHeader.biCompression = BI_RGB;
			bmpInfo.bmiHeader.biSizeImage = cx * cy * pixel_size;
			bmpInfo.bmiHeader.biXPelsPerMeter = 0;
			bmpInfo.bmiHeader.biYPelsPerMeter = 0;
			bmpInfo.bmiHeader.biClrUsed = pixel_size == 1 ? 256 : 0;
			bmpInfo.bmiHeader.biClrImportant = pixel_size == 1 ? 256 : 0;

			SetDIBitsToDevice(dc, 
								rectTarget.left, rectTarget.top, std::min(rectTarget.Width(), sizeEffective.width), std::min(rectTarget.Height(), sizeEffective.height), 
								0, 0, 0, img.rows, pImage, &bmpInfo, /*pixel_size == 1 ? DIB_PAL_COLORS : */DIB_RGB_COLORS);

			pImage = nullptr;
		} catch (...) {
			TRACE(_T(__FUNCTION__) _T(" - Unknown Error..."));
			return false;
		}

		return true;
	}

	bool MatToDCTransparent(cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, COLORREF crTransparent) {
		if (img.empty() || !dc.m_hDC || rect.IsRectEmpty())
			return false;

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(&dc, sizeView.width, sizeView.height);
		dcMem.SelectObject(&bmp);
		CBrush brush(crTransparent);
		dcMem.FillRect(rect, &brush);
		MatToDC(img, sizeView, dcMem, CRect(0, 0, sizeView.width, sizeView.height));

		CRect rectSrc, rectDst;
		CalcViewPosition(sizeView, rect, rectSrc, rectDst);

		return dc.TransparentBlt(rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), &dcMem, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), crTransparent) != FALSE;
	}

	bool MatToDCAlphaBlend(cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, BLENDFUNCTION blend) {
		if (img.empty() || !dc.m_hDC || rect.IsRectEmpty())
			return FALSE;

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(&dc, sizeView.width, sizeView.height);
		dcMem.SelectObject(&bmp);
		//CBrush brush(crTransparent);
		//dcMem.FillRect(rect, &brush);
		MatToDC(img, sizeView, dcMem, CRect(0, 0, sizeView.width, sizeView.height));

		CRect rectSrc, rectDst;
		CalcViewPosition(sizeView, rect, rectSrc, rectDst);

		return dc.AlphaBlend(rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), &dcMem, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), blend) != FALSE;
	}


}
