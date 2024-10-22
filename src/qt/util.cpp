#include "pch.h"
#include "gtl/qt/qt.h"
#include "gtl/qt/util.h"
#include "gtl/reflection_glaze.h"
#include <QImage>

namespace gtl::qt {

	/// @brief returns QImage::Format
	QImage::Format GetImageFormatType(int cv_type) {
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
		if (auto iter = m.find(cv_type); iter != m.end())
			return iter->second;
		return QImage::Format_Invalid;
	}

	static std::string s_strKeyWindowPosition = "WindowPositions";
	struct sWindowPosOption{
		bool bVisible{true};
		bool bMaximized{};
		bool bMinimized{};

		GLZ_LOCAL_META(sWindowPosOption, bVisible, bMaximized, bMinimized);

		void operator << (QWidget& wnd) {
			bVisible = wnd.isVisible();
			bMaximized = wnd.isMaximized();
			bMinimized = wnd.isMinimized();
		}
		void operator >> (QWidget& wnd) {
			//if (bVisible)
			//	wnd.show();
			//else
			//	wnd.hide();
			if (bMaximized)
				wnd.showMaximized();
			//else if (bMinimized)
			//	wnd.showMinimized();
		}
	};
	bool SaveWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd) {
		if (!wnd)
			return false;

		// option (Maximized, Minimized, ...)
		sWindowPosOption option;
		option << *wnd;
		std::string str;
		glz::write_json(option, str);
		reg.setValue(std::format("{}/{}_misc", s_strKeyWindowPosition, strWindowName), ToQString(str));

		if (!option.bMaximized and !option.bMinimized) {
			// position
			auto rc = ToCoord(wnd->geometry());
			reg.setValue(std::format("{}/{}_rect", s_strKeyWindowPosition, strWindowName), ToQString(gtl::ToString<wchar_t>(rc)));
		}

		return true;
	}
	bool LoadWindowPosition(QSettings& reg, std::string const& strWindowName, QWidget* wnd) {
		if (!wnd)
			return false;

		// options
		auto misc = reg.value(std::format("{}/{}_misc", s_strKeyWindowPosition, strWindowName), "").toString().toStdString();
		sWindowPosOption option;
		if (glz::read_json(option, misc) == glz::error_code::none) {
			option >> *wnd;
		}

		if (!option.bMaximized and !option.bMaximized) {
			auto value = reg.value(std::format("{}/{}_rect", s_strKeyWindowPosition, strWindowName), "0,0,0,0").toString().toStdString();
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
			if (!rectAdjusted.IsRectEmpty())
				wnd->setGeometry(ToQCoord(rectAdjusted));
		}

		return true;
	}

}	// namespace gtl::qt
