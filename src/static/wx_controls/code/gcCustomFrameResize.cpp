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
#include "gcCustomFrameResize.h"
#include "gcCustomFrameImpl.h"

CustomFrameResize::CustomFrameResize(wxTopLevelWindow* pParent, bool enabled)
{
	m_pParent = pParent;
	m_bResizeEnabled = enabled;

	m_MouseCursor = wxCURSOR_ARROW;
	m_iResizeDir = -1;
}

void CustomFrameResize::processMouseMove()
{
	wxPoint pos = wxGetMousePosition()-m_pParent->GetPosition();

	bool stillIn = false;
	for (size_t x=0; x<m_FrameBorder.size(); x++)
	{
		if (!m_FrameBorder[x].Contains(pos))
			continue;

		switch (x)
		{
		case NORTH:
		case SOUTH:
			m_MouseCursor = wxCURSOR_SIZENS;
			break;

		case NEAST:
		case SWEST:
			m_MouseCursor = wxCURSOR_SIZENESW;
			break;

		case EAST:
		case WEST:
			m_MouseCursor = wxCURSOR_SIZEWE;
			break;

		case SEAST:
		case NWEST:
			m_MouseCursor = wxCURSOR_SIZENWSE;
			break;
		}

		stillIn = true;
		break;
	}

	if (!stillIn)
	{
		m_pParent->SetCursor(wxCURSOR_ARROW);
		m_MouseCursor = wxCURSOR_ARROW;

		if (m_pParent->HasCapture())
			m_pParent->ReleaseMouse();
	}
	else
	{
		if (!m_pParent->HasCapture())
			m_pParent->CaptureMouse();

		m_pParent->SetCursor(m_MouseCursor);
	}
}

void CustomFrameResize::processResize()
{
	wxPoint pos = wxGetMousePosition()-m_pParent->GetPosition();

	wxPoint windowPos = m_WindowPos;
	wxSize windowSize = m_WindowSize;

	wxPoint winPosDiff = m_WindowPos - m_pParent->GetPosition();
	wxPoint diff = pos - m_StartResize - winPosDiff;

	switch (m_iResizeDir)
	{
	case NORTH:
	case SOUTH:
		diff.x=0;
		break;

	case EAST:
	case WEST:
		diff.y=0;
		break;
	}

	wxSize minSize = m_pParent->GetMinSize();

	if (m_iResizeDir == WEST || m_iResizeDir == SWEST || m_iResizeDir == NWEST)
	{
		if (windowSize.x + diff.x*-1 < minSize.x)
			diff.x = windowSize.x - minSize.x;

		windowPos.x += diff.x;
		diff.x *= -1;
	}

	if (m_iResizeDir == NWEST || m_iResizeDir == NORTH || m_iResizeDir == NEAST)
	{
		if (windowSize.y + diff.y*-1 < minSize.y)
			diff.y = windowSize.y - minSize.y;

		windowPos.y += diff.y;
		diff.y *= -1;
	}

	windowSize.x += diff.x;
	windowSize.y += diff.y;

	if (windowSize.x < minSize.x)
		windowSize.x = minSize.x;

	if (windowSize.y < minSize.y)
		windowSize.y = minSize.y;

#ifdef WIN32
	::MoveWindow((HWND)m_pParent->GetHWND(), windowPos.x, windowPos.y, windowSize.x, windowSize.y,false);
	m_pParent->Refresh(false);
	::RedrawWindow((HWND)m_pParent->GetHWND(), NULL, NULL, RDW_FRAME|RDW_INVALIDATE|RDW_NOERASE|RDW_ERASENOW);
#endif
	//m_pParent->SetSize(wxRect(windowPos, windowSize));
}

bool CustomFrameResize::onMouseMove()
{
	if (!m_bResizeEnabled)
		return false;

	if (m_iResizeDir != -1)
		processResize();
	else
		processMouseMove();

	return true;
}

bool CustomFrameResize::onMouseDown()
{
	if (!m_bResizeEnabled || m_iResizeDir != -1)
		return false;

	wxPoint pos = wxGetMousePosition()-m_pParent->GetPosition();

	for (size_t x=0; x<m_FrameBorder.size(); x++)
	{
		if (m_FrameBorder[x].Contains(pos))
		{	
			m_iResizeDir = x;
			break;
		}
	}

	if (m_iResizeDir != -1)
	{
		m_StartResize = pos;
		m_StartPos = wxGetMousePosition();

		m_WindowPos = m_pParent->GetPosition();
		m_WindowSize = m_pParent->GetSize();

		m_pParent->CaptureMouse();
	}

	return true;
}

bool CustomFrameResize::onMouseUp()
{
	if (!m_bResizeEnabled || m_iResizeDir == -1)
		return false;

	m_iResizeDir = -1;

	//post an event so that the form knows its final size
	wxSizeEvent se;
	se.SetEventObject(m_pParent);
	m_pParent->GetEventHandler()->AddPendingEvent(se);

	if (m_pParent->HasCapture())
		m_pParent->ReleaseMouse();


	return (m_StartPos == wxGetMousePosition());
}

void CustomFrameResize::calcBorders(std::vector<wxRect> &vFrameBorder)
{
	m_FrameBorder = vFrameBorder;
}
