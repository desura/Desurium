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
#include "ServiceMainThread.h"


ServiceMainThread::ServiceMainThread() : Thread::BaseThread("ServiceMain Worker")
{
}

ServiceMainThread::~ServiceMainThread()
{
	m_vLock.lock();

	for (size_t x=0; x<m_vJobList.size(); x++)
	{
		m_vJobList[x]->destroy();
	}

	m_vJobList.clear();
	m_vLock.unlock();
}

void ServiceMainThread::addTask(TaskI* task)
{
	if (!task)
		return;

	if (isStopped())
	{
		task->destroy();
		return;
	}

	m_vLock.lock();
	m_vJobList.push_back(task);
	m_vLock.unlock();

	m_WaitCond.notify();
}


void ServiceMainThread::run()
{
	while (!isStopped())
	{
		TaskI* task = NULL;

		m_vLock.lock();

		if (!m_vJobList.empty())
		{
			task = m_vJobList.front();
			m_vJobList.pop_front();
		}

		m_vLock.unlock();

		if (task)
		{
			task->doTask();
			task->destroy();
		}
		else
		{
			m_WaitCond.wait();
		}
	}
}

void ServiceMainThread::onStop()
{
	m_WaitCond.notify();
}
