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

#ifndef DESURA_GCDIALOG_H
#define DESURA_GCDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include <wx/frame.h>
#include "guiDelegate.h"
#include "gcCustomFrame.h"

//! Dialog with desura theme and gui event support
class gcDialog : public wxGuiDelegateImplementation<gcCustomFrame<wxDialog> >
{
public:
	gcDialog(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);
};

#endif
