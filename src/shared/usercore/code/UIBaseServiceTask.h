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

#ifndef DESURA_UIBASESERVICETASK_H
#define DESURA_UIBASESERVICETASK_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseItemServiceTask.h"
#include "User.h"
#include "ItemInfo.h"
#include "webcore/WebCoreI.h"

namespace UserCore
{
namespace ItemTask
{


class UIBaseServiceTask : public BaseItemServiceTask
{
public:
	UIBaseServiceTask(uint32 stage, const char* taskname, UserCore::Item::ItemHandle* handle, MCFBranch installBranch = MCFBranch(), MCFBuild installBuild = MCFBuild());
	~UIBaseServiceTask();

	virtual void onServiceError(gcException& e);

protected:
	virtual void onComplete();
	virtual void onStop();
	
	gcString getBranchMcf(DesuraId id, MCFBranch branch, MCFBuild build);
	bool checkPath(const char* path, MCFBranch branch, MCFBuild build);

	virtual bool initService();

	void completeUninstall(bool removeAll, bool removeAccount);

#ifdef NIX
	void removeScripts();
#endif

	MCFBranch m_OldBranch;
	MCFBuild m_OldBuild;

	DesuraId m_idLastInstalledMod;
};


}
}

#endif //DESURA_UIBASESERVICETASK_H
