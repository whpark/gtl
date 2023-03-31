#include "pch.h"
#include "gtl/wx/util.h"
#include "wx/Display.h"

namespace gtl::wx {

	bool SaveWindowPosition(wxRegKey& regParent, std::string const& strWindowName, wxWindow* pWindow) {
		if (!pWindow)
			return false;

		wxRegKey reg(regParent, "Position");
		if (!reg.Create())
			return false;

		gtl::xRect2i rc {pWindow->GetRect()};
		reg.SetValue(strWindowName+"_raw", gtl::ToString<wchar_t>(rc));
		rc.pt0() = pWindow->ToDIP(wxPoint(rc.pt0()));
		rc.pt1() = pWindow->ToDIP(wxPoint(rc.pt1()));
		reg.SetValue(strWindowName, gtl::ToString<wchar_t>(rc));
		return true;
	}
	bool LoadWindowPosition(wxRegKey& regParent, std::string const& strWindowName, wxWindow* pWindow) {
		if (!pWindow)
			return false;

		wxRegKey reg(regParent, "Position");
		if (!reg.Create())
			return false;

		wxString strWindowPosition;
		if (reg.HasValue(strWindowName))
			reg.QueryValue(strWindowName, strWindowPosition);
		auto rc = gtl::FromString<gtl::xRect2i, wchar_t>({strWindowPosition, strWindowPosition.size()});
		// High DPI
		rc.pt0() = pWindow->FromDIP(wxPoint(rc.pt0()));
		rc.pt1() = pWindow->FromDIP(wxPoint(rc.pt1()));
		std::vector<gtl::xPoint2i> pts{ {rc.left, rc.top}, {rc.left, rc.bottom}, {rc.right, rc.top}, {rc.right, rc.bottom} };

		// get display bounds
		std::vector<gtl::xRect2i> bounds;
		for (unsigned i{}, nDisp = wxDisplay::GetCount(); i < nDisp; i++) {
			wxDisplay disp(i);
			bounds.emplace_back(gtl::xRect2i{disp.GetClientArea()});
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
		pWindow->SetSize(wxSize(rectAdjusted.pt1()-rectAdjusted.pt0()));
		pWindow->Move(rectAdjusted.pt0());
		return true;
	}

} // namespace gtl
