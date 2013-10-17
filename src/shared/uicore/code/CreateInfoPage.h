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

#ifndef DESURA_CREATEINFOPAGE_H
#define DESURA_CREATEINFOPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "BasePage.h"
#include "wx_controls/gcControls.h"

#include "wx/wx.h"
#include <wx/filepicker.h>

#include "Managers.h"
#include "usercore/ItemInfoI.h"


///////////////////////////////////////////////////////////////////////////////
/// Class CreateMCF_Form
///////////////////////////////////////////////////////////////////////////////
class CreateInfoPage : public BasePage 
{
public:
#ifdef WIN32
	CreateInfoPage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,100 ), long style = wxTAB_TRAVERSAL );
#else // Linux has no custom border, sizes are wrong
	CreateInfoPage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 445,100 ), long style = wxTAB_TRAVERSAL );
#endif
	~CreateInfoPage();

	void dispose();

	void setInfo(DesuraId id);
	void run(){;}
	
protected:

	gcStaticText* m_labText;

	wxTextCtrl* m_tbItemFiles;
	
	gcButton* m_butCreate;
	gcButton* m_butCancel;
	gcButton* m_butFile;
	
	bool validatePath(wxTextCtrl* ctrl, bool type);

	void onButtonClicked( wxCommandEvent& event );
	void onTextChange( wxCommandEvent& event );

	void resetAllValues();
	void validateInput();

	void showDialog();

private:
	DECLARE_EVENT_TABLE()
};


#endif //DESURA_CREATEINFOPAGE_H
