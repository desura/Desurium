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
#include "DStripMenuControls.h"
#include "wx_controls/gcManagers.h"
#include "managers/CVar.h"

CVar gc_buttontol("gc_butMouseTol", "16", CFLAG_USER);

DStripMenuButton::DStripMenuButton(wxWindow *parent, const char* label, wxSize size) 
	: OverrideBorderMouseClick(parent, gcWString(label).c_str(), size)
{
	init("#menu_bg", "#menu_overlay");

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
