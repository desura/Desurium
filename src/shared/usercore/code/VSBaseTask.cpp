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
#include "VSBaseTask.h"

#include "usercore/ItemHandleI.h"

#include "ItemInfo.h"
#include "ItemHandle.h"
#include "User.h"
#include "webcore/WebCoreI.h"

namespace UserCore
{
namespace ItemTask
{

VSBaseTask::VSBaseTask()
{
	m_bIsStopped = false;

	m_pHandle = NULL;
	m_pWebCore = NULL;
	m_pUserCore = NULL;

	m_Result = RES_NONE;
}

void VSBaseTask::setWebCore(WebCore::WebCoreI *wc)
{
	m_pWebCore = wc;
}

void VSBaseTask::setUserCore(UserCore::UserI *uc)
{
	m_pUserCore = uc;
}

void VSBaseTask::setItemHandle(UserCore::Item::ItemHandle* handle)
{
	m_pHandle = handle;
}

void VSBaseTask::setMcfBuild(MCFBuild build)
{
	m_uiMcfBuild = build;
}

void VSBaseTask::setMcfBranch(MCFBranch branch)
{
	m_uiMcfBranch = branch;
}


bool VSBaseTask::loadMcf(gcString mcfPath)
{
	//read file to make sure it is what it says it is
	m_hMcf->setFile(mcfPath.c_str());
		
	try
	{
		m_hMcf->parseMCF();
	}
	catch (gcException &except)
	{
		Warning(gcString("Verify - MCF Error: {0}\n", except));
		return false;
	}

	return true;
}

void VSBaseTask::setMcfHandle(MCFCore::MCFI* handle)
{
	m_hMcf = handle;
}


void VSBaseTask::setResult(RESULT res)
{
	if (m_Result != RES_STOPPED)
		m_Result = res;
}

VSBaseTask::RESULT VSBaseTask::getResult()
{
	return m_Result;
}

void VSBaseTask::stop()
{
	m_bIsStopped = true;
	m_Result = RES_STOPPED;

	onStop();
}



UserCore::Item::ItemHandle* VSBaseTask::getItemHandle()
{
	return m_pHandle;
}

UserCore::Item::ItemInfo* VSBaseTask::getItemInfo()
{
	return m_pHandle->getItemInfoNorm();
}

UserCore::Item::ItemInfo* VSBaseTask::getParentItemInfo()
{
	UserCore::Item::ItemInfo* item = getItemInfo();

	if (!m_pUserCore || !item)
		return NULL;

	return dynamic_cast<UserCore::Item::ItemInfo*>(m_pUserCore->getItemManager()->findItemInfo(item->getParentId()));
}

DesuraId VSBaseTask::getItemId()
{
	return getItemInfo()->getId();
}

WebCore::WebCoreI* VSBaseTask::getWebCore()
{
	return m_pWebCore;
}

UserCore::UserI* VSBaseTask::getUserCore()
{
	return m_pUserCore;
}

MCFBuild VSBaseTask::getMcfBuild()
{
	return m_uiMcfBuild;
}

MCFBranch VSBaseTask::getMcfBranch()
{
	return m_uiMcfBranch;
}

bool VSBaseTask::isStopped()
{
	return m_bIsStopped;
}

}
}
