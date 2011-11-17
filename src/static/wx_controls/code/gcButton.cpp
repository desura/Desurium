/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "Common.h"
#include "gcButton.h"
#include "gcImageControl.h"
#include "gcManagers.h"

#define MIN_WIDTH 78

gcButton::gcButton(wxWindow* parent, wxWindowID id, const wxString& text, const wxPoint& pos, const wxSize& size, long style ) 
#ifdef WIN32
	: gcFlickerFreePaint<wxButton>(parent, id, text, pos, size, style)
#else
	: gcFlickerFreePaint<wxPanel>(parent, id, pos, size, style)
#endif
{
#ifdef WIN32
	init(size);
#else
	init(size, text);
#endif
}

gcButton::gcButton() 
#ifdef WIN32
	: gcFlickerFreePaint<wxButton>()
#else
	: gcFlickerFreePaint<wxPanel>()
#endif
{

}

#ifdef WIN32
bool gcButton::MSWCommand(WXUINT param, WXWORD WXUNUSED(id))
{
	return true;
}
#endif

void gcButton::Create( wxWindow* parent, wxWindowID id, const wxString& text, const wxPoint& pos, const wxSize& size, long style )
{
#ifdef WIN32
	wxButton::Create( parent, id, text, pos, size, style );
	init(size);
#else
	wxPanel::Create( parent, id, pos, size, style );
	init(size, text);
#endif
}

#ifdef WIN32
void gcButton::init(const wxSize& size)
{
#else
void gcButton::init(const wxSize& size, const wxString &text)
{
	m_szText = text;
#endif

	m_szCurImage = NULL;
		
	m_bHovering = false;
	m_bDepressed = false;
	m_bFocus = false;
	m_bClearBG = true;
	m_bTransEnabled = true;
	m_bShowDepressed = false;
	m_bEnabled = true;

	Bind(wxEVT_ENTER_WINDOW, &gcButton::onMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &gcButton::onMouseEvent, this);
	Bind(wxEVT_MOTION, &gcButton::onMouseEvent, this);

	Bind(wxEVT_LEFT_UP, &gcButton::onMouseUp, this);
	Bind(wxEVT_LEFT_DOWN, &gcButton::onMouseDown, this);
	
	Bind(wxEVT_KILL_FOCUS, &gcButton::onBlur, this);
	Bind(wxEVT_SET_FOCUS, &gcButton::onFocus, this);

	Bind(wxEVT_MOUSE_CAPTURE_LOST, &gcButton::onMouseCaptureLost, this);

	m_szImage = "#button_normal";
	m_szHoverImage = "#button_hover";
	m_szDepressedImage = "#button_depressed";
	m_szDisabledImage = "#button_disable";
	m_szFocusImage = "#button_focus";

	refreshImage(true);

	wxSize newSize = size;

	if (newSize.GetHeight() == -1)
		newSize.SetHeight(21);

	SetMinSize(newSize);
	SetSize(newSize);

	m_sLastState = BS_NORMAL;

	Managers::LoadTheme(this, "button");
	Bind(wxEVT_CHAR, &gcButton::onChar, this);
}

gcButton::~gcButton()
{
	if (HasCapture())
		ReleaseMouse();
}

#ifdef WIN32
bool gcButton::MSWOnDraw(WXDRAWITEMSTRUCT *item)
{
	invalidatePaint();
	return true;
}
#else
wxString gcButton::GetLabel() const
{
	return m_szText;
}

void gcButton::SetDefault()
{
	//todo work out how to implement this
}

wxSize gcButton::DoGetBestSize() const
{
	wxClientDC dc(wxConstCast(this, gcButton));
	wxCoord width, height;
	dc.GetMultiLineTextExtent(GetLabel(), &width, &height);
	
	if ( (width + 15) < MIN_WIDTH )
		width = MIN_WIDTH;
	else
		width += 15; //get text of button image 
	
	return wxSize(width, height);
}
#endif

void gcButton::setImage(const char* image)
{
	if (m_szCurImage == image && m_imgHandle.getImg() && m_imgHandle->IsOk())
		return;

	m_imgHandle = GetGCThemeManager()->getImageHandle(image, true);
	invalidatePaint();
}

STATE gcButton::getState()
{
	if (m_bEnabled)
	{	
		if (m_bShowDepressed)
			return BS_DEPRESSED;
		else if (m_bHovering)
			return BS_HOVER;
		else if (m_bFocus)
			return BS_FOCUSED;
		else
			return BS_NORMAL;
	}
	else
	{
		return BS_DISABLED;
	}
}

void gcButton::SetLabel(const wxString& label)
{
#ifdef WIN32
	wxButton::SetLabel(label);
#else
	m_szText = label;
#endif
	invalidatePaint();
}

void gcButton::setImage()
{
	switch (m_sLastState)
	{
	default:
	case BS_NORMAL:		setImage(m_szImage);			break;
	case BS_DISABLED:	setImage(m_szDisabledImage);	break;
	case BS_HOVER:		setImage(m_szHoverImage);		break;
	case BS_DEPRESSED:	setImage(m_szDepressedImage);	break;
	case BS_FOCUSED:	setImage(m_szFocusImage);		break;
	};
}

void gcButton::refreshImage(bool force)
{
	STATE state = getState();

	if (!force && state == m_sLastState)
		return;

	m_sLastState = state;
	setImage();

	invalidatePaint();
}

STATE gcButton::getLastState()
{
	return m_sLastState;
}

bool gcButton::Enable(bool enable)
{
#ifdef WIN32
	bool completed = wxButton::Enable(enable);
#else
	bool completed = wxPanel::Enable(enable);
#endif

	if (completed)
	{
		m_bEnabled = enable;
		refreshImage();
	}

	return completed;
}

bool gcButton::isMouseWithinButton()
{
	wxRect panelRec = GetScreenRect(); 
	wxPoint mousePoint = wxGetMousePosition();

	return panelRec.Contains(mousePoint);
}

void gcButton::updateMouse()
{
	m_bHovering = isMouseWithinButton();

	if (m_bDepressed)
	{
		if (m_bHovering)
			m_bShowDepressed = true;
		else
			m_bShowDepressed = false;
	}

	refreshImage();
}

void gcButton::onMouseEvent(wxMouseEvent& event)
{

	wxPoint pos = event.GetPosition();
	wxRect rect = this->GetRect();

	rect.x = 0;
	rect.y = 0;

	if (!rect.Contains(pos) && !m_bDepressed)
	{
		if (HasCapture())
		{
			ReleaseMouse();
		}
	}
	else
	{
		if (!HasCapture())
		{
#ifdef WIN32
			CaptureMouse();
#endif
		}
	}

	updateMouse();
	event.Skip();
}

void gcButton::onMouseUp( wxMouseEvent& event )
{
	event.Skip();

	m_bDepressed = false;
	m_bShowDepressed = false;

	wxPoint pos = event.GetPosition();
	wxRect rect = this->GetRect();

	rect.x = 0;
	rect.y = 0;

	updateMouse();

	if (rect.Contains(pos))
	{
		wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		event.SetEventObject(this);

		this->GetEventHandler()->AddPendingEvent(event);
	}
}

void gcButton::onMouseDown( wxMouseEvent& event )
{
	event.Skip();

	m_bDepressed = true;
	m_bShowDepressed = true;

	updateMouse();
}

void gcButton::onBlur( wxFocusEvent& event )
{
	m_bFocus = false;
	updateMouse();
	event.Skip();
}

void gcButton::onFocus( wxFocusEvent& event )
{
	m_bFocus = true;
	updateMouse();
	event.Skip();
}

void gcButton::onMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
	if (HasCapture())
		ReleaseMouse();

	updateMouse();
}


void gcButton::doPaint(wxDC* dc)
{
	if (!m_imgHandle.getImg() || !m_imgHandle->IsOk())
	{
		dc->SetTextForeground(wxColor(22,22,22));
		dc->Clear();
		return;
	}

	dc->SetFont(GetFont());
	dc->SetTextBackground(GetBackgroundColour());

	if (m_bEnabled)
		dc->SetTextForeground(GetForegroundColour());
	else
		dc->SetTextForeground(wxColour(75,75,75,255));


	int h = GetSize().GetHeight();
	int w = GetSize().GetWidth();
#ifdef WIN32 // unused AFAIK
	int ih = m_imgHandle->GetSize().GetHeight();
#endif
	int iw = m_imgHandle->GetSize().GetWidth();

	wxBitmap   tmpBmp(w, h);
	wxMemoryDC tmpDC(tmpBmp);

	tmpDC.SetBrush(wxBrush(wxColor(255,0,255)));
	tmpDC.SetPen( wxPen(wxColor(255,0,255),1) );
	tmpDC.DrawRectangle(0,0,w,h);

	wxImage scaled = m_imgHandle->Scale(iw, h);

	wxBitmap left = GetGCThemeManager()->getSprite(scaled, "button", "Left");
	wxBitmap right = GetGCThemeManager()->getSprite(scaled, "button", "Right");
	wxBitmap centerImg = GetGCThemeManager()->getSprite(scaled, "button", "Center");

	wxBitmap center(w-(left.GetWidth()+right.GetWidth()),h);

	wxColor bgCol(255,0,255);
	gcImage::tileImg(center, centerImg, &bgCol);

	if (left.IsOk())
		tmpDC.DrawBitmap(left, 0,0,true);

	if (center.IsOk())
		tmpDC.DrawBitmap(center, left.GetWidth(),0,true);

	if (right.IsOk())
		tmpDC.DrawBitmap(right, w-right.GetWidth(),0,true);

	tmpDC.SelectObject(wxNullBitmap);
	dc->DrawBitmap(tmpBmp, 0,0, true);

	if (m_bTransEnabled)
	{
		wxRegion region = wxRegion(tmpBmp, wxColor(255,0,255), 1);
		SetShape(region, this);
	}


	wxString lab = GetLabel();

	long x = (w - dc->GetTextExtent(lab).GetWidth())/2;
	long y = (h - dc->GetTextExtent(lab).GetHeight())/2;

	dc->DrawText(lab, x, y);
}

void gcButton::onChar(wxKeyEvent& event)
{ 
	if (event.m_keyCode == WXK_RETURN)
	{
#ifdef WIN32
		SendClickEvent();
#else
		wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		event.SetEventObject(this);
		this->GetEventHandler()->AddPendingEvent(event);
#endif // LINUX TODO
	}
	else
	{
		event.Skip();
	}
}
