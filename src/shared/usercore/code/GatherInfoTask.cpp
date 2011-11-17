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
#include "GatherInfoTask.h"
#include "managers/WildcardManager.h"

#include "User.h"
#include "ItemInfo.h"
#include "ItemHandle.h"

namespace UserCore
{
namespace ItemTask
{

GatherInfoTask::GatherInfoTask(UserCore::Item::ItemHandle* handle, MCFBranch branch, MCFBuild build, UserCore::Item::Helper::GatherInfoHandlerHelperI *helper, uint32 flags) 
	: BaseItemTask(UserCore::Item::ItemHandleI::STAGE_GATHERINFO, "GatherInfo", handle, branch, build)
{
	m_pGIHH = helper;
	onErrorEvent += delegate(this, &GatherInfoTask::onError);
	m_uiFlags = flags;

	m_bFirstTime = true;
	m_bCanceled = false;
}

GatherInfoTask::~GatherInfoTask()
{
	if (m_pGIHH)
		m_pGIHH->destroy();
}

void GatherInfoTask::doRun()
{
	UserCore::Item::ItemInfo* item = getItemInfo();

	WildcardManager wildc = WildcardManager();
	wildc.onNeedSpecialEvent += delegate(&onNeedWCEvent);

	uint32 prog = 0;
	onProgUpdateEvent(prog);

	if (HasAnyFlags(m_uiFlags, GI_FLAG_UPDATE))
	{
		if (item)
			item->delSFlag(UserCore::Item::ItemInfoI::STATUS_READY);
	}
	else
	{
		if (HasAnyFlags(m_uiFlags, GI_FLAG_EXISTING))
		{
			uint32 flags = UserCore::Item::ItemInfoI::STATUS_LINK|
				UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER|
				UserCore::Item::ItemInfoI::STATUS_INSTALLED|
				UserCore::Item::ItemInfoI::STATUS_READY;

			if (item)
				item->delSFlag(flags);
		}

		uint32 flags = GI_FLAG_EXISTING;

		//force test links for games to always install to common
		if (item && item->getId().getType() == DesuraId::TYPE_GAME)
			flags |= GI_FLAG_TEST;

		getUserCore()->getItemManager()->retrieveItemInfo(getItemId(), 0, &wildc, MCFBranch(), getMcfBuild(), HasAnyFlags(m_uiFlags, flags));

		if (isStopped())
			return;
	}

	item = getItemInfo();

	if (!item)
		throw gcException(ERR_INVALIDDATA, "The item handle was null (gather info failed)");

	if (HasAnyFlags(item->getStatus(), UserCore::Item::ItemInfo::STATUS_STUB))
		throw gcException(ERR_UNSUPPORTEDPLATFORM, 0, "There are no releases for this platform");

	prog=100;
	onProgUpdateEvent(prog);

	if (m_bCanceled)
	{
		completeStage();
		return;
	}

	uint32 itemId = item->getId().getItem();
	onCompleteEvent(itemId);
	onComplete();
}

void GatherInfoTask::completeStage()
{
	if (HasAnyFlags(m_uiFlags, GI_FLAG_UPDATE))
		getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_READY);

	getItemHandle()->completeStage(true);
}

void GatherInfoTask::resetStage()
{
	if (HasAnyFlags(m_uiFlags, GI_FLAG_UPDATE))
		completeStage();
	else
		getItemHandle()->resetStage(true);
}

void GatherInfoTask::onError(gcException &e)
{
	if (!HasAnyFlags(m_uiFlags, GI_FLAG_EXISTING|GI_FLAG_UPDATE))
		getItemHandle()->resetStage(true);
}

bool GatherInfoTask::isValidBranch()
{
	MCFBranch branch = getMcfBranch();

	if (branch.isGlobal())
	{
		branch = getItemInfo()->getBestBranch(branch);
		m_uiMcfBranch = branch;
	}

	if (branch == 0)
		return false;

	//asume that test installs are always valid
	if (HasAnyFlags(m_uiFlags, GI_FLAG_TEST))
		return true;

	UserCore::Item::BranchInfoI* bInfo = getItemInfo()->getBranchById(getMcfBranch());
	return bInfo && HasAnyFlags(bInfo->getFlags(), UserCore::Item::BranchInfoI::BF_DEMO|UserCore::Item::BranchInfoI::BF_FREE|UserCore::Item::BranchInfoI::BF_ONACCOUNT|UserCore::Item::BranchInfoI::BF_TEST);
}

void GatherInfoTask::onComplete()
{
	if (getItemHandle()->getItemInfo()->isDownloadable())
		checkRequirements();
	else
		getItemHandle()->completeStage(true);
}


bool GatherInfoTask::checkNullBranch(UserCore::Item::BranchInfoI* branchInfo)
{
	if (branchInfo)
		return true;

	if (!getUserCore()->getItemManager()->isKnownBranch(getMcfBranch(), getItemId()))
		throw gcException(ERR_BADITEM, "Branch is invalid or user doesnt have permissions to install branch.");

	if (!m_bFirstTime || HasAnyFlags(m_uiFlags, GI_FLAG_TEST))
		throw gcException(ERR_UNSUPPORTEDPLATFORM, 1, "This branch is not supported on this platform");

	m_bFirstTime = false;
	bool res = false;

	if (!m_bCanceled && m_pGIHH)
		res = m_pGIHH->showPlatformError();

	if (!res)
	{
		completeStage();
	}
	else
	{
		m_uiMcfBranch = MCFBranch::BranchFromInt(0);
		checkRequirements();
	}

	return false;
}

bool GatherInfoTask::handleInvalidBranch()
{
	MCFBranch branch = getMcfBranch();

	if (m_bCanceled || (m_pGIHH && !m_pGIHH->selectBranch(branch)))
	{
		completeStage();
		return false;
	}

	if (branch == UINT_MAX)
	{
		//means the item was set as a link and needs to be launched
		getItemHandle()->goToStageLaunch();
		return false;
	}
	else
	{
		if (branch.isGlobal())
			branch = getItemInfo()->getBestBranch(branch);

		UserCore::Item::BranchInfoI* branchInfo = getItemInfo()->getBranchById(branch);
		checkNullBranch(branchInfo);
	}

	m_uiMcfBranch = branch;
	return true;
}

void GatherInfoTask::checkRequirements()
{
	if (!isValidBranch() && !handleInvalidBranch())
		return;

	MCFBranch branch = getMcfBranch();

	UserCore::Item::BranchInfoI* branchInfo = getItemHandle()->getItemInfo()->getBranchById(branch);
	
	if (!checkNullBranch(branchInfo))
		return;

	if (m_bCanceled)
	{
		completeStage();
		return;
	}

	if (branchInfo->isPreOrder())
	{
		//show the preorder prompt
		getItemHandle()->goToStageLaunch();
		return;
	}

	uint32 res = validate();
	
#ifdef NIX
	UserCore::Item::Helper::TOOL tool = (UserCore::Item::Helper::TOOL)(res>>28);

	//check tool deps
	if ( (tool > 0) && (!m_pGIHH || !m_pGIHH->showToolPrompt(tool)) )
	{
		completeStage();
		return;				
	}
	
	res = (res<<4)>>4;
#endif
	
	if (res == 0)
	{
		getItemHandle()->getItemInfo()->addToAccount();
		getItemHandle()->goToStageDownload(branch, getMcfBuild(), HasAnyFlags(m_uiFlags, GI_FLAG_TEST));
	}
	else
	{
		if (res == UserCore::Item::Helper::V_NONEMPTY)
		{
			if (HasAnyFlags(m_uiFlags, GI_FLAG_UPDATE))
			{
				getItemHandle()->goToStageDownload(branch, getMcfBuild(), false);
			}
			else if (getItemHandle()->getItemInfo()->getStatus() & UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX)
			{
				if (!m_bCanceled && m_pGIHH && m_pGIHH->showComplexPrompt())
					getItemHandle()->goToStageDownload(branch, getMcfBuild(), HasAnyFlags(m_uiFlags, GI_FLAG_TEST));
				else
					resetStage();
			}
			else
			{
				UserCore::Item::Helper::ACTION promptRes = UserCore::Item::Helper::C_NONE;
				
				if (!m_bCanceled && m_pGIHH)
					promptRes = m_pGIHH->showInstallPrompt(getItemHandle()->getItemInfo()->getPath());

				switch (promptRes)
				{
				case UserCore::Item::Helper::C_REMOVE:
						getItemInfo()->addOFlag(UserCore::Item::ItemInfoI::OPTION_REMOVEFILES);

				case UserCore::Item::Helper::C_INSTALL:
						getItemInfo()->addToAccount();
						getItemHandle()->goToStageDownload(branch, getMcfBuild(), HasAnyFlags(m_uiFlags, GI_FLAG_TEST));
						break;

				case UserCore::Item::Helper::C_VERIFY:
						getItemInfo()->addToAccount();
						getItemInfo()->addSFlag(UserCore::Item::ItemInfoI::STATUS_INSTALLED);
						getItemInfo()->setInstalledMcf(branch, getMcfBuild());
						getItemHandle()->goToStageVerify(branch, getMcfBuild(), true, true, true);
						break;

					default:
						resetStage();
						break;
				}
			}
		}
		else
		{
			if (m_bCanceled || !m_pGIHH || m_pGIHH->showError(res))
			{
				resetStage();
			}
			else
			{
				getItemHandle()->getItemInfo()->addToAccount();
				getItemHandle()->goToStageDownload(branch, getMcfBuild(), HasAnyFlags(m_uiFlags, GI_FLAG_TEST));
			}
		}
	}
}


uint32 GatherInfoTask::validate()
{
	UserCore::Item::ItemInfoI* pItemInfo = getItemHandle()->getItemInfo();
	uint32 isValid = 0;

	if (!pItemInfo)
		return UserCore::Item::Helper::V_BADINFO;

	DesuraId par = pItemInfo->getParentId();
	UserCore::Item::ItemInfoI *parInfo = NULL;

	if (par.isOk())
	{
		parInfo = getItemHandle()->getUserCore()->getItemManager()->findItemInfo(par);

		if (!parInfo || !(parInfo->getStatus() & UserCore::Item::ItemInfoI::STATUS_INSTALLED))
			isValid |= UserCore::Item::Helper::V_PARENT;
	}

	if (!pItemInfo->getPath())
	{
		isValid |= UserCore::Item::Helper::V_BADPATH;
	}
	else
	{
		const char *comAppPath = getUserCore()->getAppDataPath();

		uint64 inFreeSpace = UTIL::OS::getFreeSpace(pItemInfo->getPath());
		uint64 dlFreeSpace = UTIL::OS::getFreeSpace(comAppPath);

		//if they are on the same drive:
		if (strncmp(comAppPath, pItemInfo->getPath(), 3) == 0)
		{
			if ((inFreeSpace+dlFreeSpace) < (pItemInfo->getDownloadSize()+pItemInfo->getInstallSize()))
			{
				isValid |= (UserCore::Item::Helper::V_FREESPACE|UserCore::Item::Helper::V_FREESPACE_DL|UserCore::Item::Helper::V_FREESPACE_INS);
			}
		}
		else
		{
			if (dlFreeSpace < pItemInfo->getDownloadSize())
				isValid |= UserCore::Item::Helper::V_FREESPACE|UserCore::Item::Helper::V_FREESPACE_DL;

			if (inFreeSpace < pItemInfo->getInstallSize())
				isValid |= UserCore::Item::Helper::V_FREESPACE|UserCore::Item::Helper::V_FREESPACE_INS;
		}

		if (pItemInfo->getStatus() & UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX)
		{
			const char* primPath = pItemInfo->getInsPrimary();

			if (primPath && strcmp(primPath, "") != 0 && UTIL::FS::isValidFolder(primPath) && !UTIL::FS::isFolderEmpty(primPath))
				isValid |= UserCore::Item::Helper::V_NONEMPTY;
		}
		else if (pItemInfo->getStatus() & UserCore::Item::ItemInfoI::STATUS_DLC)
		{
			if (!parInfo || gcString(pItemInfo->getPath()) != gcString(parInfo->getPath()))
			{
				if (!UTIL::FS::isFolderEmpty(pItemInfo->getPath()))
					isValid |= UserCore::Item::Helper::V_NONEMPTY;
			}
		}
		else if (!UTIL::FS::isFolderEmpty(pItemInfo->getPath()))
		{
			isValid |= UserCore::Item::Helper::V_NONEMPTY;
		}
	}

#ifdef NIX
	UserCore::Item::BranchInfoI* bi = pItemInfo->getBranchById(getMcfBranch());
	
	std::vector<DesuraId> toolList;
	bi->getToolList(toolList);

	uint32 res = getUserCore()->getToolManager()->hasNonInstallableTool(toolList);
	
	switch (res)
	{
	case 0:
		isValid |= UserCore::Item::Helper::V_JAVA_SUN;
		break;		
		
	case 1:
		isValid |= UserCore::Item::Helper::V_JAVA;
		break;		
	
	case 2:
		isValid |= UserCore::Item::Helper::V_MONO;
		break;
		
	case 3:
		isValid |= UserCore::Item::Helper::V_AIR;
		break;		
	};
#endif

	return isValid;
}

void GatherInfoTask::cancel()
{
	m_bCanceled = true;
}



}
}
