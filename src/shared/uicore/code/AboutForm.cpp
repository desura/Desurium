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
#include <branding/branding.h>
#include "AboutForm.h"
#include "MainForm.h"

#include "AboutPageMain.h"
#include "AboutPageDev.h"

#ifdef WIN32
	static const int ABOUT_FORM_WIDTH = 295;
	static const int ABOUT_FORM_HEIGHT = 412;
#else
	#ifdef DESURA_OFFICAL_BUILD
		static const int ABOUT_FORM_WIDTH = 287;
		static const int ABOUT_FORM_HEIGHT = 462;
	#else
		static const int ABOUT_FORM_WIDTH = 275;
		static const int ABOUT_FORM_HEIGHT = 462;
	#endif
#endif	
	
AboutForm::AboutForm(wxWindow* parent) : gcDialog(parent, wxID_ANY, "About " PRODUCT_NAME, wxDefaultPosition, wxSize(ABOUT_FORM_WIDTH,ABOUT_FORM_HEIGHT), wxDEFAULT_DIALOG_STYLE)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AboutForm::onButtonClicked, this);
	Bind(wxEVT_CLOSE_WINDOW, &AboutForm::onFormClose, this);

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->AddGrowableRow( 1 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_imgLogo = new gcImageControl( this, wxID_ANY, wxDefaultPosition, wxSize( 291,200 ), 0 );
	m_imgLogo->setImage("#about_logo");
	m_imgLogo->setTile(true);

	fgSizer6->Add( m_imgLogo, 0, wxEXPAND, 5 );
	
	m_pMainPage = new AboutMainPage(this);

	m_pDevPage = new AboutDevPage(this);
	m_pDevPage->Show(false);

	m_SizePanel = new wxBoxSizer( wxVERTICAL );
	m_SizePanel->Add( m_pMainPage, 1, wxEXPAND, 5 );
	
	fgSizer6->Add( m_SizePanel, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_butCredits = new gcButton( this, wxID_ANY, Managers::GetString(L"#AB_CREDITS"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butCredits, 0, wxLEFT|wxBOTTOM|wxTOP, 5 );
	
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	m_butClose = new gcButton( this, wxID_ANY, Managers::GetString(L"#OK"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer2->Add( m_butClose, 0, wxALL, 5 );
	
	fgSizer6->Add( bSizer2, 1, wxEXPAND, 15 );
	
	this->SetSizer( fgSizer6 );
	this->Layout();

	m_bMain = true;
}

AboutForm::~AboutForm()
{
}

void AboutForm::onFormClose( wxCloseEvent& event )
{
	MainForm *temp = dynamic_cast<MainForm*>(this->GetParent());

	if (temp)
		temp->onSubFormClose(this->GetId());
}

void AboutForm::onButtonClicked( wxCommandEvent& event )
{
	if (event.GetId() == m_butCredits->GetId())
	{
		m_SizePanel->Clear(false);

		if (m_bMain)
		{
			m_SizePanel->Add( m_pDevPage, 1, wxEXPAND, 5 );
			m_butCredits->SetLabel(Managers::GetString(L"#AB_ABOUT"));
		}
		else
		{
			m_SizePanel->Add( m_pMainPage, 1, wxEXPAND, 5 );
			m_butCredits->SetLabel(Managers::GetString(L"#AB_CREDITS"));
		}

		m_pDevPage->Show(m_bMain);
		m_pMainPage->Show(!m_bMain);

		m_bMain = !m_bMain;

		this->Layout();
	}
	else
	{
		this->Show(false);
		this->Close();
	}
}
