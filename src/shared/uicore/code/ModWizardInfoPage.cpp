///////////// Copyright 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : uicore
//   File        : mwInfoPage.cpp
//   Description :
//      [Write the purpose of mwInfoPage.cpp.]
//
//   Created On: 12/19/2008 9:29:43 AM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "ModWizardInfoPage.h"
#include "ModWizard.h"

#include "wx_controls/gcManagers.h"

BEGIN_EVENT_TABLE( MWInfoPage, BasePage )
	EVT_BUTTON( wxID_ANY, MWInfoPage::onButtonClicked )
END_EVENT_TABLE()


MWInfoPage::MWInfoPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : BasePage( parent, id, pos, size, style )
{
	m_labInfo = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#MW_PROMPT"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_labInfo->Wrap( 360 );
	Managers::LoadTheme(m_labInfo, "label");

	m_cbAddAccount = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#MW_ADDACCOUNT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbAddAccount->SetValue(true);

	m_butStart = new gcButton( this, wxID_ANY, Managers::GetString(L"#START"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );
	

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butStart, 0, wxALL, 5 );
	bSizer2->Add( m_butCancel, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );
	

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 5, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 3 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( m_labInfo, 0, wxALL, 5 );
	fgSizer1->Add( 0, 10, 0, 0, 5 );
	fgSizer1->Add( m_cbAddAccount, 0, wxALL, 5 );
	fgSizer1->Add( 0, 0, 0, 0, 5 );
	fgSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();
}

MWInfoPage::~MWInfoPage()
{
	dispose();
}

void MWInfoPage::dispose()
{

}

void MWInfoPage::onButtonClicked( wxCommandEvent& event )
{
	if (m_butCancel->GetId() == event.GetId())
	{
		GetParent()->Close();
	}
	else
	{
		ModWizardForm *temp = dynamic_cast<ModWizardForm*>(GetParent());
		if (temp)
			temp->run(m_cbAddAccount->GetValue());
	}
}
