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
#include "InstallINToolPage.h"

#include "MainApp.h"
#include "usercore/UserCoreI.h"
#include "usercore/ToolManagerI.h"

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{


InstallINToolPage::InstallINToolPage(wxWindow* parent) : BaseInstallPage(parent)
{
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallINToolPage::onButtonPressed, this);

	m_labLabel = new gcStaticText( this, wxID_ANY, Managers::GetString(L"#IF_INTOOL_WAIT_OTHER"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pbProgress = new gcSpinnerProgBar( this, wxID_ANY, wxDefaultPosition, wxSize( -1,22 ) );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CLOSE"), wxDefaultPosition, wxDefaultSize, 0 );

	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer4->Add( m_butCancel, 0, wxALL, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( m_labLabel, 0, wxEXPAND|wxLEFT|wxTOP, 5 );
	fgSizer1->Add( m_pbProgress, 0, wxALL|wxEXPAND, 5 );
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer1->Add( bSizer4, 0, wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	this->setParentSize(-1, 120);

	m_bCompleted = false;

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NORMAL);
}

InstallINToolPage::~InstallINToolPage()
{
}

void InstallINToolPage::init()
{
}

void InstallINToolPage::onButtonPressed(wxCommandEvent& event)
{
	GetParent()->Close();
}

//res will be 1 on error (i.e. hash fail) and 0 if no error
void InstallINToolPage::onComplete(uint32& res)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);

	m_pbProgress->setProgress(100);
}

void InstallINToolPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	gcErrorBox(GetParent(), "#IF_IERRTITLE", "#IF_IERROR", e);
}

void InstallINToolPage::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	if (info.flag == 1)
	{
		DesuraId id(info.totalAmmount);

		std::string name = GetUserCore()->getToolManager()->getToolName(id);
		m_labLabel->SetLabel(gcWString(Managers::GetString(L"#IF_INTOOL_LABEL"), name));
	}
	else if (info.flag == 2)
	{
		m_labLabel->SetLabel(Managers::GetString(L"#IF_INTOOL_WAIT_HELPER"));
		m_pbProgress->setCaption("");
	}
	else
	{
		gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
		if (par)
			par->setProgress(info.percent);

		m_pbProgress->setProgress(info.percent);

		gcString progLabel("{0} of {1}", info.doneAmmount, info.totalAmmount);
		m_pbProgress->setCaption(progLabel);
	}

	Update();
}

}
}
}
