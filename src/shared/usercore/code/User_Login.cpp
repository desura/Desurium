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
#include "User.h"

#ifdef WIN32
#include "shlobj.h"
#include "GameExplorerManager.h"
#endif

#include "sqlite3x.hpp"

#include "util_thread/ThreadPool.h"

#include <branding/usercore_version.h>
#include "UpdateThread.h"

#include "UserIPCPipeClient.h"
#include "IPCServiceMain.h"

#include "UserTasks.h"
#include "DownloadUpdateTask.h"
#include "UserThreadManager.h"
#include "UploadManager.h"
#include "ItemManager.h"

namespace UM = UserCore::Misc;

#define CREATE_ACCOLADES "create table accolades(id INTEGER PRIMARY KEY AUTOINCREMENT, internalid INTEGER, desuraid INTEGER, shortname TEXT, fullname TEXT, description TEXT, lastvalue INTEGER DEFAULT 0, newvalue INTEGER DEFAULT 0, maxvalue INTEGER DEFAULT 1, groupid INTEGER);"
#define COUNT_ACCOLADES "select count(*) from sqlite_master where name='accolades';"

#define CREATE_ACCOLADEGROUPS "create table accoladegroups(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT);"
#define COUNT_ACCOLADEGROUPS "select count(*) from sqlite_master where name='accoladegroups';"


#define CREATE_STATS "create table Stats(id INTEGER PRIMARY KEY AUTOINCREMENT, internalid INTEGER, desuraid INTEGER, name TEXT, type TEXT, value TEXT, dirty INTEGER DEFAULT 0, groupid INTEGER, updatetime DATE);"
#define COUNT_STATS "select count(*) from sqlite_master where name='Stats';"

#define CREATE_STATSGROUPS "create table StatsGroups(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT);"
#define COUNT_STATSGROUPS "select count(*) from sqlite_master where name='StatsGroups';"

namespace UserCore
{

void User::logOut(bool delAutoLogin, bool reInit)
{
	cleanUp();

	if (reInit)
		init();

	if (delAutoLogin)
	{
		UTIL::FS::Path path(gcString(UTIL::OS::getLocalAppDataPath()), "userinfo.upi", false);
		UTIL::FS::delFile(path);
	}
}

void User::initPipe()
{
	if (m_pPipeClient)
		return;

	m_pPipeClient = new UserIPCPipeClient(getUserName(), getAppDataPath(), true);
	m_pPipeClient->onDisconnectEvent += delegate(&onPipeDisconnect);

	size_t x=0;

	do
	{
		try
		{
			m_pPipeClient->start();
			break;
		}
		catch (gcException &)
		{
			if (x > 5)
			{
				logOut();
				throw;
			}
			else
			{
				gcSleep(100);
				x++;
			}
		}
	}
	while (true);
}

void User::logInCleanUp()
{
	m_pThreadPool->unBlockTasks();

	m_pItemManager->loadItems();
	m_pItemManager->enableSave();

	initPipe();
}

void User::logIn(const char* user, const char* pass)
{
	doLogIn(user, pass, false);
}

void User::logInTool(const char* user, const char* pass)
{
	doLogIn(user, pass, true);
}

void User::doLogIn(const char* user, const char* pass, bool bTestOnly)
{
	m_pThreadPool->unBlockTasks();

	if (!m_pWebCore)
		throw gcException(ERR_NULLWEBCORE);

	tinyxml2::XMLDocument doc;
	m_pWebCore->logIn(user, pass, doc);

	tinyxml2::XMLElement *uNode = doc.FirstChildElement("memberlogin");

	uint32 version = 0;
	XML::GetAtt("version", version, uNode);

	if (version == 0)
		version = 1;

	m_bDelayLoading = (version >= 3);

	if (!uNode)
	{
		logOut();
		throw gcException(ERR_BADXML);
	}

	tinyxml2::XMLElement *memNode = uNode->FirstChildElement("member");
	
	if (memNode)
	{

		const char* idStr =  memNode->Attribute("siteareaid");

		if (idStr)
		{
			m_iUserId = atoi(idStr);

			if ((int)m_iUserId <= 0)
			{
				logOut();
				throw gcException(ERR_BAD_PORU);
			}
		}
		else
		{
			logOut();
			throw gcException(ERR_BAD_PORU);
		}
	}

	XML::GetChild("admin", m_bAdmin,  memNode);
	XML::GetChild("name", m_szUserName,  memNode);
	XML::GetChild("nameid", m_szUserNameId,  memNode);
	XML::GetChild("url", m_szProfileUrl,  memNode);
	XML::GetChild("urledit", m_szProfileEditUrl,  memNode);

	if (bTestOnly)
		return;

	gcString appDataPath = UTIL::OS::getAppDataPath();

	initPipe();

#ifdef WIN32
	if (getServiceMain())
		getServiceMain()->fixFolderPermissions(appDataPath.c_str());

	try
	{
		testMcfCache();
	}
	catch (...)
	{
		if (getServiceMain())
			getServiceMain()->fixFolderPermissions(m_szMcfCachePath.c_str());

		try
		{
			testMcfCache();
		}
		catch (gcException &e)
		{
			Warning(gcString("Failed to set mcf cache path to be writeable: {0}", e));
		}
	}
#endif

	gcString szAvatar;
	XML::GetChild("avatar", szAvatar, memNode);

	m_pThreadPool->queueTask(new UserCore::Task::DownloadAvatarTask(this, szAvatar.c_str(), m_iUserId) );


	tinyxml2::XMLElement *msgNode = memNode->FirstChildElement("messages");
	if (msgNode)
	{
		XML::GetChild("updates", m_iUpdates, msgNode);
		XML::GetChild("privatemessages", m_iPms, msgNode);
		XML::GetChild("cart", m_iCartItems, msgNode);
		XML::GetChild("threadwatch", m_iThreads, msgNode);
	}

	m_pToolManager->loadItems();
	m_pItemManager->loadItems();

	try
	{
		if (m_bDelayLoading)
		{
			//do nothing as the update thread will grab it
		}
		else if (version == 2)
		{
			m_pItemManager->parseLoginXml2(memNode->FirstChildElement("games"), memNode->FirstChildElement("platforms"));
		}
		else
		{
			m_pItemManager->parseLoginXml(memNode->FirstChildElement("games"), memNode->FirstChildElement("developer"));
		}
	}
	catch (gcException &)
	{
		logOut();
		throw;
	}

	tinyxml2::XMLElement *newsNode = memNode->FirstChildElement("news");
	if (newsNode)
		parseNews(newsNode);

	tinyxml2::XMLElement *giftsNode = memNode->FirstChildElement("gifts");
	if (giftsNode)
		parseGifts(giftsNode);

	m_pUThread = m_pThreadManager->newUpdateThread(&onForcePollEvent, m_bDelayLoading);
	m_pUThread->start();

#ifdef WIN32
	m_pGameExplorerManager->loadItems();
#endif

	if (getServiceMain())
		getServiceMain()->updateShortCuts();

	if (!m_bDelayLoading)
		m_pItemManager->enableSave();

#ifdef WIN32
#ifdef DEBUG
	gcString val("\"{0}\"", UTIL::OS::getCurrentDir(L"desura-d.exe"));
#else
	gcString val("\"{0}\"", UTIL::OS::getCurrentDir(L"desura.exe"));
#endif

	val +=  " \"%1\" -urllink";
	updateRegKey("HKEY_CLASSES_ROOT\\Desura\\shell\\open\\command\\", val.c_str());
#endif
}


}
