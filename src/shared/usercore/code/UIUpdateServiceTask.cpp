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
#include "UIUpdateServiceTask.h"

#include "IPCServiceMain.h"
#include "IPCUninstallBranch.h"

namespace UserCore
{
namespace ItemTask
{


UIUpdateServiceTask::UIUpdateServiceTask(UserCore::Item::ItemHandle* handle, const char* path, MCFBuild lastBuild) 
	: UIBaseServiceTask(UserCore::Item::ItemHandleI::STAGE_UNINSTALL_UPDATE, "UnInstallUpdate", handle, MCFBranch(), lastBuild)
{
	m_pIPCIM = NULL;
	m_szPath = path;
}

UIUpdateServiceTask::~UIUpdateServiceTask()
{
	waitForFinish();

	if (m_pIPCIM)
	{
		m_pIPCIM->onCompleteEvent -= delegate(this, &UIUpdateServiceTask::onComplete);
		m_pIPCIM->onProgressEvent -= delegate(&onMcfProgressEvent);
		m_pIPCIM->onErrorEvent -= delegate((UIBaseServiceTask*)this, &UIBaseServiceTask::onServiceError);

		m_pIPCIM->destroy();
		m_pIPCIM = NULL;
	}
}

bool UIUpdateServiceTask::initService()
{
	if (!UIBaseServiceTask::initService())
	{
		onComplete();
		return false;
	}

	gcString oldBranchMcf = getBranchMcf(getItemInfo()->getId(), getItemInfo()->getInstalledBranch(), getMcfBuild());

	m_pIPCIM = getServiceMain()->newUninstallBranch();
			
	if (!m_pIPCIM)
	{
		gcException eFailCrtBrnch(ERR_NULLHANDLE, "Failed to create uninstall update mcf service!\n");
		onErrorEvent(eFailCrtBrnch);
		return false;
	}

	m_pIPCIM->onCompleteEvent += delegate(this, &UIUpdateServiceTask::onComplete);
	m_pIPCIM->onProgressEvent += delegate(&onMcfProgressEvent);
	m_pIPCIM->onErrorEvent += delegate((UIBaseServiceTask*)this, &UIBaseServiceTask::onServiceError);

	m_pIPCIM->start(oldBranchMcf.c_str(), m_szPath.c_str(), getItemInfo()->getPath(), "");

	return true;
}

void UIUpdateServiceTask::onComplete()
{
	MCFCore::Misc::ProgressInfo prog;
	prog.percent = 100;

	onMcfProgressEvent(prog);

	getItemHandle()->goToStageDownload(m_szPath.c_str());
	UIBaseServiceTask::onComplete();
}


}
}

