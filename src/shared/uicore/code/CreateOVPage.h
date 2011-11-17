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

#ifndef DESURA_CREATEOVPAGE_H
#define DESURA_CREATEOVPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "BasePage.h"
#include "MainApp.h"
#include "wx_controls/gcControls.h"
#include "wx_controls/gcGrid.h"

class CreateMCFOverview : public BasePage 
{
public:
#ifdef WIN32
	CreateMCFOverview( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 370,148 ), long style = wxTAB_TRAVERSAL );
#else // Linux needs more room as there is no custom border
	CreateMCFOverview( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 415,148 ), long style = wxTAB_TRAVERSAL );
#endif
	~CreateMCFOverview();

	void dispose();
	void setInfo(DesuraId itemId, const char* path);

	Event<ut> onUploadTriggerEvent;

	void run(){;}

protected:
	
	gcStaticText* m_labInfo;
	gcStaticText* m_labName;
	gcStaticText* m_labSize;
	gcStaticText* m_labPath;

	gcGrid* m_gInfo;
	
	gcButton* m_butUpload;
	gcButton* m_butClose;
	gcButton* m_butViewFile;
	

	void onButtonClick( wxCommandEvent& event );
	void onMouseDown( wxMouseEvent& event );
private:
	UserCore::Item::ItemInfoI* m_pItem;

	gcString m_szPath;
	gcString m_szFolderPath;

	DECLARE_EVENT_TABLE()
};



#endif //DESURA_CREATEOVPAGE_H

