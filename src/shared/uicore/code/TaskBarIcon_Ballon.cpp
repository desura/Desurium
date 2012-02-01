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
#include "TaskBarIcon.h"

#include "MainApp.h"
#include "ButtonStrip.h"

#include "../../../branding/branding.h"

enum
{
	BALLON_NONE = 0,
	BALLON_GIFTS,
	BALLON_APPUPDATE,
	BALLON_GAMEUPDATE,
};

bool sortGifts(UserCore::Misc::NewsItem* left, UserCore::Misc::NewsItem* right) 
{ 
	return strcmp(left->szTitle.c_str(), right->szTitle.c_str()) > 0;
}

bool sortItems(UserCore::Item::ItemInfoI* left, UserCore::Item::ItemInfoI* right) 
{ 
	return strcmp(left->getName(), right->getName()) > 0;
}


void TaskBarIcon::onBallonClick(wxTaskBarIconEvent &event)
{
	if (m_iLastBallon == BALLON_GIFTS)
	{
		g_pMainApp->showNews();
	}
	else if (m_iLastBallon == BALLON_APPUPDATE)
	{
		g_pMainApp->handleInternalLink(DesuraId(m_AppUpdateVer.build, 0), ACTION_APPUPDATELOG);
	}
	else if (m_iLastBallon == BALLON_GAMEUPDATE)
	{
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Updates).c_str(), COMMUNITY);
	}

	m_iLastBallon = BALLON_NONE;
}

void TaskBarIcon::onAppUpdate(UserCore::Misc::UpdateInfo &info)
{
	gcWString msg(Managers::GetString(L"#TB_APPUPDATE"), info.build);

	m_iLastBallon = BALLON_APPUPDATE;
	m_AppUpdateVer = info;

	ShowBalloon(Managers::GetString(L"#TB_APPUPDATE_TITLE"), msg.c_str());
}

void TaskBarIcon::onAppUpdateComplete(UserCore::Misc::UpdateInfo &info)
{

}

void TaskBarIcon::onAppUpdateProg(uint32& prog)
{
	if (m_uiLastProg == prog)
		return;

	if (prog == 0 || prog >= 100)
		m_szTooltip = PRODUCT_NAME;
	else
		m_szTooltip = gcString(PRODUCT_NAME " - Update Download Progress: {0}%", prog);

#ifdef WIN32
	SetIcon(m_wxIcon, m_szTooltip);
#else
	gcWString path(L"{0}/data/desura.png", UTIL::OS::getCurrentDir());
	wxIcon i(path, wxBITMAP_TYPE_PNG);
	SetIcon(i, m_szTooltip);	
#endif
}


void TaskBarIcon::onUpdate()
{
	doBallonMsg();
	updateIcon();
}

void TaskBarIcon::doBallonMsg()
{
	gcWString msg;

	std::sort(m_vNextUpdateList.begin(), m_vNextUpdateList.end(), sortItems);

	for (size_t x=0; x<m_vNextUpdateList.size(); x++)
	{
		if (!m_vNextUpdateList[x])
			continue;

		msg += gcWString(L" - {0}\n", m_vNextUpdateList[x]->getName());
	}

	if (msg.size() > 0)
	{
		swapUpdateList();
		m_iLastBallon = BALLON_GAMEUPDATE;
		ShowBalloon(Managers::GetString(L"#TB_TITLE"), msg.c_str());
	}
}

void TaskBarIcon::showGiftPopup(std::vector<UserCore::Misc::NewsItem*>& itemList)
{
	gcWString msg;

	std::sort(itemList.begin(), itemList.end(), sortGifts);

	for (size_t x=0; x<itemList.size(); x++)
	{
		if (itemList[x]->hasBeenShown)
			continue;

		msg += gcWString(L" - {0}\n", itemList[x]->szTitle.c_str());
	}

	m_iLastBallon = BALLON_GIFTS;
	ShowBalloon(Managers::GetString(L"#TB_GIFTTITLE"), msg.c_str());
}

bool TaskBarIcon::findUpdateItem(UserCore::Item::ItemInfoI* item)
{
	if (!item)
		return false;

	for (size_t x=0; x<m_vNextUpdateList.size(); x++)
	{
		if (!m_vNextUpdateList[x])
			continue;

		if (m_vNextUpdateList[x] == item)
			return true;
	}

	for (size_t x=0; x<m_vUpdatedList.size(); x++)
	{
		if (!m_vUpdatedList[x])
			continue;

		if (m_vUpdatedList[x] == item)
			return true;
	}

	return false;
}

void TaskBarIcon::removeUpdateItem(UserCore::Item::ItemInfoI* item)
{
	if (!item)
		return;

	for (size_t x=0; x<m_vNextUpdateList.size(); x++)
	{
		if (!m_vNextUpdateList[x])
			continue;

		if (m_vNextUpdateList[x] == item)
		{
			m_vNextUpdateList.erase(m_vNextUpdateList.begin()+x);
			break;
		}
	}

	for (size_t x=0; x<m_vUpdatedList.size(); x++)
	{
		if (!m_vUpdatedList[x])
			continue;

		if (m_vUpdatedList[x] == item)
		{
			m_vUpdatedList.erase(m_vUpdatedList.begin()+x);
			break;
		}
	}
}

void TaskBarIcon::swapUpdateList()
{
	for (size_t x=0; x<m_vNextUpdateList.size(); x++)
	{
		if (!m_vNextUpdateList[x])
			continue;

		m_vUpdatedList.push_back(m_vNextUpdateList[x]);
	}

	m_vNextUpdateList.erase(m_vNextUpdateList.begin(), m_vNextUpdateList.end());
}

void TaskBarIcon::onItemChanged(UserCore::Item::ItemInfoI::ItemInfo_s& info)
{
	if (!GetUserCore())
		return;

	if (info.changeFlags & UserCore::Item::ItemInfoI::CHANGED_STATUS)
	{
		UserCore::Item::ItemInfoI* item = GetUserCore()->getItemManager()->findItemInfo(info.id);
	
		if (!item)
			return;

		if (item->getStatus() & UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL)
		{
			if (!findUpdateItem(item))
				m_vNextUpdateList.push_back(item);
		}
		else
		{
			removeUpdateItem(item);
		}
	}

	updateIcon();
}


void TaskBarIcon::tagItems()
{
	if (!GetUserCore())
		return;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	GetUserCore()->getItemManager()->getGameList(gList, true);

	for (size_t x=0; x<gList.size(); x++)
	{
		UserCore::Item::ItemInfoI* game = gList[x];

		if (game->getStatus() & UserCore::Item::ItemInfoI::STATUS_DELETED || (!(game->getStatus() & (UserCore::Item::ItemInfoI::STATUS_ONACCOUNT|UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER)) && (game->getStatus() & UserCore::Item::ItemInfoI::STATUS_DEVELOPER)))
		{
			*game->getInfoChangeEvent() -= guiDelegate(this, &TaskBarIcon::onItemChanged);
			continue;
		}

		if (!(game->getStatus() & (UserCore::Item::ItemInfoI::STATUS_ONACCOUNT|UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER)))
			continue;

		*game->getInfoChangeEvent() += guiDelegate(this, &TaskBarIcon::onItemChanged);

#ifdef WIN32
		std::vector<UserCore::Item::ItemInfoI*> mList;
		GetUserCore()->getItemManager()->getModList(game->getId(), mList, true);

		for (size_t y=0; y<mList.size(); y++)
		{
			UserCore::Item::ItemInfoI* mod = mList[y];

			if (mod->getStatus() & UserCore::Item::ItemInfoI::STATUS_DELETED || (!(mod->getStatus() & (UserCore::Item::ItemInfoI::STATUS_ONACCOUNT|UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER)) && (mod->getStatus() & UserCore::Item::ItemInfoI::STATUS_DEVELOPER)))
			{
				*mod->getInfoChangeEvent() -= guiDelegate(this, &TaskBarIcon::onItemChanged);
				continue;
			}

			if (!(mod->getStatus() & (UserCore::Item::ItemInfoI::STATUS_ONACCOUNT|UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER)))
				continue;

			*mod->getInfoChangeEvent() += guiDelegate(this, &TaskBarIcon::onItemChanged);
		}
#endif
	}
}
