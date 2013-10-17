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
#include "ToolInstallThread.h"

#include "IPCToolMain.h"
#include "ToolIPCPipeClient.h"
#include "ToolTransaction.h"

namespace UserCore
{
namespace Misc
{


ToolInstallThread::ToolInstallThread(ToolManager* toolManager, ::Thread::Mutex &mapLock, std::map<ToolTransactionId, ToolTransInfo*> &transactions, const char* userName, HWND handle) 
	: ::Thread::BaseThread("Tool Install Thread"), m_mTransactions(transactions), m_MapLock(mapLock)
{
	m_WinHandle = handle;
	m_pIPCClient = NULL;

	m_CurrentInstall = -1;
	m_szUserName = userName;
	m_pToolManager = toolManager;

	m_bStillInstalling = false;
}

ToolInstallThread::~ToolInstallThread()
{
	stop();
	safe_delete(m_pIPCClient);
}

bool ToolInstallThread::preInstallStart()
{
	try
	{
		startIPC();
	}
	catch (gcException &e)
	{
		onINError(e);
		nonBlockStop();
		onFailedToRunEvent();
		return false;
	}
	
	return true;
}

void ToolInstallThread::onPipeDisconnect()
{
	m_MapLock.lock();
	
	gcException e(ERR_PIPE, "Pipe to Tool Install Helper Disconnected. Failed to install tools.");

	std::for_each(m_mTransactions.begin(), m_mTransactions.end(), [&e](std::pair<ToolTransactionId, Misc::ToolTransInfo*> t){
		t.second->onINError(e);
	});

	//for (size_t x=0; x<m_mTransactions.size(); x++)
	//	m_mTransactions.at(x)->onINError(e);

	m_MapLock.unlock();
	m_CurrentInstall = -1;

	m_InstallWait.notify();
	nonBlockStop();
	m_InstallWait.notify();
}

void ToolInstallThread::startIPC()
{
	if (m_pIPCClient)
		return;

	time_t t = time(NULL);
	gcString id("{0}", t);

	m_pIPCClient = new ToolIPCPipeClient(m_szUserName.c_str(), true, id.c_str(), m_WinHandle);
	m_pIPCClient->onDisconnectEvent += delegate(this, &ToolInstallThread::onPipeDisconnect);
	m_pIPCClient->onDisconnectEvent += delegate(&onPipeDisconnectEvent);

	m_MapLock.lock();
	std::map<ToolTransactionId, ToolTransInfo*>::iterator it = m_mTransactions.find(m_CurrentInstall);
	
	if (it != m_mTransactions.end())
		it->second->startingIPC();

	m_MapLock.unlock();

	m_pIPCClient->start();
	m_pIPCClient->getToolMain()->onCompleteEvent += delegate(this, &ToolInstallThread::onINComplete);
	m_pIPCClient->getToolMain()->onErrorEvent += delegate(this, &ToolInstallThread::onINError);
}

IPCToolMain* ToolInstallThread::getToolMain()
{
	return m_pIPCClient->getToolMain();
}

}
}
