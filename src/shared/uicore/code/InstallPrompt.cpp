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
#include "InstallPrompt.h"
#include "MainApp.h"

#ifdef NIX
#include "util/UtilLinux.h"
#endif

BEGIN_EVENT_TABLE( InstallPrompt, wxDialog )
	EVT_BUTTON( wxID_ANY, InstallPrompt::onButtonClick )
END_EVENT_TABLE()

InstallPrompt::InstallPrompt( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : gcDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	

	m_labQuestion = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#IF_CONFLICT"), wxDefaultPosition, wxSize(-1,30), 0 );
	m_labQuestion->Wrap( 250 );

	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 6, 1, 0, 0 );
	fgSizer2->AddGrowableCol(0);
	fgSizer2->AddGrowableRow(4);
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_butExplorer = new gcButton( this, wxID_ANY, Managers::GetString(L"#IF_SHOWFILES"));
	m_butOk = new gcButton( this, wxID_OK, wxT("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton( this, wxID_CANCEL, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );


	wxBoxSizer* bSizerD = new wxBoxSizer( wxHORIZONTAL );
	bSizerD->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizerD->Add( m_butExplorer, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizerD->Add( m_butOk, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizerD->Add( m_butCancel, 0, wxALL, 5 );

#ifdef WIN32
	textA = new gcStaticText(this, wxID_ANY, Managers::GetString(L"#IF_INSTALL_REMOVE"));
	textB = new gcStaticText(this, wxID_ANY, Managers::GetString(L"#IF_INSTALL_LEAVE"));
	textC = new gcStaticText(this, wxID_ANY, Managers::GetString(L"#IF_INSTALL_VERIFY"));

	m_rbInstallRemove = new gcRadioButton( this);
	m_rbInstallLeave = new gcRadioButton( this );
	m_rbInstallVerify = new gcRadioButton( this);

	wxBoxSizer* bSizerA = new wxBoxSizer( wxHORIZONTAL );
	bSizerA->Add(10,0,0,0,0);
	bSizerA->Add( m_rbInstallRemove, 0, 0, 5 );
	bSizerA->Add( textA, 0, wxLEFT, 5 );

	wxBoxSizer* bSizerB = new wxBoxSizer( wxHORIZONTAL );
	bSizerB->Add(10,0,0,0,0);
	bSizerB->Add( m_rbInstallLeave, 0, 0, 5 );
	bSizerB->Add( textB, 0, wxLEFT, 5 );

	wxBoxSizer* bSizerC = new wxBoxSizer( wxHORIZONTAL );
	bSizerC->Add(10,0,0,0,0);
	bSizerC->Add( m_rbInstallVerify, 0, 0, 5 );
	bSizerC->Add( textC, 0, wxLEFT, 5 );

	fgSizer2->Add( m_labQuestion, 0, wxALL|wxEXPAND, 5 );
	fgSizer2->Add( bSizerA, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	fgSizer2->Add( bSizerB, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	fgSizer2->Add( bSizerC, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	fgSizer2->Add(0,0,0,0,0);
	fgSizer2->Add( bSizerD, 0, wxEXPAND, 5 );
#else

	m_rbInstallRemove = new gcCheckBox(this, wxID_ANY, Managers::GetString(L"#IF_INSTALL_REMOVE"));
	m_rbInstallLeave = new gcCheckBox(this, wxID_ANY, Managers::GetString(L"#IF_INSTALL_LEAVE"));
	m_rbInstallVerify = new gcCheckBox(this, wxID_ANY, Managers::GetString(L"#IF_INSTALL_VERIFY"));

	fgSizer2->Add( m_labQuestion, 0, wxALL|wxEXPAND, 5 );
	fgSizer2->Add( m_rbInstallRemove, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	fgSizer2->Add( m_rbInstallLeave, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	fgSizer2->Add( m_rbInstallVerify, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5 );
	fgSizer2->Add(0,0,0,0,0);
	fgSizer2->Add( bSizerD, 0, wxEXPAND, 5 );

	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &InstallPrompt::onCheckBoxSelect, this);
#endif

	m_rbInstallRemove->SetValue(true);

	this->SetSizer( fgSizer2 );
	this->Layout();

#ifdef WIN32
	textA->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( InstallPrompt::onMouseDown ), NULL, this );
	textB->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( InstallPrompt::onMouseDown ), NULL, this );
	textC->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( InstallPrompt::onMouseDown ), NULL, this );
#endif

	

	centerOnParent();
}

InstallPrompt::~InstallPrompt()
{
}

uint32 InstallPrompt::getChoice()
{
	if (m_rbInstallRemove->GetValue())
		return C_REMOVE;

	if (m_rbInstallVerify->GetValue())
		return C_VERIFY;

	return C_INSTALL;
}

void InstallPrompt::onCheckBoxSelect(wxCommandEvent &event)
{
	m_rbInstallRemove->SetValue(false);
	m_rbInstallLeave->SetValue(false);
	m_rbInstallVerify->SetValue(false);
	
	if (event.GetId() == m_rbInstallRemove->GetId())
	{
		m_rbInstallRemove->SetValue(true);
	}
	else if (event.GetId() == m_rbInstallLeave->GetId())
	{
		m_rbInstallLeave->SetValue(true);
	}
	else if (event.GetId() == m_rbInstallVerify->GetId())
	{
		m_rbInstallVerify->SetValue(true);
	}
}

void InstallPrompt::onMouseDown( wxMouseEvent& event )
{
#ifdef WIN32
	if (event.GetId() == textA->GetId())
	{
		m_rbInstallRemove->SetValue(true);
	}
	else if (event.GetId() == textB->GetId())
	{
		m_rbInstallLeave->SetValue(true);
	}
	else if (event.GetId() == textC->GetId())
	{
		m_rbInstallVerify->SetValue(true);
	}
#endif
}

void InstallPrompt::onButtonClick(wxCommandEvent& event)
{
	if (event.GetId() == m_butExplorer->GetId())
	{
#ifdef WIN32
		ShellExecute(NULL, L"open", m_szPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#else
		UTIL::OS::launchFolder(gcString(m_szPath).c_str());
#endif
	}
	else if (event.GetId() == m_butOk->GetId())
	{
		EndModal(wxID_OK);
	}
	else if (event.GetId() == m_butCancel->GetId())
	{
		EndModal(wxID_CANCEL);
	}
}

void InstallPrompt::setInfo(DesuraId id, const char* path)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!item || !path)
	{
		Close();
		return;
	}

	m_szPath = gcWString(path);

	gcWString title(L"{0} {1}", Managers::GetString(L"#IF_INSTALL"), item->getName());
	gcWString quest(Managers::GetString(L"#IF_EXISTINGFILES"), item->getName());

	SetTitle(title);
	m_labQuestion->SetLabel(quest);


}
