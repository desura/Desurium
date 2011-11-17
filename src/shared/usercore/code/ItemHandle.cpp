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
#include "ItemHandle.h"
#include "ItemHandleEvents.h"

#include "ComplexLaunchServiceTask.h"
#include "InstallServiceTask.h"
#include "DownloadTask.h"
#include "GatherInfoTask.h"
#include "VerifyServiceTask.h"
#include "InstallCheckTask.h"
#include "LaunchTask.h"
#include "DownloadToolItemTask.h"
#include "ValidateTask.h"
#include "InstallToolTask.h"

#include "UIBranchServiceTask.h"
#include "UIServiceTask.h"
#include "UIComplexModServiceTask.h"
#include "UIPatchServiceTask.h"
#include "UIUpdateServiceTask.h"

#include "managers/Managers.h"
#include "usercore/MCFThreadI.h"

#include "ItemThread.h"
#include "ItemTaskGroup.h"



#include "User.h"

class BlankTask : public UserCore::ItemTask::BaseItemTask
{
public:
	BlankTask(UserCore::Item::ItemHandle* handle, uint32 type) : BaseItemTask(type, "", handle)
	{
	}

	virtual void doRun()
	{
#ifdef WIN32
		int a=1;
#endif
	}
};

namespace UserCore
{
namespace Item
{

ItemHandle::ItemHandle(ItemInfo* itemInfo, UserCore::User* user)
{
	m_uiHelperId = 0;
	m_bPauseOnError = false;
	m_bStopped = false;
	m_pUserCore = user;

	m_bLock = false;
	m_pLockObject = NULL;

	m_pThread = NULL;
	m_pFactory = NULL;
	m_pItemInfo = itemInfo;

	m_uiStage = STAGE_NONE;

	m_pEventHandler = new ItemHandleEvents(m_vHelperList);
	m_pGroup = NULL;
}

ItemHandle::~ItemHandle()
{
	safe_delete(m_pItemInfo);
	safe_delete(m_pEventHandler);
}

void ItemHandle::setFactory(Helper::ItemHandleFactoryI* factory)
{
	m_pFactory = factory;
}

UserCore::User* ItemHandle::getUserCore()
{
	return m_pUserCore;
}

UserCore::Item::ItemInfoI* ItemHandle::getItemInfo()
{
	return m_pItemInfo;
}

UserCore::Item::ItemInfo* ItemHandle::getItemInfoNorm()
{
	return m_pItemInfo;
}

bool ItemHandle::getLock(void* obj)
{
#ifdef DESURA_DIABLO2_HACK
	DesuraId d2Id(50, DesuraId::TYPE_GAME);
	DesuraId lodId(13824, DesuraId::TYPE_GAME);

	if (m_pItemInfo->getId() == lodId)
	{
		UserCore::Item::ItemHandle* handle = dynamic_cast<UserCore::Item::ItemHandle*>(m_pUserCore->getItemManager()->findItemHandle(d2Id));

		if (handle)
			return handle->getLock(obj);
	}
#endif

	if (!obj)
		return false;

	if (m_bLock)
		return (m_pLockObject == obj);

	m_bLock = true;
	m_pLockObject = obj;

	return true;
}

bool ItemHandle::isLocked()
{
#ifdef DESURA_DIABLO2_HACK
	DesuraId d2Id(50, DesuraId::TYPE_GAME);
	DesuraId lodId(13824, DesuraId::TYPE_GAME);

	if (m_pItemInfo->getId() == lodId)
	{
		UserCore::Item::ItemHandle* handle = dynamic_cast<UserCore::Item::ItemHandle*>(m_pUserCore->getItemManager()->findItemHandle(d2Id));

		if (handle)
			return handle->isLocked();
	}
#endif


	return m_bLock;
}

bool ItemHandle::hasLock(void* obj)
{
#ifdef DESURA_DIABLO2_HACK
	DesuraId d2Id(50, DesuraId::TYPE_GAME);
	DesuraId lodId(13824, DesuraId::TYPE_GAME);

	if (m_pItemInfo->getId() == lodId)
	{
		UserCore::Item::ItemHandle* handle = dynamic_cast<UserCore::Item::ItemHandle*>(m_pUserCore->getItemManager()->findItemHandle(d2Id));

		if (handle)
			return handle->hasLock(obj);
	}
#endif


	if (m_bLock)
		return (m_pLockObject == obj);

	return false;
}

void ItemHandle::releaseLock(void* obj)
{
#ifdef DESURA_DIABLO2_HACK
	DesuraId d2Id(50, DesuraId::TYPE_GAME);
	DesuraId lodId(13824, DesuraId::TYPE_GAME);

	if (m_pItemInfo->getId() == lodId)
	{
		UserCore::Item::ItemHandle* handle = dynamic_cast<UserCore::Item::ItemHandle*>(m_pUserCore->getItemManager()->findItemHandle(d2Id));

		if (handle)
		{
			handle->releaseLock(obj);
			return;
		}
	}
#endif

	if (m_pLockObject != obj)
		return;

	m_pLockObject = NULL;
	m_bLock = false;
}



void ItemHandle::delHelper(Helper::ItemHandleHelperI* helper)
{
	for (size_t x=0; x<m_vHelperList.size(); x++)
	{
		if (m_vHelperList[x]->getId() == helper->getId())
		{
			m_vHelperList.erase(m_vHelperList.begin()+x);
			break;
		}
	}
}

Event<uint32>* ItemHandle::getChangeStageEvent()
{
	return &onChangeStageEvent;
}

Event<gcException>* ItemHandle::getErrorEvent()
{
	return &onErrorEvent;
}

bool ItemHandle::isInStage()
{
	return m_uiStage != STAGE_NONE && m_uiStage != STAGE_CLOSE && m_uiStage != STAGE_LAUNCH;
}

bool ItemHandle::isStopped()
{
	return m_bStopped;
}

void ItemHandle::setPauseOnError(bool pause)
{
	m_bPauseOnError = pause;
}

bool ItemHandle::shouldPauseOnError()
{
	return m_bPauseOnError;
}

void ItemHandle::setPausable(bool state)
{
	if (state)
		getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSABLE);
	else
		getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSABLE);
}

void ItemHandle::stop(bool block)
{
	if (!block)
	{
		 m_bStopped = true;
	}
	else
	{
		stopThread();
	}
}

void ItemHandle::setPaused(bool state, bool forced)
{
	bool isPausable = (getItemInfo()->getStatus()&UserCore::Item::ItemInfoI::STATUS_PAUSABLE)?true:false;
	bool hasPauseFlag = HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_PAUSED);

	if ((forced || isPausable) && (hasPauseFlag != state))
	{
		bool verify = false;

		m_ThreadMutex.readLock();

		if (state)
		{
			if (m_pThread && m_pThread->hasTaskToRun())
			{
				getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSED);
				m_pThread->pause();
			}
		}
		else
		{
			getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSED);

			if (m_pThread && m_pThread->hasTaskToRun())
				m_pThread->unpause();
			else if (startUpCheck())
				verify = true;
		}

		m_ThreadMutex.readUnlock();

		if (verify)
			verifyOveride();

		m_pEventHandler->onPause(state);
	}
}

void ItemHandle::setPaused(bool state)
{
	setPaused(state, false);
}

void ItemHandle::setStage(uint32 stage)
{
	m_pEventHandler->reset();
	m_uiStage = stage;
	onChangeStageEvent(stage);

	m_pItemInfo->setPercent(0);

	UserCore::Item::ItemInfoI::ItemInfo_s info;
	info.id = m_pItemInfo->getId();
	info.changeFlags = UserCore::Item::ItemInfoI::CHANGED_STATUS;

	m_pItemInfo->getInfoChangeEvent()->operator()(info);
}

void ItemHandle::onTaskStart(uint32 &stage)
{
	if (stage == STAGE_NONE)
	{
		m_uiStage = STAGE_NONE;
		return;
	}

	setStage(stage);
}

void ItemHandle::onTaskComplete(uint32 &stage)
{
	if (stage == STAGE_NONE)
	{
		releaseComplexLock();
		stopThread();
	}
}

void ItemHandle::resetStage(bool close)
{
	//if we are updating and error out goback to last known state
	if (getItemInfo()->getStatus() & UserCore::Item::ItemInfoI::STATUS_UPDATING)
	{
		getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_UPDATING|UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL);
		getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_READY);
	}
	else
	{
		getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLED|
						UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|
						UserCore::Item::ItemInfoI::STATUS_READY|
						UserCore::Item::ItemInfoI::STATUS_VERIFING|
						UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL);

		getItemInfoNorm()->resetInstalledMcf();
	}

	completeStage(close);
}

void ItemHandle::completeStage(bool close)
{
	if (close)
		registerTask(new BlankTask(this, STAGE_CLOSE));

	registerTask(new BlankTask(this, STAGE_NONE));
}


void ItemHandle::goToStageDownloadTools(uint32 ttid, const char* downloadPath, MCFBranch branch, MCFBuild build)
{
	registerTask(new UserCore::ItemTask::DownloadToolTask(this, ttid, downloadPath, branch, build));
}

void ItemHandle::goToStageDownloadTools(bool launch)
{
	registerTask(new UserCore::ItemTask::DownloadToolTask(this, launch));
}

void ItemHandle::goToStageInstallTools(bool launch)
{
	registerTask(new UserCore::ItemTask::InstallToolTask(this, launch));
}

void ItemHandle::goToStageGatherInfo(MCFBranch branch, MCFBuild build, UserCore::ItemTask::GI_FLAGS flags)
{
	Helper::GatherInfoHandlerHelperI* helper = NULL;
	
	if (m_pFactory)
		m_pFactory->getGatherInfoHelper(&helper);

	registerTask(new UserCore::ItemTask::GatherInfoTask(this, branch, build, helper, flags));
}

void ItemHandle::goToStageDownload(MCFBranch branch, MCFBuild build, bool test)
{
	if (shouldPauseOnError())
	{
		setPauseOnError(false);
		completeStage(true);
		return;
	}

	if (!preDownloadCheck(branch, test))
	{
		completeStage(true);
		return;
	}

	registerTask(new UserCore::ItemTask::ValidateTask(this, branch, build));
}

void ItemHandle::goToStageDownload(const char* path)
{
	registerTask(new UserCore::ItemTask::DownloadTask(this, path));
}

void ItemHandle::goToStageVerify(MCFBranch branch, MCFBuild build, bool files, bool tools, bool hooks)
{
	registerTask(new UserCore::ItemTask::VerifyServiceTask(this, branch, build, files, tools, hooks));
}

void ItemHandle::goToStageInstallCheck()
{
	registerTask(new UserCore::ItemTask::InstallCheckTask(this));
}

void ItemHandle::goToStageLaunch()
{
	registerTask(new UserCore::ItemTask::LaunchTask(this));
}

void ItemHandle::goToStageUninstall(bool complete, bool account)
{
	setPaused(false, true);

	m_ThreadMutex.readLock();

	if (m_pThread)
		m_pThread->purge();

	m_ThreadMutex.readUnlock();

	if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX))
	{
		UserCore::ItemTask::UIComplexModServiceTask* uibst = new UserCore::ItemTask::UIComplexModServiceTask(this, getItemInfo()->getInstalledBranch(), getItemInfo()->getInstalledBuild());
		uibst->setCAUninstall(complete, account);
		uibst->setEndStage();

		registerTask(uibst);
	}
	else
	{
		registerTask(new UserCore::ItemTask::UIServiceTask(this, complete, account));
	}
}

void ItemHandle::goToStageUninstallBranch(MCFBranch branch, MCFBuild build, bool test)
{
	registerTask(new UserCore::ItemTask::UIBranchServiceTask(this, branch, build, test));
}


void ItemHandle::releaseComplexLock()
{
	bool isComplex = HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX);
	UserCore::Item::ItemHandle* obj = this;

	if (isComplex)
	{
		UserCore::Item::ItemHandle* parentHandle = dynamic_cast<UserCore::Item::ItemHandle*>(m_pUserCore->getItemManager()->findItemHandle(getItemInfo()->getParentId()));

		if (parentHandle)
			obj = parentHandle;
	}

	if (obj)
		obj->releaseLock(this);
}

bool ItemHandle::getComplexLock()
{
	bool isParentComplex = getItemInfo()->getInstalledModId().isOk();
	bool isComplex = HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX);


	if (!isComplex && !isParentComplex)
		return true;


	UserCore::Item::ItemHandle* obj = NULL;

	if (isComplex)
	{
		UserCore::Item::ItemHandle* parentHandle = dynamic_cast<UserCore::Item::ItemHandle*>(m_pUserCore->getItemManager()->findItemHandle(getItemInfo()->getParentId()));

		if (parentHandle)
			obj = parentHandle;
	}
	else
	{
		obj = this;
	}

	if (obj && obj->getLock(this))
		return true;

	gcException eItem(ERR_INVALID, "Failed to get lock. Another task is using this item. Please stop that task and try again.");
	gcException eGame(ERR_INVALID, "Failed to get lock. Another task is using the parent game. Please stop that task and try again.");
	
	if (obj == this)
		onErrorEvent(eItem);
	else
		onErrorEvent(eGame);
	
	completeStage(true);
	return false;
}

void ItemHandle::goToStageUninstallUpdate(const char* path, MCFBuild lastBuild)
{
	registerTask(new UserCore::ItemTask::UIUpdateServiceTask(this, path, lastBuild));
}

void ItemHandle::goToStageUninstallPatch(MCFBranch branch, MCFBuild build)
{
	bool isParentComplex = getItemInfo()->getInstalledModId().isOk();
	bool isComplex = HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX);

	UserCore::Item::ItemInfoI* parentItem = m_pUserCore->getItemManager()->findItemInfo(getItemInfo()->getParentId());

	if (isParentComplex || isComplex)
	{
		if (!getComplexLock())
			return;

		if (isParentComplex)
			parentItem = getItemInfo();
		
		//need to uninstall complex mods otherwise the backup file will be wrong
		if (parentItem->getInstalledModId().isOk())
		{
			UserCore::ItemTask::UIComplexModServiceTask* uibst = new UserCore::ItemTask::UIComplexModServiceTask(this, branch, build);

			if (isParentComplex)
				uibst->setCAUIPatch();
			else
				uibst->setCAInstall();

			registerTask(uibst);
			return;
		}
	}

	registerTask(new UserCore::ItemTask::UIPatchServiceTask(this, branch, build));
}

void ItemHandle::goToStageUninstallComplexBranch(MCFBranch branch, MCFBuild build, bool launch)
{
	if (!getComplexLock())
		return;

	UserCore::ItemTask::UIComplexModServiceTask* uibst = new UserCore::ItemTask::UIComplexModServiceTask(this, branch, build);

	//if we are the game and need our child branch removed
	if (getItemInfo()->getInstalledModId().isOk())
	{
		if (launch && HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_LINK))
		{
			uibst->setCALaunch();
		}
		else
		{
			uibst->setCAUninstallBranch();
		}
	}
	else
	{
		uibst->setCAInstall();
	}

	registerTask(uibst);
}

void ItemHandle::goToStageInstallComplex(MCFBranch branch, MCFBuild build, bool launch)
{
	if (!getComplexLock())
	{
		getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSED|UserCore::Item::ItemInfoI::STATUS_INSTALLING);
		return;
	}

	if (getItemInfo()->isComplex())
	{
		Helper::InstallerHandleHelperI* helper = NULL;

		if (m_pFactory)
			m_pFactory->getInstallHelper(&helper);

		UserCore::ItemTask::ComplexLaunchServiceTask *clst = new UserCore::ItemTask::ComplexLaunchServiceTask(this, false, branch, build, helper);

		if (launch)
			clst->launch();
		else
			clst->completeStage();

		registerTask(clst);
	}
}

void ItemHandle::goToStageInstall(const char* path, MCFBranch branch)
{
	if (!getItemInfo()->isComplex())
	{
		Helper::InstallerHandleHelperI* helper = NULL;

		if (m_pFactory)
			m_pFactory->getInstallHelper(&helper);

		registerTask(new UserCore::ItemTask::InstallServiceTask(this, path, branch, helper));
	}
}




void ItemHandle::stopThread()
{
	m_ThreadMutex.writeLock();

	m_pUserCore->getThreadPool()->queueTask(new UserCore::Task::DeleteThread(m_pUserCore, m_pThread));

	m_pThread = NULL;
	m_bStopped = true;

	m_ThreadMutex.writeUnlock();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ItemHandle::registerTask(UserCore::ItemTask::BaseItemTask* task)
{
	if (!task)
		return;

	m_pEventHandler->registerTask(task);


	m_ThreadMutex.readLock();
	bool nullThread = (m_pThread == NULL);
	m_ThreadMutex.readUnlock();

	if (nullThread)
	{
		m_ThreadMutex.writeLock();
		if (!m_pThread)
		{
			m_pThread = new ItemThread(this);

			m_pThread->setThreadManager(m_pUserCore->getThreadManager());
			m_pThread->setUserCore(m_pUserCore);
			m_pThread->setWebCore(m_pUserCore->getWebCore());

			m_pThread->onTaskStartEvent += delegate(this, &ItemHandle::onTaskStart);
			m_pThread->onTaskCompleteEvent += delegate(this, &ItemHandle::onTaskComplete);

			m_bStopped = false;
		}
		m_ThreadMutex.writeUnlock();
	}
	
	m_ThreadMutex.readLock();
	m_pThread->queueTask(task);
	m_ThreadMutex.readUnlock();
}

void ItemHandle::addHelper(Helper::ItemHandleHelperI* helper)
{
	m_vHelperList.push_back(helper);
	helper->setId(m_uiHelperId);
	m_uiHelperId++;

	//forward missed messages on
	if (isInStage())
		m_pEventHandler->postAll(helper);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool ItemHandle::preDownloadCheck(MCFBranch branch, bool test)
{
	UserCore::Item::ItemInfoI* parentInfo = m_pUserCore->getItemManager()->findItemInfo(getItemInfo()->getParentId());

#ifdef WIN32
	bool isComplex = HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX);
#endif
	bool isParentDemo = false;

	if (parentInfo && parentInfo->getCurrentBranch())
		isParentDemo = HasAllFlags(parentInfo->getCurrentBranch()->getFlags(), UserCore::Item::BranchInfoI::BF_DEMO|UserCore::Item::BranchInfoI::BF_TEST);

	UserCore::Item::BranchInfoI* branchInfo = getItemInfo()->getBranchById(branch);

	gcException eExist(ERR_INVALID, "Branch does not exist.");
	gcException eRelease(ERR_INVALID, "Branch has no releases available for download.");
	gcException eDemo(ERR_INVALID, "Parent game is a demo. Please install on the full version only.");

	if (!branchInfo)
		onErrorEvent(eExist);
	else if (!branchInfo->isDownloadable() && !test)
		onErrorEvent(eRelease);
	else if (isParentDemo)
		onErrorEvent(eDemo);
	else
		return true;

	return false;
}



bool ItemHandle::verify(bool files, bool tools, bool hooks)
{
	if (isInStage())
		return true;

	goToStageVerify(MCFBranch(), MCFBuild(), files, tools, hooks);
	return true;
}







class TestTask : public UserCore::ItemTask::BaseItemTask
{
public:
	TestTask(UserCore::Item::ItemHandle* item) : UserCore::ItemTask::BaseItemTask(UserCore::Item::ItemHandleI::STAGE_VERIFY, "Test Task", item)
	{
	}

	virtual void doRun()
	{
		for (size_t x=0; x<100; x++)
		{
			gcSleep(500);
			MCFCore::Misc::ProgressInfo p;
			p.percent = x;
			onMcfProgressEvent(p);
		}

		UserCore::Misc::VerifyComplete vCompleteEvent(UserCore::Misc::VerifyComplete::V_COMPLETE, "");
		onVerifyCompleteEvent(vCompleteEvent);
		getItemHandle()->completeStage(true);
	}
};






void ItemHandle::verifyOveride()
{
	m_ThreadMutex.readLock();

	if (m_pThread)
		m_pThread->purge();

	m_ThreadMutex.readUnlock();

	//registerTask(new TestTask(this));
	goToStageVerify(MCFBranch(), MCFBuild(), true, false, false);
}

void ItemHandle::uninstallOveride()
{

}

bool ItemHandle::update()
{
	if (isInStage())
		return true;

	UserCore::Item::BranchInfoI* branch = getItemInfo()->getCurrentBranch();

	uint32 status = getItemInfo()->getStatus();
	uint32 flags = UserCore::Item::ItemInfoI::STATUS_READY|UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL;

	gcException eNoBranches(ERR_NOUPDATE, "This item has no installed branches.");
	gcException eNoUpdates(ERR_NOUPDATE, "There is no update avaliable for this item.");

	if (!branch)
	{
		onErrorEvent(eNoBranches);
	}
	else if (HasAllFlags(status, flags))
	{
		MCFBranch mcfBranch = branch->getBranchId();
		MCFBuild mcfBuild = getItemInfo()->getNextUpdateBuild();

		return install(mcfBranch, mcfBuild, UserCore::ItemTask::GI_FLAG_UPDATE);
	}
	else
	{
		onErrorEvent(eNoUpdates);
	}

	return false;
}

bool ItemHandle::install(Helper::ItemLaunchHelperI* helper, MCFBranch branch)
{
	if (isInStage())
		return true;

	if (!getItemInfo()->isDownloadable())
	{
		if (HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_LINK) && branch != 0)
		{
			goToStageGatherInfo(branch, MCFBuild(), getItemInfo()->isLaunchable()?UserCore::ItemTask::GI_FLAG_EXISTING:UserCore::ItemTask::GI_FLAG_NONE);
			return true;
		}

		return installCheck();
	}

	if (branch != 0 && getItemInfo()->isLaunchable() && branch == getItemInfo()->getInstalledBranch())
		return launch(helper);

	return install(branch, MCFBuild());
}

bool ItemHandle::install(MCFBranch branch, MCFBuild build, bool test)
{
	return install(branch, build, UserCore::ItemTask::GI_FLAG_TEST);
}

bool ItemHandle::install(MCFBranch branch, MCFBuild build, UserCore::ItemTask::GI_FLAGS flags)
{
	if (isInStage())
		return true;

	return installPrivate(branch, build, flags);
}

bool ItemHandle::installPrivate(MCFBranch branch, MCFBuild build, UserCore::ItemTask::GI_FLAGS flags)
{
	if (checkPaused())
		return true;

	if (!HasAnyFlags(flags, UserCore::ItemTask::GI_FLAG_TEST) && HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_UNAUTHED))
		flags = (UserCore::ItemTask::GI_FLAGS)(flags & UserCore::ItemTask::GI_FLAG_TEST);

	MCFBranch iBranch = getItemInfo()->getInstalledBranch();
	BranchInfoI* bInfo = getItemInfo()->getBranchById(branch);

	bool isParentComplex	= getItemInfo()->isParentToComplex();
	bool isComplex			= getItemInfo()->isComplex();
	bool installReady		= getItemInfo()->isLaunchable();
	bool needBranchSwitch	= (iBranch != 0 && iBranch != branch);
	bool isValidBranch		= bInfo && HasAnyFlags(bInfo->getFlags(), BranchInfoI::BF_DEMO|BranchInfoI::BF_FREE|BranchInfoI::BF_ONACCOUNT|BranchInfoI::BF_TEST);

	if (isValidBranch && installReady && needBranchSwitch)
	{
		if (!preDownloadCheck(branch, HasAnyFlags(flags, UserCore::ItemTask::GI_FLAG_TEST)))
			return false;

		if (isComplex || isParentComplex)
			goToStageUninstallComplexBranch(branch, build, false);
		else
			goToStageUninstallBranch(branch, build, HasAnyFlags(flags, UserCore::ItemTask::GI_FLAG_TEST));
	}
	else
	{
		goToStageGatherInfo(branch, build, flags);
	}

	return true;
}


bool ItemHandle::installCheck()
{
	if (isInStage())
		return true;

	if (!HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLED))
	{
		goToStageInstallCheck();
		return true;
	}

	return false;
}

bool ItemHandle::cleanComplexMods()
{
	if (getItemInfo()->getInstalledModId().isOk())
	{
		UserCore::Item::ItemInfoI* mod = m_pUserCore->getItemManager()->findItemInfo(getItemInfo()->getInstalledModId());
			
		if (mod && HasAllFlags(mod->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX))
		{
			if (!getComplexLock())
				return false;

			UserCore::ItemTask::UIComplexModServiceTask* uibst = new UserCore::ItemTask::UIComplexModServiceTask(this, getItemInfo()->getInstalledBranch(), getItemInfo()->getInstalledBuild());
			uibst->setEndStage();
			registerTask(uibst);
			return true;				
		}
	}

	return false;
}


void ItemHandle::preLaunchCheck()
{
	if (!getItemInfo()->isLaunchable())
		throw gcException(ERR_LAUNCH, gcString("Failed to launch item {0}, failed status check {1}.", getItemInfo()->getName(), getItemInfo()->getStatus()));

	UserCore::Item::Misc::ExeInfoI* ei = getItemInfo()->getActiveExe();

	if (!ei)
		throw gcException(ERR_LAUNCH, gcString("Failed to launch item {0}, No executable info available.", getItemInfo()->getName()));
	
	UTIL::FS::Path path = UTIL::FS::PathWithFile(ei->getExe());

	if (!UTIL::FS::isValidFile(path))
		throw gcException(ERR_LAUNCH, gcString("Failed to launch item {0}, File is not valid exe [{1}].", getItemInfo()->getName(), ei->getExe()));

	m_pUserCore->getItemManager()->setRecent(getItemInfo()->getId());
}



bool ItemHandle::launchForReal(Helper::ItemLaunchHelperI* helper, bool offline)
{
	UserCore::Item::BranchInfoI* bi = this->getItemInfo()->getCurrentBranch();

	if (bi && bi->isPreOrder())
	{
		helper->showPreOrderPrompt();
		return false;
	}

	if (bi && !offline)
	{
		std::vector<DesuraId> toolList;
		bi->getToolList(toolList);

		if (!m_pUserCore->getToolManager()->areAllToolsDownloaded(toolList))
		{
			goToStageDownloadTools(true);
			return true;
		}
		else if (!m_pUserCore->getToolManager()->areAllToolsInstalled(toolList))
		{
			goToStageInstallTools(true);
			return true;
		}
	}

	try
	{
		doLaunch(helper);

		//steam games get this set on activate
		if (getItemInfo()->getCurrentBranch() && !getItemInfo()->getCurrentBranch()->isSteamGame())
			getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_LAUNCHED);
	}
	catch (gcException &e)
	{
		if (helper)
			helper->launchError(e);
		else
			onErrorEvent(e);
	}

	return false;
}

bool ItemHandle::checkPaused()
{
	bool hasPauseFlag = HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_PAUSED);

	if (hasPauseFlag)
	{
		setPaused(false);

		//if we have a valid thread then we should be doing something
		if (m_pThread && m_pThread->hasTaskToRun())
			return true;
	}

	return false;
}

bool ItemHandle::launch(Helper::ItemLaunchHelperI* helper, bool offline, bool ignoreUpdate)
{
	if (isInStage())
		return true;

	if (checkPaused())
		return true;

	bool res = false;

	if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_LINK))
	{
		if (getItemInfo()->getInstalledModId().isOk())
		{
			UserCore::Item::ItemInfoI* mod = m_pUserCore->getItemManager()->findItemInfo(getItemInfo()->getInstalledModId());
			
			if (!mod || !HasAllFlags(mod->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX))
			{
				return launchForReal(helper);
			}
			else
			{
				goToStageUninstallComplexBranch(getItemInfo()->getInstalledBranch(), getItemInfo()->getInstalledBuild(), true);
				return true;				
			}
		}
		else if (getItemInfo()->isLaunchable())
		{
			res = launchForReal(helper);
		}
		else
		{
			install(helper, MCFBranch());
		}
	}
	else if (offline)
	{
		if (getItemInfo()->isLaunchable())
			res = launchForReal(helper, offline);
	}
	else if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_DELETED))
	{
		if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_NONDOWNLOADABLE))
			installCheck();
		else
			install(helper, MCFBranch());
		
		res = true;
	}
	else
	{
		bool hasPreorder = false;

		for (size_t x=0; x<getItemInfo()->getBranchCount(); x++)
		{
			if (getItemInfo()->getBranch(x)->isPreOrder())
			{
				hasPreorder = true;
				break;
			}
		}		

		if (getItemInfo()->getCurrentBranch() == NULL && hasPreorder)
		{
			helper->showPreOrderPrompt();
			res = false;
		}

		if (HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|UserCore::Item::ItemInfoI::STATUS_VERIFING|UserCore::Item::ItemInfoI::STATUS_INSTALLING))
		{
			verify(true, false, false);
			res = true;
		}
		else if (!HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_READY|UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL))
		{
			if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_NONDOWNLOADABLE))
				installCheck();
			else
				install(helper, MCFBranch());

			res = true;
		}
		else if (HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|UserCore::Item::ItemInfoI::STATUS_INSTALLING))
		{
			if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_PAUSED))
				getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSED);
			
			verify(true, false, false);
			res = true;
		}
		else if (HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_PRELOADED))
		{
			if (getItemInfo()->getCurrentBranch()->isPreOrder())
			{
				helper->showPreOrderPrompt();
				res = false;
			}
			else
			{
				//preorder tag has dropped. Install
				install(MCFBranch(), MCFBuild(), UserCore::ItemTask::GI_FLAG_EXISTING);
				res = true;
			}
		}
		else if (getItemInfo()->isLaunchable())
		{
			bool isComplex = HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX);

			if (isComplex)
			{
				bool parCLF = false;

				if (getItemInfo()->getParentId().isOk())
				{
					UserCore::Item::ItemInfoI* par = m_pUserCore->getItemManager()->findItemInfo(getItemInfo()->getParentId());
					parCLF = (par && par->getInstalledModId() != getItemInfo()->getId());
				}
				else
				{
					parCLF = getItemInfo()->getInstalledModId().isOk();
				}

				if (parCLF)
				{
					install(getItemInfo()->getInstalledBranch(), getItemInfo()->getInstalledBuild(), UserCore::ItemTask::GI_FLAG_EXISTING);
					return true;
				}
			}

			if (!getItemInfo()->hasAcceptedEula())
			{
				if (helper)
					helper->showEULAPrompt();
				else
					Warning("No Helper for launch item: Show EULA\n");

				return false;
			}
			else if (!ignoreUpdate && HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL))
			{
				if (helper)
					helper->showUpdatePrompt();
				else
					Warning("No Helper for launch item: Show update form\n");

				return false;
			}

			res = launchForReal(helper);
		}
		else
		{
			Warning("Failed to launch item. :(");
		}
	}

	return res;
}

bool ItemHandle::switchBranch(MCFBranch branch)
{
	return install(branch, MCFBuild());
}

bool ItemHandle::startUpCheck()
{
	if (isInStage())
		return true;

	if (HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_READY|UserCore::Item::ItemInfoI::STATUS_PAUSED))
		return false;

	return (HasAnyFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|UserCore::Item::ItemInfoI::STATUS_VERIFING|UserCore::Item::ItemInfoI::STATUS_INSTALLING));
}

bool ItemHandle::uninstall(Helper::ItemUninstallHelperI* helper, bool complete, bool account)
{
	if (m_uiStage == STAGE_UNINSTALL)
		return true;

	if (isInStage())
	{
		if (helper && !helper->stopStagePrompt())
			return false;
	}

	goToStageUninstall(complete, account);
	return true;
}

uint32 ItemHandle::getStage()
{
	return m_uiStage;
}

void ItemHandle::cancelCurrentStage()
{
	if (!isInStage())
		return;

	if (getStage() == UserCore::Item::ItemHandleI::STAGE_NONE || getStage() == UserCore::Item::ItemHandleI::STAGE_CLOSE)
		return;

	setPaused(false);

	m_ThreadMutex.readLock();

	if (m_pThread)
		m_pThread->cancelCurrentTask();

	m_ThreadMutex.readUnlock();
}

void ItemHandle::getStatusStr(LanguageManagerI *pLangMng, char* buffer, uint32 buffsize)	
{
	gcString temp;

	uint32 status = m_pItemInfo->getStatus();

	if (!m_pItemInfo->isDownloadable())
	{
		if (m_pItemInfo->isInstalled())
			temp = gcString(pLangMng->getString("#IS_INSTALLED"));
		else
			temp = gcString(pLangMng->getString("#IS_NOTINSTALLED"));
	}
	else
	{
		bool skip = false;
		const char* stateMsg;

		if (m_uiStage == UserCore::Item::ItemHandleI::STAGE_DOWNLOADTOOL)
		{
			stateMsg = "#IS_DOWNLOADINGTOOL";
		}
		else if (m_uiStage == UserCore::Item::ItemHandleI::STAGE_INSTALLTOOL)
		{
			stateMsg = "#IS_INSTALLINGTOOL";
		}
		else if (m_uiStage == UserCore::Item::ItemHandleI::STAGE_LAUNCH)
		{
			skip = true;
			temp = pLangMng->getString("#IS_LAUNCHING");
		}
		else if (m_uiStage == UserCore::Item::ItemHandleI::STAGE_VALIDATE)
		{
			stateMsg = pLangMng->getString("#IS_VALIDATE");
		}
		else if (m_uiStage == UserCore::Item::ItemHandleI::STAGE_WAIT && m_pGroup)
		{
			skip = true;
			temp = gcString(pLangMng->getString("#IS_WAIT"), m_pGroup->getPos(this), m_pGroup->getCount());
		}
		else if (status & UserCore::Item::ItemInfoI::STATUS_VERIFING || m_uiStage == UserCore::Item::ItemHandleI::STAGE_VERIFY)
		{
			stateMsg = "#IS_VERIFY";
		}
		else if (status & UserCore::Item::ItemInfoI::STATUS_UPDATING)
		{
			stateMsg = "#IS_UPDATING";
		}
		else if (status & UserCore::Item::ItemInfoI::STATUS_DOWNLOADING || m_uiStage == UserCore::Item::ItemHandleI::STAGE_DOWNLOAD)
		{
			stateMsg = "#IS_DOWNLOADING";
		}
		else if (status & UserCore::Item::ItemInfoI::STATUS_INSTALLING || m_uiStage == UserCore::Item::ItemHandleI::STAGE_INSTALL)
		{
			stateMsg = "#IS_INSTALLING";
		}
		else 
		{
			bool hasPreorder = false;

			for (size_t x=0; x<m_pItemInfo->getBranchCount(); x++)
			{
				if (m_pItemInfo->getBranch(x)->isPreOrder())
				{
					hasPreorder = true;
					break;
				}
			}

			if (HasAnyFlags(status, UserCore::Item::ItemInfoI::STATUS_READY|UserCore::Item::ItemInfoI::STATUS_PRELOADED))
			{
				stateMsg = "#IS_READY";

				UserCore::Item::BranchInfoI* bi = m_pItemInfo->getCurrentBranch();
				if (bi && bi->isPreOrder())
					stateMsg = "#IS_PRELOADED_STATUS";
			}
			else if (m_pItemInfo->getCurrentBranch() == NULL && hasPreorder)
			{
				stateMsg = "#IS_PREORDER_STATUS";
			}
			else if (status & UserCore::Item::ItemInfoI::STATUS_DEVELOPER)
			{
				stateMsg = "#IS_DEVSTUB";
			}
			else
			{
				stateMsg = "#IS_NOTAVAIL";
			}

			skip = true;
			temp = pLangMng->getString(stateMsg);
		}

		if (!skip)
		{
			if (status & UserCore::Item::ItemInfoI::STATUS_PAUSED)
				stateMsg = "#IS_PAUSED";

			temp = gcString("{0:u}% - {1}", m_pItemInfo->getPercent(), pLangMng->getString(stateMsg));
		}
	}

	Safe::strcpy(buffer, buffsize, temp.c_str());
}


ItemHandleEvents* ItemHandle::getEventHandler()
{
	return m_pEventHandler;
}

bool ItemHandle::setTaskGroup(ItemTaskGroup* group, bool force)
{
	if (group && isInStage() && !force)
		return false;

	if (force)
	{
		setPaused(false, true);

		m_ThreadMutex.readLock();

		if (m_pThread)
			m_pThread->purge();

		m_ThreadMutex.readUnlock();
	}

	m_pGroup = group;

	if (group)
	{
		registerTask(group->newTask(this));
	}
	else
	{
		if (getStage() == UserCore::Item::ItemHandleI::STAGE_WAIT)
			cancelCurrentStage();
	}
	
	return true;
}

ItemTaskGroupI* ItemHandle::getTaskGroup()
{
	return m_pGroup;
}

void ItemHandle::force()
{
	if (!m_pGroup)
		return;

	ItemTaskGroup* group = m_pGroup;

	group->removeItem(this);
	group->startAction(this);
}

bool ItemHandle::createDektopShortcut()
{
#ifdef NIX
	return false;
#else
	gcString workingDir = UTIL::OS::getDesktopPath();
	gcString path("{0}\\{1}.lnk", workingDir, UTIL::WIN::sanitiseFileName(getItemInfo()->getName()));
	gcString link("desura://launch/{0}/{1}", getItemInfo()->getId().getTypeString(), getItemInfo()->getShortName());

	gcString icon(getItemInfo()->getIcon());

	if (UTIL::FS::isValidFile(icon))
	{
		gcString out(icon);
		out += ".ico";

		if (UTIL::MISC::convertToIco(icon.c_str(), out.c_str()))
			icon = out;
		else
			icon = "";
	}
	else
	{
		icon = "";
	}

#ifdef DEBUG
	if (icon == "")
		icon = UTIL::OS::getCurrentDir(L"\\desura.exe");
#else
	if (icon == "")
		icon = UTIL::OS::getCurrentDir(L"\\desura-d.exe");
#endif

	UTIL::FS::delFile(path);
	UTIL::WIN::createShortCut(gcWString(path).c_str(), link.c_str(), workingDir.c_str(), "", false, (icon.size()>0)?icon.c_str():NULL);

	return UTIL::FS::isValidFile(path);
#endif
}


}
}
