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
#include "NewAccountDialog.h"

#include "wx_controls/gcControls.h"


NewAccountDialog::NewAccountDialog(wxWindow* parent) : gcDialog( parent, wxID_ANY, wxT("#NA_TITLE"), wxDefaultPosition, wxSize(400,400), wxCAPTION|wxCLOSE_BOX|wxSYSTEM_MENU)
{
	SetTitle(Managers::GetString(L"#NA_TITLE"));

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &NewAccountDialog::onButtonClicked, this);

	m_pBrowser = new gcMiscWebControl(this, "http://www.desura.com/app/terms", "TermsOfService");
	m_pBrowser->onPageLoadEvent += delegate(this, &NewAccountDialog::onPageLoad);

	m_butBack = new gcButton( this, wxID_ANY, Managers::GetString(L"#BACK"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butAgree = new gcButton( this, wxID_ANY, Managers::GetString(L"#AGREE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel->SetDefault();

	m_butAgree->Enable(false);
	m_butBack->Enable(false);

	wxBoxSizer* bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	bSizer5->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer5->Add( m_butBack, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer5->Add( m_butAgree, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer5->Add( m_butCancel, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer2 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->Add( m_pBrowser, 1, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );
	fgSizer2->Add( bSizer5, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer2 );
	this->Layout();

	m_butCancel->SetFocus();
	m_bTermsOfService = true;
}

void NewAccountDialog::onPageLoad()
{
	m_butAgree->Enable(true);
}

void NewAccountDialog::onButtonClicked(wxCommandEvent& event)
{
	if (event.GetId() == m_butAgree->GetId())
	{
		if (m_bTermsOfService)
		{
			m_bTermsOfService = false;
			m_butAgree->SetLabel(Managers::GetString(L"#SUBMIT"));
			m_butBack->Enable();
			m_pBrowser->loadUrl("http://www.desura.com/members/register/app");
		}
		else
		{
			m_pBrowser->executeJScript("registerMember();");
		}
	}
	else if (event.GetId() == m_butBack->GetId())
	{
		m_bTermsOfService = true;
		m_butBack->Enable(false);
		m_butAgree->SetLabel(Managers::GetString(L"#AGREE"));
	}
	else
	{
		EndModal(0);
	}
}

#ifdef NIX
bool NewAccountDialog::Show(const bool show)
{
	bool ret = gcDialog::Show(show);
	if (ret)
		this->Raise();
		
	return ret;
}
#endif