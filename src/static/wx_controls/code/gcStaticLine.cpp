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
#include "gcStaticLine.h"
#include "gcManagers.h"
#include "gcImage.h"

#include "gcImageControl.h"

gcStaticLine::gcStaticLine(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) : gcPanel(parent, id, pos, size, style)
{
	Connect( wxEVT_PAINT, wxPaintEventHandler( gcStaticLine::onPaint ));
	Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( gcStaticLine::onEraseBG ) );

	applyTheme();

	m_imgHR = GetGCThemeManager()->getImageHandle("#horizontal_rule");
}

gcStaticLine::~gcStaticLine()
{
}

void gcStaticLine::applyTheme()
{
	Managers::LoadTheme(this, "default");
}

void gcStaticLine::onPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);

	if (!m_imgHR.getImg() || !m_imgHR->IsOk())
	{
		dc.SetTextForeground(wxColor(25,25,25));
		dc.Clear();
		return;
	}

	int h = GetSize().GetHeight();
	int w = GetSize().GetWidth();
#ifdef WIN32 // unused AFAIK
	int ih = m_imgHR->GetSize().GetHeight();
#endif 
	int iw = m_imgHR->GetSize().GetWidth();

	wxBitmap   tmpBmp(w, h);
	wxMemoryDC tmpDC(tmpBmp);

	tmpDC.SetBrush(wxBrush(wxColor(255,0,255)));
	tmpDC.SetPen( wxPen(wxColor(255,0,255),1) );
	tmpDC.DrawRectangle(0,0,w,h);

	wxImage scaled = m_imgHR->Scale(iw, h);

	wxBitmap left = GetGCThemeManager()->getSprite(scaled, "horizontal_rule", "Left");
	wxBitmap right = GetGCThemeManager()->getSprite(scaled, "horizontal_rule", "Right");
	wxBitmap centerImg = GetGCThemeManager()->getSprite(scaled, "horizontal_rule", "Center");

	wxBitmap center(w-(left.GetWidth()+right.GetWidth()),h);

	wxColor c(255,0,255);
	gcImage::tileImg(center, centerImg, &c);

	tmpDC.DrawBitmap(left, 0,0,true);
	tmpDC.DrawBitmap(center, left.GetWidth(),0,true);
	tmpDC.DrawBitmap(right, w-right.GetWidth(),0,true);

	tmpDC.SelectObject(wxNullBitmap);
	dc.DrawBitmap(tmpBmp, 0,0, true);

	wxRegion region = wxRegion(tmpBmp, wxColor(255,0,255), 1);
	SetShape(region, this);
}

void gcStaticLine::onEraseBG( wxEraseEvent& event )
{

}
