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
#include "GameDiscForm.h"
#include "MainApp.h"


GameDiskForm::GameDiskForm(wxWindow* parent, const char* exe, bool cdkey) : gcFrame(parent, wxID_ANY, wxT("{0}: Need Game Disk"), wxDefaultPosition, wxSize( 370,145 ), wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL)
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	m_labInfo = new wxStaticText( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 );
	m_labInfo->Wrap( 360 );

	m_cbReminder = new gcCheckBox( this, wxID_ANY, Managers::GetString(L"#GD_REMINDER"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butLaunch = new gcButton( this, wxID_ANY, Managers::GetString(L"#LAUNCH"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );



	wxBoxSizer* bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer1->Add( m_cbReminder, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer1->Add( m_butLaunch, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer1->Add( m_butCancel, 0, wxALL, 5 );


	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1->Add( m_labInfo, 0, wxALL, 5 );
	fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer1->Add( bSizer1, 1, wxEXPAND, 5 );
	

	this->SetSizer( fgSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GameDiskForm::onButtonClick, this);
	Bind(wxEVT_CLOSE_WINDOW, &GameDiskForm::onFormClose, this);

	m_bHasSeenCDKey = cdkey;
	m_szExe = exe;
	m_pItem = NULL;

	centerOnParent();
}

GameDiskForm::~GameDiskForm()
{
}

void GameDiskForm::setInfo(DesuraId id)
{
	m_pItem = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!m_pItem)
	{
		Close();
		return;
	}

	UserCore::Item::ItemInfoI* parent = m_pItem;

	if (m_pItem->getId().getType() == DesuraId::TYPE_MOD && m_pItem->getParentId().isOk())
		parent = GetUserCore()->getItemManager()->findItemInfo(m_pItem->getParentId());

	if (!parent)
		parent = m_pItem;

	SetTitle(gcWString(Managers::GetString(L"#GD_TITLE"), m_pItem->getName()));

	m_labInfo->SetLabel(gcWString(Managers::GetString(L"#GD_INFO"), m_pItem->getName(), parent->getName()));
	m_labInfo->Wrap( 360 );
}

void GameDiskForm::onFormClose( wxCloseEvent& event )
{
	g_pMainApp->closeForm(this->GetId());
}

void GameDiskForm::onButtonClick(wxCommandEvent& event)
{
	if (m_butLaunch->GetId() == event.GetId())
	{
		if (m_cbReminder->GetValue())
			m_pItem->addOFlag(UserCore::Item::ItemInfoI::OPTION_DONTPROMPTGAMEDISK);

		g_pMainApp->handleInternalLink(m_pItem->getId(), ACTION_LAUNCH, FormatArgs(m_szExe.size()?gcString("exe={0}", m_szExe):"", m_bHasSeenCDKey?"cdkey":"", "gamedisk"));
	}
	
	Close();
}

DesuraId GameDiskForm::getItemId()
{
	if (!m_pItem)
		return DesuraId();

	return m_pItem->getId();
}
