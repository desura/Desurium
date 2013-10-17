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

#ifndef DESURA_SERVICEMAINTHREAD_H
#define DESURA_SERVICEMAINTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"


class TaskI
{
public:
	virtual void doTask()=0;
	virtual void destroy()=0;
};

class ServiceMainThread : public ::Thread::BaseThread
{
public:
	ServiceMainThread();
	~ServiceMainThread();

	void addTask(TaskI* task);

protected:
	void run();
	void onStop();

private:
	::Thread::Mutex m_vLock;
	std::deque<TaskI*> m_vJobList;

	::Thread::WaitCondition m_WaitCond;
};

#endif //DESURA_SERVICEMAINTHREAD_H
