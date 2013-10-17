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
#include "UninstallAllThread.h"

#include "UserCore\ItemHandleI.h"
#include "UserCore\ItemInfoI.h"

#include "sqlite3x.hpp"
#include "sql\ItemInfoSql.h"

#include "UDFSettingsPanel.h"
#include "webcore\WebCoreI.h"

#include <shlobj.h>

#include "wx_controls/gcManagers.h"

UninstallAllThread::UninstallAllThread(uint32 flags, UserCore::UserI* user) : Thread::BaseThread("UninstallAll Thread")
{
	m_iFlags = flags;
	m_pUser = user;
	m_iTotal = 0;
	m_iTotalPos = 0;
}

void UninstallAllThread::run()
{
	if (!m_pUser)
		return;

	bool removeSimple	= HasAnyFlags(m_iFlags, REMOVE_SIMPLE);
	bool removeCache	= HasAnyFlags(m_iFlags, REMOVE_CACHE);
	bool removeSettings = HasAnyFlags(m_iFlags, REMOVE_SETTINGS);

	std::vector<UserCore::Item::ItemHandleI*> uninstallList;

	std::vector<UserCore::Item::ItemInfoI*> gamesList;
	m_pUser->getItemManager()->getGameList(gamesList);

	for (size_t x=0; x<gamesList.size(); x++)
	{
		UserCore::Item::ItemInfoI* game = gamesList[x];
		std::vector<UserCore::Item::ItemInfoI*> modList;

		for (size_t y=0; y<modList.size(); y++)
		{
			if (modList[x]->isInstalled() && (modList[x]->isComplex() || removeSimple))
				uninstallList.push_back(m_pUser->getItemManager()->findItemHandle(modList[x]->getId()));
		}

		if (removeSimple && game->isInstalled())
			uninstallList.push_back(m_pUser->getItemManager()->findItemHandle(game->getId()));
	}
	
	{
		gcString str("{0}\n", Managers::GetString("#DUN_THREAD_UNINSTALL"));
		onLogEvent(str);
	}
	m_iTotal = uninstallList.size() + (removeCache?1:0) + (removeSettings?1:0) + 2;

	for (size_t x=0; x<uninstallList.size(); x++)
	{
		m_iTotalPos++;
		std::pair<uint32,uint32> pair(m_iTotalPos*100/m_iTotal, 0);
		onProgressEvent(pair);

		if (isStopped())
			break;

		UserCore::Item::ItemHandleI* itemHandle = uninstallList[x];

		if (!itemHandle)
			continue;

		gcString logStr("\t{0}\n", gcString(Managers::GetString("#DUN_THREAD_UNINSTALL_SUB"), itemHandle->getItemInfo()->getName()));
		onLogEvent(logStr);
	
		itemHandle->addHelper(this);
		itemHandle->uninstall(this, true, false);

		m_WaitCondition.wait();

		itemHandle->delHelper(this);
	}

	m_pUser->logOut();

	removeUninstallInfo();
	removeGameExplorerInfo();

	if (removeCache)
		removeDesuraCache();

	if (removeSettings)
		removeDesuraSettings();

	gcString finalLogStr("{0}\n", Managers::GetString("#DUN_THREAD_FINAL"));
	onLogEvent(finalLogStr);

	UTIL::WIN::delRegValue(APPID);
	UTIL::WIN::delRegValue(APPBUILD);
	UTIL::WIN::delRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\InstallPath");

	UTIL::FS::Path path(UTIL::OS::getCommonProgramFilesPath(), L"", false);
	UTIL::FS::delFolder(path);
	UTIL::FS::delEmptyFolders(m_pUser->getAppDataPath());

	UTIL::WIN::delRegKey("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura");
	onCompleteEvent();
}

void UninstallAllThread::onStop()
{
	m_WaitCondition.notify();
}

bool UninstallAllThread::stopStagePrompt()
{
	return true;
}

void UninstallAllThread::onComplete(uint32 status)
{
	m_WaitCondition.notify();
}

void UninstallAllThread::onComplete(gcString& string)
{
}

void UninstallAllThread::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	uint32 prog = info.percent;
	std::pair<uint32,uint32> pair(m_iTotalPos*100/m_iTotal, prog);
	onProgressEvent(pair);
}

void UninstallAllThread::onProgressUpdate(uint32 progress)
{
	std::pair<uint32,uint32> pair(m_iTotalPos*100/m_iTotal, progress);
	onProgressEvent(pair);
}


void UninstallAllThread::onError(gcException e)
{
}

void UninstallAllThread::onNeedWildCard(WCSpecialInfo& info)
{
}


void UninstallAllThread::onDownloadProvider(UserCore::Misc::GuiDownloadProvider &provider)
{
}

void UninstallAllThread::onVerifyComplete(UserCore::Misc::VerifyComplete& info)
{
}


uint32 UninstallAllThread::getId()
{
	return m_uiId;
}

void UninstallAllThread::setId(uint32 id)
{
	m_uiId = id;
}

void UninstallAllThread::onPause(bool state)
{
}


void UninstallAllThread::removeUninstallInfo()
{
	gcString logStr("{0}\n", Managers::GetString("#DUN_THREAD_UNINSTALL"));
	onLogEvent(logStr);

	m_iTotalPos++;
	std::pair<uint32,uint32> pair(m_iTotalPos*100/m_iTotal, 0);
	onProgressEvent(pair);

	std::vector<std::string> regKeys;
	UTIL::WIN::getAllRegKeys("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", regKeys);

	for (size_t x=0; x<regKeys.size(); x++)
	{
		std::string key = regKeys[x];

		if (key.find("Desura") != 0)
			continue;

		gcString regKey("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{0}", key);
		UTIL::WIN::delRegTree(regKey.c_str());
	}
}

void UninstallAllThread::removeGameExplorerInfo()
{
	gcString logStr("{0}\n", Managers::GetString("#DUN_THREAD_GAMEEXPLORER"));
	onLogEvent(logStr);

	m_iTotalPos++;
	std::pair<uint32,uint32> pair(m_iTotalPos*100/m_iTotal, 0);
	onProgressEvent(pair);

	std::vector<std::string> regKeys;
	UTIL::WIN::getAllRegKeys("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameUX\\Games", regKeys, true);

	for (size_t x=0; x<regKeys.size(); x++)
	{
		gcString regKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameUX\\Games\\{0}\\AppExePath", regKeys[x]);
		std::string value = UTIL::WIN::getRegValue(regKey, true);

		if (value.find("desura://") != 0)
			continue;

		regKey = gcString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameUX\\Games\\{0}", regKeys[x]);
		UTIL::WIN::delRegTree(regKey.c_str(), true);
	}

	UTIL::FS::Path gdfFolder(UTIL::OS::getAppDataPath(), L"", false);
	gdfFolder += "GDF";

	UTIL::FS::delFolder(gdfFolder);
}


void UninstallAllThread::removeDesuraCache()
{
	gcString logStr("{0}\n", Managers::GetString("#DUN_THREAD_CACHE"));
	onLogEvent(logStr);

	m_iTotalPos++;
	std::pair<uint32,uint32> pair(m_iTotalPos*100/m_iTotal, 0);
	onProgressEvent(pair);

	UTIL::FS::Path path(getCachePath(), "", false);
	std::vector<std::string> extList;

	extList.push_back("mcf");

	for (size_t x=0; x<20; x++)
		extList.push_back(gcString("part_{0}", x));

	UTIL::FS::Path dbDir(UTIL::OS::getAppDataPath(), L"", false);

	delLeftOverMcf(path, extList);
	delLeftOverMcf(dbDir, extList);

	UTIL::FS::Path mcfUploads = dbDir;
	UTIL::FS::Path mcfStore = dbDir;

	mcfUploads += UTIL::FS::File("mcf_uploads.sqlite");
	mcfStore += UTIL::FS::File("mcfstoreb.sqlite");

	UTIL::FS::delFile(mcfUploads);
	UTIL::FS::delFile(mcfStore);
}

void UninstallAllThread::removeDesuraSettings()
{
	gcString logStr("{0}\n", Managers::GetString("#DUN_THREAD_SETTINGS"));
	onLogEvent(logStr);

	m_iTotalPos++;
	std::pair<uint32,uint32> pair(m_iTotalPos*100/m_iTotal, 0);
	onProgressEvent(pair);

	UTIL::FS::Path dbDir(UTIL::OS::getAppDataPath(), L"", false);
	
	std::vector<UTIL::FS::Path> fileList;
	std::vector<UTIL::FS::Path> folderList;

	UTIL::FS::getAllFiles(dbDir, fileList, NULL);
	UTIL::FS::getAllFolders(dbDir, folderList);

	for (size_t x=0; x<fileList.size(); x++)
	{
		std::string fileName = fileList[x].getFile().getFile();

		if (fileName == "mcfstoreb.sqlite" || fileName == "mcf_uploads.sqlite")
			continue;

		UTIL::FS::delFile(fileList[x]);
	}

	for (size_t x=0; x<folderList.size(); x++)
	{
		std::string folderName = folderList[x].getLastFolder();

		if (folderName == "mods" || folderName == "games" || folderName == "GDF")
			continue;

		UTIL::FS::delFolder(folderList[x]);
	}
}

bool UninstallAllThread::hasPaidBranch(UserCore::Item::ItemInfoI* item)
{
	for (size_t z=0; z<item->getBranchCount(); z++)
	{
		UserCore::Item::BranchInfoI* bi = item->getBranch(z);

		if (HasAllFlags(bi->getFlags(), UserCore::Item::BranchInfoI::BF_ONACCOUNT) && !HasAnyFlags(bi->getFlags(), UserCore::Item::BranchInfoI::BF_FREE|UserCore::Item::BranchInfoI::BF_DEMO|UserCore::Item::BranchInfoI::BF_TEST))
			return true;
	}
	return false;
}



void UninstallAllThread::delLeftOverMcf(UTIL::FS::Path path, std::vector<std::string> &extList)
{
	std::vector<UTIL::FS::Path> outList;
	UTIL::FS::getAllFiles(path, outList, &extList);

	for (size_t x=0; x<outList.size(); x++)
		UTIL::FS::delFile(outList[x]);

	outList.clear();

	UTIL::FS::getAllFolders(path, outList);

	for (size_t x=0; x<outList.size(); x++)
		delLeftOverMcf(outList[x], extList);
}

gcString UninstallAllThread::getCachePath()
{
	return UTIL::OS::getMcfCachePath();
}
