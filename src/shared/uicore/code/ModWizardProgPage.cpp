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
#include "MainApp.h"
#include "Managers.h"
#include "ModWizard.h"
#include "ModWizardProgPage.h"

#include "managers/WildcardDelegate.h"
#include "managers/WildcardManager.h"
#include "usercore/UserThreadManagerI.h"
#include "usercore/ItemManagerI.h"





BEGIN_EVENT_TABLE( MWProgressPage, BasePage )
	EVT_CLOSE( MWProgressPage::onClose )
	EVT_BUTTON( wxID_ANY, MWProgressPage::onButtonClicked )
END_EVENT_TABLE()

MWProgressPage::MWProgressPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : BasePage( parent, id, pos, size, style )
{

	m_labTitle = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#PROCESSING"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_labInfo = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#INITIALIZING"), wxDefaultPosition, wxDefaultSize, 0 );

	m_pbProgress = new gcSpinnerProgBar( this, wxID_ANY, wxDefaultPosition, wxSize( -1,22 ) );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );

	
	wxBoxSizer* bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	bSizer1->Add( m_labTitle, 0, wxTOP|wxLEFT, 5 );
	bSizer1->Add( m_labInfo, 0, wxTOP|wxRIGHT|wxLEFT, 5 );

	wxBoxSizer* bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer2->Add( m_butCancel, 0, wxALL, 5 );
	

	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer3->AddGrowableCol(0);
	fgSizer3->AddGrowableRow(2);
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer3->Add( bSizer1, 0, wxEXPAND, 5 );
	fgSizer3->Add( m_pbProgress, 0, wxEXPAND|wxALL, 5 );
	fgSizer3->Add( 0, 0, 0, wxEXPAND, 5 );
	fgSizer3->Add( bSizer2, 0, wxEXPAND, 5 );

	this->SetSizer( fgSizer3 );
	this->Layout();

	m_pThread = NULL;
	m_bStopped = false;
	m_bAddToAccount = true;

	this->setParentSize(-1, 120);
	this->Layout();

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NORMAL);

	GetParent()->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MWProgressPage::onClose ), NULL, this );
}

MWProgressPage::~MWProgressPage()
{
	dispose();
}

void MWProgressPage::dispose()
{
	GetParent()->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( MWProgressPage::onClose ), NULL, this );
	m_bStopped = true;
	safe_delete(m_pThread);
}

void MWProgressPage::onButtonClicked( wxCommandEvent& event )
{
	m_bStopped = true;
	GetParent()->Close();
}

void MWProgressPage::onClose( wxCloseEvent& event )
{
	m_bStopped = true;
	event.Skip();
}

void MWProgressPage::run()
{

	m_pThread = GetThreadManager()->newInstalledWizardThread();

	*m_pThread->getMcfProgressEvent() += guiDelegate(this, &MWProgressPage::onProgress);
	*m_pThread->getErrorEvent() += guiDelegate(this, &MWProgressPage::onError);
	*m_pThread->getCompleteEvent() += guiDelegate(this, &MWProgressPage::onComplete);
	*m_pThread->getNewItemEvent() += guiDelegate(this, &MWProgressPage::onNewItem);
	*m_pThread->getItemFoundEvent() += guiDelegate(this, &MWProgressPage::onItemFound);
	*m_pThread->getNeedWCEvent() += wcDelegate(this);

	m_pThread->start();
}


void MWProgressPage::onComplete(uint32&)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	ModWizardForm *temp = dynamic_cast<ModWizardForm*>(GetParent());
	if (temp)
		temp->finish();
}


void MWProgressPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	gcErrorBox(GetParent(), "#MW_ERRTITLE", "#MW_ERROR", e);

	m_labInfo->SetLabel(Managers::GetString(L"#MW_ERRTITLE"));
	m_butCancel->SetLabel(Managers::GetString(L"#CLOSE"));

	GetParent()->Close();
}

void MWProgressPage::onProgress(MCFCore::Misc::ProgressInfo& info)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgress(info.percent);

	m_pbProgress->setProgress(info.percent);
	Update();
}

void MWProgressPage::onNewItem(gcString& itemName)
{
	if (itemName.c_str())
	{
		gcWString wText(itemName);
		m_labInfo->SetLabel(wText.c_str());
		m_butCancel->Enable(true);
	}
}

void MWProgressPage::onItemFound(DesuraId& id)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (item && m_bAddToAccount)
		item->addToAccount();
}
