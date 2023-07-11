#include "pch.h"

#include "MatView_misc.h"
#include "gtl/mat_helper.h"

namespace gtl {

// returns eInternalColorFormat, ColorFormat, PixelType
std::tuple<GLint, GLenum, GLenum> GetGLImageFormatType(int type) {
	static std::unordered_map<int, std::tuple<GLint, GLenum, GLenum>> const m {
		{ CV_8UC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_UNSIGNED_BYTE	} },
		{ CV_8UC3,	{GL_RGB,		GL_RGB,				GL_UNSIGNED_BYTE	} },
		{ CV_8UC4,	{GL_RGBA,		GL_RGBA,			GL_UNSIGNED_BYTE	} },
		{ CV_16UC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_UNSIGNED_SHORT	} },
		{ CV_16UC3,	{GL_RGB,		GL_RGB,				GL_UNSIGNED_SHORT	} },
		{ CV_16UC4,	{GL_RGBA,		GL_RGBA,			GL_UNSIGNED_SHORT	} },
		{ CV_16SC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_SHORT			} },
		{ CV_16SC3,	{GL_RGB,		GL_RGB,				GL_SHORT			} },
		{ CV_16SC4,	{GL_RGBA,		GL_RGBA,			GL_SHORT			} },
		{ CV_32SC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_INT				} },
		{ CV_32SC3,	{GL_RGB,		GL_RGB,				GL_INT				} },
		{ CV_32SC4,	{GL_RGBA,		GL_RGBA,			GL_INT				} },
		{ CV_32FC1,	{GL_LUMINANCE,	GL_LUMINANCE,		GL_FLOAT			} },
		{ CV_32FC3,	{GL_RGB,		GL_RGB,				GL_FLOAT			} },
		{ CV_32FC4,	{GL_RGBA,		GL_RGBA,			GL_FLOAT			} },
	};
	if (auto iter = m.find(type); iter != m.end())
		return iter->second;
	return {};
}

bool PutMatAsTexture(GLuint textureID, cv::Mat const& img, int width, gtl::xRect2i const& rect) {
	if (!textureID or img.empty() or !img.isContinuous() /*or (img.step %4)*/)
		return false;

	glBindTexture(GL_TEXTURE_2D, textureID);
	if (img.step%4)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	else
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// Create the texture
	auto [eColorType, eFormat, ePixelType] = GetGLImageFormatType(img.type());
	glTexImage2D(GL_TEXTURE_2D, 0, eColorType, img.cols, img.rows, 0, eFormat, ePixelType, img.ptr());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// patch
	glBegin(GL_QUADS);
	double r = (double)width/img.cols;
	glTexCoord2f(0.f, 0.f);	glVertex2i(rect.left,   rect.top);
	glTexCoord2f(0.f, 1.f);	glVertex2i(rect.left,   rect.bottom);
	glTexCoord2f(r, 1.f);	glVertex2i(rect.right,  rect.bottom);
	glTexCoord2f(r, 0.f);	glVertex2i(rect.right,  rect.top);
	glEnd();

	return true;
}

bool DrawPixelValue(cv::Mat& canvas, cv::Mat const& imgOriginal, cv::Rect roi, gtl::xCoordTrans2d const& ctCanvasFromImage) {
	using xPoint2d = gtl::xPoint2d;

	// Draw Grid / pixel value
	if (ctCanvasFromImage.m_scale < 4)
		return false;
	cv::Scalar cr{127, 127, 127, 255};
	// grid - horizontal
	for (int y{roi.y}, y1{roi.y+roi.height}; y < y1; y++) {
		auto pt0 = ctCanvasFromImage(xPoint2d{roi.x, y});
		auto pt1 = ctCanvasFromImage(xPoint2d{roi.x+roi.width, y});
		cv::line(canvas, pt0, pt1, cr);
	}
	// grid - vertical
	for (int x{roi.x}, x1{roi.x+roi.width}; x < x1; x++) {
		auto pt0 = ctCanvasFromImage(xPoint2d{x, roi.y});
		auto pt1 = ctCanvasFromImage(xPoint2d{x, roi.y+roi.height});
		cv::line(canvas, pt0, pt1, cr);
	}

	// Pixel Value
	auto nChannel = imgOriginal.channels();
	auto depth = imgOriginal.depth();
	double const minHeight = 8;

	if ( ctCanvasFromImage.m_scale < ((nChannel+0.5)*minHeight) )
		return false;
	double heightFont = std::min(20., ctCanvasFromImage.m_scale/(nChannel+0.5)) / 40.;
	//auto t0 = stdc::steady_clock::now();
	for (int y{roi.y}, y1{roi.y+roi.height}; y < y1; y++) {
		auto* ptr = imgOriginal.ptr(y);
		int x1{roi.x+roi.width};
		//#pragma omp parallel for --------- little improvement
		for (int x{roi.x}; x < x1; x++) {
			auto pt = ctCanvasFromImage(xPoint2d{x, y});
			//auto p = SkPoint::Make(pt.x, pt.y);
			auto v = GetMatValue(ptr, depth, nChannel, y, x);
			auto avg = (v[0] + v[1] + v[2]) / nChannel;
			auto cr = (avg > 128) ? cv::Scalar{0, 0, 0, 255} : cv::Scalar{255, 255, 255, 255};
			for (int ch{}; ch < nChannel; ch++) {
				auto str = std::format("{:3}", v[ch]);
				cv::putText(canvas, str, cv::Point(pt.x, pt.y+(ch+1)*minHeight*heightFont*5), 0, heightFont, cr, 1, true);
			}
		}
	}
	//auto t1 = stdc::steady_clock::now();
	//auto dur = stdc::duration_cast<stdc::milliseconds>(t1-t0).count();
	//OutputDebugString(std::format(L"{} ms\n", dur).c_str());

	return true;
}

} // namespace gtl;
