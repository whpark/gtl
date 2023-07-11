#pragma once

#include <tuple>

// include openGL
#include "GL/gl.h"

#include "gtl/gtl.h"

namespace gtl {

// returns eInternalColorFormat, ColorFormat, PixelType
std::tuple<GLint, GLenum, GLenum> GetGLImageFormatType(int type);
bool PutMatAsTexture(GLuint textureID, cv::Mat const& img, int width, gtl::xRect2i const& rect);
bool DrawPixelValue(cv::Mat& canvas, cv::Mat const& imgOriginal, cv::Rect roi, gtl::xCoordTrans2d const& ctCanvasFromImage);

} // namespace gtl;
