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
#include "DispLoading.h"


DispLoading::DispLoading(wxWindow *parent, wxString text, const wxSize &size) : wxStaticText( parent, wxID_ANY, text)
{
	SetSize(size);
	SetMinSize(size);

	wxSize baseSize = wxStaticText::GetSize();
	baseSize.x += 25;
	SetMinSize(baseSize);

	m_imgHandle = GetGCThemeManager()->getImageHandle("#browser_loading");
#ifdef WIN32
	m_imgAnimation = new wxAnimationCtrl( this, wxID_ANY, wxAnimation(), wxPoint(2,0), wxSize( 17,17 ) );

	const char* spinImg = GetGCThemeManager()->getImage("browser_loading_spinner");

	if (spinImg && UTIL::FS::isValidFile(spinImg))
		m_imgAnimation->SetAnimation(wxAnimation(spinImg));
#endif
	Connect( wxEVT_PAINT, wxPaintEventHandler( DispLoading::onPaint ));
	Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( DispLoading::onEraseBG ), NULL, this);
	Connect( wxEVT_IDLE, wxIdleEventHandler( DispLoading::onIdle ) );
}

DispLoading::~DispLoading()
{
	
}

void DispLoading::onIdle(wxIdleEvent& event)
{
#ifdef WIN32
	if (!m_imgAnimation->IsPlaying())
		m_imgAnimation->Play();
#endif
	Disconnect( wxEVT_IDLE, wxIdleEventHandler( DispLoading::onIdle ) );
}

void DispLoading::onPaint( wxPaintEvent& event )
{
	wxPaintDC dc(this);
	dc.SetFont(GetFont());
	dc.SetTextForeground(GetForegroundColour());
	//dc.SetTextBackground(GetBackgroundColour());

	wxSize size = GetSize();

	long x=22;
	long y=(size.GetHeight() - dc.GetTextExtent(GetLabel()).GetHeight())/2;;

	dc.DrawText(this->GetLabel(), x, y);
}

void DispLoading::onEraseBG( wxEraseEvent& event )
{
	wxDC *dc = event.GetDC();

	if (!m_imgHandle.getImg() || !m_imgHandle->IsOk())
	{
		dc->SetTextForeground(wxColor(0,0,0));
		dc->Clear();
		return;
	}

	int h = GetSize().GetHeight();
	int w = GetSize().GetWidth();
	int iw = m_imgHandle->GetSize().GetWidth();


	//size 7
	const int l_start = 0;
	const int l_width = 2;

	//size 6
	const int m_start = 2;
	const int m_width = 15;

	//size 7 with 1 pix gap
	const int r_start = 17;
	const int r_width = 2;


	wxBitmap   tmpBmp(w, h);
	wxMemoryDC tmpDC(tmpBmp);

	tmpDC.SetBrush(wxBrush(wxColor(255,0,255)));
	tmpDC.SetPen( wxPen(wxColor(255,0,255),1) );
	tmpDC.DrawRectangle(0,0,w,h);

	wxImage scaled = m_imgHandle->Scale(iw, h);

	if (scaled.IsOk())
	{
		wxBitmap left = wxBitmap(scaled.GetSubImage( wxRect(l_start,0,l_width,h)) );
		wxBitmap right = wxBitmap(scaled.GetSubImage( wxRect(r_start,0,r_width,h)) );
		wxBitmap center(w-(l_width+r_width),h);

		wxColor c(255,0,255);
		gcImage::tileImg(center, wxBitmap(scaled.GetSubImage( wxRect(m_start,0,m_width,h)) ), &c);

		tmpDC.DrawBitmap(left, l_start,0,true);
		tmpDC.DrawBitmap(center, m_start,0,true);
		tmpDC.DrawBitmap(right, w-r_width,0,true);
	}

	tmpDC.SelectObject(wxNullBitmap);
	dc->DrawBitmap(tmpBmp, 0,0, true);

	wxRegion region = wxRegion(tmpBmp, wxColor(255,0,255), 1);
	SetShape(region, this);
}
