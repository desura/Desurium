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
#include "gcImageControl.h"

#include <wx/image.h>

#ifdef WIN32
	#include "wx/msw/private.h"
#endif

#ifdef NIX
	#include "gtk/gtk.h"
#endif

#ifdef __WXGTK__
	#include "wx/gtk/private/win_gtk.h"
#elif defined(__WXMAC__)
	#include "wx/mac/private.h"
#endif

#include <wx/dcbuffer.h>

#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/dcclient.h>
#include <wx/panel.h>
#include <wx/dcmemory.h>

#include "gcManagers.h"

gcImageControl::gcImageControl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : gcFlickerFreePaint<gcPanel>( parent, id, pos, size, style )
{
	m_uiXPro = 0;
	m_uiYPro = 0;

	m_bClearBG = true;
	m_bTile = false;
	m_bTransEnabled = true;
	m_bStoreRegion = false;
}

gcImageControl::~gcImageControl()
{
}

void gcImageControl::setImage(const char* image)
{
	wxRegion r;
	SetShape(r, this);
	m_imgHandle = GetGCThemeManager()->getImageHandle(image, true);
	invalidatePaint();
}

void gcImageControl::setPropotional(uint32 x, uint32 y)
{
	m_uiXPro = x;
	m_uiYPro = y;

	invalidatePaint();
}

void gcImageControl::doPaint(wxDC* dc)
{
	wxSize sz = GetSize();
	dc->SetBackgroundMode(wxTRANSPARENT);

	if (!m_imgHandle.getImg() || !m_imgHandle->IsOk())
	{
		dc->SetTextForeground(wxColor(0,0,0));
		dc->Clear();
		return;
	}

	if (m_bClearBG)
	{
		const wxWindow *win = UseBgCol() ? this : GetParent();
		dc->SetBrush(win->GetBackgroundColour());
		dc->SetPen(*wxTRANSPARENT_PEN);
		dc->DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
	}

	if (m_uiXPro == 0 || m_uiYPro == 0)
	{
		wxBitmap temp(sz.GetWidth(), sz.GetHeight());

		if (m_bTile)
			gcImage::tileImg(temp, wxBitmap(*m_imgHandle.getImg()));
		else
			temp = wxBitmap(m_imgHandle->Scale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH));

		dc->DrawBitmap(temp, 0, 0, true);

		if (m_bTransEnabled && m_imgHandle->HasMask())
		{
			wxRegion region = wxRegion(temp);
			SetShape(region, this);

			if (m_bStoreRegion)
				m_Region = region;
		}
	}
	else
	{
		uint32 h = sz.GetWidth() * m_uiYPro / m_uiXPro;
		uint32 w = sz.GetHeight() * m_uiXPro / m_uiYPro;

		uint32 x=0;
		uint32 y=0;

		if (!m_bClearBG)
		{
			dc->SetPen(wxPen(GetBackgroundColour(),1)); 
			dc->SetBrush(wxBrush(GetBackgroundColour()));
		}

		if (h < (uint32)sz.GetHeight())
		{
			y = (sz.GetHeight()-h)/2;
			w = sz.GetWidth();

			if (!m_bClearBG)
			{
				dc->DrawRectangle(0, 0, w, y);
				dc->DrawRectangle(0, sz.GetHeight()-y, w, y);
			}
		}
		else
		{
			x = (sz.GetWidth()-w)/2;
			h = sz.GetHeight();

			if (!m_bClearBG)
			{
				dc->DrawRectangle(0, 0, x, h);
				dc->DrawRectangle(sz.GetWidth()-x, 0, x, h);
			}
		}

		wxBitmap temp(m_imgHandle->Scale(w, h, wxIMAGE_QUALITY_HIGH));
		
		wxMemoryDC memDC;
		memDC.SelectObject(temp);

		dc->Blit(x, y,			// Draw at (100, 100)
				w, h,			// Draw full bitmap
				&memDC,			// Draw from memDC
				0, 0,			// Draw from bitmap origin
				wxCOPY,			// Logical operation
				true);			// Take mask into account

		memDC.SelectObject(wxNullBitmap);
	}
}

#ifdef NIX
//for some fucked up reason getright and get bottom return one less than needed. Fixed in newer wxWidgets :(
wxBitmap ConvertRegionToBitmap(const wxRegion& r)
{
    wxRect box = r.GetBox();
    wxBitmap bmp(box.GetRight()+1, box.GetBottom()+1);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();
    dc.SetDeviceClippingRegion(*static_cast<const wxRegion *>(&r));
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SelectObject(wxNullBitmap);
    return bmp;
}
#endif

bool SetShape(const wxRegion& region, wxWindow* frame)
{
#if defined(__WXMSW__) && !defined(__WXWINCE__)
	// The empty region signifies that the shape should be removed from the
    // window.
    if ( region.IsEmpty() )
    {
        if (::SetWindowRgn((HWND)frame->GetHWND(), NULL, TRUE) == 0)
        {
            wxLogLastError(_T("SetWindowRgn"));
            return false;
        }
        return true;
    }

    DWORD noBytes = ::GetRegionData((HRGN)(region.GetHRGN()), 0, NULL);
    RGNDATA *rgnData = (RGNDATA*) new char[noBytes];
    ::GetRegionData((HRGN)(region.GetHRGN()), noBytes, rgnData);
    HRGN hrgn = ::ExtCreateRegion(NULL, noBytes, rgnData);
    delete[] (char*) rgnData;
   
    RECT rect;
    DWORD dwStyle =   ::GetWindowLong((HWND)frame->GetHWND(), GWL_STYLE);
    DWORD dwExStyle = ::GetWindowLong((HWND)frame->GetHWND(), GWL_EXSTYLE);
    ::GetClientRect((HWND)frame->GetHWND(), &rect);
    ::AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
    ::OffsetRgn(hrgn, -rect.left, -rect.top);

    if (::SetWindowRgn((HWND)frame->GetHWND(), hrgn, TRUE) == 0)
    {
        wxLogLastError(_T("SetWindowRgn"));
        return false;
    }
    return true;
    
#elif defined(__WXMAC__)
	if ( region.IsEmpty() )
    {
        wxSize sz = T->GetClientSize();
        wxRegion rgn(0, 0, sz.x, sz.y);
        return T->SetShape(rgn);
    }

    // Make a copy of the region
    RgnHandle  shapeRegion = NewRgn();
    CopyRgn( (RgnHandle)region.GetWXHRGN(), shapeRegion );

    // Dispose of any shape region we may already have
    RgnHandle oldRgn = (RgnHandle)GetWRefCon( (WindowRef)GetHandle() );
    if ( oldRgn )
        DisposeRgn(oldRgn);

    // Save the region so we can use it later
    SetWRefCon((WindowRef)GetHandle(), (SInt32)shapeRegion);

    // Tell the window manager that the window has changed shape
    ReshapeCustomWindow((WindowRef)GetHandle());
    return true;
    
#elif defined(__WXGTK__)

	GtkWidget* m_wxwindow = frame->m_wxwindow;
	GtkWidget* m_widget = frame->m_widget;

	if (region.IsEmpty())
	{
		if (m_wxwindow && GTK_WIDGET_NO_WINDOW(m_wxwindow))
			gtk_widget_shape_combine_mask(m_wxwindow, NULL, 0, 0);
			
		if (m_widget && GTK_WIDGET_NO_WINDOW(m_widget))
			gtk_widget_shape_combine_mask(m_widget, NULL, 0, 0);
	}
	else
	{	
		wxBitmap bmp = ConvertRegionToBitmap(region);
		bmp.SetMask(new wxMask(bmp, *wxBLACK));
		
		GdkBitmap* mask = bmp.GetMask()->GetBitmap();

		if (m_wxwindow)
			gtk_widget_shape_combine_mask(m_wxwindow, mask, 0, 0);
			
		if (m_widget)
			gtk_widget_shape_combine_mask(m_widget, mask, 0, 0);
	}
	
	return true;
#else
	return false;
#endif
}


