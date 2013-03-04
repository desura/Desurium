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
#include "BaseToolBarControl.h"


BaseToolBarControl::BaseToolBarControl(wxWindow* parent) : gcPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1,29), wxTAB_TRAVERSAL),
	m_imgBg(GetGCThemeManager()->getImageHandle("#header_bg"))
{
	SetBackgroundColour( wxColour( 0, 113, 141 ) );

	Bind(wxEVT_ERASE_BACKGROUND, &BaseToolBarControl::onEraseBG, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &BaseToolBarControl::onButtonClick, this);
}

void BaseToolBarControl::onEraseBG(wxEraseEvent& event)
{
	if (!m_imgBg.getImg() || !m_imgBg->IsOk())
	{
		event.Skip();
	}
	else
	{
		wxDC *dc = event.GetDC();
		wxBitmap temp(m_imgBg->Scale(GetSize().GetWidth(), GetSize().GetHeight()));
		dc->DrawBitmap(temp, 0,0, false);
	}
}

void BaseToolBarControl::onButtonClick(wxCommandEvent &event)
{
	int32 id = event.GetId();
	onButtonClickedEvent(id);
}
