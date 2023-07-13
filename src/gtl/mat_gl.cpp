#include "pch.h"

#include "gtl/coord.h"
#include "gl/glew.h"
#include "gtl/mat_gl.h"

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
		double r = (float)width/img.cols;
		glTexCoord2f(0.f, 0.f);	glVertex2i(rect.left,   rect.top);
		glTexCoord2f(0.f, 1.f);	glVertex2i(rect.left,   rect.bottom);
		glTexCoord2f(r, 1.f);	glVertex2i(rect.right,  rect.bottom);
		glTexCoord2f(r, 0.f);	glVertex2i(rect.right,  rect.top);
		glEnd();

		return true;
	}


}	// namespace gtl

