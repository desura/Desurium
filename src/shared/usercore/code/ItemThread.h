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

#ifndef DESURA_ITEMTHREAD_H
#define DESURA_ITEMTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "BaseItemTask.h"



namespace UserCore
{
	class UserThreadManagerI;

namespace Item
{

class ItemThread : public ::Thread::BaseThread
{
public:
	ItemThread(UserCore::Item::ItemHandle *handle);
	~ItemThread();

	void setThreadManager(UserCore::UserThreadManagerI* tm);
	void setWebCore(WebCore::WebCoreI *wc);
	void setUserCore(UserCore::UserI *uc);

	void queueTask(UserCore::ItemTask::BaseItemTask *task);

	void purge();

	void cancelCurrentTask();

	//! Is current task running
	//!
	//! @return True if running, false if not
	//!
	bool isRunningTask(){return m_bRunningTask;}

	bool hasTaskToRun();

	Event<uint32> onTaskCompleteEvent;
	Event<uint32> onTaskStartEvent;

protected:
	void run();

	void onPause();
	void onUnpause();
	void onStop();

	bool performTask();
	UserCore::ItemTask::BaseItemTask* getNewTask();

private:
	std::deque<UserCore::ItemTask::BaseItemTask*> m_vTaskList;
	UserCore::ItemTask::BaseItemTask* m_pCurrentTask;

	::Thread::WaitCondition m_WaitCond;

	::Thread::Mutex m_TaskMutex;
	::Thread::Mutex m_DeleteMutex;

	bool m_bRunningTask;
	bool m_bDeleteCurrentTask;

	UserCore::UserThreadManagerI* m_pThreadManager;
	WebCore::WebCoreI* m_pWebCore;
	UserCore::UserI* m_pUserCore;

	gcString m_szBaseName;
};


}
}

#endif //DESURA_ITEMTHREAD_H
