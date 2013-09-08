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

#ifndef DESURA_INSTALLTHREAD_H
#define DESURA_INSTALLTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseItemServiceTask.h"


class IPCInstallMcf;

namespace UserCore
{
namespace Item
{
	namespace Helper
	{
		class InstallerHandleHelperI;
	}
}

namespace ItemTask
{

class InstallServiceTask : public BaseItemServiceTask
{
public:
	InstallServiceTask(UserCore::Item::ItemHandle* handle, const char* path, MCFBranch branch, UserCore::Item::Helper::InstallerHandleHelperI* ihh);
	~InstallServiceTask();


protected:
	bool initService();
	void onComplete();

	virtual void onPause();
	virtual void onUnpause();
	virtual void onStop();

	void onFinish();
	void onProgUpdate(MCFCore::Misc::ProgressInfo& info);
	void onError(gcException &e);

	gcString getCurrentMcf();
	gcString downloadMcfHeader();

private:
	IPCInstallMcf* m_pIPCIM;

	bool m_bHasError;
	bool m_bInstalling;
	bool m_bHashMissMatch;
	gcString m_szPath;

	UserCore::Item::Helper::InstallerHandleHelperI* m_pIHH;
};

}
}


#endif
