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

		int m_radius{7};

	protected:
		State m_eState{ State::State_Normal };

	public:
		void SetRadius(int radius) { m_radius = radius; }
		int GetRadius() const { return m_radius; }

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
