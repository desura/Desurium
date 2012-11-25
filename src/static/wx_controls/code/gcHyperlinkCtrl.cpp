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
#include "gcHyperlinkCtrl.h"
#include "gcManagers.h"

#include "wx/button.h"

gcHyperlinkCtrl::gcHyperlinkCtrl(wxWindow *parent, wxWindowID id, const wxString& label, const wxString& url, const wxPoint& pos, const wxSize& size, long style) : wxHyperlinkCtrl(parent, id, label, url, pos, size, wxHL_ALIGN_LEFT|wxHL_CONTEXTMENU|wxNO_BORDER)
{
	wxColor visited = wxColor( GetGCThemeManager()->getColor("hyperlink", "visted-fg") );
	wxColor hover = wxColor( GetGCThemeManager()->getColor("hyperlink", "hov-fg") );

	SetHoverColour( hover );
	SetNormalColour( wxColour(255, 0, 0, 0));
	SetVisitedColour( visited );

	SetBackgroundStyle(wxBG_STYLE_COLOUR);

	Bind(wxEVT_KILL_FOCUS, &gcHyperlinkCtrl::onBlur, this);
	Bind(wxEVT_SET_FOCUS, &gcHyperlinkCtrl::onFocus, this);
	Bind(wxEVT_PAINT, &gcHyperlinkCtrl::onPaint, this);
	Bind(wxEVT_CHAR, &gcHyperlinkCtrl::onChar, this);

#ifdef WIN32
	Bind(wxEVT_ERASE_BACKGROUND, &gcHyperlinkCtrl::onEraseBackGround, this);
#endif

	m_bUnderlined = true;
}

void gcHyperlinkCtrl::onEraseBackGround(wxEraseEvent& event)
{
}

void gcHyperlinkCtrl::onFocus(wxFocusEvent& event)
{
	m_NormalColor = GetNormalColour();
	SetNormalColour(GetHoverColour());
}

void gcHyperlinkCtrl::onBlur(wxFocusEvent& event)
{
	SetNormalColour(m_NormalColor);
}

void gcHyperlinkCtrl::setUnderlined(bool state)
{
	m_bUnderlined = state;
}

void gcHyperlinkCtrl::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxRect rect = GetLabelRect();
	rect.Offset(1,0);
	

	wxPaintDC dc(this);


	wxFont font = GetFont();
	font.SetUnderlined(m_bUnderlined);

	dc.SetFont(font);
	dc.SetTextForeground(GetForegroundColour());
	dc.SetTextBackground(GetBackgroundColour());

	dc.DrawText(GetLabel(), rect.GetTopLeft());

	if (HasFocus())
	{
		wxColor rectColor(GetGCThemeManager()->getColor("checkbox", "focus-fg"));

		wxRect rect = GetRect();
		rect.SetPosition(wxPoint(0,0));

		dc.SetBrush(wxBrush(*wxRED, wxTRANSPARENT));
		dc.SetPen(wxPen(rectColor));
		dc.DrawRectangle(rect);
	}
}

wxSize gcHyperlinkCtrl::DoGetBestSize() const
{
	wxSize size = wxHyperlinkCtrl::DoGetBestSize();

	size+=wxSize(2,0);
	return size;
}

void gcHyperlinkCtrl::onChar(wxKeyEvent& event)
{ 
	if (event.m_keyCode == WXK_RETURN)
	{
		SendEvent();
	}
	else
	{
		event.Skip(); 
	}
}
