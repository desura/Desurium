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


#ifndef DESURA_GCCHOICE_H
#define DESURA_GCCHOICE_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/choice.h>

//! Choice box with desura theme
class gcChoice : public wxChoice
{
public:
	gcChoice(wxWindow *parent, 
			wxWindowID id = wxID_ANY, 
			const wxPoint& pos = wxDefaultPosition, 
			const wxSize& size = wxDefaultSize, 
			int n = 0, 
			const wxString choices[] = NULL, 
			long style = 0);

    gcChoice(wxWindow *parent,
             wxWindowID id,
             const wxPoint& pos,
             const wxSize& size,
             const wxArrayString& choices,
             long style = 0);

	void applyTheme();
};

#endif //DESURA_GCCHOICE_H
