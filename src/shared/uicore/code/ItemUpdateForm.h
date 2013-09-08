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

#ifndef DESURA_ITEMUPDATEFORM_H
#define DESURA_ITEMUPDATEFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "wx/wx.h"
#include "gcMiscWebControl.h"

class UpdateInfoForm : public gcFrame 
{
public:
	UpdateInfoForm( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Update [Item Name]"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 370,250 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	~UpdateInfoForm();

	void setInfo(DesuraId id, bool launch = false);
	DesuraId getItemId(){return m_uiInternId;}

protected:
	
	wxStaticText* m_labInfo;
	gcMiscWebControl* m_ieBrowser;
	
	gcCheckBox* m_cbReminder;
	
	gcButton* m_butUpdate;
	gcButton* m_butLaunch;
	gcButton* nm_butCancel;

	void onButtonPressed(wxCommandEvent& event);
	void onFormClose( wxCloseEvent& event );

private:
	DesuraId m_uiInternId;
	bool m_bLaunch;

	DECLARE_EVENT_TABLE();
};

#endif //DESURA_ITEMUPDATEFORM_H
