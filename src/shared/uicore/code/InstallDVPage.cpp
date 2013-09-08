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
#include "InstallDVPage.h"

#include "usercore/ItemHandleI.h"



namespace UI
{
namespace Forms
{
namespace ItemFormPage
{


InstallDVPage::InstallDVPage(wxWindow* parent) : BaseInstallPage(parent)
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallDVPage::onButtonPressed, this);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_labInfo = new gcStaticText(this, wxID_ANY, Managers::GetString(L"#IF_VALIDATE_DEFAULT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pbProgress = new gcSpinnerProgBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1,22 ));
	m_butHide = new gcButton(this, wxID_ANY, Managers::GetString(L"#HIDE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton(this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );

	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer4->Add( m_butHide, 0, wxLEFT|wxTOP|wxBOTTOM, 5 );
	bSizer4->Add( m_butCancel, 0, wxALL, 5 );
	
	fgSizer1->Add( m_labInfo, 0, wxLEFT|wxRIGHT|wxTOP, 5 );
	fgSizer1->Add( m_pbProgress, 1, wxEXPAND|wxALL, 5 );
	fgSizer1->Add(0, 0, 0, wxEXPAND, 5);
	fgSizer1->Add( bSizer4, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	this->setParentSize(-1, 120);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NORMAL);

	m_uiLastFlag = -1;
}

InstallDVPage::~InstallDVPage()
{
}

void InstallDVPage::init()
{
}

void InstallDVPage::onButtonPressed(wxCommandEvent& event)
{
	if (m_butHide->GetId() == event.GetId())
	{
		GetParent()->Close();
	}
	else if (m_butCancel->GetId() == event.GetId())
	{
		getItemHandle()->cancelCurrentStage();
		GetParent()->Close();
	}
}

void InstallDVPage::onComplete(uint32& itemId)
{
	uint32 prog = 100;
	onProgressUpdate(prog);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);
}

void InstallDVPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	m_labInfo->SetLabel( Managers::GetString(L"#IF_GERROR_LABEL") );
	m_butCancel->SetLabel(Managers::GetString(L"#CLOSE"));

	gcErrorBox(GetParent(), "#IF_VALIDATE_TITLE", "#IF_VALIDATE_ERROR", e);
}

void InstallDVPage::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	uint8 stage = info.flag&0xF;
	uint8 section = (info.flag>>4)&0xF;

	if (stage != m_uiLastFlag)
	{
		if (stage == 1)
		{
			m_labInfo->SetLabel(Managers::GetString(L"#IF_VALIDATE_LOCALMCFS"));
		}
		else if (stage == 2)
		{
			m_labInfo->SetLabel(Managers::GetString(L"#IF_VALIDATE_EXISTINGMCF"));
		}
		else if (stage == 3)
		{
			m_labInfo->SetLabel(Managers::GetString(L"#IF_VALIDATE_LOCALFILES"));
		}
		else if (stage == 4)
		{
			m_labInfo->SetLabel(Managers::GetString(L"#IF_VALIDATE_PREALLOCATING"));
		}

		m_uiLastFlag = stage;
	}

	if (info.totalAmmount != 0)
	{
		gcString label;
		
		if (stage == 4)
			label = gcString("{0} of {1}", UTIL::MISC::niceSizeStr(info.doneAmmount), UTIL::MISC::niceSizeStr(info.totalAmmount));
		else
			label = gcString(Managers::GetString(L"#IF_VALIDATE_LABEL"), info.doneAmmount, info.totalAmmount, (uint32)section);

		m_pbProgress->setCaption(label);
	}

	m_pbProgress->setProgress(info.percent);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgress(info.percent);
}


}
}
}



