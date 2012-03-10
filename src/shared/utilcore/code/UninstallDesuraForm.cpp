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
#include "UninstallDesuraForm.h"

#include "UDFSettingsPanel.h"
#include "UDFProgressPanel.h"
#include "usercore\UserCoreI.h"

#include <branding/branding.h>

void SetExitCode(int32 code);

UninstallForm::UninstallForm(wxWindow* parent, UserCore::UserI* user) : wxFrame(parent, wxID_ANY, PRODUCT_NAME_CATW(L" UnInstaller"), wxDefaultPosition, wxSize( 500,330 ), wxCAPTION|wxMINIMIZE_BOX|wxSYSTEM_MENU|wxTAB_TRAVERSAL )
{
	m_pUser = user;

	wxPanel* pTitlePanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( -1,57 ), wxTAB_TRAVERSAL );
	pTitlePanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNHIGHLIGHT ) );

	m_labTitle = new wxStaticText(pTitlePanel, wxID_ANY, Managers::GetString(L"#DUN_UNINSTALL_HEADER"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labTitleInfo = new wxStaticText(pTitlePanel, wxID_ANY, Managers::GetString(L"#DUN_UNINSTALL_SUBHEADER"), wxDefaultPosition, wxDefaultSize, 0 );

	m_imgLogo = new wxStaticBitmap(this, wxID_ANY, wxBitmap(wxT("IDI_HEADER"), wxBITMAP_TYPE_RESOURCE ), wxDefaultPosition, wxSize( 150,57 ), 0 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );

	m_butRepair = new wxButton( this, wxID_ANY, Managers::GetString(L"#DUN_REPAIR"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butUninstall = new wxButton( this, wxID_ANY, Managers::GetString(L"#DUN_UNINSTALL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new wxButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );

	wxFont font = m_labTitle->GetFont();
	font.SetWeight(wxFONTWEIGHT_BOLD);
	m_labTitle->SetFont(font);

	m_pContent = new UDFSettingsPanel(this);

	m_bContentSizer = new wxBoxSizer( wxHORIZONTAL );
	m_bContentSizer->Add(m_pContent, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer1 = new wxBoxSizer( wxHORIZONTAL );

	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer1->Add( m_butRepair, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer1->Add( m_butUninstall, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer1->Add( m_butCancel, 0, wxALL, 5 );


	wxFlexGridSizer* fgSizer2 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer2->Add( 10, 0, 1, wxEXPAND, 5 );
	fgSizer2->Add( m_labTitle, 0, wxALL, 5 );
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer2->Add( m_labTitleInfo, 0, wxLEFT, 15 );

	pTitlePanel->SetSizer( fgSizer2 );
	pTitlePanel->Layout();


	wxFlexGridSizer* fgSizer3 = new wxFlexGridSizer( 1, 2, 0, 0 );
	fgSizer3->AddGrowableCol( 0 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer3->Add( pTitlePanel, 1, wxEXPAND, 5 );
	fgSizer3->Add( m_imgLogo, 0, 0, 5 );


	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 5, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1->Add( fgSizer3, 1, wxEXPAND, 5 );
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer1->Add( m_bContentSizer, 1, wxEXPAND|wxLEFT|wxRIGHT, 10 );
	fgSizer1->Add( m_staticline1, 1, wxEXPAND, 5 );
	fgSizer1->Add( bSizer1, 1, wxEXPAND, 5 );
	
	SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DLIGHT ) );

	this->SetSizer( fgSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );
	SetIcon(wxIcon(wxICON(IDI_ICONNORMAL)));

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UninstallForm::onButtonClicked, this);
	m_bComplete = false;
}

UninstallForm::~UninstallForm()
{
}

void UninstallForm::onButtonClicked(wxCommandEvent &event)
{
	if (m_butCancel->GetId() == event.GetId())
	{
		if (!m_bComplete)
			SetExitCode(-1);
		else
			SetExitCode(0);

		GetParent()->Close();
	}
	else if (m_butUninstall->GetId() == event.GetId())
	{
		m_butCancel->Enable(false);
		m_butUninstall->Enable(false);
		m_butRepair->Enable(false);

		uint32 flags = 0;

		UDFSettingsPanel* settings = dynamic_cast<UDFSettingsPanel*>(m_pContent);

		if (settings)
			flags = settings->getFlags();

		m_pContent = new UDFProgressPanel(this, m_pUser, flags);

		m_bContentSizer->Clear(true);
		m_bContentSizer->Add(m_pContent, 1, wxEXPAND, 5);
		this->Layout();
	}
	else if (m_butRepair->GetId() == event.GetId())
	{
#ifdef DEBUG
		UTIL::WIN::launchExe("desura-d.exe", "-forceupdate");
#else
		UTIL::WIN::launchExe("desura.exe", "-forceupdate");
#endif

		SetExitCode(-1);
		GetParent()->Close();
	}
}

void UninstallForm::completedUninstall()
{
	m_butCancel->Enable();
	m_butCancel->SetLabel(Managers::GetString(L"#CLOSE"));

	m_bComplete = true;
}
