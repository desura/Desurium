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
#include "DownloadToolItemTask.h"
#include "UserTasks.h"
#include "User.h"



namespace UserCore
{
namespace ItemTask
{


DownloadToolTask::DownloadToolTask(UserCore::Item::ItemHandle* handle, uint32 ttid, const char* downloadPath, MCFBranch branch, MCFBuild build) : BaseItemTask(UserCore::Item::ItemHandleI::STAGE_DOWNLOADTOOL, "DownloadTool", handle, branch, build)
{
	m_szDownloadPath = downloadPath;
	m_ToolTTID = ttid;
	m_bLaunch = false;
	m_bInstallAfter = false;

	m_bCancelled = false;
}

DownloadToolTask::DownloadToolTask(UserCore::Item::ItemHandle* handle, bool launch) : BaseItemTask(UserCore::Item::ItemHandleI::STAGE_DOWNLOADTOOL, "DownloadTool", handle, MCFBranch(), MCFBuild())
{
	m_ToolTTID = UINT_MAX;
	m_bLaunch = launch;
	m_bInstallAfter = true;

	m_bCancelled = false;
}

DownloadToolTask::~DownloadToolTask()
{

}

void DownloadToolTask::doRun()
{
	uint32 per = 0;
	getItemInfo()->setPercent(per);

	if (m_ToolTTID == UINT_MAX)
		validateTools();

	std::vector<DesuraId> toolList;
	getItemInfo()->getCurrentBranch()->getToolList(toolList);

	UserCore::Misc::ToolTransaction* tt = new UserCore::Misc::ToolTransaction();

	tt->onCompleteEvent += delegate(this, &DownloadToolTask::onDLComplete);
	tt->onErrorEvent += delegate(this, &DownloadToolTask::onDLError);
	tt->onProgressEvent += delegate(this, &DownloadToolTask::onDLProgress);
	tt->toolsList = toolList;
	
	if (m_ToolTTID != UINT_MAX)
	{
		bool res = getUserCore()->getToolManager()->updateTransaction(m_ToolTTID, tt);

		if (!res) //must be complete
		{
			onComplete();
			return;
		}
	}
	else
	{
		m_ToolTTID = getUserCore()->getToolManager()->downloadTools(tt);
	}

	if (m_ToolTTID != UINT_MAX)
		m_WaitCond.wait();

	onComplete();
}

void DownloadToolTask::validateTools()
{
	std::vector<DesuraId> toolList;
	getItemInfo()->getCurrentBranch()->getToolList(toolList);

	if (toolList.size() == 0)
		return;

	if (!getUserCore()->getToolManager()->areAllToolsValid(toolList))
	{
		//missing tools. Gather info again
		tinyxml2::XMLDocument doc;

		getWebCore()->getItemInfo(getItemId(), doc, MCFBranch(), MCFBuild());

		tinyxml2::XMLNode *uNode = doc.FirstChild("iteminfo");

		if (!uNode)
			throw gcException(ERR_BADXML);

		tinyxml2::XMLNode *toolNode = uNode->FirstChild("toolinfo");

		if (toolNode)
			getUserCore()->getToolManager()->parseXml(toolNode);

		tinyxml2::XMLNode *gameNode = uNode->FirstChild("games");

		if (!gameNode)
			throw gcException(ERR_BADXML);

		getItemInfo()->getCurrentBranch()->getToolList(toolList);
	}

	if (!getUserCore()->getToolManager()->areAllToolsValid(toolList))
		throw gcException(ERR_INVALID, "Tool ids cannot be resolved into tools.");
}

void DownloadToolTask::onPause()
{
}

void DownloadToolTask::onUnpause()
{
}

void DownloadToolTask::onStop()
{
	UserCore::ItemTask::BaseItemTask::onStop();
	m_WaitCond.notify();
}

void DownloadToolTask::cancel()
{
	m_bCancelled = true;
	m_WaitCond.notify();
}

void DownloadToolTask::onDLProgress(UserCore::Misc::ToolProgress &p)
{
	MCFCore::Misc::ProgressInfo m;

	m.doneAmmount = p.done;
	m.totalAmmount = p.total;
	m.percent = p.percent;

	onMcfProgressEvent(m);
	getItemInfo()->setPercent(p.percent);
}

void DownloadToolTask::onDLError(gcException &e)
{
	//Dont worry about errors here. We will sort them out on launch
	Warning(gcString("Failed to download tool: {0}\n", e));
}

void DownloadToolTask::onDLComplete()
{
	m_WaitCond.notify();
}

void DownloadToolTask::onComplete()
{
	bool notComplete = isStopped() || m_bCancelled;

	getUserCore()->getToolManager()->removeTransaction(m_ToolTTID, notComplete);
	m_ToolTTID = UINT_MAX;

	std::vector<DesuraId> toolList;
	getItemInfo()->getCurrentBranch()->getToolList(toolList);

	if (!m_bCancelled && !getUserCore()->getToolManager()->areAllToolsDownloaded(toolList))
	{
		gcException e(ERR_INVALID, "Failed to download tools.");
		onErrorEvent(e);
		notComplete = true;
	}

	if (notComplete)
	{
		getItemHandle()->completeStage(true);
		return;
	}

	uint32 blank = 0;
	onCompleteEvent(blank);

	if (m_bInstallAfter)
	{
		getItemHandle()->goToStageInstallTools(m_bLaunch);
	}
	else
	{
		if (HasAllFlags(getItemInfo()->getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX))
			getItemHandle()->goToStageInstallComplex(getMcfBranch(), getMcfBuild());
		else
			getItemHandle()->goToStageInstall(m_szDownloadPath.c_str(), getMcfBranch());
	}
}


}
}
