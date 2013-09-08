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
#include "ItemThread.h"

#include "ItemHandle.h"
#include "ItemInfo.h"
#include "webcore/WebCoreI.h"
#include "usercore/UserCoreI.h"

#include "UserThreadManager.h"

namespace UserCore
{
namespace Item
{

ItemThread::ItemThread(UserCore::Item::ItemHandle *handle) : ::Thread::BaseThread(gcString("{0} Thread", handle->getItemInfo()->getShortName()).c_str())
{
	m_szBaseName = gcString("{0}", handle->getItemInfo()->getShortName());
	m_bRunningTask = false;
	start();

	m_pThreadManager = NULL;	
	m_pWebCore = NULL;
	m_pUserCore = NULL;

	m_pCurrentTask = NULL;
	m_bDeleteCurrentTask = false;
}

ItemThread::~ItemThread()
{
	purge();

	//for some reason it blocks on stop. 

	nonBlockStop();
	m_WaitCond.notify();
	join();

	if (m_pThreadManager)
		m_pThreadManager->delist(this);
}

void ItemThread::purge()
{
	m_TaskMutex.lock();
	size_t size = m_vTaskList.size();
	safe_delete(m_vTaskList);
	m_TaskMutex.unlock();

	m_DeleteMutex.lock();

	if (m_pCurrentTask)
		m_pCurrentTask->onStop();
	else if (size > 0) //could be getting a task now
		m_bDeleteCurrentTask = true;

	m_DeleteMutex.unlock();
}

void ItemThread::setThreadManager(UserCore::UserThreadManagerI* tm)
{
	assert(tm);
	m_pThreadManager = tm;

	if (m_pThreadManager)
		m_pThreadManager->enlist(this);
}

void ItemThread::setWebCore(WebCore::WebCoreI *wc)
{
	m_pWebCore = wc;
}

void ItemThread::setUserCore(UserCore::UserI *uc)
{
	m_pUserCore = uc;
}

void ItemThread::queueTask(UserCore::ItemTask::BaseItemTask *task)
{
	if (isStopped())
	{
		safe_delete(task);
		return;
	}

	if (!task)
		return;

	task->setUserCore(m_pUserCore);
	task->setWebCore(m_pWebCore);

	m_TaskMutex.lock();
	m_vTaskList.push_back(task);
	m_TaskMutex.unlock();

	//get thread running again.
	m_WaitCond.notify();
}

void ItemThread::run()
{
	while (!isStopped())
	{
		doPause();

		if (!performTask())
			m_WaitCond.wait(2);
	}
}

bool ItemThread::performTask()
{
	if (isStopped())
		return true;

	UserCore::ItemTask::BaseItemTask* task = getNewTask();

	if (!task)
		return false;

	m_DeleteMutex.lock();
	m_bDeleteCurrentTask = false;
	m_DeleteMutex.unlock();

	uint32 taskType = task->getTaskType();

	m_DeleteMutex.lock();
	m_pCurrentTask = task;
	m_DeleteMutex.unlock();

	if (!m_bDeleteCurrentTask)
	{
		m_bRunningTask = true;
		onTaskStartEvent(taskType);

		task->doTask();

		onTaskCompleteEvent(taskType);
		m_bRunningTask = false;
	}

	m_DeleteMutex.lock();
	m_pCurrentTask = NULL;
	m_DeleteMutex.unlock();

	safe_delete(task);
	setThreadName(m_szBaseName.c_str());
	
	return true;
}

UserCore::ItemTask::BaseItemTask* ItemThread::getNewTask()
{
	if (isPaused())
		return NULL;

	UserCore::ItemTask::BaseItemTask* task = NULL;

	m_TaskMutex.lock();
	
	if (m_vTaskList.size() != 0)
	{
		task = m_vTaskList.front();
		m_vTaskList.pop_front();

		gcString name = m_szBaseName;
		name += " - ";
		name += task->getTaskName();

		this->setThreadName(name.c_str());
	}
	
	m_TaskMutex.unlock();

	return task;
}

void ItemThread::onPause()
{
	m_DeleteMutex.lock();

	if (m_pCurrentTask)
		m_pCurrentTask->onPause();

	m_DeleteMutex.unlock();
}

void ItemThread::onUnpause()
{
	m_DeleteMutex.lock();

	if (m_pCurrentTask)
		m_pCurrentTask->onUnpause();

	m_DeleteMutex.unlock();

	m_WaitCond.notify();
}

void ItemThread::onStop()
{
	m_DeleteMutex.lock();

	if (m_pCurrentTask)
		m_pCurrentTask->onStop();

	m_DeleteMutex.unlock();

	m_WaitCond.notify();
}

void ItemThread::cancelCurrentTask()
{
	m_DeleteMutex.lock();

	if (m_pCurrentTask)
		m_pCurrentTask->cancel();

	m_DeleteMutex.unlock();
}

bool ItemThread::hasTaskToRun()
{
	if (isRunningTask())
		return true;

	bool hasTasks = false;

	m_TaskMutex.lock();
	hasTasks = (m_vTaskList.size() > 0);
	m_TaskMutex.unlock();

	return hasTasks;
}

}
}
