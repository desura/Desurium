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

#ifndef DESURA_COMPLEXPROMPT_H
#define DESURA_COMPLEXPROMPT_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "wx/wx.h"

class ComplexPrompt : public gcDialog 
{
public:
	ComplexPrompt( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Complex Install Conflict"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 370,180 ), long style = wxCAPTION|wxFRAME_FLOAT_ON_PARENT|wxFRAME_NO_TASKBAR|wxSYSTEM_MENU|wxTAB_TRAVERSAL );
	~ComplexPrompt();

	void setInfo(DesuraId id);

protected:
	wxStaticText* m_labInfo;

	gcButton* m_butMoreInfo;
	gcButton* m_butOk;
	gcButton* m_butCancel;

	void onButtonClick(wxCommandEvent& event);

private:
	DECLARE_EVENT_TABLE();
};

#endif //DESURA_COMPLEXPROMPT_H
