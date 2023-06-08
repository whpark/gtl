#pragma once

#include "_lib_gtl_wx.h"

#include "wx/button.h"

namespace gtl::wx {

	class GTL__WX_CLASS wxRoundButton : public wxButton {
	public:
		using this_t = wxRoundButton;
		using base_t = wxButton;

		using base_t::base_t;
    
		virtual ~wxRoundButton() = default;

	protected:
		struct {
			int radius{7};
			wxColor cr{};
			int width{1};
		} m_border;
		State m_eState{ State::State_Normal };

	public:
		void SetBorder(wxColor cr, int radius = -1, int width = -1) {
			if (cr.IsOk())
				m_border.cr = cr;
			if (radius >= 0)
				m_border.radius = radius;
			if (width >= 0)
				m_border.width = width;
			Refresh(false);
		}
		std::tuple<wxColor, int, int> GetBorder() const { return {m_border.cr, m_border.radius, m_border.width}; }

		virtual bool Enable(bool enable = true) override;

	protected:
		void OnMouse(wxMouseEvent& evt);
		void OnPaint(wxPaintEvent& evt);
		void OnNcPaint(wxNcPaintEvent& evt);
		void OnEraseBackground(wxEraseEvent& evt);
		void OnSetFocus(wxFocusEvent& evt);

	private:
		wxDECLARE_EVENT_TABLE();
		wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxRoundButton);
	};

}	// namespace gtl::wx
