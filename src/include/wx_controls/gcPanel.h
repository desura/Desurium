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

#ifndef DESURA_GCPANEL_H
#define DESURA_GCPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include "guiDelegate.h"

//! Desura themed panel that can have gui events.
class gcPanel : public wxGuiDelegateImplementation<wxPanel>
{
public:
	gcPanel(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE, bool loadTheme = true);
	
	virtual void applyTheme();
	
	//! Sets the parent frame to be this size which takes into consideration borders and such
	//!
	void setParentSize(int width, int height);
};

#endif //DESURA_GCPANEL_H
