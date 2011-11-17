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
#include "InstallGIPage.h"
#include "usercore/ItemHandleI.h"



namespace UI
{
namespace Forms
{
namespace ItemFormPage
{


InstallGIPage::InstallGIPage(wxWindow* parent) : BaseInstallPage(parent)
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallGIPage::onButtonPressed, this);
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_labInfo = new gcStaticText(this, wxID_ANY, Managers::GetString(L"#IF_GATHERINFO"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pbProgress = new gcSpinnerProgBar(this, wxID_ANY, wxDefaultPosition, wxSize(-1,22 ));
	m_butCancel = new gcButton(this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );

	wxBoxSizer* bSizer4 = new wxBoxSizer( wxHORIZONTAL );
	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
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
}

InstallGIPage::~InstallGIPage()
{
}

void InstallGIPage::init()
{
}



void InstallGIPage::onButtonPressed(wxCommandEvent& event)
{
	nonBlockStop();
	GetParent()->Close();
}

void InstallGIPage::onItemUpdate(UserCore::Item::ItemInfoI::ItemInfo_s& info)
{
	if (info.changeFlags & UserCore::Item::ItemInfoI::CHANGED_ICON)
		BasePage::setInfo(getItemId());
}

void InstallGIPage::onComplete(uint32& itemId)
{
	uint32 prog = 100;
	onProgressUpdate(prog);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_NONE);
}

void InstallGIPage::onError(gcException& e)
{
	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgressState(gcFrame::P_ERROR);

	m_labInfo->SetLabel( Managers::GetString(L"#IF_GERROR_LABEL") );
	m_butCancel->SetLabel(Managers::GetString(L"#CLOSE"));

	if (e.getErrId() == ERR_UNSUPPORTEDPLATFORM)
	{
		UserCore::Item::ItemInfoI* item = getItemInfo();

		gcString name("Unknown Item ({0}: {0})", getItemId().getTypeString(), getItemId().getItem());

		if (item)
			name = item->getName();

		gcString errMsg;
		
		if (e.getSecErrId() == 0)
			errMsg = gcString(Managers::GetString("#IF_GERROR_PLATFORM"), name);
		else
			errMsg = gcString(Managers::GetString("#IF_GERROR_BRANCH_PLATFORM"), name);

		gcErrorBox(GetParent(), "#IF_GERRTITLE", errMsg.c_str(), e);
	}
	else
	{
		gcErrorBox(GetParent(), "#IF_GERRTITLE", "#IF_GERROR", e);
	}
}

void InstallGIPage::onProgressUpdate(uint32& prog)
{
	if (prog == 40)
		m_labInfo->SetLabel(Managers::GetString(L"#IF_CHECKINGIR"));

	m_pbProgress->setProgress(prog);

	gcFrame* par = dynamic_cast<gcFrame*>(GetParent());
	if (par)
		par->setProgress(prog);
}


}
}
}





