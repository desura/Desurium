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
#include "InstallINPage.h"

#include "MainApp.h"

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{


InstallINPage::InstallINPage(wxWindow* parent) : BaseInstallPage(parent)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallINPage::onButtonPressed, this);

	m_labLabel = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#IF_ETIME"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labInfo = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#IF_INITINSTALL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pbProgress = new gcSpinnerProgBar(this, wxID_ANY, wxDefaultPosition, wxSize( -1,22 ));

	m_butPause = new gcButton( this, wxID_ANY,Managers::GetString(L"#PAUSE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butHide = new gcButton( this, wxID_ANY, Managers::GetString(L"#HIDE"), wxDefaultPosition, wxDefaultSize, 0 );

	wxBoxSizer* bSizer17 = new wxBoxSizer( wxHORIZONTAL );
	bSizer17->Add( m_labLabel, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	bSizer17->Add( m_labInfo, 0, wxRIGHT|wxTOP, 5 );
	
	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer4->Add( m_butHide, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer4->Add( m_butPause, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( bSizer17, 0, wxEXPAND, 5 );
	fgSizer1->Add( m_pbProgress, 0, wxALL|wxEXPAND, 5 );
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer1->Add( bSizer4, 0, wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	this->setParentSize(-1, 120);

	m_bPaused = false;
	m_bCompleted = false;

	m_butPause->Enable(false);
	m_butHide->Enable(false);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NORMAL);
}

InstallINPage::~InstallINPage()
{
}

void InstallINPage::init()
{
}

void InstallINPage::onButtonPressed(wxCommandEvent& event)
{
	if (m_bCompleted)
	{
		if (m_butHide->GetId() == event.GetId())
			g_pMainApp->handleInternalLink(getItemId(), ACTION_LAUNCH);

		GetParent()->Close();
	}
	else if (event.GetId() == m_butPause->GetId())
	{
		pause(!m_bPaused);
	}
	else if (event.GetId() == m_butHide->GetId())
	{
		GetParent()->Close();
	}
}

//res will be 1 on error (i.e. hash fail) and 0 if no error
void InstallINPage::onComplete(uint32& res)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	m_pbProgress->setProgress(100);
	m_butHide->SetLabel(Managers::GetString(L"#CLOSE"));
	
	UserCore::Item::BranchInfoI *bi = getItemInfo()->getCurrentBranch();

	if (bi && bi->isPreOrder())
	{
		m_butPause->Show(false);
		m_labInfo->SetLabel(Managers::GetString(L"#IF_COMPLETEPREORDER"));
		m_butHide->Enable(false);
	}
	else 
	{
		m_butPause->Enable(true);
		m_butHide->Enable(true);
		m_butPause->SetLabel(Managers::GetString(L"#CLOSE"));
		m_labInfo->SetLabel(Managers::GetString(L"#COMPLTETED"));
		m_butHide->SetLabel(Managers::GetString(L"#LAUNCH"));
	}

	m_bCompleted = true;
}

void InstallINPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	if (!getItemHandle()->shouldPauseOnError())
		gcErrorBox(GetParent(), "#IF_IERRTITLE", "#IF_IERROR", e);
}

void InstallINPage::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	if (info.flag == 0)
	{
		std::string lab = UTIL::MISC::genTimeString(info.hour, info.min, 0);
		m_labInfo->SetLabel(lab);
		m_butPause->Enable(true);
		m_butHide->Enable(true);

		gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
		if (par)
			par->setProgress(info.percent);
	}

	m_pbProgress->setProgress(info.percent);
	Update();
}

void InstallINPage::onPause(bool &state)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());

	if (!state)
	{
		m_labInfo->SetLabel(Managers::GetString(L"#UDF_UNKNOWNPAUSE"));
		m_butPause->SetLabel(Managers::GetString(L"#PAUSE"));
		m_bPaused = false;

		if (par)
			par->setProgressState(gcFrame::P_NORMAL);
	}
	else
	{
		m_bPaused = true;
		m_butPause->SetLabel(Managers::GetString(L"#RESUME"));

		if (par)
			par->setProgressState(gcFrame::P_PAUSED);
	}
}

}
}
}