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

#ifndef DESURA_USERIPCPIPECLIENT_H
#define DESURA_USERIPCPIPECLIENT_H
#ifdef _WIN32
#pragma once
#endif

#if defined NIX || defined MACOS
#include "SharedObjectLoader.h"
#endif

class IPCServiceMain;

#include "IPCPipeClient.h"
class IPCServerI;

namespace UserCore
{

class UserIPCPipeClient : public IPC::PipeClient
{
public:
	UserIPCPipeClient(const char* user, const char* appDataPath, bool uploadDumps);
	~UserIPCPipeClient();
#ifdef WIN32
	void restart();
#endif
	void start();

	IPCServiceMain* getServiceMain(){return m_pServiceMain;}

protected:
#ifdef WIN32
	void stopService();
	void startService();

	void onDisconnect();
#else
	static void recvMessage(void* obj, const char* buffer, size_t size);
	void recvMessage(const char* buffer, size_t size);
	static void sendMessage(void* obj, const char* buffer, size_t size);
	void sendMessage(const char* buffer, size_t size);
	virtual void setUpPipes();
#endif

private:
	bool m_bUploadDumps;
	gcString m_szUser;
	gcString m_szAppDataPath;

	IPCServiceMain *m_pServiceMain;
#if defined NIX || defined MACOS
	SharedObjectLoader m_hServiceDll;
	IPCServerI *m_pServer;
#endif
};


}

#endif //DESURA_USERIPCPIPECLIENT_H
