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

#ifndef DESURA_VALIDATETASK_H
#define DESURA_VALIDATETASK_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"

#include "BaseItemTask.h"
#include "usercore/ItemHelpersI.h"
#include "mcfcore/MCFI.h"


namespace UserCore
{
namespace Task
{
	class DownloadBannerTask;
}
namespace ItemTask
{

class ValidateTask : public UserCore::ItemTask::BaseItemTask
{
public:
	ValidateTask(UserCore::Item::ItemHandle* handle, MCFBranch branch, MCFBuild build = MCFBuild());
	virtual ~ValidateTask();

protected:
	void doRun();

	virtual void onStop();
	virtual void onPause();
	virtual void onUnpause();
	virtual void cancel();

	void onError(gcException &e);
	void onComplete(gcString &savePath);
	void onProgress(MCFCore::Misc::ProgressInfo& p);

	void updateStatusFlags();
	void validateHeader(MCFBuild &build, MCFBranch &branch);

	bool checkExistingMcf(gcString savePath);
	void copyLocalMcfs(MCFBranch branch, MCFBuild build);
	void copyLocalFiles();

	void onLocalFileProgress(MCFCore::Misc::ProgressInfo& p);

private:
	bool m_bInError;
	bool m_bUpdating;
	bool m_bUnAuthed;
	bool m_bUseLocalMcf;

	bool m_bLocalMcf;
	bool m_bLeaveLocalFiles;

	MCFBuild m_LastInsBuild;
	gcString m_szInstallPath;

	enum ACTION
	{
		CHECK_LOCALFILES,
		CHECK_EXISTINGMCF,
		CHECK_LOCALMCFS,
		PRE_ALLOCATING,
	};

	ACTION m_Action;
	void setAction(ACTION action);

	std::vector<uint32> m_vMcfFileCountList;
	uint32 m_CurMcfIndex;
	uint32 m_TotalFileCount;


	McfHandle *m_CurrentMcf;
};

}
}


#endif //DESURA_VALIDATETASK_H
