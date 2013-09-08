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

#ifndef DESURA_INSTALLTOOLTASK_H
#define DESURA_INSTALLTOOLTASK_H
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


class InstallToolTask : public UserCore::ItemTask::BaseItemTask
{
public:
	InstallToolTask(UserCore::Item::ItemHandle* handle, bool launch);
	virtual ~InstallToolTask();

protected:
	void doRun();

	virtual void onPause();
	virtual void onUnpause();
	virtual void onStop();

	void onIPCStart();
	void onINStart(DesuraId &id);
	void onINProgress(UserCore::Misc::ToolProgress &p);
	void onINError(gcException &e);
	void onINComplete();

	void onComplete();

private:
	bool m_bInError;
	bool m_bLaunch;
	
	uint32 m_ToolTTID;
	::Thread::WaitCondition m_WaitCond;
};

}
}


#endif //DESURA_INSTALLTOOLTASK_H
