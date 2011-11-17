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
#include "UIBaseServiceTask.h"
#include "McfManager.h"

#include "mcfcore/MCFI.h"
#include "mcfcore/MCFHeaderI.h"
#include "mcfcore/MCFMain.h"
#include "mcfcore/UserCookies.h"

namespace UserCore
{
namespace ItemTask
{


UIBaseServiceTask::UIBaseServiceTask(uint32 stage, const char* taskname, UserCore::Item::ItemHandle* handle, MCFBranch installBranch, MCFBuild installBuild) : BaseItemServiceTask(stage, taskname, handle, installBranch, installBuild)
{
}

UIBaseServiceTask::~UIBaseServiceTask()
{
}

bool UIBaseServiceTask::initService()
{
	UserCore::Item::ItemInfo* pItem = getItemInfo();

	if (!pItem)
		return false;

	pItem->delSFlag(	UserCore::Item::ItemInfoI::STATUS_INSTALLED|
						UserCore::Item::ItemInfoI::STATUS_READY|
						UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|
						UserCore::Item::ItemInfoI::STATUS_VERIFING|
						UserCore::Item::ItemInfoI::STATUS_INSTALLING|
						UserCore::Item::ItemInfoI::STATUS_UPDATING|
						UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER|
						UserCore::Item::ItemInfoI::STATUS_PAUSED|
						UserCore::Item::ItemInfoI::STATUS_PAUSABLE);

	if (!pItem->isDownloadable() || HasAllFlags(pItem->getStatus(), UserCore::Item::ItemInfoI::STATUS_LINK))
	{
		pItem->delSFlag(UserCore::Item::ItemInfoI::STATUS_LINK);
		return false;
	}

	m_OldBranch = getItemInfo()->getInstalledBranch();
	m_OldBuild = getItemInfo()->getInstalledBuild();

	gcString installPath = getItemInfo()->getPath();

	if (installPath == "")
		return false;

	return true;
}

void UIBaseServiceTask::onServiceError(gcException& e)
{
	onComplete();
}

gcString UIBaseServiceTask::getBranchMcf(DesuraId id, MCFBranch branch, MCFBuild build)
{
	UserCore::MCFManager *mm = UserCore::GetMCFManager();
	gcString filePath = mm->getMcfPath(id, branch, build);

	if (filePath == "" || !checkPath(filePath.c_str(), branch, build))
	{
		if (filePath == "")
			filePath =  mm->newMcfPath(id, branch, build);

		MCFCore::Misc::UserCookies uc;
		getWebCore()->setMCFCookies(&uc);

		try
		{
			McfHandle mcfHandle;
			mcfHandle->setHeader(id, branch, build);
			mcfHandle->setFile(filePath.c_str());
			mcfHandle->getDownloadProviders(getWebCore()->getMCFDownloadUrl(), &uc);
			mcfHandle->dlHeaderFromWeb();
			mcfHandle->saveBlankMcf();
		}
		catch (gcException &)
		{
		}

		if (!checkPath(filePath.c_str(), branch, build))
			filePath = "";
	}

	return filePath;
}


bool UIBaseServiceTask::checkPath(const char* path, MCFBranch branch, MCFBuild build)
{
	if (!UTIL::FS::isValidFile(UTIL::FS::PathWithFile(path)))
		return false;

	McfHandle mcfHandle;
	mcfHandle->setFile(path);

	try
	{
		mcfHandle->parseMCF();
	}
	catch (gcException &except)
	{
		Warning(gcString("Uninstall Branch: MCF Error: {0}\n", except));
		return false;
	}

	MCFCore::MCFHeaderI *mcfHead = mcfHandle->getHeader();
	return (mcfHead && (build == 0 || mcfHead->getBuild() == build) && mcfHead->getBranch() == branch);
}

void UIBaseServiceTask::onStop()
{
	BaseItemServiceTask::onStop();
}

void UIBaseServiceTask::completeUninstall(bool removeAll, bool removeAccount)
{
#ifdef NIX
	removeScripts();
#endif
	
	MCFCore::Misc::ProgressInfo prog;
	prog.percent = 100;
	onMcfProgressEvent(prog);

	UserCore::Item::ItemInfo* pItem = getItemInfo();
	pItem->resetInstalledMcf();

	getUserCore()->removeUninstallInfo(getItemId());

	if (removeAll)
	{
		UserCore::MCFManager *mm = UserCore::GetMCFManager();
		mm->delAllMcfPath(getItemId());

		UTIL::FS::Path path(getUserCore()->getAppDataPath(), "", false);
		path += pItem->getId().getFolderPathExtension();

		UTIL::FS::delFolder(path);
	}

	if (removeAccount)
	{
		pItem->removeFromAccount();
		getUserCore()->getItemManager()->removeItem(pItem->getId());
	}
}

void UIBaseServiceTask::onComplete()
{
	uint32 com = 0;
	onCompleteEvent(com);
	onFinish();
}


#ifdef NIX
void UIBaseServiceTask::removeScripts()
{
	UserCore::Item::ItemInfoI* item = getItemInfo();
	
	if (!item)
		return;
	
	const char* insPath = item->getPath();
	
	if (!insPath)
		return;
	
	UTIL::FS::Path path(insPath, "", false);
	std::vector<std::string> filter;
	filter.push_back("sh");
	
	std::vector<UTIL::FS::Path> out;
	UTIL::FS::getAllFiles(path, out, &filter);
	
	for (size_t x=0; x<out.size(); x++)
	{
		if (out[x].getFile().getFile().find("desura_launch") == 0)
			UTIL::FS::delFile(out[x]);
	}
	
	UTIL::FS::delEmptyFolders(path);
}
#endif


}
}
