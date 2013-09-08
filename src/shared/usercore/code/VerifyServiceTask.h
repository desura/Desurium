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

#ifndef DESURA_VERIFYTHREAD_H
#define DESURA_VERIFYTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseItemTask.h"

namespace UserCore
{
namespace ItemTask
{
class VSBaseTask;

class VerifyServiceTask : public BaseItemTask
{
public:
	VerifyServiceTask(UserCore::Item::ItemHandle* handle, MCFBranch branch = MCFBranch(), MCFBuild build = MCFBuild(), bool files = true, bool tools = false, bool hooks = false);
	~VerifyServiceTask();

protected:
	void doRun();
	void onStop();


	void onProgress(MCFCore::Misc::ProgressInfo& prog);
	void finishVerify(UserCore::Misc::VerifyComplete::VSTATUS status = UserCore::Misc::VerifyComplete::V_COMPLETE, const char* installpath = NULL, bool endStage = true);

	void onError(gcException& e);
	void switchBranch();

	bool checkMcfDownload(gcString &path);
	bool checkMcf(bool &completeMcf);
	bool checkInstall(bool completeMcf);
	bool downloadMissingFiles();
	bool installMissingFiles();

	void updateStatus();
	bool checkItem();
	bool checkBranch();
	bool checkUnAuthed();

	void setupCurTask();
	void setTier(uint8 tier);

	bool checkFiles();
	bool checkTools();
	void checkHooks();

	void refreshInfo();

private:
	uint8 m_iTier;
	bool m_bError;

	bool m_bCheckFiles;
	bool m_bCheckTools;
	bool m_bCheckHooks;
	bool m_bRefreshedInfo;

	uint32 m_uiLastPercent;
	uint32 m_uiOldStatus;

	MCFBranch m_McfBranch;
	MCFBuild m_McfBuild;

	VSBaseTask* m_pCurTask;

	McfHandle m_hMcf;
};

}
}

#endif //DESURA_VERIFYTHREAD_H
