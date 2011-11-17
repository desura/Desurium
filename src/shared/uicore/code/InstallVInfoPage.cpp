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
#include "InstallVInfoPage.h"

#include "ItemForm.h"

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

InstallVerifyInfoPage::InstallVerifyInfoPage(wxWindow* parent) : BaseInstallPage(parent)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallVerifyInfoPage::onButtonClicked, this);

	m_labInfo = new wxStaticText( this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), 0 );
	m_labInfo->Wrap( 360 );

	m_cbFiles = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#IF_VERIFYFILES"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbTools = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#IF_VERIFYTOOLS"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbInstallHook = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#IF_VERIFYHOOKS"), wxDefaultPosition, wxDefaultSize, 0 );

	m_cbFiles->SetValue(true);
	m_cbTools->SetValue(true);
	m_cbInstallHook->SetValue(true);

	m_butVerify = new gcButton( this, wxID_ANY, Managers::GetString(L"#VERIFY"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );

	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer4->Add( m_butVerify, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer4->Add( m_butCancel, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 6, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer2->Add( m_labInfo, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5 );
	fgSizer2->Add( m_cbFiles, 0, wxTOP|wxLEFT|wxRIGHT, 5 );
	fgSizer2->Add( m_cbTools, 0, wxTOP|wxLEFT|wxRIGHT, 5 );
	fgSizer2->Add( m_cbInstallHook, 0, wxTOP|wxLEFT|wxRIGHT, 5 );
	fgSizer2->Add( 0, 5, 1, wxEXPAND, 5 );
	fgSizer2->Add( bSizer4, 1, wxEXPAND, 5 );

	this->SetSizer( fgSizer2 );
	this->Layout();

#ifdef WIN32
	this->setParentSize(-1, 160);
#else
	this->setParentSize(-1, 200);
#endif
}

InstallVerifyInfoPage::~InstallVerifyInfoPage()
{
}

void InstallVerifyInfoPage::init()
{
	UserCore::Item::ItemInfoI *info = getItemInfo();

	if (!info)
	{
		Close();
		return;
	}

	m_labInfo->SetLabel(gcWString(Managers::GetString(L"#IF_VERIFYCONFIRM"), info->getName()));
	m_labInfo->Wrap( 360 );
}

void InstallVerifyInfoPage::onButtonClicked( wxCommandEvent& event )
{
	if (event.GetId() == m_butCancel->GetId())
	{
		GetParent()->Close();
	}
	else if (event.GetId() == m_butVerify->GetId())
	{
		ItemForm* itemForm = dynamic_cast<ItemForm*>(GetParent());

		if (!itemForm)
		{
			GetParent()->Close();
			return;
		}

		bool res = itemForm->startVerify(m_cbFiles->GetValue(), m_cbTools->GetValue(), m_cbInstallHook->GetValue());

		if (res)
		{
			m_cbFiles->Enable(false);
			m_cbTools->Enable(false);
			m_cbInstallHook->Enable(false);
			m_butVerify->Enable(false);
			m_butCancel->Enable(false);
		}
		else
		{
			GetParent()->Close();
		}
	}
}


}
}
}

