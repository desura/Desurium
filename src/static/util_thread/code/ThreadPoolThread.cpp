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

#include "Common.h"
#include "ThreadPoolThread.h"
#include "util_thread/ThreadPool.h"

namespace Thread
{


ThreadPoolThread::ThreadPoolThread(ThreadPoolTaskSourceI* taskSource, bool forced) : BaseThread( "Thread Pool Worker" )
{
	m_bForced = forced;
	m_bCompTask = false;

	m_pTask = NULL;
	m_pTaskSource = taskSource;
}

ThreadPoolThread::~ThreadPoolThread()
{
	nonBlockStop();
	m_WaitCondition.notify();
	stop();
}

void ThreadPoolThread::newTask()
{
	m_WaitCondition.notify();
}

void ThreadPoolThread::run()
{
	if (m_bForced)
	{
		doTask();
	}
	else
	{
		while (!isStopped())
		{
			doPause();

			if (!doTask() && !isStopped())
				m_WaitCondition.wait();
		}
	}

	m_bCompTask = true;
}

bool ThreadPoolThread::doTask()
{
	BaseTask* task = m_pTaskSource->getTask();

	if (!task)
		return false;

	std::string name = task->getName();

	m_pTask = task;
	task->doTask();
	m_pTask = NULL;

	safe_delete(task);
	onCompleteEvent();

	return true;
}

void ThreadPoolThread::onStop()
{
	if (m_pTask)
		m_pTask->onStop();

	m_WaitCondition.notify();
}

void ThreadPoolThread::stopTask()
{
	if (m_pTask)
		m_pTask->onStop();
}

}
