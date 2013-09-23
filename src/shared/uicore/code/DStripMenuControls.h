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

#ifndef DESURA_DSTRIPMENUCONTROLS_H
#define DESURA_DSTRIPMENUCONTROLS_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "wx_controls/wxEventDelegate.h"
#include "StripMenuButton.h"
#include "MenuSeperator.h"


extern CVar gc_buttontol;

template <typename T, typename K = wxEventDelegateWrapper<T>>
class OverrideBorderMouseClick : public K
{
public:
	OverrideBorderMouseClick(wxWindow *parent, wxWindowID id, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0) 
		: K(parent, id, label, pos, size, style)
	{
		m_bMouseDown = false;
	}

	OverrideBorderMouseClick(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0) 
		: K(parent, id, pos, size, style)
	{
		m_bMouseDown = false;
	}

	OverrideBorderMouseClick(wxWindow *parent, const wxString& label, const wxSize& size = wxDefaultSize) 
		: K(parent, label.c_str(), size)
	{
		m_bMouseDown = false;
	}

	void onMouseEvent(wxMouseEvent& event)
	{
		handleEvent(event);
	}

	virtual bool handleEvent(wxMouseEvent& event)
	{
		wxPoint mouse = wxGetMousePosition() - GetScreenPosition();

		if (event.GetEventType() == wxEVT_MOTION)
		{
			if (m_bMouseDown && !m_IgnoreRect.Contains(mouse))
			{
				wxMouseEvent fakeEvent;
				fakeEvent.SetEventType(wxEVT_LEFT_DOWN);
				fakeEvent.SetEventObject(event.GetEventObject());

				wxEventDelegate<T>::handleEvent(fakeEvent);
				wxEventDelegate<T>::handleEvent(event);
				m_bMouseDown = false;
			}
		}
		else if (event.GetEventType() == wxEVT_LEFT_DOWN)
		{
			uint32 is = gc_buttontol.getInt();

			m_bMouseDown = true;

			wxPoint startPos = mouse;
			startPos.x -= is / 2;
			startPos.y -= is / 2;

			wxRect mouseRect(startPos, wxSize(is, is));

			//need to take 6 of for drag border
			wxSize size = GetSize();
			size.y -= 6;
			wxRect curRect(wxPoint(0, 6), size);

			m_IgnoreRect = curRect.Intersect(mouseRect);

			event.Skip();
			return true;
		}
		else if (event.GetEventType() == wxEVT_LEFT_UP)
		{
			bool res = wxEventDelegate<T>::handleEvent(event);

			if (m_bMouseDown)
				event.Skip();

			m_bMouseDown = false;
			return res;
		}

		return wxEventDelegate<T>::handleEvent(event);
	}

private:
	wxRect m_IgnoreRect;
	bool m_bMouseDown;
};


class DStripMenuButton : public OverrideBorderMouseClick < StripMenuButton, wxEventDelegate<StripMenuButton>>
{
public:
	DStripMenuButton(wxWindow *parent, const char* label, wxSize size = wxDefaultSize);
	void setActive(bool state);

private:
	wxColor m_NormColor;
	wxColor m_NonActiveColor;
};

class DMenuSeperator : public wxEventDelegateWrapper<MenuSeperator>
{
public:
	DMenuSeperator(wxWindow *parent, const wxSize &size = wxDefaultSize) : wxEventDelegateWrapper<MenuSeperator>(parent, "#menu_separator", size)
	{
	}

	void setActive(bool state)
	{
		if (!isSelected())
			m_imgBG->setImage(state?"#menu_separator":"#menu_separator_nonactive");
	}
};

#endif //DESURA_DSTRIPMENUCONTROLS_H
