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
#include "gcUpProgBar.h"
#include "gcManagers.h"





gcULProgressBar::gcULProgressBar(wxWindow *parent, int id) : gcProgressBar(parent, id)
{
	m_uiLastMS = 0;
}

void gcULProgressBar::setMileStone()
{
	m_uiLastMS = getProgress();
	Refresh();
}

void gcULProgressBar::revertMileStone()
{
	setProgress( m_uiLastMS );
	Refresh();
}


void gcULProgressBar::doHandPaint(wxPaintDC& dc)
{
	wxSize size = GetSize();
	uint32 w = size.GetWidth();
	uint32 h = size.GetHeight()-2;

	uint32 wm = w*m_uiLastMS/100;
	uint32 wp = w*getProgress()/100;

	wxColour red(255, 0, 0);
	wxColour green(0, 255, 0);
	wxColour white(255, 255, 255);
	wxColour black(0, 0, 0);

	dc.SetPen(wxPen(green,1)); 
	dc.SetBrush(wxBrush(green));
	dc.DrawRectangle(0,0, wm, h);

	dc.SetPen(wxPen(red,1)); 
	dc.SetBrush(wxBrush(red));
	dc.DrawRectangle(wm,0, wp, h);

	dc.SetPen(wxPen(white,1)); 
	dc.SetBrush(wxBrush(white));
	dc.DrawRectangle(wp,0, w, h);
}



void gcULProgressBar::doImgPaint(wxPaintDC& dc)
{
	gcProgressBar::doImgPaint(dc);
	

	int h = GetSize().GetHeight();
	int w = GetSize().GetWidth();
#ifdef WIN32 // unused AFAIK
	int ih = getImage()->GetSize().GetHeight();
#endif
	int iw = getImage()->GetSize().GetWidth();

	uint32 wm = w*m_uiLastMS/100;
	uint32 wp = w*getProgress()/100 - wm;
	
	if (wp == 0)
		return;

	wxImage scaled = getImage()->Scale(iw, h);
	wxBitmap err = GetGCThemeManager()->getSprite(scaled, "progressbar", "Error");
	wxBitmap eedge = GetGCThemeManager()->getSprite(scaled, "progressbar", "ErrorEdge");

	wxBitmap   tmpBmp(wp, h);
	wxMemoryDC tmpDC(tmpBmp);

	tmpDC.SetBrush(wxBrush(wxColor(255,0,255)));
	tmpDC.SetPen( wxPen(wxColor(255,0,255),1) );
	tmpDC.DrawRectangle(0,0,w,h);


	uint32 eeWidth = eedge.GetWidth();

	if (wp <= eeWidth)
	{
		wxBitmap left = eedge.ConvertToImage().GetSubImage(wxRect(eeWidth-wp,0,eeWidth,h));
		tmpDC.DrawBitmap(left, 0, 0, true);
	}
	else
	{
		wxBitmap left(wp-eeWidth, h);
		wxColor c(255,0,255);
		gcImage::tileImg(left, err, &c);

		tmpDC.DrawBitmap(left, 0, 0, true);
		tmpDC.DrawBitmap(eedge, wp-eeWidth, 0, true);
	}

	tmpDC.SelectObject(wxNullBitmap);
	dc.DrawBitmap(tmpBmp, wm, 0, true);
}
