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

#include "Common.h"
#include "SteamUser.h"

BEGIN_EVENT_TABLE( SteamUserDialog, gcDialog )
	EVT_BUTTON( wxID_ANY, SteamUserDialog::onButClick )
END_EVENT_TABLE()

#define STEAMPATH "HKEY_CURRENT_USER\\Software\\Valve\\Steam\\SteamPath"
#define NOSTEAM "Steam Not Installed"

int GetSteamUsers(wxChoice *cbBox)
{
	if (!cbBox)
		return 0;

	cbBox->Clear();
	std::string steampath = UTIL::OS::getConfigValue(STEAMPATH);

	if (steampath.size() == 0)
	{
		cbBox->Append(wxT(NOSTEAM));
		return 0;
	}

	gcString searchPath("{0}\\steamapps\\", steampath);

	std::vector<UTIL::FS::Path> fileList;
	UTIL::FS::getAllFolders(UTIL::FS::Path(searchPath, "", false), fileList);

	if (fileList.empty())
	{
		cbBox->Append(wxT(NOSTEAM));
		return 0;
	}

	int num = 0;

	for (size_t x=0; x<fileList.size(); x++)
	{
		if (fileList[x].getLastFolder() == "common" || fileList[x].getLastFolder() == "SourceMods" || fileList[x].getLastFolder() == "media")
			continue;

		cbBox->Append(fileList[x].getLastFolder());
		num++;
	}

	return num;
}

SteamUserDialog::SteamUserDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : gcDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxArrayString m_cbSteamUserChoices;

	m_cbSteamUser = new gcChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cbSteamUserChoices, 0 );
	m_cbSteamUser->SetSelection( 0 );

	m_labInfo = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#STEAMWARN"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labInfo->Wrap( 240 );
	m_butOk = new gcButton( this, wxID_ANY, Managers::GetString(L"#OK"), wxDefaultPosition, wxDefaultSize, 0 );


	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	bSizer2->Add( m_cbSteamUser, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butOk, 0, wxLEFT, 5 );



	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( m_labInfo, 0, wxALL, 5 );
	fgSizer1->Add( bSizer2, 1, wxEXPAND|wxALL, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();
}

SteamUserDialog::~SteamUserDialog()
{
}

int SteamUserDialog::ShowModal()
{
	int num = GetSteamUsers(m_cbSteamUser);
	m_cbSteamUser->SetSelection( 0 );

	switch (num)
	{
	case 1:
		saveSteamUser();
	case 0:
		return 0;
		break;

	default:
		return wxDialog::ShowModal();
		break;
	}
}

void SteamUserDialog::saveSteamUser()
{
	wxString str = m_cbSteamUser->GetStringSelection();

	if (wcscmp(str.c_str(), wxT(NOSTEAM)) !=0)
		m_szSteamUser = gcString(str.c_str().AsChar());
}

void SteamUserDialog::onButClick( wxCommandEvent& event )
{
	saveSteamUser();
	EndModal(0);
}
