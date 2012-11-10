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
#include "util_thread/BaseThread.h"

#include "boost/thread.hpp"
#include "boost/bind.hpp"
#include "util_thread/ReadWriteMutex.h"

#define MS_VC_EXCEPTION 0x406D1388

#ifdef NIX
	#include <sys/prctl.h>
#endif




namespace Thread
{


void waitOnMutex(boost::condition_variable &waitCond, boost::mutex &waitMutex)
{
	boost::unique_lock<boost::mutex> lock(waitMutex);
	waitCond.wait(lock);
}

bool timedWaitOnMutex(boost::condition_variable &waitCond, boost::mutex &waitMutex, int secs, int msecs)
{
	boost::posix_time::ptime waitTime(boost::posix_time::microsec_clock::universal_time());
	waitTime += boost::posix_time::seconds(secs);
	waitTime += boost::posix_time::millisec(msecs);

	boost::unique_lock<boost::mutex> lock(waitMutex);
	return waitCond.timed_wait(lock, waitTime) == false;
}


class ReadWriteMutex::ReadWriteMutexPrivates
{
public:
	Thread::Internal::ReadWriteMutex m_Mutex;
};


ReadWriteMutex::ReadWriteMutex()
{
	m_pPrivates = new ReadWriteMutexPrivates();
}

ReadWriteMutex::~ReadWriteMutex()
{
	safe_delete(m_pPrivates);
}

void ReadWriteMutex::readLock()
{
	m_pPrivates->m_Mutex.acquireReadLock();
}

void ReadWriteMutex::readUnlock()
{
	m_pPrivates->m_Mutex.releaseReadLock();
}

void ReadWriteMutex::writeLock()
{
	m_pPrivates->m_Mutex.acquireWriteLock();
}

void ReadWriteMutex::writeUnlock()
{
	m_pPrivates->m_Mutex.releaseWriteLock();
}




class Mutex::MutexPrivates
{
public:
	boost::mutex m_WaitMutex;
};


Mutex::Mutex()
{
	m_pPrivates = new MutexPrivates();
}

Mutex::~Mutex()
{
	safe_delete(m_pPrivates);
}

void Mutex::lock()
{
	if (m_pPrivates)
		m_pPrivates->m_WaitMutex.lock();
}

void Mutex::unlock()
{
	if (m_pPrivates)
		m_pPrivates->m_WaitMutex.unlock();
}



class WaitCondition::WaitConditionPrivates
{
public:
	WaitConditionPrivates()
	{
		m_bNotify = false;
	}

	volatile bool m_bNotify;
	boost::condition_variable m_WaitCond;
	boost::mutex m_WaitMutex;
	boost::mutex m_NotifyLock;

	bool checkNotify()
	{
		bool notify = false;

		m_NotifyLock.lock();

		if (m_bNotify)
		{
			m_bNotify = false;
			notify = true;
		}

		m_NotifyLock.unlock();

		return notify;
	}

	void setNotify(bool state)
	{
		m_NotifyLock.lock();
		m_bNotify = state;

		if (state)
			m_WaitCond.notify_all();

		m_NotifyLock.unlock();
	}
};


WaitCondition::WaitCondition()
{
	m_pPrivates = new WaitConditionPrivates();
}

WaitCondition::~WaitCondition()
{
	safe_delete(m_pPrivates);
}

void WaitCondition::wait()
{
	while (wait(0, 500))
	{
	}
}

bool WaitCondition::wait(int sec, int msec)
{
	if (m_pPrivates->checkNotify())
		return false;

	bool res = timedWaitOnMutex(m_pPrivates->m_WaitCond, m_pPrivates->m_WaitMutex, sec, msec);

	if (m_pPrivates->m_bNotify)
		res = false;

	m_pPrivates->setNotify(false);
	return res;
}

void WaitCondition::notify()
{
	m_pPrivates->setNotify(true);
}














class BaseThread::ThreadPrivates
{
public:
	char* m_szName;

	bool m_bIsRunning;
	volatile bool m_bPause;
	volatile bool m_bStop;

	BaseThread::PRIORITY m_uiPriority;

	boost::thread *m_pThread;
	boost::condition_variable m_PauseCond;
	boost::mutex m_PauseMutex;
};



BaseThread::BaseThread(const char* name)
{
	m_pPrivates = new ThreadPrivates();

	m_pPrivates->m_szName = NULL;
	Safe::strcpy(&m_pPrivates->m_szName, name, 255);
	m_pPrivates->m_pThread = NULL;
	m_pPrivates->m_bStop = false;
	m_pPrivates->m_bPause = false;
	m_pPrivates->m_bIsRunning = false;

	m_pPrivates->m_uiPriority = NORMAL;
}

BaseThread::~BaseThread()
{
	stop();
	safe_delete(m_pPrivates->m_pThread);
	safe_delete(m_pPrivates->m_szName);

	safe_delete(m_pPrivates);
}

const char* BaseThread::getName()
{
	return m_pPrivates->m_szName;
}

void BaseThread::start()
{
	if (m_pPrivates->m_pThread)
		return;

	m_pPrivates->m_pThread = new boost::thread(boost::bind(&BaseThread::doRun, this));
}

void BaseThread::doRun()
{
	m_pPrivates->m_bIsRunning = true;
	applyPriority();
	setThreadName();
	run();
}

bool BaseThread::isRunning()
{
	return m_pPrivates->m_bIsRunning;
}

volatile bool BaseThread::isStopped()
{
	return m_pPrivates->m_bStop;
}

volatile bool BaseThread::isPaused()
{
	return (!m_pPrivates->m_bStop && m_pPrivates->m_bPause);
}




void BaseThread::doPause()
{
	if (isPaused())
	{
		waitOnMutex(m_pPrivates->m_PauseCond, m_pPrivates->m_PauseMutex);
	}
}

void BaseThread::pause()
{
	m_pPrivates->m_bPause = true;
	onPause();
}

void BaseThread::unpause()
{
	m_pPrivates->m_bPause = false;
	onUnpause();
	m_pPrivates->m_PauseCond.notify_all();
}

void BaseThread::stop()
{
	if (m_pPrivates->m_bPause)
		unpause();

	nonBlockStop();

	if (m_pPrivates->m_pThread)
	{
		m_pPrivates->m_pThread->interrupt();
		if (m_pPrivates->m_pThread->joinable())
		{
			m_pPrivates->m_pThread->join();
		}
	}
}

void BaseThread::nonBlockStop()
{
	if (!m_pPrivates->m_pThread)
		return;

	m_pPrivates->m_bStop = true;
	onStop();
}

void BaseThread::join()
{
	if (!m_pPrivates->m_pThread)
		return;

	m_pPrivates->m_pThread->join();
}

void BaseThread::setPriority(PRIORITY priority)
{
	m_pPrivates->m_uiPriority = priority;
}

void BaseThread::applyPriority()
{

#ifdef WIN32
	if (!m_pPrivates->m_pThread)
		return;

	BOOL res;
	HANDLE th = GetCurrentThread();

	switch (m_pPrivates->m_uiPriority)
	{
	case REALTIME		: res = SetThreadPriority(th, THREAD_PRIORITY_TIME_CRITICAL);	break;
	case HIGH			: res = SetThreadPriority(th, THREAD_PRIORITY_HIGHEST);			break;
	case ABOVE_NORMAL	: res = SetThreadPriority(th, THREAD_PRIORITY_ABOVE_NORMAL);	break;
	case NORMAL			: res = SetThreadPriority(th, THREAD_PRIORITY_NORMAL);			break;
	case BELOW_NORMAL	: res = SetThreadPriority(th, THREAD_PRIORITY_BELOW_NORMAL);	break;
	case IDLE			: res = SetThreadPriority(th, THREAD_PRIORITY_LOWEST);			break;
	}
#endif
}

void BaseThread::setThreadName(const char* nameOveride)
{
#if defined(WIN32)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = nameOveride?nameOveride:m_pPrivates->m_szName;
	info.dwThreadID = (DWORD)GetCurrentThreadId();;
	info.dwFlags = 0;

   __try
   {
	  RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
   }
   __except(EXCEPTION_CONTINUE_EXECUTION)
   {
   }
#else
	char name[16];
	
	if (nameOveride)
		strncpy(name, nameOveride, 15);
	else
		strncpy(name, m_pPrivates->m_szName, 15);
		
	name[15] = '\0'; 
	prctl(PR_SET_NAME, name, 0, 0, 0);
#endif
}


uint64 BaseThread::GetCurrentThreadId()
{
#ifdef WIN32
	return ::GetCurrentThreadId();
#else
	return (uint64)pthread_self();
#endif
}

}
