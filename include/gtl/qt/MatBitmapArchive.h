/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MatHelper.h : cv::Mat, CDC,...
//
// PWH.
//
// 2023.07.17 from gtl/win_util/MatHelper.h
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/qt/_lib_gtl_qt.h"
#include "gtl/mat_helper.h"
#include "gtl/qt/qt.h"
#include "opencv2/opencv.hpp"


namespace gtl::qt {
#pragma pack(push, 8)	// default align. (8 bytes)

	GTL__QT_API bool SaveBitmapMatProgress(
		std::filesystem::path const& path, cv::Mat const& img, int nBPP, gtl::xSize2i const& pelsPerMeter,
		std::span<gtl::color_bgra_t const> palette = {}, bool bNoPaletteLookup = false, bool bBottom2Top = false);

	GTL__QT_API sLoadBitmapMatResult LoadBitmapMatProgress(std::filesystem::path const& path);
	GTL__QT_API sLoadBitmapMatPixelArrayResult LoadBitmapMatPixelArrayProgress(std::filesystem::path const& path);

#pragma pack(pop)
}
