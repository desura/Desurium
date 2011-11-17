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
#include "gcCustomFrameMove.h"
#include "gcCustomFrameImpl.h"

CustomFrameMove::CustomFrameMove(wxTopLevelWindow* pParent)
{
	m_bMouseDrag = false;
	m_pParent = pParent;
}


bool CustomFrameMove::onMouseUp()
{
	if (!m_bMouseDrag)
		return false;

	m_bMouseDrag = false;

	if (m_pParent->HasCapture())
		m_pParent->ReleaseMouse();
		
	return true;
}

bool CustomFrameMove::onMouseDown(bool canResize)
{
	wxPoint pos = wxGetMousePosition()-m_pParent->GetScreenPosition();

	if (m_TitleBorder.Contains(pos) || (!canResize && m_FrameBorder.Contains(pos)))
	{
		m_bMouseDrag = true;
		m_StartDrag = pos;
		m_StartPos = wxGetMousePosition();

		if (!m_pParent->HasCapture())
			m_pParent->CaptureMouse();

		return true;
	}

	return false;
}

bool CustomFrameMove::onMouseMove()
{
	if (!m_bMouseDrag)
		return false;

	wxPoint oPos = wxGetMousePosition()-m_pParent->GetScreenPosition();

	if (oPos == m_StartDrag)
		return false;

	wxPoint pos = wxGetMousePosition();

	if (m_pParent->IsMaximized())
	{
		m_pParent->Maximize(false);
		wxRect rect = m_pParent->GetRect();

		m_StartDrag.x = rect.width/2;
	}
	
	m_pParent->SetPosition(pos-m_StartDrag);
	return true;
}

void CustomFrameMove::calcBorders(wxRect title, wxRect frame)
{
	m_TitleBorder = title;
	m_FrameBorder = frame;
}