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

#ifndef DESURA_IPCLOCKABLE_H
#define DESURA_IPCLOCKABLE_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "IPCParameter.h"

namespace IPC
{

//! Reperesnts a basic lock for function calls across IPC
//!
class IPCLock
{
public:
	IPCLock()
	{
		id = 0;
		result = NULL;
		m_bTriggered = false;
	}

	~IPCLock()
	{
		delMutex.lock();
		waitCond.notify();
		delMutex.unlock();
	}

	void wait()
	{
		if (!m_bTriggered)
			waitCond.wait();
	}

	void timedWait()
	{
		if (!m_bTriggered)
			waitCond.wait(5); //wait on mutex
	}

	bool wait(int sec, int milli)
	{
		if (!m_bTriggered)
			return waitCond.wait(sec, milli); //wait on mutex

		return false;
	}

	void trigger()
	{
		m_bTriggered = true;
		alert();
	}

	void alert()
	{
		delMutex.lock();
		waitCond.notify();
		delMutex.unlock();
	}

	volatile bool isTriggered()
	{
		return m_bTriggered;
	}

	uint32 id;
	IPCParameterI* result;

private:
	volatile bool m_bTriggered;

	Thread::WaitCondition waitCond;
	Thread::Mutex delMutex;
};


template <typename T>
class IPCScopedLock
{
public:
	IPCScopedLock(T* pParent, IPCLock* pLock)
	{
		m_pLock = pLock;
		m_pParent = pParent;
	}

	~IPCScopedLock()
	{
		if (m_pParent && m_pLock)
			m_pParent->delLock(m_pLock->id);
	}

	IPCLock* operator ->()
	{
		return m_pLock;
	}

	IPCLock* m_pLock;
	T* m_pParent;
};

//! Class holds locks for function calls
//!
class IPCLockable
{
public:
	IPCLockable()
	{
		m_uiIdCount = 0;
	}

	virtual ~IPCLockable()
	{

	}

	void cancelLocks(gcException &reason)
	{
		m_lockMutex.lock();

		for (size_t x=0; x<m_vLockList.size(); x++)
		{
			m_vLockList[x]->result = newParameterS(reason);
			m_vLockList[x]->trigger();
		}

		m_lockMutex.unlock();
	}

protected:
	IPCLock* newLock()
	{
		IPCLock* lock = new IPCLock();

		m_lockMutex.lock();

		lock->id = m_uiIdCount;
		m_uiIdCount++;
		m_vLockList.push_back(lock);

		m_lockMutex.unlock();

		//printf("Creating lock with id %d\n", lock->id);

		return lock;
	}

	void delLock(uint32 id)
	{
		//printf("Deleting lock with id %d\n", id);

		m_lockMutex.lock();

		for (size_t x=0; x<m_vLockList.size(); x++)
		{
			if (m_vLockList[x]->id == id)
			{
				delete m_vLockList[x];
				m_vLockList.erase(m_vLockList.begin()+x);
				break;
			}
		}

		m_lockMutex.unlock();
	}

	IPCLock* findLock(uint32 id)
	{
		//printf("Finding lock with id %d\n", id);

		m_lockMutex.lock();
		
		for (size_t x=0; x<m_vLockList.size(); x++)
		{
			if (m_vLockList[x]->id == id)
			{
				m_lockMutex.unlock();
				return m_vLockList[x];
			}
		}

		m_lockMutex.unlock();

		return NULL;
	}

private:
	template <class T> friend class IPCScopedLock;

	uint32 m_uiIdCount;
	std::vector<IPCLock*> m_vLockList;
	Thread::Mutex m_lockMutex;
};

}

#endif //DESURA_IPCLOCKABLE_H
