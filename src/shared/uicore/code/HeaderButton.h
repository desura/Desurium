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

#ifndef DESURA_HEADERBUTTON_H
#define DESURA_HEADERBUTTON_H
#ifdef _WIN32
#pragma once
#endif


#include "wxControls/gcControls.h"
#include "wxControls/gcImageHandle.h"

///////////////////////////////////////////////////////////////////////////////
/// Class headerButton
///////////////////////////////////////////////////////////////////////////////
class headerButton : public gcPanel 
{
public:
	headerButton( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 267,42 ), long style = wxTAB_TRAVERSAL );
	~headerButton();

	void setText(const char* text);

	void inverseSortDir();

	void setSelected(bool state);
	bool isSelected(){return m_bSelected;}

	uint8 getSortMode(){return m_iSortMode;}
	void setSortMode(uint8 mode){m_iSortMode = mode;}

	Event<int32> onClickedEvent;

	void setTextColor(wxColor& col){m_labTitle->SetForegroundColour(col);}

protected:
	gcStaticText* m_labTitle;
	gcImageControl* m_imgSortDir;
	
	void OnMouseClick( wxMouseEvent& event );
	
private:
	uint8 m_iSortMode;
	bool m_bSelected;
};

#endif //DESURA_HEADERBUTTON_H
