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

#ifndef DESURA_UPLOADFORM_H
#define DESURA_UPLOADFORM_H
#ifdef _WIN32
#pragma once
#endif


#include "wx_controls/gcControls.h"
#include "BasePage.h"

#include "usercore/ItemInfoI.h"
#include "UploadInfoPage.h"
#include "UploadProgPage.h"

///////////////////////////////////////////////////////////////////////////////
/// Class create form
///////////////////////////////////////////////////////////////////////////////
class UploadMCFForm : public gcFrame 
{
public:
	UploadMCFForm( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Uploading Item"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 370,130 ), long style = wxCAPTION|wxCLOSE_BOX|wxFRAME_FLOAT_ON_PARENT|wxSYSTEM_MENU );
	~UploadMCFForm();

	//this is used for a new upload
	void setInfo(DesuraId id);

	//this is used to upload a file from the MCF create process
	void setInfo_path(DesuraId id, const char* path);

	//this is used to resume uploading of a file
	void setInfo_key(DesuraId id, const char* key);

	void run();

	void showInfo();
	void showProg(uint32 hash, uint32 start);

	DesuraId getItemId(){return m_uiInternId;}

	void setTrueClose(){m_bTrueClose = true;}

protected:
	void setTitle(const char* name);
	void updateInfo(uint32& itemId);

	BasePage* m_pPage;	
	wxBoxSizer* m_bsSizer;

	void onFormClose( wxCloseEvent& event );
	void cleanUpPages();

private:

	bool m_bTrueClose;

	gcString m_szKey;
	gcString m_szPath;

	DesuraId m_uiInternId;
	UserCore::Item::ItemInfoI* m_pItemInfo;

	DECLARE_EVENT_TABLE();
};

#endif //DESURA_UPLOADFORM_H
