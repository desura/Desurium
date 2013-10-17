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
#include "IPCPipeServer.h"
#include "util_thread\BaseThread.h"
#include "MiniDumpGenerator.h"
#include "SharedObjectLoader.h"

void PrintfMsg(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);

#ifdef DEBUG
	char out[1024]={0};
	vsnprintf_s(out, 1024, 1024, format, args);
	OutputDebugStringA(out);
#endif

	va_end(args);
}

typedef BOOL (WINAPI* WaitForDebuggerFunc)();
typedef BOOL (WINAPI* SetDllDirectoryFunc)(LPCTSTR lpPathName);

namespace BootLoaderUtil
{
	bool SetDllDir(const char* dir)
	{
		SharedObjectLoader sol;

		if (sol.load("kernel32.dll"))
		{
			SetDllDirectoryFunc set_dll_directory = sol.getFunction<SetDllDirectoryFunc>("SetDllDirectoryA");
	
			if (set_dll_directory && set_dll_directory(dir)) 
				return true;
		}

		return false;
	}

	void SetCurrentDir()
	{
		char exePath[255];
		GetModuleFileName(NULL, exePath, 255);

		size_t exePathLen = strlen(exePath);
		for (size_t x=exePathLen; x>0; x--)
		{
			if (exePath[x] == '\\')
				break;
			else
				exePath[x] = '\0';
		}

		SetCurrentDirectory(exePath);
	}

	void WaitForDebugger()
	{
		HMODULE kernel32_dll = GetModuleHandle("kernel32.dll");
		if (kernel32_dll != NULL)
		{
			WaitForDebuggerFunc waitfor_debugger = (WaitForDebuggerFunc)GetProcAddress(kernel32_dll, "IsDebuggerPresent");
	
			if (waitfor_debugger != NULL) 
			{
				while( !waitfor_debugger() )
					Sleep( 500 );
			}
		}
	}
}

class Color;

void LogMsg(int type, std::string msg, Color *col)
{
	printf("%s", msg.c_str());
}

void LogMsg(int type, std::wstring msg, Color *col)
{
	wprintf(L"%s", msg.c_str());
}

::Thread::WaitCondition g_WaitCond;
volatile bool g_bClientConnected = false;

void OnPipeDisconnect(uint32 &e)
{
	g_WaitCond.notify();
}

void OnPipeConnect(uint32 &e)
{
	g_bClientConnected = true;
	g_WaitCond.notify();
}

bool InitWebControl(const char* hostName, const char* userAgent);
void ShutdownWebControl();

int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef DEBUG
	//BootLoaderUtil::WaitForDebugger();
#endif

#ifndef DEBUG
	MiniDumpGenerator miniDump;
	miniDump.showMessageBox(false);
#endif

	BootLoaderUtil::SetCurrentDir();

	if (!BootLoaderUtil::SetDllDir(".\\bin"))
		exit(-100);			

	std::vector<std::string> out;
	UTIL::STRING::tokenize(gcString(lpCmdLine), out, " ");

	std::string userAgent;

	UTIL::STRING::base64_decode(out[2], [&userAgent](const unsigned char* buff, size_t size) -> bool
	{
		userAgent.append((const char*)buff, size);
		return true;
	});

	InitWebControl(out[1].c_str(), userAgent.c_str());

	{
		IPC::PipeServer pipeserver(out[0].c_str(), 1, true);

		pipeserver.onConnectEvent += delegate(&OnPipeConnect);
		pipeserver.onDisconnectEvent += delegate(&OnPipeDisconnect);

		pipeserver.start();

		g_WaitCond.wait(15);

		if (g_bClientConnected)
			g_WaitCond.wait();
	}

	ShutdownWebControl();
}
