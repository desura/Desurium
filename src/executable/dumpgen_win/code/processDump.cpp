///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : dumpgen
//   File        : processDump.cpp
//   Description :
//      [TODO: Write the purpose of processDump.cpp.]
//
//   Created On: 9/24/2010 7:11:54 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////
#include "Common.h"

#include <string.h>

#include "CrashAlert.h"
#include "SharedObjectLoader.h"

#include <deque>

#include "UtilBootloader.h"

#ifdef WIN32
#include <process.h>
#include <Psapi.h>
#endif

#include "DesuraWnd.h"

using namespace Desurium;

UINT __stdcall UploadDump(void* dumpInfo);

bool RestartDesura(const char* args);
void GetBuildBranch(int &build, int &branch);
void GetString(const char* str, const char* input, char* out, size_t outSize);

typedef bool (*UploadCrashFn)(const char* path, const char* user, int build, int branch);

class DumpInfo
{
public:
	DumpInfo(const char* file, const char* user, volatile bool &complete) : m_szComplete(complete)
	{
		m_szFile = file;
		m_szUser = user;
	}

	const char* m_szFile;
	const char* m_szUser;
	volatile bool &m_szComplete;
};

void TerminateDesura()
{
	unsigned long aProcesses[1024], cbNeeded, cProcesses;
	if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		return;

	unsigned long curPID = GetCurrentProcessId();

	cProcesses = cbNeeded / sizeof(unsigned long);
	for (unsigned int i = 0; i < cProcesses; i++)
	{
		if(aProcesses[i] == 0)
			continue;

		if (aProcesses[i] == curPID)
			continue;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_TERMINATE, 0, aProcesses[i]);

		if (!hProcess)
			continue;

		char buffer[50] = {0};
		GetModuleBaseName(hProcess, 0, buffer, 50);
		
		if(strcmp("desura.exe", buffer)==0)
		{
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);

			break;
		}
		else
		{
			CloseHandle(hProcess);
		}
	}
}


void ProcessDump(const char* m_lpCmdLine)
{
//#ifdef DEBUG
//	BootLoaderUtil::WaitForDebugger();
//#endif

	BootLoaderUtil::CMDArgs args(m_lpCmdLine);

	volatile bool uploadComplete = false;
	char file[255] = {0};
	char user[255] = {0};

	bool msgbox = args.hasArg("msgbox");
	bool upload = (args.hasArg("noupload") == false);

	if (args.hasArg("file"))
		args.getString("file", file, 255);

	if (args.hasArg("user"))
		args.getString("user", user, 255);

	if (file[0] && upload)
	{
		DumpInfo *di = new DumpInfo(file, user, uploadComplete);
		CDesuraWnd::BeginThread(&UploadDump, (void*)di);
	}
	else
	{
		uploadComplete = true;
	}

	if (msgbox)
	{
		CrashAlert ca;
		int res = ca.DoModal();

		TerminateDesura();

		if (res == IDOK) //restart
		{
			RestartDesura("");	
		}
		else if (res == IDYES) //force update
		{
			RestartDesura("-forceupdate");
		}
	}

	while (uploadComplete == false)
		Sleep(500);
}

UINT __stdcall UploadDump(void* dumpInfo)
{
	int build = 0;
	int branch = 0;

	GetBuildBranch(build, branch);

	DumpInfo* di = static_cast<DumpInfo*>(dumpInfo);

	SharedObjectLoader sol;

	const char* modualName = "crashuploader.dll";

	if (!sol.load(modualName))
	{
		di->m_szComplete = true;
		return -1;
	}

	UploadCrashFn uploadCrash = sol.getFunction<UploadCrashFn>("UploadCrash");

	if (uploadCrash == NULL)
	{
		di->m_szComplete = true;
		return -2;
	}

	if (!uploadCrash(di->m_szFile, di->m_szUser, build, branch))
	{
		di->m_szComplete = true;
		return -3;		
	}

	di->m_szComplete = true;
	return 0;
}

bool RestartDesura(const char* args)
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

	PROCESS_INFORMATION ProcInfo = {0};
	STARTUPINFO StartupInfo = {0};

	char launchArg[512];
	const char* exeName = "desura.exe";

	_snprintf_s(launchArg, 512, _TRUNCATE, "%s %s", exeName, args?args:"");
	BOOL res = CreateProcess(NULL, launchArg, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, exePath, &StartupInfo, &ProcInfo);

	CloseHandle(ProcInfo.hProcess);
	CloseHandle(ProcInfo.hThread);

	return res?true:false;
}

void GetBuildBranch(int &build, int &branch)
{
	char lszValue[255];

	DWORD dwType=REG_SZ;
	DWORD dwSize=255;
	HKEY hk;

	DWORD err1 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Desura\\DesuraApp", 0, KEY_QUERY_VALUE, &hk);

	lszValue[0] = 0;
	DWORD err2 = RegQueryValueEx(hk, "appid", NULL, &dwType,(LPBYTE)&lszValue, &dwSize);
	branch = atoi(lszValue);

	lszValue[0] = 0;
	DWORD err3 = RegQueryValueEx(hk, "appver", NULL, &dwType,(LPBYTE)&lszValue, &dwSize);
	build = atoi(lszValue);

	RegCloseKey(hk);
}
