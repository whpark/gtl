/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Mat_gl.h : cv::Mat, and openGL
//
// PWH.
//
// 2023.07.13 from gtl::wx/MatView_misc
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "_lib_gtl.h"

#include <tuple>
// include openGL
#include "GL/gl.h"
#include "opencv2/opencv.hpp"

namespace gtl {
#pragma pack(push, 8)	// default align. (8 bytes)

	// returns eInternalColorFormat, ColorFormat, PixelType
	GTL__API std::tuple<GLint, GLenum, GLenum> GetGLImageFormatType(int type);
	GTL__API bool PutMatAsTexture(GLuint textureID, cv::Mat const& img, int width, gtl::xRect2i const& rect);

#pragma pack(pop)
}	// namespace gtl

