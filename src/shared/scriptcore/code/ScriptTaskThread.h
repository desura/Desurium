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

#ifndef DESURA_SCRIPTTASKTHREAD_H
#define DESURA_SCRIPTTASKTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "ScriptCoreI.h"
#include "util_thread/BaseThread.h"

#include <deque>

class ScriptTaskThread : public ::Thread::BaseThread, public ScriptTaskRunnerI
{
public:
	ScriptTaskThread();
	~ScriptTaskThread();

	virtual void queTask(ScriptTaskI* task);
	virtual void setLastTask(ScriptTaskI* task);

protected:
	virtual void run();
	virtual void onStop();

private:
	ScriptTaskI* m_pLastTask;

	::Thread::WaitCondition m_WaitCond;
	::Thread::Mutex m_LockMutex;
	::Thread::Mutex m_StartMutex;

	std::deque<ScriptTaskI*> m_TaskQue;
};

#endif //DESURA_SCRIPTTASKTHREAD_H
