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

#ifndef DESURA_WXEVENTDELEGATE_H
#define DESURA_WXEVENTDELEGATE_H
#ifdef _WIN32
#pragma once
#endif

#ifdef WIN32
	#include "MainFormCustomFrame.h"
#else
	#include "../../shared/uicore/code/MainFormCustomFrame.h"
#endif

template <class T>
class wxEventDelegate : public T
{
public:
	wxEventDelegate(wxWindow *parent, wxWindowID id, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0) : T(parent, id, label, pos, size, style)
	{
		m_pMainCustFrame = NULL;
	}

	wxEventDelegate(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0) : T(parent, id, pos, size, style)
	{
		m_pMainCustFrame = NULL;
	}

	wxEventDelegate(wxWindow *parent, const wxString& label, const wxSize& size = wxDefaultSize) : T(parent, label.c_str(), size)
	{
		m_pMainCustFrame = NULL;
	}

	virtual void regCustomFrame(gcMainCustomFrameImpl* mcf)
	{
		m_pMainCustFrame = mcf;
	}

	//! Returns true if handled
	virtual bool handleEvent(wxMouseEvent &event)
	{
		if (!m_pMainCustFrame)
			return false;

		return m_pMainCustFrame->handleEvent(this, event);
	}

private:
	gcMainCustomFrameImpl* m_pMainCustFrame;
};


template <class T>
class wxEventDelegateWrapper : public wxEventDelegate<T>
{
public:
	wxEventDelegateWrapper(wxWindow *parent, const char* image, const wxSize& size = wxDefaultSize) : wxEventDelegate<T>(parent, image, size)
	{
		initEDW();
	}


	wxEventDelegateWrapper( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0 ) : wxEventDelegate<T>(parent, id, pos, size, style)
	{
		initEDW();
	}

private:
	void onMouseEvent( wxMouseEvent& event )
	{
		wxEventDelegate<T>::handleEvent(event);
	}

	void initEDW()
	{
		this->Bind( wxEVT_LEFT_DCLICK, &wxEventDelegateWrapper<T>::onMouseEvent, this);
		this->Bind( wxEVT_MOTION, &wxEventDelegateWrapper<T>::onMouseEvent, this);
		this->Bind( wxEVT_LEFT_DOWN, &wxEventDelegateWrapper<T>::onMouseEvent, this);
		this->Bind( wxEVT_LEFT_UP, &wxEventDelegateWrapper<T>::onMouseEvent, this);
	}
};


#endif //DESURA_WXEVENTDELEGATE_H
