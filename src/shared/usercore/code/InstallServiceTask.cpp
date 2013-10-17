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
#include "InstallServiceTask.h"

#include "User.h"
#include "ItemInfo.h"
#include "ItemHandle.h"

#include "IPCServiceMain.h"
#include "IPCInstallMcf.h"

#include "mcfcore/UserCookies.h"
#include "usercore/ItemHelpersI.h"

#include "McfManager.h"

#ifdef NIX
#include "util/UtilLinux.h"
#endif

namespace UserCore
{
namespace ItemTask
{

InstallServiceTask::InstallServiceTask(UserCore::Item::ItemHandle* handle, const char* path, MCFBranch branch, UserCore::Item::Helper::InstallerHandleHelperI* ihh) : BaseItemServiceTask(UserCore::Item::ItemHandleI::STAGE_INSTALL, "Install", handle, branch)
{
	m_szPath = gcString(path);

	m_bHasError = false;
	m_bInstalling = false;
	m_bHashMissMatch = false;
	m_pIPCIM = NULL;
	m_pIHH = ihh;
}

InstallServiceTask::~InstallServiceTask()
{
	gcException eFailCreateHandle(ERR_NULLHANDLE, "Failed to create install mcf service!\n");

	if (hasStarted())
		waitForFinish();

	if (m_pIPCIM)
	{
		m_pIPCIM->onCompleteEvent -= delegate(this, &InstallServiceTask::onComplete);
		m_pIPCIM->onProgressEvent -= delegate(this, &InstallServiceTask::onProgUpdate);
		m_pIPCIM->onErrorEvent -= delegate(this, &InstallServiceTask::onError);
		m_pIPCIM->onFinishEvent -= delegate(this, &InstallServiceTask::onFinish);
		m_pIPCIM->destroy();
	}

	if (m_pIHH)
		m_pIHH->destroy();
}

bool InstallServiceTask::initService()
{
	gcException eBadItem(ERR_BADITEM);

	UserCore::Item::ItemInfo *pItem = getItemInfo();
	if (!pItem)
	{
		onErrorEvent(eBadItem);
		return false;
	}

	m_pIPCIM = getServiceMain()->newInstallMcf();
	if (!m_pIPCIM)
	{
		gcException eFailCreateHandle(ERR_NULLHANDLE, "Failed to create install mcf service!\n");
		onErrorEvent(eFailCreateHandle);
		return false;
	}

	if (!pItem->isUpdating())
	{
		pItem->setPercent(0);
		pItem->delSFlag(UserCore::Item::ItemInfoI::STATUS_DOWNLOADING|UserCore::Item::ItemInfoI::STATUS_READY);
		pItem->addSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLING);
	}

	const char* val = getUserCore()->getCVarValue("gc_corecount");
	bool removeFiles = (pItem->getOptions() & UserCore::Item::ItemInfoI::OPTION_REMOVEFILES)?true:false;

	gcString gc_writeable = getUserCore()->getCVarValue("gc_ignore_windows_permissions_against_marks_wishes");

	bool makeWritable = (gc_writeable == "true" || gc_writeable == "1");

	m_pIPCIM->onCompleteEvent += delegate(this, &InstallServiceTask::onComplete);
	m_pIPCIM->onProgressEvent += delegate(this, &InstallServiceTask::onProgUpdate);
	m_pIPCIM->onErrorEvent += delegate(this, &InstallServiceTask::onError);
	m_pIPCIM->onFinishEvent += delegate(this, &InstallServiceTask::onFinish);

	uint8 workers = 1;
		
	if (val)
		workers = atoi(val);

	if (workers == 0)
		workers = 1;

	m_pIPCIM->start(m_szPath.c_str(), pItem->getPath(), getItemInfo()->getInstallScriptPath(), workers, removeFiles, makeWritable);

	return true;
}

void InstallServiceTask::onComplete()
{
	if (m_bHasError)
	{
		onFinish();
		return;
	}


#ifdef NIX
	getItemHandle()->installLaunchScripts();
#endif

	UserCore::Item::ItemInfo *pItem = getItemInfo();

	if (pItem->isUpdating() && getMcfBuild() == pItem->getNextUpdateBuild())
		pItem->updated();

	pItem->delSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLING|UserCore::Item::ItemInfoI::STATUS_UPDATING|UserCore::Item::ItemInfoI::STATUS_DOWNLOADING);
	pItem->addSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_READY);
	
	if (pItem->isUpdating())
		pItem->addSFlag(UserCore::Item::ItemInfoI::STATUS_NEEDCLEANUP);

	MCFCore::Misc::ProgressInfo temp;
	temp.percent = 100;

	onMcfProgressEvent(temp);

	UserCore::Item::ItemInfoI *item = getItemHandle()->getItemInfo();
	item->delSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSABLE);

	bool verify = false;

	if (m_bHashMissMatch && m_pIHH)
		verify = m_pIHH->verifyAfterHashFail();

	uint32 com = m_bHashMissMatch?1:0;
	onCompleteEvent(com);

	if (verify)
		getItemHandle()->goToStageVerify(getItemInfo()->getInstalledBranch(), getItemInfo()->getInstalledBuild(), true, true, true);
	else
		getItemHandle()->completeStage(false);

	onFinish();
}


void InstallServiceTask::onPause()
{
	if (m_pIPCIM)
		m_pIPCIM->pause();
}


void InstallServiceTask::onUnpause()
{
	if (m_pIPCIM)
		m_pIPCIM->unpause();
}

void InstallServiceTask::onStop()
{
	if (m_pIPCIM)
		m_pIPCIM->stop();

	BaseItemServiceTask::onStop();
}

void InstallServiceTask::onProgUpdate(MCFCore::Misc::ProgressInfo& info)
{
	if (info.flag == 0)
	{
		if (getItemInfo()->isUpdating())
		{
			//for updating installing is the second 50%
			getItemInfo()->setPercent(50+info.percent/2);
		}
		else
		{
			getItemInfo()->setPercent(info.percent);
		}

		if (!(getItemHandle()->getItemInfo()->getStatus() & UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX))
			getItemHandle()->getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_PAUSABLE);
	}

	onMcfProgressEvent(info);
}

void InstallServiceTask::onError(gcException &e)
{
	if (e.getErrId() == ERR_HASHMISSMATCH)
	{
		m_bHashMissMatch = true;
		return;
	}

	m_bHasError = true;

	Warning(gcString("Error in MCF install: {0}\n", e));
	getItemHandle()->setPausable(false);

	if (!getItemHandle()->shouldPauseOnError())
	{
		getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLING|UserCore::Item::ItemInfoI::STATUS_UPDATING|UserCore::Item::ItemInfoI::STATUS_DOWNLOADING);
		getItemHandle()->resetStage(true);
	}
	else
	{
		getItemHandle()->setPaused(true, true);
	}

	onErrorEvent(e);
}

void InstallServiceTask::onFinish()
{
	BaseItemServiceTask::onFinish();
}


#ifdef NIX

#endif


}
}
