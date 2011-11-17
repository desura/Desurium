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
#include "serviceMain.h"

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);

SERVICE_STATUS g_ServiceStatus; 
SERVICE_STATUS_HANDLE g_hStatus; 

CGCServiceApp g_ServiceApp;

void SetCrashSettings(const wchar_t* user, bool upload)
{
	g_ServiceApp.setCrashSettings(user, upload);
}

void OnPipeDisconnect()
{
	g_ServiceApp.onDisconnect();
}


void PrintfMsg(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

int main(int argc, char** argv) 
{ 
#ifdef DEBUG
#if 0
	while (!IsDebuggerPresent())
		gcSleep( 500 );
#endif
#endif

	if (argc >= 2 && strcmp(argv[1], "-exe")== 0)
	{
		if (g_ServiceApp.start(argc, argv))
		{
			printf("Service has started. Press any key to exit....\n");
			getchar();
		}
		else
		{
			printf("Failed to start service.\n");
		}

		g_ServiceApp.stop();
	}
	else
	{
		SERVICE_TABLE_ENTRY ServiceTable[2];
		ServiceTable[0].lpServiceName = SERVICE_NAME;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

		ServiceTable[1].lpServiceName = NULL;
		ServiceTable[1].lpServiceProc = NULL;

		StartServiceCtrlDispatcher(ServiceTable);
	}
}

HANDLE g_WaitEvent = NULL;

void ServiceMain(int argc, char** argv) 
{ 
	g_WaitEvent = CreateEvent(NULL, false, false, NULL);

    g_ServiceStatus.dwServiceType        = SERVICE_WIN32; 
    g_ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    g_ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwWin32ExitCode      = 0; 
    g_ServiceStatus.dwServiceSpecificExitCode = 0; 
    g_ServiceStatus.dwCheckPoint         = 0; 
    g_ServiceStatus.dwWaitHint           = 0; 

	g_hStatus = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)ControlHandler); 
	
	if (!g_ServiceApp.start(argc, argv))
	{
		g_ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
		g_ServiceStatus.dwWin32ExitCode      = -1; 
		SetServiceStatus(g_hStatus, &g_ServiceStatus);
	}
	else
	{
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
		SetServiceStatus(g_hStatus, &g_ServiceStatus);

		while (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
			WaitForSingleObject(g_WaitEvent, INFINITE);
	}

	g_ServiceApp.stop();

	CloseHandle(g_WaitEvent);
	g_WaitEvent = NULL;
}

void ControlHandler(DWORD request) 
{ 
	switch(request) 
	{ 
	case SERVICE_CONTROL_STOP: 
		g_ServiceStatus.dwWin32ExitCode = 0; 
		g_ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
		break;
 
	case SERVICE_CONTROL_SHUTDOWN: 
		g_ServiceStatus.dwWin32ExitCode = 0; 
		g_ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
		break;
	} 

	SetEvent(g_WaitEvent);
	SetServiceStatus(g_hStatus,  &g_ServiceStatus);
}