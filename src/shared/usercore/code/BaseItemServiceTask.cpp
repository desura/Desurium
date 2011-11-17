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

#include "Common.h"
#include "BaseItemServiceTask.h"
#include "User.h"

namespace UserCore
{
namespace ItemTask
{


BaseItemServiceTask::BaseItemServiceTask(uint32 type, const char* name, UserCore::Item::ItemHandle* handle, MCFBranch branch, MCFBuild build) : BaseItemTask(type, name, handle, branch, build)
{
	m_bFinished = false;
	m_bStarted = false;
}

BaseItemServiceTask::~BaseItemServiceTask()
{
	waitForFinish();
}

void BaseItemServiceTask::setFinished()
{
	m_bFinished = true;
}

void BaseItemServiceTask::resetFinish()
{
	m_bFinished = false;
}

void BaseItemServiceTask::waitForFinish()
{
	boost::mutex waitMutex;

	if (!m_bFinished)
		m_WaitCond.wait();
}

void BaseItemServiceTask::doRun()
{
	m_bStarted = true;
	m_bFinished = false;
	boost::mutex waitMutex;

	bool shouldWait = initService();
	if (shouldWait && !m_bFinished && !isStopped())
		m_WaitCond.wait();

	m_bFinished = true;

	//some one might be waitForFinish on us
	m_WaitCond.notify();
}

void BaseItemServiceTask::onStop()
{
	BaseItemTask::onStop();
	m_WaitCond.notify();
}

IPCServiceMain* BaseItemServiceTask::getServiceMain()
{
	return getUserCore()->getServiceMain();
}

void BaseItemServiceTask::onFinish()
{
	setFinished();
	m_WaitCond.notify();
}

bool BaseItemServiceTask::hasStarted()
{
	return m_bStarted;
}


}
}
