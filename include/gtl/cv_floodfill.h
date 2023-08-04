#pragma once

#include "_lib_gtl.h"
#include "opencv2/opencv.hpp"

/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

//=================================================================================================================================
// form opencv. FFillSegment::ushort -->> uint32_t
// PWH. 2023-08-04
// 

#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 8)
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif

namespace cv::gtl {

    GTL__API int floodFill( cv::InputOutputArray image, cv::InputOutputArray mask,
        cv::Point seedPoint, cv::Scalar newVal, CV_OUT cv::Rect* rect=0,
        cv::Scalar loDiff = cv::Scalar(), cv::Scalar upDiff = cv::Scalar(),
        int flags = 4 );
    GTL__API int floodFill( cv::InputOutputArray image,
        cv::Point seedPoint, cv::Scalar newVal, CV_OUT cv::Rect* rect = 0,
        cv::Scalar loDiff = cv::Scalar(), cv::Scalar upDiff = cv::Scalar(),
        int flags = 4 );

}

/* End of file. */
