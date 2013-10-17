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

#ifndef DESURA_GCUPDATEFORM_H
#define DESURA_GCUPDATEFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "gcMiscWebControl.h"


///////////////////////////////////////////////////////////////////////////////
/// Class GCUpdateInfo
///////////////////////////////////////////////////////////////////////////////
class GCUpdateInfo : public gcFrame 
{
public:
#ifdef WIN32
	GCUpdateInfo( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("UF_TITLE"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
#else // LINUX ISNT BIG ENOUGH TODO
	GCUpdateInfo( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("UF_TITLE"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 445,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
#endif
	~GCUpdateInfo();

	void setInfo(uint32 appver);

	virtual bool Show(bool show = true);
	virtual void Raise();

protected:
	wxStaticText* m_labInfo;
	gcMiscWebControl* m_ieBrowser;
	gcButton* m_butRestartNow;
	gcButton* m_butRestartLater;

	void onFormClose( wxCloseEvent& event );
	void onButClick( wxCommandEvent& event );

	void onPageLoad();
	void doRestart();

private:
};





#endif //DESURA_GCUPDATEFORM_H
