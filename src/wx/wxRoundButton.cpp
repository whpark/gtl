#include "pch.h"
#include "wx/graphics.h"
#include "gtl/wx/wxRoundButton.h"
#include "wx/generic/private/markuptext.h"

namespace gtl::wx {

	wxIMPLEMENT_DYNAMIC_CLASS(wxRoundButton, wxRoundButton::base_t)

	wxBEGIN_EVENT_TABLE(wxRoundButton, wxRoundButton::base_t)
		EVT_MOUSE_EVENTS(OnMouse)
		EVT_PAINT(OnPaint)
		//EVT_NC_PAINT(OnNcPaint)
		EVT_SET_FOCUS(OnSetFocus)
		EVT_ERASE_BACKGROUND(OnEraseBackground)
	wxEND_EVENT_TABLE()

	bool wxRoundButton::Enable(bool enable) {
		auto r = base_t::Enable(enable);
		Refresh(false);
		return r;
	}

	void wxRoundButton::OnMouse(wxMouseEvent& evt) {
		evt.Skip();

		bool bInRect{};
		auto pos = evt.GetPosition();
		auto rect = GetClientRect();
		bInRect = rect.Contains(pos);

		if (evt.GetEventType() == wxEVT_LEFT_DOWN) {
			OutputDebugStringA("LDown \n");
			//if (!HasCapture())
			//	CaptureMouse();
			m_eState = State::State_Pressed;
		}
		else if (evt.GetEventType() == wxEVT_LEFT_UP) {
			OutputDebugStringA("LDown - released \n");
			m_eState = GetNormalState();
			//if (HasCapture())
			//	ReleaseMouse();
			// fire
			//if (bInRect) {
			//	wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, this->GetId());
			//	evt.SetEventObject(this);
			//	this->GetEventHandler()->ProcessEvent(evt);
			//}
		}
		else if (HasCapture()) {
			auto pos = evt.GetPosition();
			auto rect = GetClientRect();
			if (rect.Contains(pos)) {
				m_eState = State::State_Pressed;
			} else {
				m_eState = GetNormalState();
			}
		}
		Refresh(false);
	}

	void wxRoundButton::OnPaint(wxPaintEvent& evt) {
		//evt.Skip(false);
		wxPaintDC dc(this);
		//wxMemoryDC dc(&dcMain);

		auto rectClient = GetClientRect();
		auto rect(rectClient);
		dc.SetPen(*wxTRANSPARENT_PEN);
		auto* parent = GetParent();
		while (parent->IsKindOf(wxCLASSINFO(wxStaticBox))) {
			if (auto* p = parent->GetParent())
				parent = p;
			else
				break;
		}
		// Get Default Background color
		auto crBackground = parent->GetBackgroundColour();
		wxBrush brushBkgnd(crBackground);
		dc.SetBrush(brushBkgnd);
		dc.DrawRectangle(rect);

		bool bEnabled = this->IsEnabled();

		auto cr = this->GetBackgroundColour();
		// Get Mouse Position
		wxPoint mousePosScreen = wxGetMousePosition();
		wxPoint mousePosClient = ScreenToClient(mousePosScreen);
		if (bEnabled and rect.Contains(mousePosClient)) {
			cr = wxColor(std::clamp((int)(cr.Red()*1.2), 0, 255), std::clamp((int)(cr.Green()*1.2), 0, 255), std::clamp((int)(cr.Blue()*1.2), 0, 255));
		}

		if (m_eState == State::State_Pressed
		#if defined(__WXMSW__)
			or MSWIsPushed()
		#endif
			) {
			rect.x+=1, rect.y+=1;
		}
		wxBrush brush(cr);
		dc.SetPen(wxPen(m_border.cr, m_border.width, wxPENSTYLE_SOLID));
		dc.SetBrush(brush);
		rect.Deflate(2, 2);
		int r = std::max(0, m_border.radius);
		dc.DrawRoundedRectangle(rect, r);
		if (HasFocus()) {
			auto rectFocus = rect;
			rectFocus.Deflate(2, 2);
			static wxPen pen(wxColour(0, 0, 0), 1, wxPenStyle::wxPENSTYLE_DOT);
			dc.SetPen(pen);
			dc.DrawRoundedRectangle(rectFocus, std::max(0, r-1));
		}

		if (this->m_markupText) {
			m_markupText->Render(dc, rect, 0);
		} else {
			wxMarkupText markup(GetLabelText());
			auto a = this->GetWindowStyleFlag() & wxBU_ALIGN_MASK;
			markup.Render(dc, rect, wxAlignment::wxALIGN_RIGHT);
		}

		// disabled
		if (!bEnabled and !rect.IsEmpty()) {
			wxBitmap bmp;
			bmp.Create(rect.GetSize(), 32);
			wxMemoryDC dcMem(bmp);
			static wxBrush brushBackground(wxColour(0, 0, 0));
			static wxBrush brushDisabled(wxColour(63, 63, 63));
			dcMem.SetPen(*wxTRANSPARENT_PEN);
			dcMem.SetBrush(brushBackground);
			dcMem.DrawRectangle(0, 0, bmp.GetWidth(), bmp.GetHeight());
			dcMem.SetBrush(brushDisabled);
			dcMem.DrawRoundedRectangle(0, 0, bmp.GetWidth(), bmp.GetHeight(), r);
			dc.Blit(rect.GetTopLeft(), rect.GetSize(), &dcMem, wxPoint(0, 0), wxOR);
		}

	}

	void wxRoundButton::OnNcPaint(wxNcPaintEvent& evt) {
	}

	void wxRoundButton::OnEraseBackground(wxEraseEvent& evt) {
		evt.Skip(false);
	}

	void wxRoundButton::OnSetFocus(wxFocusEvent& evt) {
		evt.Skip();
		this->Refresh();
	}

}	// namespace gtl::wx


#if 0

class GTL__WX_CLASS wxRoundButton : public wxAnyButtonBase {
public:
	using this_t = wxRoundButton;
	using base_t = wxAnyButtonBase;
	using base_t::base_t;

	virtual ~wxRoundButton() = default;

protected:
	int m_radius{ 7 };

	wxString m_strLabel;
	State m_eState{ State::State_Normal };

public:
	wxRoundButton(wxWindow *parent,
		wxWindowID id,
		const wxString& label = wxEmptyString,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxASCII_STR(wxButtonNameStr))
	{
		m_strLabel = label;

		Create(parent, id, pos, size, style, validator, name);
	}

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

wxIMPLEMENT_DYNAMIC_CLASS(wxRoundButton, wxRoundButton::base_t)

wxBEGIN_EVENT_TABLE(wxRoundButton, wxRoundButton::base_t)
	EVT_MOUSE_EVENTS(OnMouse)
	EVT_PAINT(OnPaint)
	EVT_NC_PAINT(OnNcPaint)
	EVT_SET_FOCUS(OnSetFocus)
	EVT_ERASE_BACKGROUND(OnEraseBackground)
wxEND_EVENT_TABLE()

void wxRoundButton::OnMouse(wxMouseEvent& evt) {
	evt.Skip();

	bool bInRect{};
	auto pos = evt.GetPosition();
	auto rect = GetClientRect();
	bInRect = rect.Contains(pos);

	if (evt.GetEventType() == wxEVT_LEFT_DOWN) {
		OutputDebugStringA("LDown \n");
		if (!HasCapture())
			CaptureMouse();
		m_eState = State::State_Pressed;
	}
	if (evt.GetEventType() == wxEVT_LEFT_UP) {
		OutputDebugStringA("LDown - released \n");
		m_eState = GetNormalState();
		if (HasCapture())
			ReleaseMouse();
		// fire
		if (bInRect) {
			wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, this->GetId());
			evt.SetEventObject(this);
			this->GetEventHandler()->ProcessEvent(evt);
		}
	}
	if (HasCapture()) {
		auto pos = evt.GetPosition();
		auto rect = GetClientRect();
		if (rect.Contains(pos)) {
			m_eState = State::State_Pressed;
		} else {
			m_eState = GetNormalState();
		}
	}
	Refresh();
	//OutputDebugStringA(std::format(": {}\n", m_bPressed).c_str());
}

void wxRoundButton::OnPaint(wxPaintEvent& evt) {
	//evt.Skip(false);

	wxPaintDC dc(this);
	auto rect = GetClientRect();

	dc.SetPen(*wxTRANSPARENT_PEN);
	auto* parent = GetParent();
	auto crBackground = GetParent()->GetBackgroundColour();
	wxBrush brushBkgnd(crBackground);
	dc.SetBrush(brushBkgnd);
	dc.DrawRectangle(rect);

	auto cr = this->GetBackgroundColour();
	// Get Mouse Position
	wxPoint mousePosScreen = wxGetMousePosition();
	wxPoint mousePosClient = ScreenToClient(mousePosScreen);
	if (rect.Contains(mousePosClient)) {
		cr = wxColor(std::clamp((int)(cr.Red()*1.2), 0, 255), std::clamp((int)(cr.Green()*1.2), 0, 255), std::clamp((int)(cr.Blue()*1.2), 0, 255));
	}

	if (m_eState == State::State_Pressed) {
		rect.x+=1, rect.y+=1;
	}
	wxBrush brush(cr);
	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(brush);
	rect.Deflate(2, 2);
	dc.DrawRoundedRectangle(rect, m_radius);
	if (HasFocus()) {
		rect.Deflate(2, 2);
		static wxPen pen(wxColour(0, 0, 0), 1, wxPenStyle::wxPENSTYLE_DOT);
		dc.SetPen(pen);
		dc.DrawRoundedRectangle(rect, m_radius);
	}

	//if (this->m_markupText) {
	//	m_markupText->Render(dc, rect, 0);
	//} else {
		wxMarkupText markup(GetLabelText());
		auto a = this->GetWindowStyleFlag() & wxBU_ALIGN_MASK;
		markup.Render(dc, rect, wxAlignment::wxALIGN_RIGHT);
	//}

}

void wxRoundButton::OnNcPaint(wxNcPaintEvent& evt) {
}

void wxRoundButton::OnEraseBackground(wxEraseEvent& evt) {
	evt.Skip(false);
}

void wxRoundButton::OnSetFocus(wxFocusEvent& evt) {
	evt.Skip();
	this->Refresh();
}

#endif
