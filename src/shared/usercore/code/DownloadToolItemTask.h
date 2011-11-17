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

#ifndef DESURA_DOWNLOADTOOLITEMTASK_H
#define DESURA_DOWNLOADTOOLITEMTASK_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseItemTask.h"
#include "usercore/ItemHelpersI.h"
#include "ToolManager.h"

#include "util_thread/BaseThread.h"

namespace UserCore
{
namespace ItemTask
{


class DownloadToolTask : public UserCore::ItemTask::BaseItemTask
{
public:
	DownloadToolTask(UserCore::Item::ItemHandle* handle, uint32 ttid, const char* downloadPath, MCFBranch branch, MCFBuild build);
	DownloadToolTask(UserCore::Item::ItemHandle* handle, bool launch);

	virtual ~DownloadToolTask();
	virtual void cancel();

protected:
	void doRun();

	virtual void onPause();
	virtual void onUnpause();
	virtual void onStop();

	void onDLProgress(UserCore::Misc::ToolProgress &p);
	void onDLError(gcException &e);
	void onDLComplete();

	void onComplete();
	void validateTools();

private:
	gcString m_szDownloadPath;
	uint32 m_ToolTTID;

	bool m_bLaunch;
	bool m_bInstallAfter;
	bool m_bCancelled;

	::Thread::WaitCondition m_WaitCond;
};

}
}

#endif //DESURA_DOWNLOADTOOLITEMTASK_H
