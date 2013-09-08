/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_GCFLICKERFREEPAINT_H
#define DESURA_GCFLICKERFREEPAINT_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#ifdef NIX
#include <gtk/gtk.h>
#endif

//! Enables controls to paint them selfs with out flicker
template <class T>
class gcFlickerFreePaint : public T
{
public:
	// gcStaticText
	gcFlickerFreePaint(wxWindow* parent, wxWindowID id, wxString text, const wxPoint& pos, const wxSize& size, long style)
		: T(parent, id, text, pos, size, style)
	{
		init();
	}

	// gcImageControl
	gcFlickerFreePaint(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
		: T(parent, id, pos, size, style)
	{
		init();
	}

	// gcHyperlinkCtrl 
	gcFlickerFreePaint(wxWindow *parent, wxWindowID id, const wxString& label, const wxString& url, const wxPoint& pos, const wxSize& size, long style) 
		: T(parent, id, label, url, pos, size, style)
	{
		init();
	}

	// StripMenuButton
	gcFlickerFreePaint(wxWindow *parent, wxString text, const char* image, const char* overlay, const wxSize &size, int32 spacing)
		: T(parent, text, image, overlay, size, spacing)
	{
		init();
	}

	gcFlickerFreePaint(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) : T(parent, id, pos, size)
	{
		init();
	}

	gcFlickerFreePaint(wxWindow *parent)
		: T(parent)
	{
		init();
	}

	gcFlickerFreePaint()
		: T()
	{
		init();
	}

	//! Force redraw
	virtual void invalidatePaint()
	{
		m_bValidPaintBMP = false;
		T::Refresh(false);
	}

protected:
	void init()
	{
#ifdef NIX
		if(T::m_widget == NULL)
			return;
#endif
		this->Bind( wxEVT_SIZE, &gcFlickerFreePaint::onResize, this);
		this->Bind( wxEVT_PAINT, &gcFlickerFreePaint::onPaint, this);
		this->Bind( wxEVT_ERASE_BACKGROUND, &gcFlickerFreePaint::onPaintBG, this);

		m_PaintBMP = wxBitmap(T::GetSize());
		m_bValidPaintBMP = false;
	}

	void init(wxWindow* parent, wxString label, wxSize size, wxPoint pos, int flags)
	{
		T::Init(parent, label, size, pos, flags);

#ifdef NIX
		init();
#endif
	}

	virtual void doPaint(wxDC* dc)=0;
	virtual void doResize( wxSizeEvent& event ){}
	virtual void doPaintBG(wxDC* dc){}

	void onResize( wxSizeEvent& event )
	{
		doResize(event);

		if (T::GetSize() != m_PaintBMP.GetSize())
		{
			m_PaintBMP = wxBitmap(T::GetSize());
			m_bValidPaintBMP = false;
		}
	}

	void onPaintBG( wxEraseEvent& event )
	{
		doPaintBG(event.GetDC());

		if (m_bValidPaintBMP)
			event.GetDC()->DrawBitmap(m_PaintBMP, wxPoint(0,0), true);
	}

	void onPaint( wxPaintEvent& event )
	{
		wxPaintDC dc(this);

		if (!m_bValidPaintBMP)
		{
			wxMemoryDC tmpDC(m_PaintBMP);
			doPaint(&tmpDC);
			tmpDC.SelectObject(wxNullBitmap);
			m_bValidPaintBMP = true;
		}

		if (dc.IsOk())
			dc.DrawBitmap(m_PaintBMP, wxPoint(0,0), true);
	}

	wxBitmap m_PaintBMP;
	bool m_bValidPaintBMP;
};

#endif //DESURA_GCFLICKERFREEPAINT_H
