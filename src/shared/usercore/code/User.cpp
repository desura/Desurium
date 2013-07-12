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
#include <shlobj.h>
#endif

#include "UpdateThread.h"

#include <branding/usercore_version.h>

#include "UserIPCPipeClient.h"
#include "IPCServiceMain.h"

#include "util_thread/BaseThread.h"
#include "util_thread/ThreadPool.h"
#include "DownloadUpdateTask.h"
#include "UserTasks.h"
#include "UserThreadManager.h"
#include "UploadManager.h"

#ifdef WIN32
#include "GameExplorerManager.h"
#endif

#include "sqlite3x.hpp"

#include "BDManager.h"
#include "McfManager.h"

namespace UM = UserCore::Misc;

namespace UserCore
{

User::User()
{
	m_bLocked = false;
	m_pWaitCond = new ::Thread::WaitCondition();
	
	m_pPipeClient = NULL;
	m_pThreadPool = NULL;
	m_pWebCore = NULL;
	m_pThreadManager = NULL;
	m_pUploadManager = NULL;
	m_pUThread = NULL;
	m_pItemManager = NULL;
	m_pToolManager = NULL;
	m_pGameExplorerManager = NULL;
	m_pCDKeyManager = NULL;
	m_pBannerDownloadManager = NULL;
	m_pCIPManager = NULL;
	
	m_bAdmin = false;
	m_bDelayLoading = false;
	m_bDownloadingUpdate = false;

	m_iCartItems = 0;
	m_iUserId = 0;
	m_iUpdates = 0;
	m_iPms = 0;
	m_iThreads = 0;

	m_iSelectedIndex = 0;
	m_uiLastUpdateBuild = 0;
	m_uiLastUpdateVer = 0;	

	onLoginItemsLoadedEvent += delegate(this, &User::onLoginItemsLoaded);
}

User::~User() 
{
	while (m_bLocked)
	{
		m_pWaitCond->wait(0, 500);
	}

	cleanUp();

	onNeedWildCardEvent -= delegate(this, &User::onNeedWildCardCB);

	safe_delete(m_pThreadPool);
	safe_delete(m_pWaitCond);

	m_pWebCore->destroy();
	m_pWebCore = NULL;

	DelMCFManager();
}

void User::onLoginItemsLoaded()
{
	m_bDelayLoading = false;
}

void User::lockDelete()
{
	m_bLocked = true;
}

void User::unlockDelete()
{
	m_bLocked = false;
	m_pWaitCond->notify();
}

void User::init(const char* appDataPath)
{
	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(appDataPath));

	m_szAppDataPath = appDataPath;

	m_pThreadPool = new ::Thread::ThreadPool(2);
	m_pThreadPool->blockTasks();

	m_pWebCore = (WebCore::WebCoreI*)WebCore::FactoryBuilder(WEBCORE);
	m_pWebCore->init(appDataPath);

	m_pBannerDownloadManager = new BDManager(this);
	m_pCDKeyManager = new CDKeyManager(this);

	m_szMcfCachePath = UTIL::OS::getMcfCachePath();

	InitMCFManager(appDataPath, m_szMcfCachePath.c_str());
	init();
}

const char* User::getAppDataPath()
{
	return m_szAppDataPath.c_str();
}

const char* User::getMcfCachePath()
{
	return m_szMcfCachePath.c_str();
}

void User::cleanUp()
{
	m_pThreadPool->purgeTasks();
	m_pThreadPool->blockTasks();
	m_pWebCore->logOut();

	//must delete this one first as upload threads are apart of threadmanager
	safe_delete(m_pUploadManager);
	safe_delete(m_pUThread);
	safe_delete(m_pThreadManager);
#ifdef WIN32
	safe_delete(m_pGameExplorerManager);
#endif
	safe_delete(m_pCIPManager);
	safe_delete(m_pItemManager);
	safe_delete(m_pToolManager);
	
	safe_delete(m_pPipeClient);

	safe_delete(m_pCDKeyManager);
	safe_delete(m_pBannerDownloadManager);

	m_szUserName = gcString("");
	m_szUserNameId = gcString("");
	m_szAvatar = gcString("");
	m_bDelayLoading = false;
}

void User::init()
{
	m_pUploadManager = new UserCore::UploadManager(this);
	m_pThreadManager = new UserCore::UserThreadManager();
	m_pThreadManager->setUserCore(this);

	m_iUserId = 0;
	m_pUThread = NULL;
	m_bAdmin = false;
	m_bDelayLoading = false;

	m_uiLastUpdateVer = 0;
	m_uiLastUpdateBuild = 0;

	m_pItemManager = new ItemManager(this);
	m_pToolManager = new ToolManager(this);

#ifdef WIN32
	m_pGameExplorerManager = new GameExplorerManager(this);
#endif

	m_pCIPManager = new CIPManager(this);
	m_pPipeClient = NULL;

	m_bDownloadingUpdate = false;
	onNeedWildCardEvent += delegate(this, &User::onNeedWildCardCB);

	m_iThreads = 0;
	m_iUpdates = 0;
	m_iPms = 0;
	m_iCartItems = 0;
#ifdef WIN32
	m_WinHandle = NULL;
#endif
}

//call this if a update needs to be downloaded
void User::appNeedUpdate(uint32 appver, uint32 appbuild)
{
	if (m_bDownloadingUpdate)
		return;

	if (appver == m_uiLastUpdateVer && appbuild <= m_uiLastUpdateBuild)
		return;

	m_bDownloadingUpdate = true;

	std::string szAppid = UTIL::OS::getConfigValue(APPID);

	if (m_uiLastUpdateVer == 0)
	{
		if (szAppid.size() > 0)
			m_uiLastUpdateVer = atoi(szAppid.c_str());
		
		if (m_uiLastUpdateVer == 0)
			m_uiLastUpdateVer = 100;
	}

	if (m_uiLastUpdateBuild == 0)
	{
		std::string szAppBuild = UTIL::OS::getConfigValue(APPBUILD);

		if (szAppBuild.size() > 0)
			m_uiLastUpdateBuild = atoi(szAppBuild.c_str());

		if (m_uiLastUpdateBuild == 0) 
			m_uiLastUpdateBuild = 0;
	}

	uint32 curAppVer=0;

	if (szAppid.size() > 0)
		curAppVer = atoi(szAppid.c_str());

	//if we changed the appver dont keep downloading updates for the old one
	if (appver == 0 && m_uiLastUpdateVer != curAppVer)
	{
		m_bDownloadingUpdate = false;
		return;
	}

	if (appver != 0)
	{
		m_uiLastUpdateVer = appver;	
		m_uiLastUpdateBuild = 0;
	}

	UserCore::Task::DownloadUpdateTask *task = new UserCore::Task::DownloadUpdateTask(this, m_uiLastUpdateVer, m_uiLastUpdateBuild);

	task->onDownloadCompleteEvent += delegate(this, &User::onUpdateComplete);
	task->onDownloadStartEvent += delegate(this, &User::onUpdateStart);
	task->onDownloadProgressEvent += delegate(getAppUpdateProgEvent());

	m_pThreadPool->forceTask(task);
}

const char* User::getCVarValue(const char* cvarName)
{
	if (!cvarName)
		return NULL;
		
	UserCore::Misc::CVar_s temp;
	temp.name = cvarName;
	temp.value = NULL;

	onNeedCvarEvent(temp);
	return temp.value;
}

void User::onUpdateComplete(UserCore::Misc::update_s& info)
{
	m_uiLastUpdateBuild = info.build;
	m_bDownloadingUpdate = false;
	
	if (info.alert)
	{
		UserCore::Misc::UpdateInfo uLast(m_uiLastUpdateVer, m_uiLastUpdateBuild);
		onAppUpdateCompleteEvent(uLast);
	}
}

void User::onUpdateStart(UserCore::Misc::update_s& info)
{
	m_uiLastUpdateBuild = info.build;
	
	if (info.alert)
	{
		UserCore::Misc::UpdateInfo uLast(m_uiLastUpdateVer, m_uiLastUpdateBuild);
		onAppUpdateEvent(uLast);
	}
}

void User::onNeedWildCardCB(WCSpecialInfo& info)
{
	if (info.handled)
		return;

#ifdef WIN32
	if (Safe::stricmp("PROGRAM_FILES", info.name.c_str()) == 0)
	{
		wchar_t path[MAX_PATH]  = {0};
		SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, path);
		info.result = path;
		info.handled = true;
	}
	else if (Safe::stricmp("DOCUMENTS", info.name.c_str()) == 0)
	{
		wchar_t path[MAX_PATH] = {0};
		SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
		info.result = path;
		info.handled = true;
	}
	else if (Safe::stricmp("JAVA_PATH", info.name.c_str()) == 0)
	{
		std::string cur = UTIL::WIN::getRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Runtime Environment\\CurrentVersion");

		if (cur.size() > 0)
		{
			info.result = UTIL::WIN::getRegValue(gcString("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Runtime Environment\\{0}\\JavaHome", cur));
			info.handled = true;
		}
	}
	else if (Safe::stricmp("APP_DATA", info.name.c_str())==0)
	{
		wchar_t path[MAX_PATH]  = {0};
		SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
		info.result = path;
		info.handled = true;
	}
	else if (Safe::stricmp("USER_APP_DATA", info.name.c_str())==0)
	{
		wchar_t path[MAX_PATH]  = {0};
		SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
		info.result = path;
		info.handled = true;
	}
#else
	if (Safe::stricmp("XDG_OPEN", info.name.c_str()) == 0)
	{
		info.result = gcString(UTIL::OS::getCurrentDir(L"xdg-open"));
		info.handled = true;
	}
#endif
	else if (Safe::stricmp("APPLICATION", info.name.c_str()) == 0)
	{
		info.result = UTIL::OS::getAppInstallPath();
		info.handled = true;
	}
}



void User::setAvatarPath(const char* path)
{
	m_szAvatar = gcString(path);
	onNewAvatarEvent(m_szAvatar);
}

void User::downloadImage(UserCore::Item::ItemInfo* itemInfo, uint8 image)
{
	m_pThreadPool->queueTask(new UserCore::Task::DownloadImgTask(this, itemInfo, image));
}

void User::changeAccount(DesuraId id, uint8 action)
{
	m_pThreadPool->queueTask(new UserCore::Task::ChangeAccountTask(this, id, action) );
}



void User::parseNews(tinyxml2::XMLNode* newsNode)
{
	parseNewsAndGifts(newsNode, onNewsUpdateEvent);
}


void User::parseGifts(tinyxml2::XMLNode* giftNode)
{

	parseNewsAndGifts(giftNode, onGiftUpdateEvent);
}

void User::parseNewsAndGifts(tinyxml2::XMLNode* xmlNode, Event<std::vector<UserCore::Misc::NewsItem*> > &onEvent)
{
	if (!xmlNode)
		return;

	std::vector<UserCore::Misc::NewsItem*> itemList;

	tinyxml2::XMLNode* pChild = xmlNode->FirstChild();
	while (pChild)
	{
		if (XML::isValidElement(pChild))
		{
			tinyxml2::XMLElement *itemElem = pChild->ToElement();

			const char* szId = itemElem->Attribute("id");

			gcString szTitle;
			gcString szUrl;

			XML::GetChild("title", szTitle, itemElem);
			XML::GetChild("url", szUrl, itemElem);
			
			if (szId && szTitle != "" && szUrl != "")
			{
				uint32 id = (uint32)atoi(szId);

				UserCore::Misc::NewsItem *temp = new UserCore::Misc::NewsItem(id, 0, szTitle.c_str(), szUrl.c_str());
				itemList.push_back(temp);
			}
		}

		pChild = pChild->NextSibling();
	}

	if (itemList.size() > 0)
		onEvent(itemList);

	safe_delete(itemList);
}

void User::restartPipe()
{
	if (m_pPipeClient)
		m_pPipeClient->restart();
}

void User::forceUpdatePoll()
{
	onForcePollEvent();
}

void User::setCounts(uint32 msgs, uint32 updates, uint32 threads, uint32 cart)
{
	bool needsUpdate = false;

	if (cart != UINT_MAX && m_iCartItems != cart)
	{
		m_iCartItems = cart;
		needsUpdate = true;
	}

	if (msgs != UINT_MAX && m_iPms != msgs)
	{
		m_iPms = msgs;
		needsUpdate = true;
	}

	if (updates != UINT_MAX && m_iUpdates != updates)
	{
		m_iUpdates = updates;
		needsUpdate = true;
	}

	if (threads != UINT_MAX && m_iThreads != threads)
	{
		m_iThreads = threads;
		needsUpdate = true;
	}

	if (needsUpdate)
		onUserUpdateEvent();
}

void User::updateUninstallInfo()
{
	if (getServiceMain())
		getServiceMain()->updateAllUninstallRegKey();
}

void User::updateUninstallInfo(DesuraId id, uint64 installSize)
{
	if (getServiceMain())
		getServiceMain()->setUninstallRegKey(id.toInt64(), installSize);
}

void User::removeUninstallInfo(DesuraId id)
{
	if (getServiceMain())
		getServiceMain()->removeUninstallRegKey(id.toInt64());
}

void User::updateRegKey(const char* key, const char* value)
{
	if (getServiceMain())
		getServiceMain()->updateRegKey(key, value);
}

void User::updateBinaryRegKey(const char* key, const char* value, size_t size)
{
	if (getServiceMain())
		getServiceMain()->updateBinaryRegKey(key, value, size);
}

void User::runInstallScript(const char* file, const char* installPath, const char* function)
{
	if (getServiceMain())
		getServiceMain()->runInstallScript(file, installPath, function);
}

bool User::platformFilter(tinyxml2::XMLElement* platform, PlatformType type)
{
	if (!platform)
		return true;

	const char* szId = platform->Attribute("id");

	if (!szId)
		return true;

	uint32 id = atoi(szId);

	if (id == 0)
		return true;

#ifdef WIN32
	return (id != 100);
#elif defined NIX
	if (type == PT_Tool)
		return (id != 110 && id != 120);
#ifdef NIX64
	if (id == 120)
		return false;
#endif
	//linux will have windows and nix
	return (id != 110); //id != 100 && 
#else
	return true;
#endif
}

void User::testMcfCache()
{
	UTIL::FS::Path p(m_szMcfCachePath, "temp", false);
	UTIL::FS::FileHandle fh(p, UTIL::FS::FILE_WRITE);

	fh.write("1234", 4);
	fh.close();

	UTIL::FS::delFile(p);
}

}
