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

	GTL_WINUTIL_API bool MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, std::span<RGBQUAD> palette = {});
	GTL_WINUTIL_API bool MatToDC            (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, CBitmap const& mask);	// mask : monochrome bitmap, background of img value must be zero.
	GTL_WINUTIL_API bool MatToDCTransparent (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, COLORREF crTransparent);
	GTL_WINUTIL_API bool MatToDCAlphaBlend  (cv::Mat const& img, cv::Size const& sizeView, CDC& dc, CRect const& rect, BLENDFUNCTION blend);

#pragma pack(pop)
}
