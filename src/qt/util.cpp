#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/util.h"
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


	bool SaveWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd) {
		if (!wnd)
			return false;
		auto strKey = std::format("WindowPositions/{}_rect", strWindowName);

		auto rc  = ToCoord(wnd->geometry());
		reg.setValue(strKey, ToQString(gtl::ToString<wchar_t>(rc)));

		return true;
	}
	bool LoadWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd) {
		if (!wnd)
			return false;
		auto strKey = std::format("WindowPositions/{}_rect", strWindowName);

		auto value = reg.value(strKey, "0,0,0,0").toString().toStdString();
		auto rc = gtl::FromString<gtl::xRect2i, char>(value);

		//// High DPI
		//rc.pt0() = pWindow->FromDIP(wxPoint(rc.pt0()));
		//rc.pt1() = pWindow->FromDIP(wxPoint(rc.pt1()));
		std::vector<gtl::xPoint2i> pts{ {rc.left, rc.top}, {rc.left, rc.bottom}, {rc.right, rc.top}, {rc.right, rc.bottom} };

		// get display bounds
		std::vector<gtl::xRect2i> bounds;
		for (auto* screen: qApp->screens()) {
			bounds.emplace_back(ToCoord(screen->geometry()));
		}

		// count vertex in the display
		int nPts{};
		for (auto const& rect : bounds) {
			for (auto const& pt : pts) {
				if (!rect.PtInRect(pt))
					continue;
				nPts++;
			}
			if (nPts >= 4)
				break;
		}

		gtl::xRect2i rectAdjusted;
		if (nPts >= 3) {
			// if 3 more points are in the display rect, accept it.
			rectAdjusted = rc;
		}
		else {
			// if not, adjust rectangle.
			int maxArea{};
			for (auto const& bound : bounds) {
				auto u = rc;
				u &= bound;
				auto area = u.Width() * u.Height();
				if (maxArea < area) {
					maxArea = area;
					rectAdjusted = u;
				}
			}
		}
		if (rectAdjusted.IsRectEmpty())
			return false;
		//wnd->resize(ToQCoord(xSize2i(rectAdjusted.pt1()-rectAdjusted.pt0())));
		//wnd->move(ToQCoord(rectAdjusted.pt0()));
		wnd->setGeometry(ToQCoord(rectAdjusted));
		return true;
	}

}	// namespace gtl::qt
