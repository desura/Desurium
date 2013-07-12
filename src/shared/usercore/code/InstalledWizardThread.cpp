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
#include "InstalledWizardThread.h"
#include "InstallInfo.h"
#include "ItemTaskGroup.h"
#include "ItemManager.h"

#include "User.h"

//This is also defined in CIPGrid in uicore
#define CREATE_CIPITEMINFO "create table cipiteminfo(internalid INTEGER PRIMARY KEY, name TEXT);"
#define COUNT_CIPITEMINFO "select count(*) from sqlite_master where name='cipiteminfo';"

#include "sqlite3x.hpp"
#include "sql/CustomInstallPathSql.h"

namespace UserCore
{
namespace Thread
{

InstalledWizardThread::InstalledWizardThread() : MCFThread( "Installed Wizard Thread" )
{
	m_pUser = dynamic_cast<UserCore::User*>(getUserCore());
	m_bTriggerNewEvent = true;
	m_pTaskGroup = NULL;

	m_uiDone = 0;
	m_uiTotal = 0;
}

InstalledWizardThread::~InstalledWizardThread()
{
	stop();
}


void InstalledWizardThread::doRun()
{
	m_szDbName = getCIBDb(getUserCore()->getAppDataPath());

	tinyxml2::XMLDocument doc;
	getWebCore()->getInstalledItemList(doc);


	int ver = XML::processStatus(doc, "itemwizard");

	tinyxml2::XMLElement *infoNode = doc.FirstChildElement("itemwizard");

	if (!infoNode)
		throw gcException(ERR_BADXML);

	WildcardManager wMng = WildcardManager();
	wMng.onNeedSpecialEvent += delegate(&onNeedWCEvent);
	wMng.onNeedSpecialEvent += delegate(getUserCore()->getNeedWildCardEvent());
	

	if (isStopped())
		return;

	parseItemsQuick(infoNode);
	
	MCFCore::Misc::ProgressInfo pi = MCFCore::Misc::ProgressInfo();
	pi.percent = 0;
	onMcfProgressEvent(pi);

	if (ver == 1)
		parseItems1(infoNode, &wMng);
	else
		parseItems2(infoNode, &wMng);

	if (m_pTaskGroup)
	{
		m_pTaskGroup->finalize();
		m_pTaskGroup = NULL;
	}

	try
	{
		createCIPDbTables(getUserCore()->getAppDataPath());

		sqlite3x::sqlite3_connection db(m_szDbName.c_str());
		sqlite3x::sqlite3_command cmd(db, "REPLACE INTO cipiteminfo (internalid, name) VALUES (?,?);");

		for (size_t x=0; x<m_vGameList.size(); x++)
		{
			cmd.bind(1, (long long int)m_vGameList[x].getId().toInt64());
			cmd.bind(2, std::string(m_vGameList[x].getName()) ); 
			cmd.executenonquery();
		}
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to update cip item list: {0}\n", e.what()));
	}


	pi.percent = 100;
	onMcfProgressEvent(pi);

	uint32 prog = 0;
	onCompleteEvent(prog);
}


void InstalledWizardThread::onGameFound(UserCore::Misc::InstallInfo &game)
{
	if (isStopped())
		return;	

	m_vGameList.push_back(game);
	DesuraId id = game.getId();

	UserCore::Item::ItemInfoI *info = getUserCore()->getItemManager()->findItemInfo(id);
	if (info && (info->getStatus() & (UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_READY)))
		return;

	if (game.isInstalled())
	{
		try
		{
			WildcardManager tempwMng;
			tempwMng.onNeedSpecialEvent += delegate(&onNeedWCEvent);
			tempwMng.onNeedSpecialEvent += delegate(getUserCore()->getNeedWildCardEvent());

			getUserCore()->getItemManager()->retrieveItemInfo(id, 0, &tempwMng);

			UserCore::Item::ItemInfoI *item = getUserCore()->getItemManager()->findItemInfo(id);
			if ((!item)|| (item->getParentId().isOk() && !getUserCore()->getItemManager()->isInstalled(item->getParentId())))
				return;

			onItemFound(item);
		}
		catch (gcException &except)
		{
			Warning(gcString("Installed wizard had an error in checking games: {0}\n", except));
		}
	}
}

void InstalledWizardThread::onModFound(UserCore::Misc::InstallInfo &mod)
{
	if (isStopped())
		return;

	DesuraId id = mod.getId();

	UserCore::Item::ItemInfoI *info = getUserCore()->getItemManager()->findItemInfo(id);
	if (info && (info->getStatus() & (UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_READY)))
		return;

	if (mod.isInstalled())
	{
		try
		{
			WildcardManager tempwMng;
			tempwMng.onNeedSpecialEvent += delegate(&onNeedWCEvent);
			tempwMng.onNeedSpecialEvent += delegate(getUserCore()->getNeedWildCardEvent());

			getUserCore()->getItemManager()->retrieveItemInfo(id, 0, &tempwMng);


			UserCore::Item::ItemInfoI *item = getUserCore()->getItemManager()->findItemInfo(id);
			if ((!item ) || (item->getParentId().isOk() && !getUserCore()->getItemManager()->isInstalled(item->getParentId())))
				return;

			if (selectBranch(item))
			{
				//we have to do this here as the item info above doesnt save this flag for downloadable items.
				item->addSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLED);
				onItemFound(item);
			}
		}
		catch (gcException &except)
		{
			Warning(gcString("Installed wizard had an error in checking mods: {0}\n", except));
		}
	}
}

bool InstalledWizardThread::selectBranch(UserCore::Item::ItemInfoI *item)
{
	std::vector<uint32> vBranchIdList;

	for (uint32 x=0; x<item->getBranchCount(); x++)
	{
		UserCore::Item::BranchInfoI* bi = item->getBranch(x);

		if (!bi)
			continue;

		uint32 flags = bi->getFlags();

		if (HasAnyFlags(flags, UserCore::Item::BranchInfoI::BF_NORELEASES|UserCore::Item::BranchInfoI::BF_DEMO|UserCore::Item::BranchInfoI::BF_TEST))
			continue;

		if (!HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_ONACCOUNT))
		{
			if (HasAnyFlags(flags, (UserCore::Item::BranchInfoI::BF_MEMBERLOCK|UserCore::Item::BranchInfoI::BF_REGIONLOCK)))
				continue;
		}

		if (!HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_ONACCOUNT) && !(HasAllFlags(flags, UserCore::Item::BranchInfoI::BF_FREE)))
			continue;

		vBranchIdList.push_back(bi->getBranchId());
	}

	if (vBranchIdList.size() == 0)
		return false;
	
	//select the first free branch
	item->setInstalledMcf(MCFBranch::BranchFromInt(vBranchIdList[0]));
	return true;
}

void InstalledWizardThread::parseGameQuick(tinyxml2::XMLElement* game)
{
	if (!game)
		return;

	m_uiTotal++;

	XML::for_each_child("mod", game->FirstChildElement("mods"), [&](tinyxml2::XMLElement*)
	{
		m_uiTotal++;
	});
}

void InstalledWizardThread::parseItemsQuick(tinyxml2::XMLNode *fNode)
{
	if (!fNode)
		return;

	tinyxml2::XMLElement* platforms = fNode->FirstChildElement("platforms");

	if (platforms)
	{
		XML::for_each_child("platform", platforms, [&](tinyxml2::XMLElement* platform)
		{
			if (m_pUser->platformFilter(platform, PT_Item))
				return;

			parseItemsQuick(platform);
		});
	}
	else
	{
		XML::for_each_child("game", fNode->FirstChildElement("games"), [&](tinyxml2::XMLElement* game)
		{
			const char* id = game->Attribute("siteareaid");
			DesuraId gameId(id, "games");

			if (gameId.isOk())
				parseGameQuick(game);
		});
	}
}

void InstalledWizardThread::triggerProgress()
{
	MCFCore::Misc::ProgressInfo pi = MCFCore::Misc::ProgressInfo();

	if (m_uiTotal != 0)
	{
		pi.percent = (uint8)(m_uiDone*100/m_uiTotal)+1;
		onMcfProgressEvent(pi);
	}
}

void InstalledWizardThread::parseGame(DesuraId id, tinyxml2::XMLElement* game, WildcardManager *pWildCard, tinyxml2::XMLElement* info)
{
	pWildCard->updateInstallWildcard("INSTALL_PATH", "INSTALL_PATH");
	pWildCard->updateInstallWildcard("PARENT_INSTALL_PATH", "%INSTALL_PATH%");


	gcString name;
	XML::GetChild("name", name, game);

	if (name == "")
		XML::GetChild("name", name, info);
	
	if (name == "")
		return;

	if (m_bTriggerNewEvent)
		onNewItemEvent(name);

	triggerProgress();
	m_uiDone++;

	UserCore::Misc::InstallInfo temp(id);

	try
	{
		if (info)
			temp.loadXmlData(info, pWildCard);

		temp.loadXmlData(game, pWildCard);
	}
	catch (gcException &except)
	{
		Warning(gcString("ItemWizard: Error in xml parsing (installed wizard, games): {0}\n", except));
		return;
	}
		
	if (m_bTriggerNewEvent)
		onGameFound(temp);
	else
		m_vGameList.push_back(temp);

	pWildCard->updateInstallWildcard("INSTALL_PATH", "INSTALL_PATH");
	pWildCard->updateInstallWildcard("PARENT_INSTALL_PATH", temp.getPath());

	std::map<uint64, tinyxml2::XMLElement*> mModMap;

	XML::for_each_child("mods", info->FirstChildElement("mods"), [&mModMap](tinyxml2::XMLElement* mod)
	{
		const char* szId = mod->Attribute("siteareaid");
		DesuraId internId(szId, "mods");

		if (internId.isOk())
			mModMap[internId.toInt64()] = mod;
	});

	XML::for_each_child("mods", game->FirstChildElement("mods"), [&](tinyxml2::XMLElement* mod)
	{
		const char* szId = mod->Attribute("siteareaid");
		DesuraId internId(szId, "mods");

		if (internId.isOk())
			parseMod(id, internId, mod, pWildCard, mModMap[internId.toInt64()]);
	});
}

void InstalledWizardThread::parseMod(DesuraId parId, DesuraId id, tinyxml2::XMLElement* mod, WildcardManager *pWildCard, tinyxml2::XMLElement* info)
{
	gcString name;
	XML::GetChild("name", name, mod);

	if (name == "")
		XML::GetChild("name", name, info);
	
	if (name == "")
		return;

	if (m_bTriggerNewEvent)
		onNewItemEvent(name);

	triggerProgress();

	m_uiDone++;

	UserCore::Misc::InstallInfo temp(id, parId);

	try
	{
		temp.loadXmlData(mod, pWildCard);

		if (m_bTriggerNewEvent)
			onModFound(temp);
	}
	catch (gcException &except)
	{
		Warning(gcString("ItemWizard: Error in xml parsing (installed wizard, mods): {0}\n", except));
	}
}

void InstalledWizardThread::parseItems1(tinyxml2::XMLNode *fNode, WildcardManager *pWildCard, std::map<uint64, tinyxml2::XMLElement*> *vMap)
{
	assert(pWildCard);

	if (!fNode)
		return;

	XML::for_each_child("game", fNode->FirstChildElement("games"), [&](tinyxml2::XMLElement* game)
	{
		const char* id = game->Attribute("siteareaid");
		DesuraId gameId(id, "games");

		if (!gameId.isOk())
			return;

			tinyxml2::XMLElement* info = NULL;

		if (vMap)
			info = vMap->operator[](gameId.toInt64());

		parseGame(gameId, game, pWildCard, info);
	});
}

void InstalledWizardThread::parseItems2(tinyxml2::XMLNode *fNode, WildcardManager *pWildCard)
{
	assert(pWildCard);

	if (!fNode)
		return;

	std::map<uint64, tinyxml2::XMLElement*> vMap;

	XML::for_each_child("game", fNode->FirstChildElement("games"), [&](tinyxml2::XMLElement* game)
	{
		const char* id = game->Attribute("siteareaid");
		DesuraId gameId(id, "games");

		if (gameId.isOk())
			vMap[gameId.toInt64()] = game;
	});

	XML::for_each_child("platform", fNode->FirstChildElement("platforms"), [&](tinyxml2::XMLElement* platform)
	{
		if (m_pUser->platformFilter(platform, PT_Item))
			return;

		WildcardManager wm(pWildCard);
		tinyxml2::XMLElement *wildCardNode = platform->FirstChildElement("wcards");

		if (wildCardNode)
		{
			wm.parseXML(wildCardNode);
			wm.compactWildCards();
		}

		parseItems1(platform, &wm, &vMap);
	});
}


void InstalledWizardThread::onItemFound(UserCore::Item::ItemInfoI *item)
{
	bool verify = (item->isInstalled() && item->isDownloadable() && !(item->getStatus()&UserCore::Item::ItemInfoI::STATUS_LINK));

	if (verify)
	{
		if (!m_pTaskGroup)
		{
			UserCore::ItemManager* im = dynamic_cast<UserCore::ItemManager*>(getUserCore()->getItemManager());
			assert(im);
			m_pTaskGroup = im->newTaskGroup(UserCore::Item::ItemTaskGroupI::A_VERIFY);
			m_pTaskGroup->start();
		}

		item->addSFlag(UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER|UserCore::Item::ItemInfoI::STATUS_VERIFING);

		UserCore::Item::ItemHandleI* handle = getUserCore()->getItemManager()->findItemHandle(item->getId());
		handle->setPauseOnError(true);

		m_pTaskGroup->addItem(item);
	}

	DesuraId id = item->getId();
	onItemFoundEvent(id);
}


}
}
