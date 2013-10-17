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
#include "ComplexLaunchServiceTask.h"

#include "IPCServiceMain.h"
#include "IPCComplexLaunch.h"

#include "User.h"
#include "ItemInfo.h"
#include "ItemHandle.h"

#include "McfManager.h"

namespace UserCore
{
namespace ItemTask
{

enum
{
	REMOVING,
	BACKUP,
	INSTALLING,
};

enum
{
	T_REMOVEING,
	T_INSTALLING,
	T_INSTALLREMOVE,
	T_UPDATEREMOVE,
};

ComplexLaunchServiceTask::ComplexLaunchServiceTask(UserCore::Item::ItemHandle *handle, bool clean, MCFBranch branch, MCFBuild build, UserCore::Item::Helper::InstallerHandleHelperI* ihh) : BaseItemServiceTask(UserCore::Item::ItemHandleI::STAGE_INSTALL_COMPLEX, "ComplexLaunch", handle, branch, build)
{
	m_pIPCIM = NULL;
	m_iTier = 0;
	m_iMode = 0;
	m_bClean = clean;
	m_bCompleteStage = false;
	m_bLaunch = false;
	m_bHashMissMatch = false;
	m_pIHH = ihh;
}

ComplexLaunchServiceTask::~ComplexLaunchServiceTask()
{
	waitForFinish();

	if (m_pIPCIM)
	{
		m_pIPCIM->onCompleteEvent -= delegate(this, &ComplexLaunchServiceTask::onComplete);
		m_pIPCIM->onProgressEvent -= delegate(this, &ComplexLaunchServiceTask::onProgress);
		m_pIPCIM->onErrorEvent -= delegate(this, &ComplexLaunchServiceTask::onError);
		
		m_pIPCIM->destroy();
		m_pIPCIM = NULL;
	}
}

void ComplexLaunchServiceTask::onFinish()
{
	BaseItemServiceTask::onFinish();
}

bool ComplexLaunchServiceTask::initService()
{
	gcException eBadItem(ERR_BADITEM);
	gcException eFailCrtInstSvc(ERR_NULLHANDLE, "Failed to create install mcf service!\n");
	
	UserCore::Item::ItemInfo *pItem = getItemInfo();
	if (!pItem)
	{
		onErrorEvent(eBadItem);
		return false;
	}

	m_pIPCIM = getServiceMain()->newComplexLaunch();
	if (!m_pIPCIM)
	{
		onErrorEvent(eFailCrtInstSvc);
		return false;
	}

	pItem->setPercent(0);
	pItem->delSFlag(UserCore::Item::ItemInfoI::STATUS_READY);
	pItem->addSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLING);

	m_pIPCIM->onCompleteEvent += delegate(this, &ComplexLaunchServiceTask::onComplete);
	m_pIPCIM->onProgressEvent += delegate(this, &ComplexLaunchServiceTask::onProgress);
	m_pIPCIM->onErrorEvent += delegate(this, &ComplexLaunchServiceTask::onError);
	
	if (pItem->getId().getType() == DesuraId::TYPE_GAME)
	{
		//if this is the game and no complex mods are installed just launch
		if (!pItem->isParentToComplex())
		{
			onComplete();
			return false;
		}
		else
		{
			UserCore::MCFManager *mm = UserCore::GetMCFManager();
			gcString backup = mm->getMcfBackup(pItem->getId(), pItem->getInstalledModId());

			if (!m_bClean)
			{

				try
				{
					McfHandle mcfH;
					mcfH->setFile(backup.c_str());
					mcfH->parseMCF();


					bool empty = true;
					for (size_t x=0; x<mcfH->getFileCount(); x++)
					{
						MCFCore::MCFFileI* file = mcfH->getMCFFile(x);

						if (file && file->isSaved())
						{
							empty = false;
							break;
						}
					}

					//if no conflict files, just launch
					if (empty)
					{
						onComplete();
						return false;
					}
				}
				catch (gcException &)
				{
				}
			}

			//need to uninstall mod
			m_iTier = T_REMOVEING;
			m_iRemoveId = pItem->getInstalledModId();
			return remove();
		}
	}
	else
	{
		UserCore::Item::ItemInfo *pParentItem = getParentItemInfo();
		gcException eParentNull(ERR_BADITEM, "Parent was NULL");

		if (!pParentItem)
		{
			onErrorEvent(eParentNull);
			return false;
		}

		//no complex mods installed
		if (!pParentItem->getInstalledModId().isOk())
		{
			m_iTier = T_INSTALLING;
			return install();
		}
		else
		{
			m_iTier = T_INSTALLREMOVE;
			m_iRemoveId = pParentItem->getInstalledModId();
			return removeAndInstall();
		}
	}

	return false;
}


bool ComplexLaunchServiceTask::install()
{
	m_iMode = BACKUP;

	UserCore::MCFManager *mm = UserCore::GetMCFManager();

	gcString path = getFullMcf();

	if (path == "")
		return false;

	gcString insPath = getItemInfo()->getPath();

	if ( UTIL::FS::isValidFolder(insPath) )
	{
		gcString parPath = mm->newMcfBackup(getItemInfo()->getParentId(), getItemId());

		MCFCore::MCFHeaderI *head = (MCFCore::MCFHeaderI*)MCFCore::FactoryBuilder(MCF_HEADER_FACTORY);
		head->setId( getItemInfo()->getParentId().getItem() );
		head->setType( getItemInfo()->getParentId().getType() );
		head->addFlags( MCFCore::MCFHeaderI::FLAG_NOCLEANUP );
		head->setBuild( MCFBuild() );

		McfHandle mcfParH;
		mcfParH->getErrorEvent() += delegate(&onErrorEvent);
		mcfParH->getProgEvent() += delegate(this, &ComplexLaunchServiceTask::onProgress);
		mcfParH->setHeader(head);
		mcfParH->setFile(parPath.c_str());

		FactoryDelete(head, MCF_HEADER_FACTORY);

		McfHandle mcfChildH;
		mcfChildH->setFile(path.c_str());

		try
		{
			mcfParH->parseFolder(insPath.c_str());
			mcfChildH->parseMCF();

			mcfParH->hashFiles(mcfChildH.handle());
			mcfParH->markFiles(mcfChildH.handle(), true, true, false, false);
			mcfParH->removeNonSavedFiles();

			if (mcfParH->getFileCount() > 0)
				mcfParH->saveMCF();
			else
				mm->delMcfBackup(getItemInfo()->getParentId(), getItemId());
		}
		catch (gcException &e)
		{
			mm->delMcfBackup(getItemInfo()->getParentId(), getItemId());
			onErrorEvent(e);
			return false;
		}
	}

	getUserCore()->getItemManager()->setInstalledMod(getParentItemInfo()->getId(), getItemId());
	m_iMode = INSTALLING;

	resetFinish();
	m_pIPCIM->startInstall(path.c_str(), insPath.c_str(), getItemInfo()->getInstallScriptPath());

	return true;
}

gcString ComplexLaunchServiceTask::getFullMcf()
{
	UserCore::MCFManager *mm = UserCore::GetMCFManager();
	
	gcString path = mm->getMcfPath(getItemInfo());

	if (path == "")
		return path;

	McfHandle mcfChildH;
	mcfChildH->setFile(path.c_str());

	try
	{
		mcfChildH->parseMCF();
	}
	catch (gcException &)
	{
		return "";
	}

	return path;
}

bool ComplexLaunchServiceTask::remove()
{
	gcException eItemNull(ERR_NULLHANDLE, "Item that is ment to be removed for complex install is null.");
	gcException eNoInstBrch(ERR_NULLHANDLE, "Item that is ment to be removed for complex install has no installed branches.");
	gcException eBadPath(ERR_BADPATH, "Mcf path was null or invalid.");

	UserCore::Item::ItemInfoI *item = getUserCore()->getItemManager()->findItemInfo(m_iRemoveId);
	
	if (!item)
	{
		onError(eItemNull);
		return false;
	}

	if (!item->getCurrentBranch())
	{
		onError(eNoInstBrch);
		return false;
	}

	m_iMode = REMOVING;
	UserCore::MCFManager *mm = UserCore::GetMCFManager();

	MCFBuild build = item->getInstalledBuild();
	MCFBranch branch = item->getCurrentBranch()->getBranchId();

	if (m_iRemoveId == getItemId())
	{
		build = item->getLastInstalledBuild();
		branch = item->getLastInstalledBranch();
	}

	gcString path = mm->getMcfPath(item->getId(), branch, build, false);

	//Might not have a backup
	if (path == "" || !UTIL::FS::isValidFile(path))
	{
		onError(eBadPath);
		return false;
	}

	gcString parPath = mm->getMcfBackup(item->getParentId(), item->getId());
	gcString insPath = item->getPath();

	m_pIPCIM->startRemove(path.c_str(), parPath.c_str(), insPath.c_str(), getItemInfo()->getInstallScriptPath());

	return true;
}

bool ComplexLaunchServiceTask::removeAndInstall()
{
	if (!remove())
		return false;

	waitForFinish();
	completeRemove();
	install();

	if (isStopped())
		return false;

	waitForFinish();
	completeInstall();

	onTrueComplete();
	return false;
}

void ComplexLaunchServiceTask::onComplete()
{
	if (m_iTier != T_INSTALLREMOVE)
	{
		if (m_iTier == T_INSTALLING)
		{
			completeInstall();
		}
		else if (m_iTier == T_REMOVEING)
		{
			completeRemove();
		}

		onTrueComplete();
	}

	onFinish();
}

void ComplexLaunchServiceTask::onTrueComplete()
{
	getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLING);
	getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_READY);

	bool verify = false;

	if (m_bHashMissMatch && m_pIHH)
		verify = m_pIHH->verifyAfterHashFail();

	if (verify)
		getItemHandle()->goToStageVerify(getItemInfo()->getInstalledBranch(), getItemInfo()->getInstalledBuild(), true, true, true);
	else if (m_bCompleteStage)
		getItemHandle()->completeStage(false);
	else if (m_bLaunch)
		getItemHandle()->goToStageLaunch();

	uint32 com = m_bHashMissMatch?1:0;
	onCompleteEvent(com);
}

void ComplexLaunchServiceTask::completeRemove()
{
	UserCore::Item::ItemInfoI *item = getUserCore()->getItemManager()->findItemInfo(m_iRemoveId);

	if (!item)
		return;

	UserCore::MCFManager *mm = UserCore::GetMCFManager();
	mm->delMcfBackup(item->getParentId(), m_iRemoveId);

	getUserCore()->getItemManager()->setInstalledMod(item->getParentId(), DesuraId());
}

void ComplexLaunchServiceTask::completeInstall()
{
	UserCore::Item::ItemInfo *pItem = getItemInfo();

	if (pItem->isUpdating() && getMcfBuild() == pItem->getNextUpdateBuild())
		pItem->updated();

	pItem->delSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLING|UserCore::Item::ItemInfoI::STATUS_UPDATING|UserCore::Item::ItemInfoI::STATUS_DOWNLOADING);
	pItem->addSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_READY);

	pItem->setInstalledMcf(getMcfBranch(), getMcfBuild());
}

void ComplexLaunchServiceTask::onError(gcException &e)
{
	if (e.getErrId() == ERR_HASHMISSMATCH)
	{
		m_bHashMissMatch = true;
		return;
	}

	Warning(gcString("Error in complex launch service: {0}\n", e));
	getItemHandle()->setPausable(false);

	if (!getItemHandle()->shouldPauseOnError())
	{
		getItemInfo()->delSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLING|UserCore::Item::ItemInfoI::STATUS_UPDATING|UserCore::Item::ItemInfoI::STATUS_DOWNLOADING);
		getItemHandle()->resetStage(true);
		onStop();
	}
	else
	{
		getItemHandle()->setPaused(true, true);
	}

	onErrorEvent(e);
}

void ComplexLaunchServiceTask::onProgress(MCFCore::Misc::ProgressInfo &p)
{
	uint32 prog = p.percent;
	uint32 percent = 0;
	uint32 oFlag = p.flag;

	if (m_iTier == T_REMOVEING)
	{
		percent = prog; 
		p.flag = 1;
	}
	else if (m_iTier == T_INSTALLING)
	{
		switch (m_iMode)
		{
			case BACKUP: 
				percent = 00 + (prog/5);  
				p.flag = 2;
				break;

			case INSTALLING: 
				percent = 20 + (prog*4/5); 
				p.flag = 3;
				break;
		};
	}
	else //installing and removing
	{
		switch (m_iMode)
		{
			case REMOVING: 
				percent = 00 + (prog/10);  
				p.flag = 1;
				break;

			case BACKUP: 
				percent = 10 + (prog/10);  
				p.flag = 2;
				break;

			case INSTALLING: 
				percent = 20 + (prog*8/10); 
				p.flag = 3;
				break;
		};
	}

	if (oFlag == 0)
		getItemInfo()->setPercent(percent);
#ifdef WIN32
	else
		int a =1;
#endif

	p.flag = (p.flag << 4) + oFlag;

	p.percent = percent;
	onMcfProgressEvent(p);
}


}
}
