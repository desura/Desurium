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
#include "VerifyServiceTask.h"

#include "ItemHandle.h"
#include "ItemInfo.h"
#include "webcore/WebCoreI.h"
#include "usercore/UserCoreI.h"

#include "mcfcore/MCFHeaderI.h"
#include "mcfcore/MCFMain.h"
#include "mcfcore/UserCookies.h"

#include "UserTasks.h"
#include "User.h"

#include "IPCServiceMain.h"

#include "VSCheckMcfDownload.h"
#include "VSCheckMcf.h"
#include "VSCheckInstall.h"
#include "VSDownloadMissing.h"
#include "VSInstallMissing.h"

enum TIER
{
	START,
	VERIFYMCF,
	VERIFYINSTALL,
	DOWNLOADMISSINGFILES,
	INSTALLMISSINGFILES,
};

namespace UserCore
{
namespace ItemTask
{

VerifyServiceTask::VerifyServiceTask(UserCore::Item::ItemHandle* handle, MCFBranch branch, MCFBuild build, bool files, bool tools, bool hooks) 
	: BaseItemTask(UserCore::Item::ItemHandleI::STAGE_VERIFY, "Verify", handle, branch, build)
{
	m_iTier = START;

	m_bCheckFiles = files;
	m_bCheckTools = tools;
	m_bCheckHooks = hooks;

	m_bError = false;
	m_uiLastPercent = 0;

	m_pCurTask = NULL;

	m_McfBranch = branch;
	m_McfBuild = build;

	m_uiOldStatus = 0;
	m_bRefreshedInfo = false;
}

VerifyServiceTask::~VerifyServiceTask()
{
}

void VerifyServiceTask::finishVerify(UserCore::Misc::VerifyComplete::VSTATUS status, const char* installpath, bool endStage)
{
	if (m_bError)
		return;

	MCFCore::Misc::ProgressInfo prog;
	prog.percent = 100;
	onMcfProgressEvent(prog);

	UserCore::Item::ItemInfo* pItem = getItemInfo();

	if (pItem)
	{
		pItem->delSFlag(UserCore::Item::ItemInfoI::STATUS_VERIFING);

		if (status == UserCore::Misc::VerifyComplete::V_COMPLETE)
			pItem->addSFlag(UserCore::Item::ItemInfoI::STATUS_READY);
	
		if (status == UserCore::Misc::VerifyComplete::V_RESET)
		{
			uint32 flags = UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER|UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|UserCore::Item::ItemInfoI::STATUS_INSTALLING|UserCore::Item::ItemInfoI::STATUS_UPDATING|UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL;

			pItem->resetInstalledMcf();
			pItem->delSFlag(flags);
		}
	}

	UserCore::Misc::VerifyComplete vInstallPath(status, installpath);
	onVerifyCompleteEvent(vInstallPath);

	if (status == UserCore::Misc::VerifyComplete::V_INSTALL)
	{
		getItemInfo()->addToAccount();
		getItemHandle()->goToStageInstall(installpath, getMcfBranch());
	}
	else if (status == UserCore::Misc::VerifyComplete::V_DOWNLOAD)
	{
		getItemInfo()->addToAccount();

		if (installpath)
			getItemHandle()->goToStageDownload(installpath);
		else
			getItemHandle()->goToStageDownload(getItemInfo()->getCurrentBranch()->getBranchId(), m_McfBuild);	
	}
	else if (status == UserCore::Misc::VerifyComplete::V_SWITCHBRANCH)
	{
		getItemInfo()->addToAccount();
		getItemHandle()->switchBranch(m_McfBranch);	
	}
	else if (endStage)
	{
		getItemHandle()->completeStage(true);
	}
}

void VerifyServiceTask::setTier(uint8 tier)
{
	m_iTier = tier;

	MCFCore::Misc::ProgressInfo prog;
	onProgress(prog);
}

void VerifyServiceTask::doRun()
{
	if (checkItem())
		return;

	updateStatus();

	if (checkBranch())
		return;

	if (checkUnAuthed())
		return;

	if (m_bCheckFiles && !checkFiles())
		return;
	
	//do hooks before tools as tools will change to the downloading tool stage
	if (m_bCheckHooks)
		checkHooks();

	bool end = true;

	if (m_bCheckTools)
		end = !checkTools();

	finishVerify(UserCore::Misc::VerifyComplete::V_COMPLETE, NULL, end);
}

bool VerifyServiceTask::checkFiles()
{
	if (HasAnyFlags(m_uiOldStatus, UserCore::Item::ItemInfoI::STATUS_DOWNLOADING))
	{
		gcString path;

		if (checkMcfDownload(path))
			finishVerify(UserCore::Misc::VerifyComplete::V_DOWNLOAD, path.c_str());
		else
			finishVerify(UserCore::Misc::VerifyComplete::V_DOWNLOAD);

		return false;
	}
	else if (HasAnyFlags(m_uiOldStatus, (UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_INSTALLING)))
	{
		m_hMcf->getProgEvent() += delegate(this, &VerifyServiceTask::onProgress);
		m_hMcf->getErrorEvent() += delegate(this, &VerifyServiceTask::onError);

		bool completeMcf = false;

		if (m_McfBuild == 0)
			m_McfBuild = getItemInfo()->getInstalledBuild();

		setTier(VERIFYMCF);

		if (!checkMcf(completeMcf))
			return false;

		setTier(VERIFYINSTALL);

		if (!checkInstall(completeMcf))
			return false;

		setTier(DOWNLOADMISSINGFILES);

		if (completeMcf == false && !downloadMissingFiles())
			return false;

		setTier(INSTALLMISSINGFILES);

		if (!installMissingFiles())
			return false;
	}
	else
	{
		getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_READY);
		finishVerify(UserCore::Misc::VerifyComplete::V_RESET);
		return false;
	}

	return true;
}

bool VerifyServiceTask::checkTools()
{
	refreshInfo();

	std::vector<DesuraId> toolList;
	getItemInfo()->getCurrentBranch()->getToolList(toolList);

	if (toolList.size() == 0)
		return false;

	getUserCore()->getToolManager()->invalidateTools(toolList);

	if (!getUserCore()->getToolManager()->areAllToolsDownloaded(toolList))
		getItemHandle()->goToStageDownloadTools(false);
	else
		getItemHandle()->goToStageInstallTools(false);

	return true;
}

void VerifyServiceTask::checkHooks()
{
	refreshInfo();

	const char* hookPath = getItemInfo()->getInstallScriptPath();
	const char* insPath = getItemInfo()->getPath();

	if (hookPath && UTIL::FS::isValidFile(hookPath))
		getUserCore()->getServiceMain()->runInstallScript(hookPath, insPath, "PostInstall");
}

void VerifyServiceTask::refreshInfo()
{
	if (m_bRefreshedInfo)
		return;

	m_bRefreshedInfo = true;

	getUserCore()->getItemManager()->retrieveItemInfo(getItemId());
}

void VerifyServiceTask::setupCurTask()
{
	m_pCurTask->onErrorEvent += delegate(this, &VerifyServiceTask::onError);

	m_pCurTask->setItemHandle(getItemHandle());
	m_pCurTask->setMcfBuild(m_McfBuild);
	m_pCurTask->setMcfBranch(m_McfBranch);

	m_pCurTask->setUserCore(getUserCore());
	m_pCurTask->setWebCore(getWebCore());

	m_pCurTask->setMcfHandle(m_hMcf.handle());
}

bool VerifyServiceTask::checkMcfDownload(gcString &path)
{
	bool res = false;
	VSCheckMcfDownload task;

	m_pCurTask = &task;
	setupCurTask();

	try
	{
		res = task.doTask(path);
	}
	catch (gcException)
	{
	}

	m_pCurTask = NULL;
	return res;
}

bool VerifyServiceTask::checkMcf(bool &completeMcf)
{
	bool res = false;
	VSCheckMcf task;

	m_pCurTask = &task;
	setupCurTask();

	try
	{
		res = task.doTask(completeMcf);
	}
	catch (gcException)
	{
	}

	m_pCurTask = NULL;
	return res;
}

bool VerifyServiceTask::checkInstall(bool completeMcf)
{
	bool res = false;
	VSCheckInstall task;

	m_pCurTask = &task;
	setupCurTask();

	try
	{
		res = task.doTask(completeMcf);
	}
	catch (gcException)
	{
	}

	m_pCurTask = NULL;

	if (res)
	{
		res = false;

		switch (task.getResult())
		{
		case VSBaseTask::RES_COMPLETE:
			finishVerify(UserCore::Misc::VerifyComplete::V_COMPLETE);
			break;

		case VSBaseTask::RES_DOWNLOAD:
			finishVerify(UserCore::Misc::VerifyComplete::V_DOWNLOAD);
			break;

		case VSBaseTask::RES_INSTALL:
			finishVerify(UserCore::Misc::VerifyComplete::V_INSTALL, m_hMcf->getFile());
			break;

		case VSBaseTask::RES_DOWNLOADMISSING:
		default:
			res = true;
		};
	}

	return res;
}


bool VerifyServiceTask::downloadMissingFiles()
{
	bool res = false;
	VSDownloadMissing task;
	task.onNewProviderEvent += delegate(&onNewProviderEvent);

	m_pCurTask = &task;
	setupCurTask();


	try
	{
		res = task.doTask();
	}
	catch (gcException)
	{
	}

	if (res && task.getResult() == VSBaseTask::RES_COMPLETE)
	{
		finishVerify(UserCore::Misc::VerifyComplete::V_COMPLETE);
		return false;
	}

	m_pCurTask = NULL;
	return res;
}

bool VerifyServiceTask::installMissingFiles()
{
	bool res = false;
	VSInstallMissing task;

	m_pCurTask = &task;
	setupCurTask();

	task.onProgressEvent += delegate(this, &VerifyServiceTask::onProgress);

	try
	{
		res = task.doTask();
	}
	catch (gcException)
	{
	}

	m_pCurTask = NULL;
	return res;
}




void VerifyServiceTask::updateStatus()
{
	UserCore::Item::ItemInfo* pItem = getItemInfo();

	m_uiOldStatus = pItem->getStatus();

	uint32 addFlags = UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER|UserCore::Item::ItemInfoI::STATUS_VERIFING;
	uint32 delFlags = UserCore::Item::ItemInfoI::STATUS_READY|UserCore::Item::ItemInfoI::STATUS_PAUSED|UserCore::Item::ItemInfoI::STATUS_PAUSABLE|UserCore::Item::ItemInfoI::STATUS_LINK|UserCore::Item::ItemInfoI::STATUS_DELETED;

	UserCore::Item::ItemInfo* parent = getParentItemInfo();

	if (parent)
		parent->delSFlag(UserCore::Item::ItemInfoI::STATUS_DELETED);

	pItem->addSFlag(addFlags);
	pItem->delSFlag(delFlags);

	pItem->setPercent(0);
}


bool VerifyServiceTask::checkItem()
{
	gcException eBadItem(ERR_BADITEM);
	gcException eBrchNull(ERR_BADITEM, "Item branch is null");
	
	UserCore::Item::ItemInfo* pItem = getItemInfo();

	if (!pItem)
	{
		onError(eBadItem);
		return true;
	}

	if (!pItem->isDownloadable())
	{
		finishVerify(UserCore::Misc::VerifyComplete::V_COMPLETE);
		return true;
	}

	if (!pItem->getCurrentBranch())
	{
		if (HasAnyFlags(pItem->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|UserCore::Item::ItemInfoI::STATUS_INSTALLING))
			onError(eBrchNull);
		else
			finishVerify(UserCore::Misc::VerifyComplete::V_RESET);

		return true;
	}

	return false;
}

bool VerifyServiceTask::checkBranch()
{
	UserCore::Item::ItemInfo* pItem = getItemInfo();
	UserCore::Item::BranchInfoI* pBranch = pItem->getCurrentBranch();
	
	if (!pBranch)
		return false;

	if (pBranch && pItem->isDownloadable() && !pBranch->isDownloadable())
	{
		finishVerify(UserCore::Misc::VerifyComplete::V_RESET);
		return true;
	}

	if (m_McfBranch != 0 && pBranch && pBranch->getBranchId() != m_McfBranch)
	{
		finishVerify(UserCore::Misc::VerifyComplete::V_SWITCHBRANCH);
		return true;
	}

	m_McfBranch = pItem->getCurrentBranch()->getBranchId();
	return false;
}

bool VerifyServiceTask::checkUnAuthed()
{
	UserCore::Item::ItemInfo* pItem = getItemInfo();

	if (m_McfBuild != 0 || HasAllFlags(pItem->getStatus(), UserCore::Item::ItemInfoI::STATUS_UNAUTHED) == false)
		return false;

	McfHandle hMcf;
	hMcf->setHeader(getItemId(), m_McfBranch, m_McfBuild);

	try
	{
		MCFCore::Misc::UserCookies uc;
		getWebCore()->setMCFCookies(&uc); 
		hMcf->getDownloadProviders(getWebCore()->getMCFDownloadUrl(), &uc);
	}
	catch (gcException &except)
	{
		onError(except);
		return true;
	}		

	m_McfBuild = hMcf->getHeader()->getBuild();

	if (m_McfBuild == 0)
	{
		//cant remove this as this is the first version
		finishVerify();
		return true;
	}

	pItem->overideInstalledBuild(m_McfBuild);
	pItem->delSFlag(UserCore::Item::ItemInfoI::STATUS_UNAUTHED);
	
	return false;
}

void VerifyServiceTask::onStop()
{
	if (m_pCurTask)
		m_pCurTask->stop();

	BaseItemTask::onStop();
}

void VerifyServiceTask::onProgress(MCFCore::Misc::ProgressInfo& prog)
{
	if (m_pCurTask)
		m_pCurTask->onProgress(prog);


	MCFCore::Misc::ProgressInfo p(&prog);
	uint32 percent = 0;

	switch (m_iTier)
	{
		case START: 
			percent = 00 + ((uint32)prog.percent/20);  
			p.flag = 0;
			break;

		case VERIFYMCF: 
			percent = 20 + ((uint32)prog.percent/20); 
			p.flag = 1;
			break;

		case VERIFYINSTALL: 
			percent = 40 + ((uint32)prog.percent/20); 
			p.flag = 2;
			break;

		case DOWNLOADMISSINGFILES: 
			percent = 60 + ((uint32)prog.percent/20); 
			p.flag = 3;
			break;

		case INSTALLMISSINGFILES: 
			percent = 80 + ((uint32)prog.percent/20);
			p.flag = 4;
			break;
	}
	
	if (m_uiLastPercent != percent || p.doneAmmount > 0)
	{
		p.percent = percent;
		onMcfProgressEvent(p);

		getItemInfo()->setPercent(percent);

		m_uiLastPercent = percent;
	}
}


void VerifyServiceTask::onError(gcException& e)
{
	m_bError = true;

	Warning(gcString("Error in verify install: {0}\n", e));
	getItemHandle()->setPausable(false);

	if (!getItemHandle()->shouldPauseOnError())
		getItemHandle()->resetStage(true);
	else
		getItemHandle()->setPaused(true, true);

	onErrorEvent(e);
}


}
}
