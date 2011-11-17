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

#ifndef DESURA_UIUPDATESERVICETASK_H
#define DESURA_UIUPDATESERVICETASK_H
#ifdef _WIN32
#pragma once
#endif

#include "UIBaseServiceTask.h"

class IPCUninstallBranch;

namespace UserCore
{
namespace ItemTask
{


class UIUpdateServiceTask : public UIBaseServiceTask
{
public:
	UIUpdateServiceTask(UserCore::Item::ItemHandle* handle, const char* path, MCFBuild lastBuild);
	~UIUpdateServiceTask();

protected:
	virtual bool initService();
	virtual void onComplete();

private:
	IPCUninstallBranch* m_pIPCIM;
	gcString m_szPath;
};

}
}


#endif //DESURA_UIUPDATESERVICETASK_H
