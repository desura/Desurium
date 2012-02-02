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
#include "ToolIPCPipeClient.h"
#include "IPCToolMain.h"

#include "../../../branding/branding.h"

namespace UserCore
{

ToolIPCPipeClient::ToolIPCPipeClient(const char* user, bool uploadDumps, const char* key, HWND handle) : IPC::PipeClient(gcString("DesuraToolHelper-{0}", key).c_str())
{
	m_szKey = key;
	m_WinHandle = handle;

	onDisconnectEvent += delegate(this, &ToolIPCPipeClient::onDisconnect);

	m_szUser = user;
	m_bUploadDumps = uploadDumps;

	m_pServiceMain = NULL;
}

ToolIPCPipeClient::~ToolIPCPipeClient()
{
	stopHelper();
}

void ToolIPCPipeClient::onDisconnect()
{
	Warning(PRODUCT_NAME " Tool Helper Disconnected unexpectedly!\n");
	m_pServiceMain = NULL;
}

void ToolIPCPipeClient::start()
{
	startHelper();

	size_t x=0;

	do
	{
		try
		{
			tryStart();
			break;
		}
		catch (gcException)
		{
			if (x > 5)
			{
				throw;
			}
			else
			{
				gcSleep(500);
				x++;
			}
		}
	}
	while (true);
}

void ToolIPCPipeClient::tryStart()
{
	try
	{
		setUpPipes();
		IPC::PipeClient::start();

		m_pServiceMain = IPC::CreateIPCClass<IPCToolMain>(this, "IPCToolMain");

		if (!m_pServiceMain)
			throw gcException(ERR_IPC, "Failed to create tool main");

		m_pServiceMain->setCrashSettings(m_szUser.c_str(), m_bUploadDumps);
	}
	catch (gcException &e)
	{
		throw gcException((ERROR_ID)e.getErrId(), e.getSecErrId(), gcString("Failed to start " PRODUCT_NAME " install helper: {0}", e));
	}
}

void ToolIPCPipeClient::startHelper()
{
#ifdef DEBUG
	char launchArg[255] = "toolhelper-d.exe";
#else
	char launchArg[255] = "toolhelper.exe";
#endif

	SHELLEXECUTEINFO info;
	memset(&info, 0, sizeof(SHELLEXECUTEINFO));

	gcString param("-key {0}", m_szKey);

	info.hwnd = m_WinHandle;
	info.cbSize = sizeof(SHELLEXECUTEINFO);

	uint32 os = UTIL::WIN::getOSId();

	if (os == WINDOWS_XP || os == WINDOWS_XP64)
		info.lpVerb = "open";
	else
		info.lpVerb = "runas";

	info.lpFile = launchArg;
	info.nShow = SW_SHOW;
	info.fMask = SEE_MASK_NOCLOSEPROCESS|SEE_MASK_WAITFORINPUTIDLE;
	info.lpParameters = param.c_str();

	BOOL res = ShellExecuteEx(&info);

	if (res == 0)
		throw gcException(ERR_SERVICE, GetLastError(), "Failed to start tool install helper.");

	DWORD dwExitCode = 0;
	GetExitCodeProcess(info.hProcess, &dwExitCode);

	CloseHandle(info.hProcess);

	if (dwExitCode != STILL_ACTIVE)
		throw gcException(ERR_SERVICE, "Failed to start tool install helper.");
}

void ToolIPCPipeClient::stopHelper()
{
	m_pServiceMain = NULL;
}

IPCToolMain* ToolIPCPipeClient::getToolMain()
{
	if (isDisconnected())
		return NULL;

	return m_pServiceMain;
}


}
