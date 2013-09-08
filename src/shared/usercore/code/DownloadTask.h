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

#ifndef DESURA_DOWNLOADTHREAD_H
#define DESURA_DOWNLOADTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"

#include "BaseItemTask.h"
#include "usercore/ItemHelpersI.h"
#include "mcfcore/MCFI.h"

#include "BDManager.h"

namespace UserCore
{
namespace Task
{
	class DownloadBannerTask;
}
namespace ItemTask
{

class DownloadTask : public UserCore::ItemTask::BaseItemTask, public UserCore::Misc::BannerNotifierI
{
public:
	DownloadTask(UserCore::Item::ItemHandle* handle, const char* mcfPath);
	virtual ~DownloadTask();

protected:
	void doRun();

	virtual void onPause();
	virtual void onUnpause();
	virtual void onStop();
	virtual void cancel();

	void clearEvents();

	void onProgress(MCFCore::Misc::ProgressInfo& p);

	void onBannerError(gcException &e);
	void onBannerComplete(MCFCore::Misc::DownloadProvider &info);

	void onNewProvider(MCFCore::Misc::DP_s& dp);

	void onError(gcException &e);
	void sortLocalMcfs(MCFBranch branch, MCFBuild build);

	void onComplete(gcString &savePath);
	void onToolComplete();

	void startToolDownload();

private:
	uint32 m_uiOldBuild;
	uint32 m_uiOldBranch;

	uint32 m_ToolTTID;

	bool m_bInError;
	bool m_bToolDownloadComplete;
	bool m_bInitFinished;
	

	gcString m_szMcfPath;
};

}
}

#endif //DESURA_DOWNLOADTHREAD_H
