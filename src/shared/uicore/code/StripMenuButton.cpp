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
#include "StripMenuButton.h"
#include "wx_controls/gcManagers.h"

StripMenuButton::StripMenuButton(wxWindow *parent, wxString text, const wxSize &size) : gcFlickerFreePaint<wxPanel>(parent, wxID_ANY, wxDefaultPosition, size, wxFULL_REPAINT_ON_RESIZE|wxWANTS_CHARS), BaseMenuButton((wxPanel*)this)
{
	SetSize(size);
	SetMinSize(size);

	Bind( wxEVT_LEFT_DCLICK, &StripMenuButton::onMouseEvent, this);
	Bind( wxEVT_MOTION, &StripMenuButton::onMouseEvent, this);
	Bind( wxEVT_LEFT_DOWN, &StripMenuButton::onMouseDown, this);
	Bind( wxEVT_LEFT_UP, &StripMenuButton::onMouseUp, this);

	Bind( wxEVT_ENTER_WINDOW, &StripMenuButton::onMouseOver, this);
	Bind( wxEVT_LEAVE_WINDOW, &StripMenuButton::onMouseOut, this);
	Bind( wxEVT_UPDATE_UI, &StripMenuButton::onUpdateUI, this);

	GetParent()->Bind( wxEVT_MOTION, &StripMenuButton::onMouseEvent, this);

	m_bHovering = false;
	m_bBold = false;
	m_bHasFocus = false;
	m_bUsesFocus = false;

	m_uiOffset = 0;

	m_colHover = wxColor(GetGCThemeManager()->getColor("hyperlinkmenu", "hov-fg"));
	m_colNormal = wxColor(GetGCThemeManager()->getColor("hyperlinkmenu", "fg"));
	SetForegroundColour( m_colNormal );
	
	m_szLabel = text;
}

void StripMenuButton::showFocusBox(wxColor focusColor)
{
	Bind( wxEVT_KILL_FOCUS, &StripMenuButton::onBlur, this);
	Bind( wxEVT_SET_FOCUS, &StripMenuButton::onFocus, this);
	m_colFocus = focusColor;
	m_bUsesFocus = true;
}

void StripMenuButton::setColors(wxColor normal, wxColor hover)
{
	m_colHover = hover;
	m_colNormal = normal;
	SetForegroundColour( m_colNormal );
}

wxString StripMenuButton::GetLabel()
{
	return m_szLabel;
}

void StripMenuButton::init(const char* image, const char* overlay, int32 spacing)
{
	if (image)
		m_imgBg = GetGCThemeManager()->getImageHandle(image);

	if (overlay)
		m_imgOverlay = GetGCThemeManager()->getImageHandle(overlay);

	wxSize baseSize = GetTextSize();

	if (spacing == -1)
		baseSize.x += 20;
	else
		baseSize.x += spacing;

	SetMinSize(baseSize);
}

StripMenuButton::~StripMenuButton()
{
}

wxSize StripMenuButton::GetTextSize()
{
	wxClientDC dc(wxConstCast(this, StripMenuButton));
	wxCoord width, height;
	dc.GetMultiLineTextExtent(GetLabel(), &width, &height);
	return wxSize(width, height);	
}


void StripMenuButton::onSelected()
{
	invalidatePaint();
	Refresh();
}

void StripMenuButton::paintBGImage(wxDC* dc)
{
	wxSize sz = GetSize();
	wxMemoryDC memDC;

	uint32 w = sz.GetWidth();
	uint32 h = sz.GetHeight();

	dc->SetPen(wxPen(GetBackgroundColour(),1)); 
	dc->SetBrush(wxBrush(GetBackgroundColour()));

	wxBitmap temp(w, h);
	gcImage::tileImg(temp, wxBitmap(*m_imgBg.getImg()));

	memDC.SelectObject(temp);

	dc->Blit(0, 0,			// Draw at (100, 100)
			w, h,			// Draw full bitmap
			&memDC,			// Draw from memDC
			0, 0,			// Draw from bitmap origin
			wxCOPY,			// Logical operation
			true);			// Take mask into account

	memDC.SelectObject(wxNullBitmap);

	//draw overlay
	if (isSelected() && m_imgOverlay.getImg() && m_imgOverlay->IsOk())
	{
		wxMemoryDC memDC;

		dc->SetPen(wxPen(GetBackgroundColour(),1)); 
		dc->SetBrush(wxBrush(GetBackgroundColour()));

		wxBitmap temp(m_imgOverlay->Scale(w, h));
		
		memDC.SelectObject(temp);

		dc->Blit(0, 0,			// Draw at (100, 100)
				w, h,			// Draw full bitmap
				&memDC,			// Draw from memDC
				0, 0,			// Draw from bitmap origin
				wxCOPY,			// Logical operation
				true);			// Take mask into account

		memDC.SelectObject(wxNullBitmap);	
	}
}

void StripMenuButton::paintLabel(wxDC* dc)
{
	wxSize sz = GetSize();
	wxSize txtExt = dc->GetTextExtent(GetLabel());
	
	int width = sz.GetWidth() - txtExt.GetWidth();
	int height = sz.GetHeight() - txtExt.GetHeight();


	int focOffset = 0;

	if (m_bUsesFocus)
		focOffset = 3;

	long style = GetWindowStyle();

	long x = m_uiOffset + focOffset;
	long y = height/2;

	if (style & wxALIGN_RIGHT)
		x = m_uiOffset + width - focOffset;
	else if (style & wxALIGN_CENTER)
		x = m_uiOffset + width/2;
	
	dc->DrawText(this->GetLabel(), x, y);

	if (m_bHasFocus)
	{
		dc->SetBrush(*wxTRANSPARENT_BRUSH);
		dc->SetPen(wxPen(m_colFocus));
		dc->DrawRoundedRectangle(wxPoint(x-3,0), wxSize(txtExt.GetWidth()+4, sz.GetHeight()), 2);
	}
}

void StripMenuButton::doPaint(wxDC* dc)
{
	wxFont font = GetFont();

	if (m_bBold)
		font.SetWeight(wxFONTWEIGHT_BOLD);

	dc->SetFont(font);
	dc->SetTextBackground(GetBackgroundColour());
	dc->SetTextForeground(GetForegroundColour());

	if (!m_imgBg.getImg() || !m_imgBg->IsOk())
	{
		wxBrush bg = dc->GetBackground();
		dc->SetBackground(wxBrush(this->GetBackgroundColour()));
		dc->Clear();
		dc->SetBackground(bg);
	}
	else
	{
		paintBGImage(dc);
	}

	paintLabel(dc);
}

void StripMenuButton::sendClickCommand()
{
	wxCommandEvent e(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	e.SetEventObject(this);
	HandleWindowEvent(e);
}

void StripMenuButton::onMouseDown( wxMouseEvent& event )
{
	handleEvent(event);
	event.Skip();
}

void StripMenuButton::onMouseUp( wxMouseEvent& event )
{
	if (!handleEvent(event))
		sendClickCommand();

	event.Skip();
}

void StripMenuButton::updateMouse()
{
	wxRect panelRec = GetScreenRect(); 
	panelRec.x += 2;
	panelRec.y += 2;
	panelRec.width -= 4;
	panelRec.height -= 4;

	wxPoint mousePoint = wxGetMousePosition();

	bool t1 = panelRec.x <= mousePoint.x;
	bool t2 = panelRec.y <= mousePoint.y;
	bool t3 = (panelRec.x + panelRec.width ) >= mousePoint.x;
	bool t4 = (panelRec.y + panelRec.height) >= mousePoint.y;

	if (t1 && t2 && t3 && t4)
	{
		if (!m_bHovering)
		{
			SetForegroundColour( m_colHover );
			m_bHovering = true;
			invalidatePaint();
			this->Refresh();
		}
	}
	else if (m_bHovering)
	{
		SetForegroundColour( m_colNormal );
		m_bHovering = false;
		invalidatePaint();
		this->Refresh();
	}
}

void StripMenuButton::onMouseOver( wxMouseEvent& event )
{
	updateMouse();
	event.Skip();
}

void StripMenuButton::onMouseOut( wxMouseEvent& event )
{
	updateMouse();
	event.Skip();
}

void StripMenuButton::onMouseEvent( wxMouseEvent& event )
{
	handleEvent(event);
	updateMouse();
	event.Skip();
}

void StripMenuButton::onUpdateUI( wxUpdateUIEvent& event)
{
	updateMouse();
	event.Skip();
}

bool StripMenuButton::handleEvent(wxMouseEvent& event)
{
	return false;
}

void StripMenuButton::onBlur(wxFocusEvent& event)
{
	if (!m_bHasFocus)
		return;

	m_bHasFocus = false;
	invalidatePaint();
}

void StripMenuButton::onFocus(wxFocusEvent& event)
{
	if (m_bHasFocus)
		return;

	m_bHasFocus = true;
	invalidatePaint();
}