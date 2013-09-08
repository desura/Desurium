/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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
#include "InstallWaitPage.h"


class ItemPanel : public gcPanel
{
public:
	ItemPanel(wxWindow* parent, UserCore::Item::ItemInfoI* item, bool last) : gcPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL),
		m_Id(item->getId()),
		m_imgIcon(new gcImageControl(this, wxID_ANY, wxDefaultPosition, wxSize( 22,22 ), 0)),
		m_labTitle(new wxStaticText(this, wxID_ANY, item->getName(), wxDefaultPosition, wxDefaultSize, 0)),
		m_pbProgress(new gcProgressBar(this, wxID_ANY, wxDefaultPosition, wxSize( 150,18 )))
	{
		const char* ico = item->getIcon();

		if (ico && UTIL::FS::isValidFile(ico))
			m_imgIcon->setImage(ico);
		else
			m_imgIcon->setImage(GetGCThemeManager()->getImage("icon_default"));

		wxFlexGridSizer* fgSizer3;
		fgSizer3 = new wxFlexGridSizer( 1, 3, 0, 0 );
		fgSizer3->AddGrowableCol( 1 );
		fgSizer3->SetFlexibleDirection( wxBOTH );
		fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
		uint32 flags = wxLEFT|wxRIGHT|wxTOP;

		if (last)
			flags |= wxBOTTOM;

		fgSizer3->Add( m_imgIcon, 0, flags, 5 );
		fgSizer3->Add( m_labTitle, 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
		fgSizer3->Add( m_pbProgress, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	

		Managers::LoadTheme(this, "formlogin");

		this->SetSizer( fgSizer3 );
		this->Layout();
	}

	void setProgress(uint8 prog)
	{
		m_pbProgress->setProgress(prog);
	}

	DesuraId getItemId()
	{
		return m_Id;
	}

protected:
	DesuraId m_Id;

	gcImageControl *m_imgIcon;
	wxStaticText *m_labTitle;
	gcProgressBar* m_pbProgress;
};

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

InstallWaitPage::InstallWaitPage(wxWindow* parent) : BaseInstallPage(parent)
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	m_labInfo = new wxStaticText( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0 );

	m_swItemList = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_swItemList->SetScrollRate( 5, 5 );
	Managers::LoadTheme(m_swItemList, "formlogin");

	m_butCancelAll = new gcButton( this, wxID_ANY, Managers::GetString(L"#IF_WAIT_CANCELALL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butForce = new gcButton( this, wxID_ANY, Managers::GetString(L"#IF_WAIT_FORCE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butHide = new gcButton( this, wxID_ANY, Managers::GetString(L"#HIDE"), wxDefaultPosition, wxDefaultSize, 0 );



	m_pSWSizer = new wxFlexGridSizer(100, 1, 0, 0 );
	m_pSWSizer->AddGrowableCol( 0 );
	m_pSWSizer->SetFlexibleDirection( wxBOTH );
	m_pSWSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_swItemList->SetSizer(m_pSWSizer);
	m_swItemList->Layout();
	m_pSWSizer->Fit(m_swItemList);


	wxBoxSizer* bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer1->Add( m_butCancelAll, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer1->Add( m_butForce, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer1->Add( m_butHide, 0, wxALL, 5 );


	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1->Add( m_labInfo, 0, wxALL, 5 );
	fgSizer1->Add( m_swItemList, 1, wxEXPAND|wxALL, 5 );
	fgSizer1->Add( bSizer1, 1, wxEXPAND, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	this->Centre(wxBOTH);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallWaitPage::onButtonPressed, this);
	this->setParentSize(-1, 300);
}

InstallWaitPage::~InstallWaitPage()
{
}

void InstallWaitPage::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	DesuraId id(info.totalAmmount);

	for (size_t x=0; x<m_vPanelList.size(); x++)
	{
		if (m_vPanelList[x]->getItemId() == id)
		{
			m_vPanelList[x]->setProgress(info.percent);
			break;
		}
	}

	Update();
}

void InstallWaitPage::onProgressUpdate(uint32& progress)
{
	UserCore::Item::ItemHandleI* item = getItemHandle();
	UserCore::Item::ItemTaskGroupI* group = item->getTaskGroup();

	if (!group)
	{
		m_pSWSizer->Clear(true);
		m_vPanelList.clear();
		Layout();
		return;
	}

	std::vector<UserCore::Item::ItemHandleI*> list;
	group->getItemList(list);

	std::vector<ItemPanel*> oldList = m_vPanelList;
	m_vPanelList.clear();
	m_pSWSizer->Clear(false);

	for (size_t x=0; x<list.size(); x++)
	{
		uint32 index = -1;
		for (size_t y=0; y<oldList.size(); y++)
		{
			if (oldList[y] && oldList[y]->getItemId() == list[x]->getItemInfo()->getId())
			{
				index = y;
				break;
			}
		}

		ItemPanel* ip = NULL;

		if (index != UINT_MAX)
		{
			ip = oldList[index];
			oldList[index] = NULL;
		}
		else
		{
			ip = new ItemPanel(m_swItemList, list[x]->getItemInfo(), (list.size() == x+1));
		}

		m_pSWSizer->Add(ip, 1, wxEXPAND|wxTOP|wxRIGHT, 5 );
		m_vPanelList.push_back(ip);
	}

	m_pSWSizer->Layout();
	Layout();

	for (size_t x=0; x<oldList.size(); x++)
	{
		if (oldList[x])
			oldList[x]->Destroy();
	}
}

void InstallWaitPage::init()
{
	UserCore::Item::ItemHandleI* item = getItemHandle();
	UserCore::Item::ItemTaskGroupI* group = item->getTaskGroup();

	const wchar_t* action = L"#IF_WAIT_ACTION_UNINSTALL";

	if (group->getAction() == UserCore::Item::ItemTaskGroupI::A_VERIFY)
		action = L"#IF_WAIT_ACTION_VERIFY";

	m_labInfo->SetLabel(gcWString(Managers::GetString(L"#IF_WAIT_INFO"), item->getItemInfo()->getName(), Managers::GetString(action)));
	m_labInfo->Wrap(350);

	std::vector<UserCore::Item::ItemHandleI*> list;
	group->getItemList(list);

	for (size_t x=0; x<list.size(); x++)
	{
		ItemPanel* ip = new ItemPanel(m_swItemList, list[x]->getItemInfo(), (list.size() == x+1));
		m_pSWSizer->Add(ip, 1, wxEXPAND|wxRIGHT, 5 );

		m_vPanelList.push_back(ip);
	}

	m_swItemList->Layout();
	m_pSWSizer->Fit(m_swItemList);

	Layout();
}

void InstallWaitPage::onButtonPressed(wxCommandEvent& event)
{
	if (m_butCancelAll->GetId() == event.GetId())
	{
		UserCore::Item::ItemTaskGroupI* group = getItemHandle()->getTaskGroup();

		if (group)
			group->cancelAll();
	}
	else if (m_butForce->GetId() == event.GetId())
	{
		getItemHandle()->force();
	}
	else if (m_butHide->GetId() == event.GetId())
	{
		GetParent()->Close();
	}
}

}
}
}
