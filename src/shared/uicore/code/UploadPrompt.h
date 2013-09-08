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


#ifndef DESURA_UPLOADPROMPT_H
#define DESURA_UPLOADPROMPT_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

class gcUploadPrompt : public gcDialog 
{
public:
	gcUploadPrompt( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Upload Reminder"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 285,161 ), long style = wxCAPTION|wxTAB_TRAVERSAL );
	~gcUploadPrompt();


protected:
	wxStaticText* m_labInfo;
	gcCheckBox* m_cbReminder;
	gcButton* m_butClose;
	
	void onButClick( wxCommandEvent& event );
	void EndModal(int code);

private:

	DECLARE_EVENT_TABLE();	
};

#endif //DESURA_UPLOADPROMPT_H
