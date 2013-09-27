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

class DStripMenuButton : public wxEventDelegate<StripMenuButton>
{
public:
	DStripMenuButton(wxWindow *parent, const char* label, wxSize size = wxDefaultSize);
	void setActive(bool state);

	virtual bool handleEvent(wxMouseEvent& event);

private:
	wxRect m_IgnoreRect;
	bool m_bMouseDown;

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
