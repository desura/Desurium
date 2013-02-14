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
#include "util/UtilWindows.h"
#include "UtilBootloader.h"

#ifndef SECURITY_MANDATORY_UNTRUSTED_RID

#define SECURITY_MANDATORY_UNTRUSTED_RID            (0x00000000L)
#define SECURITY_MANDATORY_LOW_RID                  (0x00001000L)
#define SECURITY_MANDATORY_MEDIUM_RID               (0x00002000L)
#define SECURITY_MANDATORY_HIGH_RID                 (0x00003000L)
#define SECURITY_MANDATORY_SYSTEM_RID               (0x00004000L)
#define SECURITY_MANDATORY_PROTECTED_PROCESS_RID    (0x00005000L)

typedef struct _TOKEN_MANDATORY_LABEL 
{
	SID_AND_ATTRIBUTES Label;
} TOKEN_MANDATORY_LABEL, *PTOKEN_MANDATORY_LABEL;

typedef enum _MANDATORY_LEVEL 
{
	MandatoryLevelUntrusted = 0,
	MandatoryLevelLow,
	MandatoryLevelMedium,
	MandatoryLevelHigh,
	MandatoryLevelSystem,
	MandatoryLevelSecureProcess,
	MandatoryLevelCount
} MANDATORY_LEVEL, *PMANDATORY_LEVEL;

#define TokenIntegrityLevel ((TOKEN_INFORMATION_CLASS)25)
#endif //!SECURITY_MANDATORY_UNTRUSTED_RID


//! @brief Function enables/disables/removes a privelege associated with the given token
//! @detailed Calls LookupPrivilegeValue() and AdjustTokenPrivileges()
//! @param hToken - access token handle
//! @param lpszPrivilege - name of privilege to enable/disable
//! @param dwAttributes - (SE_PRIVILEGE_ENABLED) to enable or (0) disable or (SE_PRIVILEGE_REMOVED) to remove privilege
//! @return HRESULT code

HRESULT SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, DWORD dwAttributes=SE_PRIVILEGE_ENABLED) 
{
	HRESULT hr=S_OK;
	LUID luid;

	if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid ))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = dwAttributes;

		// Enable the privilege or disable all privileges.

		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL))
			hr=HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
	}
	
	return hr;
}

/*!
Function removes the priveleges which are not associated by default with explorer.exe at Medium Integration Level in Vista
@returns HRESULT of the operation on SE_CREATE_GLOBAL_NAME (="SeCreateGlobalPrivilege")
*/
inline HRESULT ReducePrivilegesForMediumIL(HANDLE hToken) 
{
	HRESULT hr=S_OK;
	hr=SetPrivilege(hToken, SE_CREATE_GLOBAL_NAME, SE_PRIVILEGE_REMOVED);

	SetPrivilege(hToken, SE_BACKUP_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_CREATE_PAGEFILE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, TEXT("SeCreateSymbolicLinkPrivilege"), SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_DEBUG_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_IMPERSONATE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_INC_BASE_PRIORITY_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_INCREASE_QUOTA_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_LOAD_DRIVER_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_MANAGE_VOLUME_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_PROF_SINGLE_PROCESS_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_REMOTE_SHUTDOWN_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_RESTORE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SECURITY_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SYSTEM_ENVIRONMENT_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SYSTEM_PROFILE_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_SYSTEMTIME_NAME, SE_PRIVILEGE_REMOVED);
	SetPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, SE_PRIVILEGE_REMOVED);

	return hr;
}

#define TraceLN(Function, Line) OutputDebugStringA(Function " line:" #Line "\n")

/*!
@brief Gets Integration level of the given process in Vista. 
In the older OS assumes the integration level is equal to SECURITY_MANDATORY_HIGH_RID

The function opens the process for all access and opens its token for all access. 
Then it extracts token information and closes the handles.
@param[in] dwProcessId ID of the process to operate
@param[out] pdwProcessIL pointer to write the value
@return HRESULT
@retval <return value> { description }
@remarks Function check for OS version by querying the presence of Kernel32.GetProductInfo function. 
This way is used due to the function is called from InstallShield12 script, so GetVersionEx returns incorrect value.
@todo restrict access rights when quering for tokens
*/
inline HRESULT GetProcessIL(DWORD dwProcessId, LPDWORD pdwProcessIL)
{
	HRESULT hr=S_OK;
	if(!pdwProcessIL)
		hr=E_INVALIDARG;
	if(SUCCEEDED(hr))
	{
		bool bVista=false;
		{
			// When the function is called from IS12, GetVersionEx returns dwMajorVersion=5 on Vista!
			HMODULE hmodKernel32=LoadLibraryA("Kernel32");
			if(hmodKernel32 && GetProcAddress(hmodKernel32, "GetProductInfo"))
				bVista=true;
			if(hmodKernel32) FreeLibrary(hmodKernel32);
		}

		DWORD dwIL=SECURITY_MANDATORY_HIGH_RID;
		if(bVista)
		{//Vista
			HANDLE hToken=NULL;
			HANDLE hProcess=OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if(hProcess)
			{
				if(OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
				{
					PTOKEN_MANDATORY_LABEL pTIL=NULL;
					DWORD dwSize=0;
					if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwSize) 
						&& ERROR_INSUFFICIENT_BUFFER==GetLastError() && dwSize)
						pTIL=(PTOKEN_MANDATORY_LABEL)HeapAlloc(GetProcessHeap(), 0, dwSize);

					if(pTIL && GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize, &dwSize))
					{
						LPBYTE lpb=GetSidSubAuthorityCount(pTIL->Label.Sid);
						if(lpb)
							dwIL = *GetSidSubAuthority(pTIL->Label.Sid, *lpb-1);
						else
							hr=E_UNEXPECTED;
					}
					if(pTIL)
						HeapFree(GetProcessHeap(), 0, pTIL);
					CloseHandle(hToken);
				}//if(OpenProcessToken(...))
				CloseHandle(hProcess);
			}//if(hProcess)
		}//if(bVista)
		if(SUCCEEDED(hr))
			*pdwProcessIL=dwIL;
	}//if(SUCCEEDED(hr))
	return hr;
}

/*!
@brief Function launches process with the integration level of Explorer on Vista. On the previous OS, simply creates the process.

Function gets the integration level of the current process and Explorer, then launches the new process.
If the integration levels are equal, CreateProcess is called. 
If Explorer has Medium IL, and the current process has High IL, new token is created, its rights are adjusted 
and CreateProcessWithTokenW is called. 
If Explorer has Medium IL, and the current process has High IL, error is returned.
@param[in] szProcessName - the name of exe file (see CreatePorcess()) 
@param[in] szCmdLine - the name of exe file (see CreatePorcess())
@return HRESULT code
@note The function cannot be used in services, due to if uses USER32.FindWindow() to get the proper instance of Explorer. 
The parent of new process in taskmgr.exe, but not the current process.
@sa ReducePrivilegesForMediumIL()
*/
HRESULT CreateProcessWithExplorerIL(LPWSTR szProcessName, LPWSTR szCmdLine, LPWSTR szWorkingDir)
{
	HRESULT hr=S_OK;
	
	BOOL bRet;
	HANDLE hToken;
	HANDLE hNewToken;

	unsigned int osid = BootLoaderUtil::GetOSId();

	bool bVista= !(osid == WINDOWS_XP || osid == WINDOWS_XP64);

	wchar_t cmdArg[255];
	_snwprintf_s(cmdArg, 255, _TRUNCATE, L"\"%s\" %s", szProcessName, szCmdLine);

	PROCESS_INFORMATION ProcInfo = {0};
	STARTUPINFOW StartupInfo = {0};

	if(bVista)
	{
		DWORD dwCurIL=SECURITY_MANDATORY_HIGH_RID; 
		DWORD dwExplorerID=0, dwExplorerIL=SECURITY_MANDATORY_HIGH_RID;

		HWND hwndShell=::FindWindowA("Progman", NULL);
		if(hwndShell)
			GetWindowThreadProcessId(hwndShell, &dwExplorerID);
		
		hr=GetProcessIL(dwExplorerID, &dwExplorerIL);
		if(SUCCEEDED(hr))
			hr=GetProcessIL(GetCurrentProcessId(), &dwCurIL);
		if(SUCCEEDED(hr))
		{
			if(dwCurIL==SECURITY_MANDATORY_HIGH_RID && dwExplorerIL==SECURITY_MANDATORY_MEDIUM_RID)
			{
				HANDLE hProcess=OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwExplorerID);
				if(hProcess)
				{
					if(OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
					{
						if(!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL,
							SecurityImpersonation, TokenPrimary, &hNewToken))
							hr=HRESULT_FROM_WIN32(GetLastError());
						if(SUCCEEDED(hr))
						{
							if(dwCurIL==SECURITY_MANDATORY_MEDIUM_RID && dwExplorerIL==SECURITY_MANDATORY_MEDIUM_RID)
							{
								hr=ReducePrivilegesForMediumIL(hNewToken);
							}//if(dwCurIL==...)

							if(SUCCEEDED(hr))
							{

								typedef BOOL (WINAPI *LPFN_CreateProcessWithTokenW)(
									HANDLE hToken,
									DWORD dwLogonFlags,
									LPCWSTR lpApplicationName,
									LPWSTR lpCommandLine,
									DWORD dwCreationFlags,
									LPVOID lpEnvironment,
									LPCWSTR lpCurrentDirectory,
									LPSTARTUPINFOW lpStartupInfo,
									LPPROCESS_INFORMATION lpProcessInfo
									);



								LPFN_CreateProcessWithTokenW fnCreateProcessWithTokenW=NULL;
								HINSTANCE hmodAdvApi32=LoadLibraryA("AdvApi32");

								if(hmodAdvApi32)
									fnCreateProcessWithTokenW=(LPFN_CreateProcessWithTokenW)GetProcAddress(hmodAdvApi32, "CreateProcessWithTokenW");


								if(fnCreateProcessWithTokenW)
								{
									bRet=fnCreateProcessWithTokenW(hNewToken, 0, NULL, cmdArg, 0, NULL, szWorkingDir, &StartupInfo, &ProcInfo);

									if(!bRet)
										hr=HRESULT_FROM_WIN32(GetLastError());
								}
								else
								{
									hr=E_UNEXPECTED;
								}


								if(hmodAdvApi32)
									FreeLibrary(hmodAdvApi32);

							}//if(SUCCEEDED(hr))


							CloseHandle(hNewToken);
						}//if (DuplicateTokenEx(...)
						else
						{
							hr=HRESULT_FROM_WIN32(GetLastError());
						}

						CloseHandle(hToken);
					}//if(OpenProcessToken(...))
					else
					{
						hr=HRESULT_FROM_WIN32(GetLastError());
					}


					CloseHandle(hProcess);
				}//if(hProcess)


			}//if(dwCurIL==SECURITY_MANDATORY_HIGH_RID && dwExplorerIL==SECURITY_MANDATORY_MEDIUM_RID)
			else if(dwCurIL==SECURITY_MANDATORY_MEDIUM_RID && dwExplorerIL==SECURITY_MANDATORY_HIGH_RID)
			{
				hr=E_ACCESSDENIED;
			}

		}//if(SUCCEEDED(hr))
	}//if(bVista)

	if(SUCCEEDED(hr) && !ProcInfo.dwProcessId)
	{// 2K | XP | Vista & !UAC
		bRet = CreateProcessW(NULL, cmdArg, NULL, NULL, FALSE, 0, NULL, szWorkingDir, &StartupInfo, &ProcInfo);

		if(!bRet)
			hr=HRESULT_FROM_WIN32(GetLastError());
	}// 2K | XP | Vista & !UAC

	return hr;
}


