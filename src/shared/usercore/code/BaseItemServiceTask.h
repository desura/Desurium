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

#ifndef DESURA_BASEITEMSERVICETASK_H
#define DESURA_BASEITEMSERVICETASK_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseItemTask.h"
#include "util_thread/BaseThread.h"

class IPCServiceMain;

namespace UserCore
{
namespace ItemTask
{

class BaseItemServiceTask : public BaseItemTask
{
public:
	BaseItemServiceTask(uint32 type, const char* name, UserCore::Item::ItemHandle* handle, MCFBranch branch = MCFBranch(), MCFBuild = MCFBuild());
	~BaseItemServiceTask();

	bool hasStarted();

protected:
	virtual bool initService()=0;

	virtual void onStop();
	virtual void doRun();
	virtual void onFinish();



	IPCServiceMain* getServiceMain();

	void waitForFinish();
	void resetFinish();
	void setFinished();

private:
	::Thread::WaitCondition m_WaitCond;
	volatile bool m_bFinished;
	volatile bool m_bStarted;
};



}
}




#endif //DESURA_BASEITEMSERVICETASK_H
