#include "pch.h"
#include "gtl/qt/qt.h"
#include <QImage>

namespace gtl::qt {

	/// @brief returns QImage::Format
	QImage::Format GetImageFormatType(int type) {
		static std::unordered_map<int, QImage::Format> const m {
			{ CV_8UC1,	QImage::Format_Grayscale8 },
			{ CV_8UC3,	QImage::Format_RGB888 },
			{ CV_8UC4,	QImage::Format_RGBA8888 },
			{ CV_16UC1,	QImage::Format_Grayscale16 },
			{ CV_16UC3,	QImage::Format_RGB16 },
				//{ CV_16UC4,	QImage::Format_RGB },
				//{ CV_16SC1,	QImage:: },
				//{ CV_16SC3,	QImage:: },
				//{ CV_16SC4,	QImage:: },
				//{ CV_32SC1,	QImage::{GL_LUMINANCE,	GL_LUMINANCE,		GL_INT				} },
			{ CV_32SC3,	QImage::Format_RGB32 },
				//{ CV_32SC4,	QImage:: },
				//{ CV_32FC1,	QImage:: },
				//{ CV_32FC3,	QImage:: },
			{ CV_32FC4,	QImage::Format_RGBA32FPx4 },
		};
		if (auto iter = m.find(type); iter != m.end())
			return iter->second;
		return QImage::Format_Invalid;
	}


}	// namespace gtl::qt
