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

#ifndef DESURA_GCCUSTOMFRAMEMOVE_H
#define DESURA_GCCUSTOMFRAMEMOVE_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>

//! Move handler for the custom window frame
class CustomFrameMove
{
public:
	CustomFrameMove(wxTopLevelWindow* pParent);

	bool onMouseMove();
	bool onMouseDown(bool canResize);
	bool onMouseUp();

	bool isDragging()
	{
		return m_bMouseDrag;
	}

	void calcBorders(wxRect title, wxRect frame);

private:
	bool m_bMouseDrag;

	wxPoint m_StartPos;
	wxPoint m_StartDrag;
	wxTopLevelWindow* m_pParent;

	wxRect m_TitleBorder;
	wxRect m_FrameBorder;
};

#endif //DESURA_GCCUSTOMFRAMEMOVE_H
