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
#include "ServiceCore.h"
#include "IPCPipeServer.h"

ServiceCore* g_pServiceCore = NULL;
void StopLogging();

void SetCrashSettings(const wchar_t* user, bool upload)
{
	if (g_pServiceCore)
		g_pServiceCore->setCrashSettings(user, upload);
}




ServiceCore::ServiceCore()
{
	m_pIPCServer = NULL;
	m_pDisconnect = NULL;

	g_pServiceCore = this;
}

ServiceCore::~ServiceCore()
{
	g_pServiceCore = NULL;
	stopPipe();
}

void ServiceCore::startPipe()
{
	if (m_pIPCServer)
		return;

	m_pIPCServer = new IPC::PipeServer("DesuraIS", 1, true);
	m_pIPCServer->onDisconnectEvent += delegate(this, &ServiceCore::onDisconnect);
	m_pIPCServer->start();
}

void ServiceCore::stopPipe()
{
	if (m_pIPCServer)
		m_pIPCServer->stop();

	safe_delete(m_pIPCServer);
}

bool ServiceCore::stopService(const char* serviceName)
{
#ifdef WIN32
	try
	{
		UTIL::WIN::stopService(SERVICE_NAME);
	}
	catch (gcException &)
	{
		return false;
	}
#endif
	return true;
}

void ServiceCore::destroy()
{
	delete this;
}

void ServiceCore::setCrashSettingCallback(CrashSettingFn crashSettingFn)
{
	m_pCrashSetting = crashSettingFn;
}

void ServiceCore::setDisconnectCallback(DisconnectFn disconnectFn)
{
	m_pDisconnect = disconnectFn;
}

void ServiceCore::setCrashSettings(const wchar_t* user, bool upload)
{
	if (m_pCrashSetting)
		m_pCrashSetting(user, upload);
}

void ServiceCore::onDisconnect(uint32 &)
{
	StopLogging();

	if (m_pDisconnect)
		m_pDisconnect();
}
