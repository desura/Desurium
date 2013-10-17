/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Wojciech Zylinski <voitek@boskee.co.uk>

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
#include "InstallBranch.h"

#include "MainApp.h"


BEGIN_EVENT_TABLE( InstallBranch, gcDialog )
	EVT_BUTTON( wxID_ANY, InstallBranch::onButtonClick )
	EVT_CHOICE( wxID_ANY, InstallBranch::onChoice )
END_EVENT_TABLE()


class BranchData : public wxClientData
{
public:
	BranchData(MCFBranch b, MCFBranch g)
	{
		branch = b;
		global = g;
	}

	MCFBranch branch;
	MCFBranch global;
};


InstallBranch::InstallBranch( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : gcDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );


	m_labInfo = new wxStaticText( this, wxID_ANY, Managers::GetString(L"#IF_BRANCHINFO"), wxDefaultPosition, wxDefaultSize, 0 );
	m_labInfo->Wrap( 350 );


	wxArrayString m_cbBranchListChoices;
	m_cbBranchList = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cbBranchListChoices, 0 );
	m_cbBranchList->SetSelection( 0 );

	m_butInfo = new gcButton( this, wxID_ANY, Managers::GetString(L"#MOREINFO"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butInstall = new gcButton( this, wxID_ANY, Managers::GetString(L"#INSTALL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_butCancel = new gcButton( this, wxID_ANY, Managers::GetString(L"#CANCEL"), wxDefaultPosition, wxDefaultSize, 0 );






	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer2->AddGrowableCol( 0 );
	fgSizer2->AddGrowableRow( 0 );
	fgSizer2->AddGrowableRow( 2 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	fgSizer2->Add( m_cbBranchList, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );
	fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	



	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );
	bSizer1->Add( m_butInfo, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer1->Add( m_butInstall, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	bSizer1->Add( m_butCancel, 0, wxALL, 5 );




	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer1->Add( m_labInfo, 0, wxALL, 5 );
	fgSizer1->Add( fgSizer2, 1, wxEXPAND, 5 );
	fgSizer1->Add( bSizer1, 1, wxEXPAND, 5 );
	

	this->SetSizer( fgSizer1 );
	this->Layout();

	centerOnParent();
}

InstallBranch::~InstallBranch()
{
}

void InstallBranch::onButtonClick(wxCommandEvent &event)
{
	if (event.GetId() == m_butInfo->GetId())
	{
		if ((m_bIsMod || m_bIsExpansion) && !m_bSelectBranch)
			g_pMainApp->handleInternalLink(m_Item, ACTION_PROFILE, FormatArgs("?help=needgame"));
		else
			g_pMainApp->handleInternalLink(m_Item, ACTION_PROFILE, FormatArgs("?help=branches"));

		EndModal(wxID_CANCEL);
	}
	else if (event.GetId() == m_butInstall->GetId())
	{
		MCFBranch b = getBranch();

		if (b == UINT_MAX)
		{
			UserCore::Item::ItemInfoI* pItemInfo = GetUserCore()->getItemManager()->findItemInfo(m_Item);

			if ((m_bIsMod || m_bIsExpansion) && !m_bSelectBranch)
				pItemInfo = GetUserCore()->getItemManager()->findItemInfo(pItemInfo->getParentId());

			pItemInfo->addSFlag(UserCore::Item::ItemInfoI::STATUS_LINK|UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_READY);
			EndModal(wxID_OK);
		}
		else if (b == (UINT_MAX - 1))
		{
			g_pMainApp->handleInternalLink(m_Item, ACTION_SHOWSETTINGS, FormatArgs("tab=cip"));
			EndModal(wxID_CANCEL);
		}
		else if (m_bBuy)
		{
			MCFBranch g = getGlobal();
			gcString args;

			if (g == 0 || !g.isGlobal())
				args = gcString("?help=buy&branch={0}", b);
			else
				args = gcString("?help=buy&branchglobal={0}", g);
			
			g_pMainApp->handleInternalLink(m_Item, ACTION_PROFILE, FormatArgs(args));
			EndModal(wxID_CANCEL);
		}
		else
		{
			EndModal(wxID_OK);
		}
	}
	else if (event.GetId() == m_butCancel->GetId())
	{
		EndModal(wxID_CANCEL);
	}
}

void InstallBranch::fixName(gcWString &name)
{
	size_t pos = name.find(L"&");

	while (pos != std::wstring::npos)
	{
		name.insert(name.begin()+pos, L'&');
		pos+=2;

		pos = name.find(L"&", pos);
	}
}

int InstallBranch::setInfo(DesuraId id, bool selectBranch)
{
	m_bSelectBranch = selectBranch;
	UserCore::Item::ItemInfoI* pItemInfo = GetUserCore()->getItemManager()->findItemInfo(id);

	if (!pItemInfo)
		return 1;

	m_bIsMod = id.getType() == DesuraId::TYPE_MOD;
	m_bIsExpansion = m_bIsMod == false && pItemInfo->getParentId().getType() == DesuraId::TYPE_GAME;

	m_Item = id;

	gcWString parName;
	gcWString itemName = pItemInfo->getName();
	
	DesuraId par = pItemInfo->getParentId();
	UserCore::Item::ItemInfoI *parInfo = NULL;
	if (par.isOk())
	{
		parInfo = GetUserCore()->getItemManager()->findItemInfo(par);

		if (parInfo)
			parName = gcWString(parInfo->getName());
	}

	fixName(parName);
	fixName(itemName);

	if (selectBranch == false && m_bIsMod)
	{
		m_labInfo->SetLabel(gcWString(Managers::GetString(L"#IF_NOTFOUND"), itemName, parName));
		m_labInfo->Wrap( 350 );
	}
	else if (selectBranch == false && m_bIsExpansion)
	{
		m_labInfo->SetLabel(gcWString(Managers::GetString(L"#IF_NOTFOUND_GAME"), itemName, parName));
		m_labInfo->Wrap( 350 );
	}
	else
	{
		m_labInfo->SetLabel(gcWString(Managers::GetString(L"#IF_BRANCHINFO"), itemName));
		m_labInfo->Wrap( 350 );
	}

	uint32 count = 0;
	int32 full = -1;
	uint32 fullReadyCount = 0;
	m_bBuy = true;

	UserCore::Item::ItemInfoI *i = pItemInfo;

	bool isCheckingParent = (m_bIsMod || m_bIsExpansion) && !selectBranch;

	if (isCheckingParent)
	{
		if (!parInfo)
		{
			gcMessageBox(GetParent(), Managers::GetString(L"#IF_IIPARENT"), Managers::GetString(L"#IF_IIERRTITLE"));
			return 1;
		}

		i = parInfo;
	}

	std::vector<UserCore::Item::BranchInfoI*> bList;

	for (uint32 x=0; x<i->getBranchCount(); x++)
	{
		UserCore::Item::BranchInfoI* bi = i->getBranch(x);

		if (!bi)
			continue;

		uint32 flags = bi->getFlags();

		bool noRelease = HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_NORELEASES);
		bool isPreorder = bi->isPreOrder();
		bool isDemo = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_DEMO);
		bool onAccount = HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_ONACCOUNT);
		bool locked = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_MEMBERLOCK|UserCore::Item::BranchInfoI::BF_REGIONLOCK);
		bool test = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_TEST);
		bool free = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_FREE);

		if (noRelease && !isPreorder)
			continue;

		if (!onAccount && locked)
			continue;
		
		if (!selectBranch && (isDemo || test))
			continue;

		if ((free || onAccount) && isPreorder && !selectBranch)
			continue;

		bool globalFound = false;

		for (size_t x=0; x<bList.size(); x++)
		{
			if (bList[x]->getGlobalId() == bi->getGlobalId())
			{
				globalFound = true;
				break;
			}
		}

		if (globalFound)
			continue;

		if (isDemo || test)
		{
		}
		else if (full == -1 || ((!m_bBuy || onAccount) && !free))
		{
			if (full == -1 || m_bBuy)
			{
				//if this is the first full game or this is the first full game that you dont have to buy
				m_bBuy = (!onAccount && !free);
				full = count;
			}
			
			if (onAccount || free)
				fullReadyCount++;
		}

		bList.push_back(bi);
		count++;
	}

	for (size_t x=0; x<bList.size(); x++)
	{
		UserCore::Item::BranchInfoI* bi = bList[x];
		gcString name = bi->getName();
		gcWString title;
		
		uint32 flags = bi->getFlags();

		bool noRelease = HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_NORELEASES);
		bool isPreorder = bi->isPreOrder();
		bool onAccount = HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_ONACCOUNT);
		bool free = HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_FREE);

		if (!free)
			title = gcString("{0} - {1}", name, Managers::GetString("#IF_BROUGHT"));
		else
			title = gcString("{0} - {1}", name, Managers::GetString("#IF_FREE"));
		
		if (!free && !onAccount)
		{
			gcWString cost(bi->getCost());

			if (cost == "")
				cost = gcString(Managers::GetString("#TBA"));

			title = gcString("{0} - {1}", name, cost.c_str());
		}
		else if (isPreorder)
		{
			if (noRelease)
				title = gcString("{0} - {1}", name, Managers::GetString("#IF_PREORDERED_NORELEASE"));
			else
				title = gcString("{0} - {1}", name, Managers::GetString("#IF_PREORDERED"));
		}

		m_cbBranchList->Append(title, new BranchData(bi->getBranchId(), bi->getGlobalId()));
	}

	count = bList.size();

	if (full == -1)
		full = 0;

#ifdef WIN32
	if (HasAnyFlags(i->getStatus(), UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER))
		m_cbBranchList->Append(Managers::GetString("#IF_ONCOMPUTER"), new BranchData(MCFBranch::BranchFromInt(-1), MCFBranch::BranchFromInt(-1)));
	else
		m_cbBranchList->Append(Managers::GetString("#IF_FINDONCOMPUTER"), new BranchData(MCFBranch::BranchFromInt(-2), MCFBranch::BranchFromInt(-2)));
#endif

	m_cbBranchList->SetSelection(full);

	if (pItemInfo->getIcon() && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(pItemInfo->getIcon())))
		setIcon(pItemInfo->getIcon());

	SetTitle(gcWString(Managers::GetString(L"#IF_BRANCHTITLE"), itemName));


	uint32 ret = 0;

	if (count == 0)
	{
		if (!isCheckingParent)
			gcMessageBox(GetParent(), Managers::GetString(L"#IF_IINOBRANCHES"), Managers::GetString(L"#IF_IIERRTITLE"));
		else if (selectBranch)
			ret = 1;
	}
	else if ((count == 1 || fullReadyCount == 1) && !m_bBuy)
	{	
		ret =  1;
	}
	else
	{
		wxCommandEvent e;
		onChoice(e);
	}

	return ret;
}

void InstallBranch::onChoice(wxCommandEvent& event)
{
	MCFBranch b = getBranch();

	if (b == UINT_MAX || b == (UINT_MAX - 1))
	{
		if (b == UINT_MAX)
			m_butInstall->SetLabel(Managers::GetString(L"#PLAY"));
		else
			m_butInstall->SetLabel(Managers::GetString(L"#FIND"));

		m_butInstall->Enable(true);
		return;
	}

	UserCore::Item::ItemInfoI* pItemInfo = GetUserCore()->getItemManager()->findItemInfo(m_Item);

	if ((m_bIsMod || m_bIsExpansion) && !m_bSelectBranch)
		pItemInfo = GetUserCore()->getItemManager()->findItemInfo(pItemInfo->getParentId());

	UserCore::Item::BranchInfoI* bi = pItemInfo->getBranchById(b);

	m_bBuy = (!(bi->getFlags()&UserCore::Item::BranchInfoI::BF_ONACCOUNT) && !(bi->getFlags()&UserCore::Item::BranchInfoI::BF_FREE));

	if (m_bBuy)
		m_butInstall->SetLabel(Managers::GetString(L"#PURCHASE"));
	else
		m_butInstall->SetLabel(Managers::GetString(L"#INSTALL"));

	bool noRelease = HasAllFlags(bi->getFlags(), UserCore::Item::BranchInfoI::BF_NORELEASES);
	bool isPreorder = bi->isPreOrder();
	bool onAccount = HasAllFlags(bi->getFlags(), UserCore::Item::BranchInfoI::BF_ONACCOUNT);

	m_butInstall->Enable(!(noRelease && isPreorder && onAccount));
}

MCFBranch InstallBranch::getBranch()
{
	wxClientData* val = m_cbBranchList->GetClientObject(m_cbBranchList->GetSelection());
	BranchData* b = dynamic_cast<BranchData*>(val);

	if (b)
		return b->branch;

	return MCFBranch();
}

MCFBranch InstallBranch::getGlobal()
{
	wxClientData* val = m_cbBranchList->GetClientObject(m_cbBranchList->GetSelection());
	BranchData* b = dynamic_cast<BranchData*>(val);

	if (b)
		return b->global;

	return MCFBranch();
}

