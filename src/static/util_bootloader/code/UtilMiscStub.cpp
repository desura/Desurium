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

typedef bool (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, DWORD*);
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

namespace BootLoaderUtil
{

unsigned int GetOSId()
{
	unsigned int ver = 0;	

	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi)))
		return WINDOWS_PRE2000;

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");

	if (NULL != pGNSI)
		pGNSI(&si);
	else 
		GetSystemInfo(&si);

	if ( VER_PLATFORM_WIN32_NT==osvi.dwPlatformId && osvi.dwMajorVersion > 4 )
	{
		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
		{
			if( osvi.wProductType == VER_NT_WORKSTATION )
				ver = WINDOWS_7;
			else 
				ver = WINDOWS_SERVER2008;
		}

		// Test for the specific product.
		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
		{
			if( osvi.wProductType == VER_NT_WORKSTATION )
				ver = WINDOWS_VISTA;
			else 
				ver = WINDOWS_SERVER2008;
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
		{
			if( GetSystemMetrics(SM_SERVERR2) )
				ver = WINDOWS_SERVER2003;
			else if ( osvi.wSuiteMask==VER_SUITE_STORAGE_SERVER )
				ver = WINDOWS_SERVER2003;
			else if ( osvi.wSuiteMask==VER_SUITE_WH_SERVER )
				ver = WINDOWS_HOMESERVER;
			else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
				ver = WINDOWS_XP64;
			else 
				ver = WINDOWS_SERVER2003;
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
		{
			ver = WINDOWS_XP;
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
		{
			if ( osvi.wProductType == VER_NT_WORKSTATION )
			{
				ver = WINDOWS_2000;
			}
			else 
			{
				ver = WINDOWS_SERVER2000;
			}
		}
	}
	else
	{
		ver = WINDOWS_PRE2000;
	}

	return ver;
}

}