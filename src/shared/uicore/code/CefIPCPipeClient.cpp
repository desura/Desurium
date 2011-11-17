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
#include "CefIPCPipeClient.h"
#include "webcore/WebCoreI.h"


CefIPCPipeClient::CefIPCPipeClient(const char* id, const char* hostName) : IPC::PipeClient(id)
{
	onDisconnectEvent += delegate(this, &CefIPCPipeClient::onDisconnect);
	m_szPipeId = id;
	m_szHostName = hostName;
}

CefIPCPipeClient::~CefIPCPipeClient()
{
	try
	{
		stopService();
	}
	catch (...)
	{
	}
}

void CefIPCPipeClient::onDisconnect()
{
	Warning("Browser host died. :'(\n");
}

void CefIPCPipeClient::start()
{
	try
	{
		startService();

		sleep(500);

		setUpPipes();

		IPC::PipeClient::start();
	}
	catch (gcException &e)
	{
		throw gcException((ERROR_ID)e.getErrId(), e.getSecErrId(), gcString("Failed to start desura service: {0}", e));
	}
}

typedef gcString (*UserAgentFN)();

void CefIPCPipeClient::startService()
{
#ifdef DEBUG
	const char* exeName = "desura_browserhost-d.exe";
#else
	const char* exeName = "desura_browserhost.exe";
#endif


	UserAgentFN userAgent = (UserAgentFN)WebCore::FactoryBuilder(WEBCORE_USERAGENT);
		
	gcString ua = userAgent();

	gcString exe(exeName);
	gcString params("{3} {0} {1} {2}", m_szPipeId, m_szHostName, UTIL::STRING::base64_encode(ua.c_str(), ua.size()), exeName);
	


	DWORD dwExitCode = 0; 

	
	STARTUPINFOA siStartupInfo; 
	PROCESS_INFORMATION piProcessInfo; 

	memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
	memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
	siStartupInfo.cb = sizeof(siStartupInfo); 

	gcString workingDir(UTIL::OS::getCurrentDir());

	UINT oldErrMode = SetErrorMode(0);

	BOOL res = CreateProcessA(const_cast<char*>(exe.c_str()), const_cast<char*>(params.c_str()), 0, 0, false,  CREATE_DEFAULT_ERROR_MODE, 0, workingDir.c_str(),  &siStartupInfo, &piProcessInfo);
	DWORD lastError = GetLastError();

	SetErrorMode(oldErrMode);
	CloseHandle( piProcessInfo.hThread );

	if (res == false)
	{
		CloseHandle( piProcessInfo.hProcess );
		throw gcException(ERR_LAUNCH, lastError, "Failed to start render helper");
	}
	else
	{
#ifdef DEBUG
		WaitForInputIdle(piProcessInfo.hProcess, INFINITE); 
#else
		WaitForInputIdle(piProcessInfo.hProcess, 5000); 
#endif
		CloseHandle( piProcessInfo.hProcess );
	}
}

void CefIPCPipeClient::stopService()
{

}

IPCBrowser* CefIPCPipeClient::createBrowser(HWND hwnd, const char* defaultUrl)
{
	IPCBrowser* ret = NULL;

	try
	{
		ret = IPC::CreateIPCClass< IPCBrowser >(this, "IPCBrowser");
		ret->newBrowser((uint64)hwnd, defaultUrl);
	}
	catch (gcException)
	{
		if (ret)
			ret->destroy();

		ret = NULL;
	}

	return ret;
}