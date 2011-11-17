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

#ifndef DESURA_GCSCROLLEDWINDOW_H
#define DESURA_GCSCROLLEDWINDOW_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include "guiDelegate.h"

//! A window that can scroll that also is themed in the desura style with the ability to have gui events
class gcScrolledWindow : public wxGuiDelegateImplementation<wxScrolledWindow>
{
public:
	gcScrolledWindow(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);
	
	virtual void applyTheme();
};


#endif //DESURA_GCSCROLLEDWINDOW_H
