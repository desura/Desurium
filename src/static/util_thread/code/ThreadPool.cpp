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
#include "util_thread/ThreadPool.h"
#include "ThreadPoolThread.h"

class ThreadPoolTaskSource : public Thread::ThreadPoolTaskSourceI
{
public:
	ThreadPoolTaskSource(Thread::BaseTask* task)
	{
		m_pTask = task;
	}

	virtual Thread::BaseTask* getTask()
	{
		Thread::BaseTask* task = m_pTask;
		delete this;
		return task;
	}

	Thread::BaseTask* m_pTask;
};

namespace Thread
{

BaseTask::~BaseTask()
{

}

ThreadPool::ThreadPool(uint8 num) : BaseThread( "Thread Pool" )
{
	if (num == 0)
		m_uiCount = 2;
	else
		m_uiCount = num;

	m_bIsTaskBlocked = false;
	start();
}

ThreadPool::~ThreadPool()
{
	nonBlockStop();
	m_WaitCondition.notify();

	join();

	m_TaskMutex.lock();
	safe_delete(m_vTaskList);
	m_TaskMutex.unlock();

	m_ThreadMutex.writeLock();
		for (size_t x=0; x<m_vThreadList.size(); x++)
		{
			m_vThreadList[x]->onCompleteEvent -= delegate(this, &ThreadPool::onThreadComplete);
			m_vThreadList[x]->stop();
		}

		safe_delete(m_vThreadList);
	m_ThreadMutex.writeUnlock();

	m_ForcedMutex.writeLock();
		for (size_t x=0; x<m_vForcedList.size(); x++)
		{
			m_vForcedList[x]->onCompleteEvent -= delegate(this, &ThreadPool::onThreadComplete);
			m_vForcedList[x]->stop();
		}

		safe_delete(m_vForcedList);
	m_ForcedMutex.writeUnlock();
}

void ThreadPool::blockTasks()
{
	m_bIsTaskBlocked = true;
}

void ThreadPool::unBlockTasks()
{
	m_bIsTaskBlocked = false;
}

void ThreadPool::purgeTasks()
{
	{
		m_TaskMutex.lock();

		m_ThreadMutex.readLock();

			for (size_t x=0; x<m_vThreadList.size(); x++)
			{
				m_vThreadList[x]->stopTask();
			}

		m_ThreadMutex.readUnlock();
		m_ForcedMutex.writeLock();

			for (size_t x=0; x<m_vForcedList.size(); x++)
			{
				m_vForcedList[x]->stop();
				m_vForcedList[x]->onCompleteEvent -= delegate(this, &ThreadPool::onThreadComplete);
			}

			safe_delete(m_vForcedList);
		m_ForcedMutex.writeUnlock();

		safe_delete(m_vTaskList);
		m_TaskMutex.unlock();
	}

	while (activeThreads() > 0)
		gcSleep(50);
}

void ThreadPool::queueTask(BaseTask *task)
{
	if (!task)
		return;

	if (m_bIsTaskBlocked)
	{
		Warning("Thread pool task blocking active and new task was added.\n");
		safe_delete(task);
		return;
	}

	m_TaskMutex.lock();
	m_vTaskList.push_back(task);
	m_TaskMutex.unlock();

	//get thread running again.
	m_WaitCondition.notify();
}


void ThreadPool::forceTask(BaseTask *task)
{
	if (!task)
		return;

	m_ForcedMutex.writeLock();

	ThreadPoolThread *thread = new ThreadPoolThread(new ThreadPoolTaskSource(task), true);
	thread->onCompleteEvent += delegate(this, &ThreadPool::onThreadComplete);

	m_vForcedList.push_back( thread );

	thread->start();


	m_ForcedMutex.writeUnlock();
}

void ThreadPool::run()
{
	for (uint8 x=0; x<m_uiCount; x++)
	{
		ThreadPoolThread *thread = new ThreadPoolThread(this, false);
		thread->onCompleteEvent += delegate(this, &ThreadPool::onThreadComplete);
		m_vThreadList.push_back( thread );


		thread->start();
	}

	while (!isStopped())
	{
		doPause();

		removedForced();

		uint32 runTaskCount = activeThreads();

		if (runTaskCount < m_uiCount)
			startNewTasks();

		runTaskCount = activeThreads();

		if ((runTaskCount == m_uiCount || m_vTaskList.size() == 0) && !isStopped())
			m_WaitCondition.wait();
	}
}

uint8 ThreadPool::activeThreads()
{
	uint8 count = 0;

	m_ThreadMutex.readLock();

	for (size_t x=0; x<m_vThreadList.size(); x++)
	{
		if (!m_vThreadList[x])
			continue;

		if (m_vThreadList[x]->hasTask())
			count++;
	}

	m_ThreadMutex.readUnlock();

	return count;
}

void ThreadPool::startNewTasks()
{
	m_TaskMutex.lock();
	m_ThreadMutex.readLock();

	for (size_t x=0; x<m_vThreadList.size(); x++)
	{
		if (m_vTaskList.size() == 0)
			break;

		if (!m_vThreadList[x])
			continue;

		if (!m_vThreadList[x]->hasTask())
			m_vThreadList[x]->newTask();
	}

	m_ThreadMutex.readUnlock();
	m_TaskMutex.unlock();
}

void ThreadPool::onPause()
{
	m_ThreadMutex.readLock();
		for (size_t x=0; x<m_vThreadList.size(); x++)
			m_vThreadList[x]->pause();
	m_ThreadMutex.readUnlock();

	m_ForcedMutex.readLock();
		for (size_t x=0; x<m_vForcedList.size(); x++)
			m_vForcedList[x]->pause();
	m_ForcedMutex.readUnlock();

	//get thread running again.
	m_WaitCondition.notify();
}

void ThreadPool::onUnpause()
{
	m_ThreadMutex.readLock();
		for (size_t x=0; x<m_vThreadList.size(); x++)
			m_vThreadList[x]->unpause();
	m_ThreadMutex.readUnlock();

	m_ForcedMutex.readLock();
		for (size_t x=0; x<m_vForcedList.size(); x++)
			m_vForcedList[x]->unpause();
	m_ForcedMutex.readUnlock();

	//get thread running again.
	m_WaitCondition.notify();
}

void ThreadPool::onStop()
{
	m_ThreadMutex.readLock();
		for (size_t x=0; x<m_vThreadList.size(); x++)
			m_vThreadList[x]->stop();
	m_ThreadMutex.readUnlock();

	m_ForcedMutex.readLock();
	for (size_t x=0; x<m_vForcedList.size(); x++)
		m_vForcedList[x]->stop();
	m_ForcedMutex.readUnlock();

	//get thread running again.
	m_WaitCondition.notify();
}

void ThreadPool::removedForced()
{
	m_ForcedMutex.writeLock();

	if (m_vForcedList.size() > 0)
	{
		std::vector<size_t> delVect;

		for (size_t x=0; x<m_vForcedList.size(); x++)
		{
			if (m_vForcedList[x]->hasCompletedTask())
			{
				m_vForcedList[x]->onCompleteEvent -= delegate(this, &ThreadPool::onThreadComplete);
				safe_delete(m_vForcedList[x]);
				delVect.push_back(x);
			}
		}


		std::vector<size_t>::reverse_iterator it = delVect.rbegin();
		while (it != delVect.rend())
		{
			m_vForcedList.erase(m_vForcedList.begin()+*it);
			it++;
		}
	}

	m_ForcedMutex.writeUnlock();
}

void ThreadPool::onThreadComplete()
{
	if (isStopped())
		return;

	//get thread running again.
	m_WaitCondition.notify();
}

BaseTask* ThreadPool::getTask()
{
	BaseTask* task = NULL;

	m_TaskMutex.lock();

	if (m_vTaskList.size() > 0)
	{
		task = m_vTaskList.front();
		m_vTaskList.pop_front();
	}

	m_TaskMutex.unlock();

	return task;
}

}
