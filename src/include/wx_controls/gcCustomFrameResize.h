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

#ifndef DESURA_GCCUSTOMFRAMERESIZE_H
#define DESURA_GCCUSTOMFRAMERESIZE_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>

//! Resize handler for the custom window frame
class CustomFrameResize
{
public:
	CustomFrameResize(wxTopLevelWindow* pParent, bool enabled);

	bool onMouseMove();
	bool onMouseDown();
	bool onMouseUp();

	bool isResizing()
	{
		return (m_iResizeDir != -1);
	}

	bool canResize()
	{
		return m_bResizeEnabled;
	}

	void reset()
	{
		m_pParent->SetCursor(wxCURSOR_ARROW);
		m_MouseCursor = wxCURSOR_ARROW;
		m_iResizeDir = -1;
	}

	void calcBorders(std::vector<wxRect> &vFrameBorder);

protected:
	void processMouseMove();
	void processResize();

private:
	int32 m_MouseCursor;

	bool m_bResizeEnabled;
	int32 m_iResizeDir;

	wxTopLevelWindow* m_pParent;

	wxPoint m_StartPos;
	wxPoint m_StartResize;
	wxPoint m_WindowPos;
	wxSize m_WindowSize;

	std::vector<wxRect> m_FrameBorder;
};

#endif //DESURA_GCCUSTOMFRAMERESIZE_H
