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
#include "ScriptTaskThread.h"


ScriptTaskThread::ScriptTaskThread() : BaseThread("ScriptCore Thread")
{
	m_pLastTask = NULL;
	m_StartMutex.lock();
}

ScriptTaskThread::~ScriptTaskThread()
{
	m_StartMutex.lock();
	m_StartMutex.unlock();

	stop();

	m_LockMutex.lock();

	for (size_t x=0; x<m_TaskQue.size(); x++)
		m_TaskQue[x]->destory();

	m_TaskQue.clear();
	m_LockMutex.unlock();
}

void ScriptTaskThread::queTask(ScriptTaskI* task)
{
	m_LockMutex.lock();
	m_TaskQue.push_back(task);
	m_LockMutex.unlock();

	m_WaitCond.notify();
}

void ScriptTaskThread::setLastTask(ScriptTaskI* task)
{
	m_pLastTask = task;
}

void ScriptTaskThread::run()
{
	m_StartMutex.unlock();

	ScriptTaskI* curTask = NULL;

	while (!isStopped())
	{
		curTask = NULL;

		m_LockMutex.lock();

		if (m_TaskQue.size() > 0)
		{
			curTask = m_TaskQue.front();
			m_TaskQue.pop_front();
		}

		m_LockMutex.unlock();

		if (curTask)
		{
			curTask->doTask();
			curTask->destory();
			curTask = NULL;
		}
		else
		{
			m_WaitCond.wait(2);
		}
	}

	if (m_pLastTask)
	{
		m_pLastTask->doTask();
		m_pLastTask->destory();
	}
}

void ScriptTaskThread::onStop()
{
	m_WaitCond.notify();
}