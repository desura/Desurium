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

#ifndef DESURA_GCCUSTOMFRAME_H
#define DESURA_GCCUSTOMFRAME_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include "gcManagers.h"

extern wxWindow* GetMainWindow(wxWindow* p);

class FrameIcon
{
public:
	virtual wxIcon getFrameIcon()=0;
};

//! Custom window frame that allows for self painting of borders and title bar
template <typename T>
class gcCustomFrame : public T, public FrameIcon
{
public:
	gcCustomFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) 
		: T()
		, m_uiTitleHeight(0)
	{	
		this->Create(parent, id, title, pos, size + wxSize(16, 38), style);
		
#ifdef WIN32
		SetIcon(wxIcon(wxICON(IDI_ICONNORMAL)));
		m_FrameIcon = wxIcon("IDI_ICONNORMAL",wxBITMAP_TYPE_ICO_RESOURCE,16,16);
#else
		m_uiTitleHeight = 0;
		gcWString path(L"{0}/desura.png", UTIL::OS::getDataPath());
		wxIcon i(path, wxBITMAP_TYPE_PNG);
		SetIcon(i);
		wxSize s = this->GetSize() - this->GetClientSize();
		m_uiTitleHeight = s.GetHeight();

		if (m_uiTitleHeight < 0)
			m_uiTitleHeight = 0;
#endif

		this->Layout();
		this->Refresh(true);
	}

	virtual void loadFrame(long style)
	{
		applyTheme();
	}

	virtual void DoSetSize(int x, int y, int width, int height, int sizeFlags = wxSIZE_AUTO)
	{
		if (height != -1)
			height += m_uiTitleHeight;

		T::DoSetSize(x, y, width, height, sizeFlags);
	}

	virtual wxSize DoGetBestSize() const
	{
		wxSize size = T::DoGetBestSize();
		size += wxSize(0, m_uiTitleHeight);
		return size;
	}

	virtual void DoGetSize(int *width, int *height) const
	{
		T::DoGetSize(width, height);
		
		if (height)
		{
			*height -= m_uiTitleHeight;
			
			if (*height < 0)
				*height = 0;
		}
	}

	virtual wxSize GetFullSize()
	{
		int w, h;
		T::DoGetSize(&w, &h);
		return wxSize(w,h);
	}

	void setIcon(const char* path)
	{
		if (!path)
			return;

		if (!UTIL::FS::isValidFile(UTIL::FS::PathWithFile(path)))
			return;

		gcWString wPath(path);
		wxImage img(wPath.c_str());

		if (!img.IsOk())
			return;

		m_FrameIcon.CopyFromBitmap(wxBitmap(img.Scale(16,16))); 

		wxIcon FrameIcon;
		FrameIcon.CopyFromBitmap(wxBitmap(img));
		SetIcon(FrameIcon);
	}
	
	virtual void SetIcon(const wxIcon& icon)
	{
		T::SetIcon(icon);
	}

	wxIcon getFrameIcon()
	{
		if (m_FrameIcon.IsOk())
			return m_FrameIcon;

		return T::GetIcon();
	}

	void setFrameIcon(wxIcon icon)
	{
		m_FrameIcon = icon;
	}

	void applyTheme()
	{
		Managers::LoadTheme(this, "formdefault");
	}

	void centerOnParent()
	{
		wxWindow* p = T::GetParent();

		p = GetMainWindow(p);

		if (!p)
			return;

		wxSize ps = p->GetSize();
		wxPoint pp = p->GetPosition();

		wxSize mid = ps/2+wxSize(pp.x, pp.y);

		wxSize ts = this->GetSize();
		ts = ts/2;

		wxSize fin = mid - ts;

		this->SetPosition(wxPoint(fin.x, fin.y));
	}

	bool  setSavedWindowPos(int x, int y, int w, int h)
	{
		if (UTIL::OS::isPointOnScreen(x+8, y+30))
		{
			T::SetSize(x, y, w, h);
			return true;
		}
		T::SetSize(-1, -1, w, h);
		return false;
	}

	Event<bool> onActiveEvent;

private:
	uint32 m_uiTitleHeight;
	wxIcon m_FrameIcon;
};



#endif //DESURA_GCCUSTOMFRAME_H

