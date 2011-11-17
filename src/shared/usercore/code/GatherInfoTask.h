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

#ifndef DESURA_GATHERINFOTHREAD_H
#define DESURA_GATHERINFOTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseItemTask.h"
#include "usercore/ItemHelpersI.h"
#include "ItemHandle.h"


namespace UserCore
{
namespace ItemTask
{



class GatherInfoTask : public BaseItemTask
{
public:
	GatherInfoTask(UserCore::Item::ItemHandle *handle, MCFBranch branch, MCFBuild build, UserCore::Item::Helper::GatherInfoHandlerHelperI *helper, uint32 flags);
	~GatherInfoTask();

protected:
	virtual void onError(gcException& e);
	virtual void onComplete();

	virtual void cancel();

	void doRun();

	void checkRequirements();
	uint32 validate();

	bool isValidBranch();
	bool handleInvalidBranch();

	bool checkNullBranch(UserCore::Item::BranchInfoI* branchInfo);

	void completeStage();
	void resetStage();

private:
	UserCore::Item::Helper::GatherInfoHandlerHelperI* m_pGIHH;
	uint32 m_uiFlags;

	bool m_bFirstTime;
	volatile bool m_bCanceled;
};

}
}

#endif //DESURA_GATHERINFOTHREAD_H
