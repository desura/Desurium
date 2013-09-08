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
#include "gcGrid.h"

#include <wx/tooltip.h>

gcGrid::gcGrid( wxWindow *parent,wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxGrid(parent, id, pos, size, style, name)
{
	m_iLastCol = -1;
	m_iLastRow = -1;

	Bind(wxEVT_GRID_SELECT_CELL, &gcGrid::onSelSelect, this);
	GetGridWindow()->Bind(wxEVT_MOTION, &gcGrid::onMouseMotion, this);
}

gcGrid::~gcGrid()
{
	safe_delete(m_vToolTipGrid);
}

void gcGrid::CreateGrid(int numRows, int numCols, wxGrid::wxGridSelectionModes selmode)
{
	wxGrid::CreateGrid(numRows, numCols, selmode);

	if (numRows>0 && numCols>0)
	{
		safe_delete(m_vToolTipGrid);

		for (int32 x=0; x<numRows*numCols; x++)
		{
			m_vToolTipGrid.push_back(NULL);
		}
	}
}

void gcGrid::setToolTip(int32 col, int32 row, const wchar_t* text)
{
	if (col < 0 || row < 0 || col > this->GetNumberCols() || row > this->GetNumberRows())
		return;

	uint32 pos = this->GetNumberRows()*col+row;

	delete m_vToolTipGrid[pos];
	m_vToolTipGrid[pos] = new gcWString(text);
}

const wchar_t* gcGrid::getToolTip(int32 col, int32 row)
{
	if (col < 0 || row < 0 || col > this->GetNumberCols() || row > this->GetNumberRows())
		return NULL;

	uint32 pos = this->GetNumberRows()*col+row;
	return m_vToolTipGrid[pos]?m_vToolTipGrid[pos]->c_str():NULL;
}

void gcGrid::onSelSelect( wxGridEvent& event )
{ 
	showToolTip(event.GetCol(), event.GetRow());
	event.Skip(); 
}

void gcGrid::onMouseMotion( wxMouseEvent &event)
{
	wxPoint pos = this->CalcUnscrolledPosition( event.GetPosition() );
	int32 row = this->YToRow(pos.y);
	int32 col = this->XToCol(pos.x);

	showToolTip(col, row);
	event.Skip();
}


void gcGrid::showToolTip(int32 col, int32 row)
{
	if (row >= 0 && col >= 0)
	{
		const wchar_t *tool = getToolTip(col, row);

		if (tool)
		{
			wxToolTip *tip = GetGridWindow()->GetToolTip();

			if (!tip)
			{
				tip = new wxToolTip(tool);
#ifdef WIN32
				tip->SetMaxWidth(5000);
#endif
				this->GetGridWindow()->SetToolTip(tip);
			}

			this->GetGridWindow()->SetToolTip(tool);
		}
		else
		{
			this->GetGridWindow()->SetToolTip(NULL);
		}

		
	}
}
