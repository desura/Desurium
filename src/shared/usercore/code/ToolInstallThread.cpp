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
#include "ToolInstallThread.h"

#ifdef WIN32
#include "IPCToolMain.h"
#include "ToolIPCPipeClient.h"
#endif

#include "ToolTransaction.h"

namespace UserCore
{
namespace Misc
{


void ToolInstallThread::run()
{

	uint32 timeout = 5*60; //five mins

	while (!isStopped())
	{
		if (m_CurrentInstall != -1)
			doNextInstall();
		else
			doFirstInstall();

		if (isStopped())
			break;

		bool timedout = m_InstallWait.wait(timeout);

		if (m_bStillInstalling)
			continue;

		m_InstallLock.lock();
		uint32 size = m_dvInstallQue.size();
		m_InstallLock.unlock();

		if (timedout && size == 0)
		{
			//timeout happened. Stop
			
#ifdef WIN32
			safe_delete(m_pIPCClient);
#endif
			nonBlockStop();
			onFailedToRunEvent();
		}
	}
}

void ToolInstallThread::onStop()
{
	m_InstallWait.notify();
}

void ToolInstallThread::doFirstInstall()
{
	m_InstallLock.lock();
	uint32 size = m_dvInstallQue.size();
	m_InstallLock.unlock();

	if (size == 0)
		return;

	m_InstallLock.lock();
	m_CurrentInstall = m_dvInstallQue.front();
	m_dvInstallQue.pop_front();
	m_InstallLock.unlock();

	if (!preInstallStart())
		return;

	bool startRes = false;

	m_MapLock.lock();
	std::map<ToolTransactionId, ToolTransInfo*>::iterator it = m_mTransactions.find(m_CurrentInstall);
	
	if (it != m_mTransactions.end())
		startRes = it->second->startNextInstall(getToolMain(), m_CurrentInstallId);

	m_MapLock.unlock();

	if (!startRes)
	{
		m_CurrentInstall = -1;
		doFirstInstall();
	}
	else
	{
		m_bStillInstalling = true;
	}
}

bool ToolInstallThread::hasToolMain()
{
	return !!getToolMain();
}

void ToolInstallThread::doNextInstall()
{
	if (m_bStillInstalling)
		return;

	if (m_CurrentInstall == -1)
		return;

	m_MapLock.lock();
	std::map<ToolTransactionId, ToolTransInfo*>::iterator it = m_mTransactions.find(m_CurrentInstall);
	
	if (it != m_mTransactions.end() && hasToolMain())
	{
		it->second->onINComplete();

		if (!it->second->startNextInstall(getToolMain(), m_CurrentInstallId))
			m_CurrentInstall = -1;
	}
	else
	{
		//install must be canceled
		m_CurrentInstall = -1;
	}

	m_MapLock.unlock();
}

void ToolInstallThread::onINComplete(int32 &result)
{
	m_bStillInstalling = false;

	ToolInfo* tool = m_pToolManager->findItem(m_CurrentInstallId.toInt64());

	bool installError = false;

	if (tool)
	{
		installError = tool->checkExpectedResult(result) == false;

		if (!installError)
			tool->setInstalled();
	}

	if (installError)
	{
		m_MapLock.lock();
		std::map<ToolTransactionId, ToolTransInfo*>::iterator it = m_mTransactions.find(m_CurrentInstall);



		if (it != m_mTransactions.end() && hasToolMain())
		{
			int32 r = result;

			Warning(gcString("The tool install [{3}] result didnt match what was expected [Actual: {0}, Expected: {1}]", r, tool->getResultString(), tool->getName()));
			gcException e(ERR_BADRESPONSE, gcString("The tool {0} failed to install (Bad result)", tool->getName()));
			it->second->onINError(e);
			m_CurrentInstall = -1;
		}
		
		m_MapLock.unlock();
	}

	m_pToolManager->saveItems();
	m_InstallWait.notify();
}

void ToolInstallThread::onINError(gcException &error)
{
	m_bStillInstalling = false;

	m_MapLock.lock();
	std::map<ToolTransactionId, ToolTransInfo*>::iterator it = m_mTransactions.find(m_CurrentInstall);
	
	if (it != m_mTransactions.end())
		it->second->onINError(error);

	m_CurrentInstall = -1;

	m_MapLock.unlock();
	m_InstallWait.notify();
}

void ToolInstallThread::startInstall(ToolTransactionId ttid)
{
	m_InstallLock.lock();
	m_dvInstallQue.push_back(ttid);
	m_InstallLock.unlock();

	m_InstallWait.notify();
}

void ToolInstallThread::cancelInstall(ToolTransactionId ttid)
{
	m_InstallLock.lock();

	if (m_CurrentInstall == ttid)
	{

	}
	else
	{
		for (size_t x=0; x<m_dvInstallQue.size(); x++)
		{
			if (ttid != m_dvInstallQue[x])
				continue;

			m_dvInstallQue.erase(m_dvInstallQue.begin()+x);
			break;
		}
	}

	m_InstallLock.unlock();
}

}
}
