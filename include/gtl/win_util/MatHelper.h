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

#include "gtl/win_util/win_util.h"
#include "opencv2/opencv.hpp"


namespace gtl::win_util {
#pragma pack(push, 8)	// default align. (8 bytes)

	GTL__WINUTIL_API bool MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, std::span<RGBQUAD> palette = {});
	GTL__WINUTIL_API bool MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, CBitmap const& mask);	// mask : monochrome bitmap, background of img value must be zero.
	GTL__WINUTIL_API bool MatToDCTransparent (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, COLORREF crTransparent);
	GTL__WINUTIL_API bool MatToDCAlphaBlend  (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, BLENDFUNCTION blend);

	GTL__WINUTIL_API bool SaveBitmapMatProgress(std::filesystem::path const& path, cv::Mat const& img, int nBPP, std::span<gtl::color_bgra_t> palette = {}, bool bPixelIndex = false);
	GTL__WINUTIL_API cv::Mat LoadBitmapMatProgress(std::filesystem::path const& path);

#pragma pack(pop)
}
