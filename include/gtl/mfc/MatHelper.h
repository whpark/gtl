/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MatHelper.h : cv::Mat, CDC,...
//
// PWH.
//
// 2021.06.11 from Mocha xMathUtil
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/mfc/mfc.h"
#include "opencv2/opencv.hpp"


namespace gtl::win::inline mfc {
#pragma pack(push, 8)	// default align. (8 bytes)

	GTL__MFC_API bool MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, std::span<RGBQUAD> palette = {});
	GTL__MFC_API bool MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, CBitmap const& mask);	// mask : monochrome bitmap, background of img value must be zero.
	GTL__MFC_API bool MatToDCTransparent (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, COLORREF crTransparent);
	GTL__MFC_API bool MatToDCAlphaBlend  (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, BLENDFUNCTION blend);

	GTL__MFC_API bool SaveBitmapMatProgress(std::filesystem::path const& path, cv::Mat const& img, int nBPP, gtl::xSize2i const& pelsPerMeter, std::span<gtl::color_bgra_t> palette = {}, bool bNoPaletteLookup = false, bool bBottom2Top = false);
	GTL__MFC_API cv::Mat LoadBitmapMatProgress(std::filesystem::path const& path, gtl::xSize2i& pelsPerMeter);
	GTL__MFC_API cv::Mat LoadBitmapMatPixelArrayProgress(std::filesystem::path const& path, gtl::xSize2i& pelsPerMeter, std::vector<gtl::color_bgra_t>& palette);

#pragma pack(pop)
}
