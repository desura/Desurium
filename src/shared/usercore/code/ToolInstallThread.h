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

#ifndef DESURA_TOOLINSTALLTHREAD_H
#define DESURA_TOOLINSTALLTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "ToolManager.h"

#ifdef WIN32
class IPCToolMain;
#else
namespace UserCore
{
	class ToolInfo;
}

class IPCToolMain
{
public:
	gcException installTool(UserCore::ToolInfo* toolInfo);
	Event<int32> onCompleteEvent;
};
#endif

namespace UserCore
{
namespace Misc
{

class ToolInstallThread : public ::Thread::BaseThread
{
public:
#ifdef WIN32
	ToolInstallThread(ToolManager* toolManager, ::Thread::Mutex &mapLock, std::map<ToolTransactionId, ToolTransInfo*> &transactions, const char* userName, HWND handle);
#else
	ToolInstallThread(ToolManager* toolManager, ::Thread::Mutex &mapLock, std::map<ToolTransactionId, ToolTransInfo*> &transactions);
#endif

	~ToolInstallThread();

	void startInstall(ToolTransactionId ttid);
	void cancelInstall(ToolTransactionId ttid);

	EventV onPipeDisconnectEvent;
	EventV onFailedToRunEvent;

protected:
	void run();
	void onStop();
	void startIPC();

	void onINComplete(int32 &result);
	void onINError(gcException &error);

	void startInstallItem(uint32 index);

	void onPipeDisconnect();

	void doNextInstall();
	void doFirstInstall();
	void hideForm();

	bool hasToolMain();

	IPCToolMain* getToolMain();
	bool preInstallStart();

private:
	::Thread::Mutex &m_MapLock;
	std::map<ToolTransactionId, Misc::ToolTransInfo*> &m_mTransactions;

	::Thread::WaitCondition m_InstallWait;
	::Thread::Mutex m_InstallLock;
	
	std::deque<ToolTransactionId> m_dvInstallQue;

	DesuraId m_CurrentInstallId;
	ToolTransactionId m_CurrentInstall;

	ToolManager* m_pToolManager;
	
	
#ifdef WIN32
	ToolIPCPipeClient* m_pIPCClient;
	HWND m_WinHandle;
	gcString m_szUserName;
#else
	IPCToolMain* m_pToolMain;
#endif

	bool m_bStillInstalling;
};

}
}

#endif //DESURA_TOOLINSTALLTHREAD_H
