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
#include "ICheckProgressPage.h"

#include "MainApp.h"
#include "managers/WildcardManager.h"

#include "ItemForm.h"

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{


ICheckProgressPage::ICheckProgressPage(wxWindow* parent) : BaseInstallPage(parent)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ICheckProgressPage::onButtonPressed, this);

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer3->AddGrowableCol( 0 );
	fgSizer3->AddGrowableRow( 2 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	m_labInfo = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#INITIALIZING"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labInfo->Wrap( -1 );
	
	m_pbProgress = new gcSpinningBar( this, wxID_ANY, wxDefaultPosition, wxSize( -1,22 ) );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );

	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );

	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butCancel, 0, wxALL, 5 );

	fgSizer3->Add( m_labInfo, 0, wxTOP|wxRIGHT|wxLEFT, 5 );	
	fgSizer3->Add( m_pbProgress, 0, wxEXPAND|wxALL, 5 );
	fgSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer3->Add( bSizer2, 0, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer3 );
	this->Layout();

	setParentSize(-1, 120);
}

ICheckProgressPage::~ICheckProgressPage()
{
}

void ICheckProgressPage::init()
{
}

void ICheckProgressPage::onButtonPressed(wxCommandEvent& event)
{
	GetParent()->Close();
}

void ICheckProgressPage::onComplete(uint32&)
{
	UI::Forms::ItemForm* parent = dynamic_cast<UI::Forms::ItemForm*>(GetParent());

	if (parent)
		parent->finishInstallCheck();
}

void ICheckProgressPage::onError(gcException& e)
{
	gcErrorBox(this, "#IC_ERRTITLE", "#IC_ERROR", e);
	GetParent()->Close();
}

void ICheckProgressPage::onProgress(uint32& prog)
{
}


}
}
}