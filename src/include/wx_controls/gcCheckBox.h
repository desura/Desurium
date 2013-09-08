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


#ifndef DESURA_GCCHECKBOX_H
#define DESURA_GCCHECKBOX_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/checkbox.h>

//! Check box with desura theme
class gcCheckBox : public wxCheckBox
{
public:
	gcCheckBox(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& label = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	
	void applyTheme();
	
#ifdef WIN32
	bool MSWOnDraw(WXDRAWITEMSTRUCT *item);
#endif
};

#endif //DESURA_GCCHECKBOX_H
