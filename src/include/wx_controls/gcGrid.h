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

#ifndef DESURA_GCGRID_H
#define DESURA_GCGRID_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/grid.h>


class gcGrid : public wxGrid
{
public:
	gcGrid( wxWindow *parent,wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxGridNameStr );
	~gcGrid();

	virtual void CreateGrid(int x, int y, wxGrid::wxGridSelectionModes selmode = wxGrid::wxGridSelectCells);

	void setToolTip(int32 col, int32 row, const wchar_t* text);
	const wchar_t* getToolTip(int32 col, int32 row);

protected:
	void onSelSelect( wxGridEvent& event );
	void onMouseMotion( wxMouseEvent &event);

	void showToolTip(int32 col, int32 row);
		
private:
	int32 m_iLastCol;
	int32 m_iLastRow;

	std::vector<gcWString*> m_vToolTipGrid;
};

#endif
