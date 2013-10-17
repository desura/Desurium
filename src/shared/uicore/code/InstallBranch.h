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

#ifndef DESURA_INSTALLBRANCH_H
#define DESURA_INSTALLBRANCH_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

namespace UserCore
{
namespace Item
{
	class BranchInfoI;
}
}

class InstallBranch : public gcDialog 
{
public:
	InstallBranch( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 370,150 ), long style = wxCAPTION|wxFRAME_FLOAT_ON_PARENT|wxFRAME_NO_TASKBAR|wxSYSTEM_MENU|wxTAB_TRAVERSAL  );
	~InstallBranch();

	int setInfo(DesuraId id, bool selectBranch);

	MCFBranch getBranch();
	MCFBranch getGlobal();

protected:
	wxStaticText* m_labInfo;
	wxChoice* m_cbBranchList;
	
	gcButton* m_butInfo;
	gcButton* m_butInstall;
	gcButton* m_butCancel;

	void onButtonClick(wxCommandEvent& event);
	void onChoice(wxCommandEvent& event);
	void fixName(gcWString &name);

private:
	bool m_bBuy;
	bool m_bSelectBranch;
	bool m_bIsMod;
	bool m_bIsExpansion;

	DesuraId m_Item;
	DECLARE_EVENT_TABLE();
};

#endif //DESURA_INSTALLBRANCH_H
