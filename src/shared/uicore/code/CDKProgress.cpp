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
#include "CDKProgress.h"
#include "CDKeyForm.h"

#include "Managers.h"
#include "MainApp.h"

CDKProgress::CDKProgress(wxWindow* parent, bool launch) : BasePage(parent)
{
	m_bLaunch = launch;

	m_labInfo = new wxStaticText( this, wxID_ANY, Managers::GetString("#CDK_PROGRESSINFO"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pbProgress = new gcSpinningBar(this, wxID_ANY, wxDefaultPosition, wxSize( -1,22 ));
	m_butClose = new gcButton( this, wxID_ANY, Managers::GetString("#CLOSE"), wxDefaultPosition, wxDefaultSize, 0 );

	wxBoxSizer* bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer1->Add( m_butClose, 0, wxALL, 5 );


	wxFlexGridSizer* fgSizer2 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer2->Add( m_labInfo, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	fgSizer2->Add( m_pbProgress, 0, wxEXPAND|wxALL, 5 );
	

	wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer1->Add( fgSizer2, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );
	fgSizer1->Add( bSizer1, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();


	setParentSize(-1, 120);
	onCompleteEvent += guiDelegate(this, &CDKProgress::onComplete);
	onErrorEvent += guiDelegate(this, &CDKProgress::onError);
}

CDKProgress::~CDKProgress()
{
	
}

void CDKProgress::run()
{
	GetUserCore()->getCDKeyManager()->getCDKeyForCurrentBranch(getItemId(), this);
}

void CDKProgress::dispose()
{
	GetUserCore()->getCDKeyManager()->cancelRequest(getItemId(), this);
}

void CDKProgress::onCDKeyComplete(DesuraId id, gcString &cdKey)
{
	onCompleteEvent(cdKey);
}

void CDKProgress::onCDKeyError(DesuraId id, gcException& e)
{
	onErrorEvent(e);
}

void CDKProgress::onComplete(gcString& cdKey)
{
	CDKeyForm* keyForm = dynamic_cast<CDKeyForm*>(GetParent());

	if (keyForm)
		keyForm->finish(cdKey.c_str());
}

void CDKProgress::onError(gcException& e)
{
	if (e.getErrId() == ERR_CDKEY && e.getSecErrId() == 117) //not required
	{
		if (m_bLaunch)
			g_pMainApp->handleInternalLink(getItemId(), ACTION_LAUNCH, FormatArgs("cdkey"));

		GetParent()->Close();
	}

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	gcErrorBox(GetParent(), "#CDK_ERRTITLE", "#CDK_ERROR", e);
	GetParent()->Close();
}
