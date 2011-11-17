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


#include "UtilBootloader.h"


typedef bool (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, DWORD*);
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);


#define PRODUCT_BUSINESS 0x00000006
#define PRODUCT_BUSINESS_N 0x00000010
#define PRODUCT_CLUSTER_SERVER 0x00000012
#define PRODUCT_DATACENTER_SERVER 0x00000008
#define PRODUCT_DATACENTER_SERVER_CORE 0x0000000C
#define PRODUCT_DATACENTER_SERVER_CORE_V 0x00000027
#define PRODUCT_DATACENTER_SERVER_V 0x00000025
#define PRODUCT_ENTERPRISE 0x00000004
#define PRODUCT_ENTERPRISE_N 0x0000001B
#define PRODUCT_ENTERPRISE_SERVER 0x0000000A
#define PRODUCT_ENTERPRISE_SERVER_CORE 0x0000000E
#define PRODUCT_ENTERPRISE_SERVER_CORE_V 0x00000029
#define PRODUCT_ENTERPRISE_SERVER_IA64 0x0000000F
#define PRODUCT_ENTERPRISE_SERVER_V 0x00000026
#define PRODUCT_HOME_BASIC 0x00000002
#define PRODUCT_HOME_BASIC_N 0x00000005
#define PRODUCT_HOME_PREMIUM 0x00000003
#define PRODUCT_HOME_PREMIUM_N 0x0000001A
#define PRODUCT_HYPERV 0x0000002A
#define PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT 0x0000001E
#define PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING 0x00000020
#define PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY 0x0000001F
#define PRODUCT_SERVER_FOR_SMALLBUSINESS 0x00000018
#define PRODUCT_SERVER_FOR_SMALLBUSINESS_V 0x00000023
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM 0x00000019
#define PRODUCT_SMALLBUSINESS_SERVER 0x00000009
#define PRODUCT_STANDARD_SERVER 0x00000007
#define PRODUCT_STANDARD_SERVER_CORE 0x0000000D
#define PRODUCT_STANDARD_SERVER_CORE_V 0x00000028
#define PRODUCT_STANDARD_SERVER_V 0x00000024
#define PRODUCT_STARTER 0x0000000B
#define PRODUCT_STORAGE_ENTERPRISE_SERVER 0x00000017
#define PRODUCT_STORAGE_EXPRESS_SERVER 0x00000014
#define PRODUCT_STORAGE_STANDARD_SERVER 0x00000015
#define PRODUCT_STORAGE_WORKGROUP_SERVER 0x00000016
#define PRODUCT_UNDEFINED 0x00000000
#define PRODUCT_ULTIMATE 0x00000001
#define PRODUCT_ULTIMATE_N 0x0000001C
#define PRODUCT_WEB_SERVER 0x00000011
#define PRODUCT_WEB_SERVER_CORE 0x0000001D
#define VER_SUITE_WH_SERVER 0x00008000
#define SM_SERVERR2 89 

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