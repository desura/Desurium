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
#include "gcCustomFrameImpl.h"
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
	gcCustomFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : T(), m_pCustomFrame(NULL)
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

	~gcCustomFrame()
	{
		delete m_pCustomFrame;
	}

	virtual bool Destroy()
	{
#ifdef WIN32
		if (m_pCustomFrame)
		{
			m_pCustomFrame->destroy();
			m_pCustomFrame->onActiveEvent -= delegate(&onActiveEvent);
		}
#endif

		return T::Destroy();
	}

	void init(gcCustomFrameImpl* custFrame)
	{
#ifdef WIN32
		m_pCustomFrame = custFrame;

		if (m_pCustomFrame)
			m_pCustomFrame->onActiveEvent += delegate(&onActiveEvent);
			
#else
		if (custFrame)
			custFrame->destroy();
#endif
		this->Layout();
	}

	virtual void loadFrame(long style)
	{
		applyTheme();
#ifdef WIN32
		if (!getCustomFrame())
			init(new gcCustomFrameImpl(this, this, style));
#endif
	}
	
	gcCustomFrameImpl* getCustomFrame()
	{
		return m_pCustomFrame;
	}

	virtual void DoGetClientSize(int *width, int *height) const
	{
#ifdef WIN32
		if (m_pCustomFrame)
			m_pCustomFrame->DoGetClientSize(width, height);
		else
#endif
			T::DoGetClientSize(width, height);
	}

	virtual wxPoint GetClientAreaOrigin() const
	{
#ifdef WIN32
		if (m_pCustomFrame)
			return m_pCustomFrame->GetClientAreaOrigin();
#endif
		return T::GetClientAreaOrigin();
	}

#ifdef WIN32
	virtual WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
	{
		bool processed = false;
		WXLRESULT res = 0;

		if (m_pCustomFrame)
			res = m_pCustomFrame->PreMSWWindowProc(message, wParam, lParam, processed);

		if (processed)
			return res;

		res = T::MSWWindowProc(message, wParam, lParam);

		if (!m_pCustomFrame)
			return res;

		return m_pCustomFrame->PostMSWWindowProc(message, wParam, lParam, res);
	}
#endif

	virtual void SetTitle(const wxString &title)
	{
#ifdef WIN32
		if (m_pCustomFrame)
			m_pCustomFrame->invalidatePaint();
#endif
		return T::SetTitle(title);
	}

	virtual bool IsMaximized() const
	{
#ifdef WIN32
		if (m_pCustomFrame)
			return m_pCustomFrame->isMaximized();
#endif
		return T::IsMaximized();
	}

	virtual void Maximize(bool state)
	{
#ifdef WIN32
		if (m_pCustomFrame)
			m_pCustomFrame->maximize(state);
		else
#endif
			T::Maximize(state);
	}

	virtual void Iconize(bool state)
	{
#ifdef WIN32
		if (m_pCustomFrame)
			m_pCustomFrame->minimize(state);
#endif
		T::Iconize(state);
	}

	virtual void Restore()
	{
#ifdef WIN32
		if (m_pCustomFrame)
			m_pCustomFrame->restore();
#endif
		T::Restore();
	}

	virtual void DoSetSize(int x, int y, int width, int height, int sizeFlags = wxSIZE_AUTO)
	{
#ifdef WIN32
		if (m_pCustomFrame)
		{
			if (width != -1)
				width += 16;

			if (height != -1)
				height += 38;
		}
#else
		if (height != -1)
			height += m_uiTitleHeight;
#endif

		T::DoSetSize(x, y, width, height, sizeFlags);
	}

	virtual wxSize DoGetBestSize() const
	{
		wxSize size = T::DoGetBestSize();
#ifdef WIN32
		if (m_pCustomFrame)
			size += wxSize(16,38);
#else
		size += wxSize(0, m_uiTitleHeight);
#endif
		return size;
	}

	virtual void DoGetSize(int *width, int *height) const
	{
		T::DoGetSize(width, height);
		
#ifdef WIN32
		//Due to a bug with pointers this never worked and since it does now it messes with things
		//if (m_pCustomFrame)
		//{
		//	if (width)
		//		*width -= 16;
		//		
		//	if (height)
		//		*height -= 38;
		//}
#else
		if (height)
		{
			*height -= m_uiTitleHeight;
			
			if (*height < 0)
				*height = 0;
		}
#endif
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

		if (m_pCustomFrame)
			m_pCustomFrame->invalidatePaint();
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

	bool isResizing()
	{
		if (!m_pCustomFrame)
			return false;

		return m_pCustomFrame->isResizing();
	}

	bool isActive()
	{
		if (m_pCustomFrame)
			return m_pCustomFrame->isActive();

		return true;
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
#if defined NIX || MACOS
	uint32 m_uiTitleHeight;
#endif

	gcCustomFrameImpl* m_pCustomFrame;
	wxIcon m_FrameIcon;
};



#endif //DESURA_GCCUSTOMFRAME_H

