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

#include "Common.h"
#include "ModWizardFinPage.h"
#include "ModWizard.h"

#include "wx_controls/gcManagers.h"
#include "MainApp.h"

BEGIN_EVENT_TABLE( MWFinishPage, BasePage )
	EVT_BUTTON( wxID_ANY, MWFinishPage::onButtonClicked )
END_EVENT_TABLE()


MWFinishPage::MWFinishPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : BasePage( parent, id, pos, size, style )
{
	m_butClose = new gcButton(this, wxID_ANY, Managers::GetString(L"#CLOSE"));
	m_butPlay = new gcButton(this, wxID_ANY, Managers::GetString(L"#PLAY"));
	m_butCustom = new gcButton(this, wxID_ANY, Managers::GetString(L"#MW_CUSTOM"));

	m_labInfo = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#MW_FINISH"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_labInfo->Wrap( 360 );
	Managers::LoadTheme(m_labInfo, "label");
	
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butCustom, 0, wxTOP|wxBOTTOM, 5 );
	bSizer2->Add( m_butPlay, 0, wxALL, 5 );
	bSizer2->Add( m_butClose, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( m_labInfo, 0, wxALL|wxEXPAND, 5 );
	fgSizer1->Add( 0, 0, 0, 0, 5 );
	fgSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();

	setParentSize(-1, 165);
}

MWFinishPage::~MWFinishPage()
{
	dispose();
}

void MWFinishPage::dispose()
{

}

void MWFinishPage::onButtonClicked( wxCommandEvent& event )
{
	if (m_butPlay->GetId() == event.GetId())
	{
		g_pMainApp->handleInternalLink(DesuraId(), ACTION_PLAY);
	}
	else if (m_butCustom->GetId() == event.GetId())
	{
		g_pMainApp->handleInternalLink(0, ACTION_SHOWSETTINGS, FormatArgs("tab=cip"));
	}

	GetParent()->Close();
}
