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

#include "UtilBootloader.h"

#include <process.h>
#include <Psapi.h>

#include <vector>
#include <map>
#include <string>

#include <commctrl.h>

#include "SharedObjectLoader.h"
#include "third_party/PEImage.h"

HRESULT CreateProcessWithExplorerIL(LPWSTR szProcessName, LPWSTR szCmdLine, LPWSTR szWorkingDir);

typedef BOOL (WINAPI* WaitForDebuggerFunc)();
typedef BOOL (WINAPI* SetDllDirectoryFunc)(LPCTSTR lpPathName);


//keep pipe manager happy
void Warning(char const *, ...)
{
}

void Debug(char const *, ...)
{
}

void Msg(char const *, ...)
{
}


namespace BootLoaderUtil
{


#if !defined(DONT_INCLUDE_AFXWIN) && defined(MFC_FOUND)

CDummyWindow::CDummyWindow(HWND hWnd)
{
	Attach(hWnd);
}

// Don't let the CWnd destructor delete the HWND
CDummyWindow::~CDummyWindow()
{
	Detach();
}

#endif




bool IsExeRunning(char* pName)
{
	unsigned long aProcesses[1024], cbNeeded, cProcesses;
	if(!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		return false;

	unsigned long curPID = GetCurrentProcessId();

	cProcesses = cbNeeded / sizeof(unsigned long);
	for (unsigned int i = 0; i < cProcesses; i++)
	{
		if(aProcesses[i] == 0)
			continue;

		if (aProcesses[i] == curPID)
			continue;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, aProcesses[i]);

		if (!hProcess)
			continue;

		char buffer[50] = {0};
		GetModuleBaseName(hProcess, 0, buffer, 50);
		CloseHandle(hProcess);
		
		if(strcmp(pName, buffer)==0)
			return true;
	}
	
	return false;
}

void GetLastFolder(char* dest, size_t destSize, const char* src)
{
	size_t len = strlen(src)+1;
	size_t y =0;

	for (size_t x=len-1; x>0; x--)
	{
		if (src[x] == '\\' || src[x] == '/')
		{
			strncpy_s(dest, destSize, src+x+1, y);
			dest[y] = '\0';
			break;
		}
		y++;
	}
}


bool RestartAsNormal(const char* args)
{
	wchar_t name[255];
	GetModuleFileNameW(NULL, name, 255);

	wchar_t szWorkingDir[255];
	GetModuleFileNameW(NULL, szWorkingDir, 255);

	size_t exePathLen = Safe::wcslen(szWorkingDir, 255);
	for (size_t x=exePathLen; x>0; x--)
	{
		if (szWorkingDir[x] == L'\\')
			break;
		else
			szWorkingDir[x] = L'\0';
	}

	wchar_t wArgs[255];
	wArgs[0] = 0;

	if (args)
	{
		for (size_t x=0; x<strlen(args); x++)
			wArgs[x] = (wchar_t)args[x];

		wArgs[strlen(args)] = 0;
	}

	HRESULT res = CreateProcessWithExplorerIL(name, wArgs, szWorkingDir);
	return SUCCEEDED(res);
}

bool RestartAsAdmin(const char* args)
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

	char name[255];
	GetModuleFileName(NULL, name, 255);

	char restartArgs[255];
	
	if (args)
		strncpy_s(restartArgs, 255, args, 255);
	else
		restartArgs[0] =0;

	INT_PTR r = (INT_PTR)ShellExecute(NULL, "runas", name, restartArgs, exePath, SW_SHOW);
	return !(r < 32);
}

bool Restart(const char* args, bool wait)
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

	char name[255];
	GetModuleFileName(NULL, name, 255);

	PROCESS_INFORMATION ProcInfo = {0};
	STARTUPINFO StartupInfo = {0};

	char launchArg[255];

	if (args)
		_snprintf_s(launchArg, 255, _TRUNCATE, "\"%s\"%s %s", name, wait?" -wait":"", args);
	else
		_snprintf_s(launchArg, 255, _TRUNCATE, "\"%s\"%s", name, wait?" -wait":"");

	BOOL res = CreateProcess(NULL, launchArg, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, exePath, &StartupInfo, &ProcInfo );

	CloseHandle(ProcInfo.hProcess);
	CloseHandle(ProcInfo.hThread);

	return res?true:false;
}

bool StartProcess(const char* name, const char* args)
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

	char launchArg[255];

	if (args)
		_snprintf_s(launchArg, 255, _TRUNCATE, "\"%s\\%s\" -wait %s", exePath, name, args);
	else
		_snprintf_s(launchArg, 255, _TRUNCATE, "\"%s\\%s\" -wait", exePath, name);

	BOOL res = CreateProcess(NULL, launchArg, NULL, NULL, false, NORMAL_PRIORITY_CLASS, NULL, exePath, &StartupInfo, &ProcInfo );

	CloseHandle(ProcInfo.hProcess);
	CloseHandle(ProcInfo.hThread);

	return res?true:false;
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

void InitCommonControls()
{
	INITCOMMONCONTROLSEX InitCtrlEx;

	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS|ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&InitCtrlEx);
}

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


bool CheckForOtherInstances(HINSTANCE hinstant)
{
	char buffer[MAX_PATH+1];
	GetModuleFileName(hinstant, buffer, sizeof(buffer));

	char exe[MAX_PATH];
	BootLoaderUtil::GetLastFolder(exe, MAX_PATH, buffer);

	return BootLoaderUtil::IsExeRunning(exe);
}


void WaitForOtherInstance(char* name)
{
	while (BootLoaderUtil::IsExeRunning(name))
	{
		Sleep(500);
	}
}


void WaitForOtherInstance(HINSTANCE hinstant)
{
	while (CheckForOtherInstances(hinstant))
	{
		Sleep(500);
	}
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


void ConvertToArgs(const char* args, std::vector<std::string> &argv)
{
	size_t argLen = strlen(args);

	if (argLen > 0)
	{
		size_t count = 0;

		bool bSQuote = false;
		bool bDQuote = false;

		for (size_t x=0; x<argLen; x++)
		{
			if (args[x] == '\'')
				bSQuote = !bSQuote;

			if (args[x] == '"')
				bDQuote = !bDQuote;

			if (!bSQuote && !bDQuote && args[x] == ' ')
				count++;
		}

		size_t lastIndex = 0;

		for (size_t x=0; x<=argLen; x++)
		{
			if (args[x] == '\'')
				bSQuote = !bSQuote;

			if (args[x] == '"')
				bDQuote = !bDQuote;

			if (!bSQuote && !bDQuote && (args[x] == ' ' || args[x] == '\0'))
			{
				size_t end = x-1;

				while (args[lastIndex] == '\'' && args[end] == '\'' || args[lastIndex] == '"' && args[end] == '"')
				{
					lastIndex++;
					end--;
				}

				if (end < lastIndex)
					continue;

				size_t len = end-lastIndex+1;

				std::string str(args+lastIndex, len);
				lastIndex = x+1;

				argv.push_back(str);
			}
		}
	}
}



class CMDArgInternal
{
public:
	CMDArgInternal()
	{
		m_pszArgv = NULL;
	}

	~CMDArgInternal()
	{
		delete [] m_pszArgv;
	}

	const char** m_pszArgv;

	std::vector<std::string> m_vArgList;
	std::map<std::string, std::string> m_mArgv;
};

CMDArgs::CMDArgs(const char* args)
{
	m_pInternal = new CMDArgInternal();

	ConvertToArgs(args, m_pInternal->m_vArgList);

	m_pInternal->m_pszArgv = new const char*[m_pInternal->m_vArgList.size()];

	for (size_t x=0; x<m_pInternal->m_vArgList.size(); x++)
		m_pInternal->m_pszArgv[x] = m_pInternal->m_vArgList[x].c_str();

	process();
}

CMDArgs::~CMDArgs()
{
	delete m_pInternal;
}

void CMDArgs::addValue(const char* name, const char* value)
{
	if (!name)
		return;

	m_pInternal->m_mArgv[name] = value?value:"";
	m_pInternal->m_vArgList.push_back(name);

	if (value)
		m_pInternal->m_vArgList.push_back(value);

	delete [] m_pInternal->m_pszArgv;
	m_pInternal->m_pszArgv = new const char*[m_pInternal->m_vArgList.size()];

	for (size_t x=0; x<m_pInternal->m_vArgList.size(); x++)
		m_pInternal->m_pszArgv[x] = m_pInternal->m_vArgList[x].c_str();
}

void CMDArgs::process()
{
	size_t nArgs = m_pInternal->m_vArgList.size();
	size_t i=0; 

	while (i<nArgs)
	{
		std::string arg(m_pInternal->m_vArgList[i]);
		std::string narg;

		if (i+1 < nArgs)
			narg = std::string(m_pInternal->m_vArgList[i+1]);
		
		if (arg[0] == '-')
		{
			while (arg[0] == '-')
				arg.erase(0, 1);

			if (narg.size() > 0 && narg[0] != '-')
			{
				m_pInternal->m_mArgv[arg] = narg;
				i++;
			}
			else
			{
				m_pInternal->m_mArgv[arg] = "";
			}
		}
		else
		{
			m_pInternal->m_mArgv[arg] = "";
		}

		i++;
	}
}

bool CMDArgs::hasArg(const char* name)
{
	return (m_pInternal->m_mArgv.find(name) != m_pInternal->m_mArgv.end());
}

bool CMDArgs::hasValue(const char* name)
{
	return (hasArg(name) && m_pInternal->m_mArgv[name].size() > 0);
}

int CMDArgs::getInt(const char* name)
{
	int res = 0;

	if (hasValue(name))
		res = atoi(m_pInternal->m_mArgv[name].c_str());
	
	return res;
}

void CMDArgs::getString(const char* name, char* buff, size_t buffSize)
{
	if (!hasValue(name))
		return;

	std::string val = m_pInternal->m_mArgv[name];
	strncpy_s(buff, buffSize, val.c_str(), _TRUNCATE);
}

const char** CMDArgs::getArgv()
{
	return m_pInternal->m_pszArgv;
}

int CMDArgs::getArgc()
{
	return m_pInternal->m_vArgList.size();
}


#define uint8 unsigned char

void PreReadImage(const char* file_path)
{
	unsigned int win = GetOSId();
	const DWORD actual_step_size = static_cast<DWORD>(1024*1024);

	if (win > WINDOWS_VISTA) 
	{
		// Vista+ branch. On these OSes, the forced reads through the DLL actually
		// slows warm starts. The solution is to sequentially read file contents
		// with an optional cap on total amount to read.
		HANDLE file = CreateFile(file_path,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN, NULL);

		if (file != INVALID_HANDLE_VALUE)
			return;

		// Default to 1MB sequential reads.
		LPVOID buffer = ::VirtualAlloc(NULL, actual_step_size, MEM_COMMIT, PAGE_READWRITE);

		if (buffer == NULL)
		{
			CloseHandle(file);
			return;
		}

		DWORD len;
		size_t total_read = 0;

		while (::ReadFile(file, buffer, actual_step_size, &len, NULL) && len > 0) 
		{
			total_read += static_cast<size_t>(len);
		}

		::VirtualFree(buffer, 0, MEM_RELEASE);
		CloseHandle(file);
	} 
	else 
	{
		// WinXP branch. Here, reading the DLL from disk doesn't do
		// what we want so instead we pull the pages into memory by loading
		// the DLL and touching pages at a stride.
		HMODULE dll_module = ::LoadLibraryExA(file_path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH|DONT_RESOLVE_DLL_REFERENCES);

		if (!dll_module)
			return;

		base::win::PEImage pe_image(dll_module);
	
		PIMAGE_NT_HEADERS nt_headers = pe_image.GetNTHeaders();
		size_t actual_size_to_read = nt_headers->OptionalHeader.SizeOfImage;
		volatile uint8* touch = reinterpret_cast<uint8*>(dll_module);
	
		size_t offset = 0;
		while (offset < actual_size_to_read) 
		{
			uint8 unused = *(touch + offset);
			offset += actual_step_size;
		}
	
		FreeLibrary(dll_module);
	}
}

}