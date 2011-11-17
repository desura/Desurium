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
#include "CreateProgPage.h"

#include "MainApp.h"
#include "CreateForm.h"
#include "Managers.h"

#include "usercore/ItemInfoI.h"
#include "usercore/MCFThreadI.h"


///////////////////////////////////////////////////////////////////////////////
/// Create MCF dialog
///////////////////////////////////////////////////////////////////////////////

CreateProgPage::CreateProgPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : BasePage( parent, id, pos, size, style )
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &CreateProgPage::onButtonClick, this); 

	m_staticText3 = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#CF_PROG"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labPercent = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#CF_NOTSTARTED"), wxDefaultPosition, wxDefaultSize, 0 );

	m_pbProgress = new gcSpinnerProgBar( this, wxID_ANY, wxDefaultPosition, wxSize( -1,22 ) );


	m_butPause = new gcButton(this, wxID_ANY, Managers::GetString(L"#PAUSE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butPause->Enable(false);

	m_butCancel = new gcButton(this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );


	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );



	wxBoxSizer* bSizer5 = new wxBoxSizer( wxHORIZONTAL );
	bSizer5->Add( m_staticText3, 0, wxTOP|wxLEFT, 5 );
	bSizer5->Add( m_labPercent, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	
	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer4->Add( m_butPause, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer4->Add( m_butCancel, 0, wxALL, 5 );



	fgSizer1->Add( bSizer5, 0, wxEXPAND, 5 );
	fgSizer1->Add( m_pbProgress, 0, wxALL|wxEXPAND, 5 );
	fgSizer1->Add( 0, 0, 0, wxEXPAND, 5 );
	fgSizer1->Add( bSizer4, 0, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();


	m_pThread = NULL;
	m_bThreadPaused = false;

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NORMAL);
}

CreateProgPage::~CreateProgPage()
{
	dispose();
}

void CreateProgPage::dispose()
{
	if (m_pThread)
	{
		*m_pThread->getMcfProgressEvent() -= delegate(this, &CreateProgPage::onProgress);
		*m_pThread->getErrorEvent() -= delegate(this, &CreateProgPage::onError);
		*m_pThread->getCompleteStringEvent() -= delegate(this, &CreateProgPage::onComplete);
	}

	safe_delete(m_pThread);
}

void CreateProgPage::onButtonClick( wxCommandEvent& event )
{
	if (event.GetId() == m_butCancel->GetId())
	{
		//need to remove these so they dont cause any more wxEvents once this dies
		//other wise app crashes and god kills a kitten. 
		if (m_pThread)
		{
			*m_pThread->getMcfProgressEvent() -= delegate(this, &CreateProgPage::onProgress);
			*m_pThread->getErrorEvent() -= delegate(this, &CreateProgPage::onError);
			*m_pThread->getCompleteStringEvent() -= delegate(this, &CreateProgPage::onComplete);
			m_pThread->stop();
		}

		CreateMCFForm* temp = dynamic_cast<CreateMCFForm*>(GetParent());

		if (temp)
		{
			temp->cancelPrompt();
			temp->Close();
		}
	}
	else if (event.GetId() == m_butPause->GetId())
	{
		gcFrame* par = dynamic_cast<gcFrame*>(GetParent());

		if (m_bThreadPaused)
		{
			m_butPause->SetLabel(Managers::GetString(L"#PAUSE"));

			if (par)
				par->setProgressState(gcFrame::P_NORMAL);
		}
		else
		{
			m_butPause->SetLabel(Managers::GetString(L"#RESUME"));

			if (par)
				par->setProgressState(gcFrame::P_PAUSED);
		}

		
		if (m_pThread)
		{
			if (m_bThreadPaused)
				m_pThread->unpause();
			else
				m_pThread->pause();
			
			m_bThreadPaused = !m_bThreadPaused;
		}
	}
}

void CreateProgPage::setInfo(DesuraId id, const char* path)
{
	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!item && !GetUserCore()->isAdmin())
	{	
		GetParent()->Close();
		return;
	}

	BasePage::setInfo(id);

	m_szFolderPath = gcString(path);
	Refresh();
}

void CreateProgPage::run()
{
	if (!getItemInfo() && !GetUserCore()->isAdmin())
		return;

	Show(true);
	Raise();

	if (m_pThread)
		return;

	m_pThread = GetThreadManager()->newCreateMCFThread(getItemId(), m_szFolderPath.c_str());

	*m_pThread->getMcfProgressEvent() += guiDelegate(this, &CreateProgPage::onProgress);
	*m_pThread->getErrorEvent() += guiDelegate(this, &CreateProgPage::onError);
	*m_pThread->getCompleteStringEvent() += guiDelegate(this, &CreateProgPage::onComplete);

	m_pThread->start();
}

void CreateProgPage::setPercent(uint32 per)
{
	gcWString prog(L"{0} %", per);
	m_labPercent->SetLabel(prog);
	m_pbProgress->setProgress(per);

	m_butPause->Enable(true);
	Refresh(false);
}

void CreateProgPage::finished()
{

}

void CreateProgPage::onComplete(gcString& path)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	gcException eFailCrtMCF(ERR_BADPATH, "Failed to create MCF");
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	if (UTIL::FS::isValidFile(UTIL::FS::PathWithFile(path)))
	{
		CreateMCFForm* temp = dynamic_cast<CreateMCFForm*>(GetParent());

		if (temp)
			temp->showOverView(path.c_str());
	}
	else
	{
		onError(eFailCrtMCF);
	}
}

void CreateProgPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	gcErrorBox(this, "#CF_ERRTITLE", "#CF_ERROR", e);
	Close();
}

void CreateProgPage::onProgress(MCFCore::Misc::ProgressInfo& info)
{
	setPercent(info.percent);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgress(info.percent);

	Update();
}
