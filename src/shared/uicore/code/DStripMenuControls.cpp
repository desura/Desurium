/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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
#include "DStripMenuControls.h"
#include "wx_controls/gcManagers.h"
#include "managers/CVar.h"

CVar gc_buttontol("gc_butMouseTol", "16", CFLAG_USER);

DStripMenuButton::DStripMenuButton(wxWindow *parent, const char* label, wxSize size) : wxEventDelegate<StripMenuButton>(parent, gcWString(label).c_str(), size)
{
	init("#menu_bg", "#menu_overlay");
	m_bMouseDown = false;

	m_NormColor = this->GetForegroundColour();
	m_NonActiveColor = wxColor(GetGCThemeManager()->getColor("label", "na-fg"));
}

void DStripMenuButton::setActive(bool state)
{
	setNormalCol(state?m_NormColor:m_NonActiveColor);

	if (!isSelected())
		m_imgBg = GetGCThemeManager()->getImageHandle(state?"#menu_bg":"#menu_bg_nonactive");

	this->invalidatePaint();
}

bool DStripMenuButton::handleEvent(wxMouseEvent& event)
{
	wxPoint mouse = wxGetMousePosition()-GetScreenPosition();

	if (event.GetEventType() == wxEVT_MOTION)
	{
		if (m_bMouseDown)
		{
			if (!m_IgnoreRect.Contains(mouse))
			{
				wxMouseEvent fakeEvent;
				fakeEvent.SetEventType(wxEVT_LEFT_DOWN);
				fakeEvent.SetEventObject(event.GetEventObject());

				wxEventDelegate<StripMenuButton>::handleEvent(fakeEvent);
				wxEventDelegate<StripMenuButton>::handleEvent(event);
				m_bMouseDown = false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return wxEventDelegate<StripMenuButton>::handleEvent(event);
		}
	}
	else if (event.GetEventType() == wxEVT_LEFT_DOWN)
	{
		uint32 is = gc_buttontol.getInt(); 

		m_bMouseDown = true;

		wxPoint startPos = mouse;
		startPos.x -= is/2;
		startPos.y -= is/2;

		wxRect mouseRect(startPos, wxSize(is,is));

		//need to take 6 of for drag border
		wxSize size = GetSize();
		size.y -= 6;
		wxRect curRect(wxPoint(0,6), size);

		m_IgnoreRect = curRect.Intersect(mouseRect);

		return true;
	}
	else if (event.GetEventType() == wxEVT_LEFT_UP)
	{
		if (!m_bMouseDown)
		{
			return wxEventDelegate<StripMenuButton>::handleEvent(event);
		}
		else
		{
			m_bMouseDown = false;
			return false;
		}
	}

	return wxEventDelegate<StripMenuButton>::handleEvent(event);
}
