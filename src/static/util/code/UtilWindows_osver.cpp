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


#include <windows.h>
#include <tchar.h>
#include <stdio.h>
//#include <strsafe.h>

#ifdef TEXT
#undef TEXT
#define TEXT(a) a
#endif

namespace UTIL
{
namespace WIN
{



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

OS_VERSION getOSId()
{
	OS_VERSION ver = WINDOWS_UNKNOWN;	

	OSVERSIONINFOEXA osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);

	if( !(bOsVersionInfoEx = GetVersionExA((OSVERSIONINFOA*)&osvi)))
		return WINDOWS_PRE2000;

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandleA(TEXT("kernel32.dll")), "GetNativeSystemInfo");

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

// The following code was taken from http://msdn.microsoft.com/en-us/library/ms724429(VS.85).aspx and remains the property of its respected owner.
void getOSString(char* dest, size_t destSize)
{
	#define BUFSIZE 1000
	char pszOS[BUFSIZE];

	OSVERSIONINFOEXA osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	PGPI pGPI;
	BOOL bOsVersionInfoEx;
	DWORD dwType;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);

	if( !(bOsVersionInfoEx = GetVersionExA((LPOSVERSIONINFOA)&osvi)))
		return;

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandleA(TEXT("kernel32.dll")), "GetNativeSystemInfo");

	if (NULL != pGNSI)
		pGNSI(&si);
	else 
		GetSystemInfo(&si);

	if ( VER_PLATFORM_WIN32_NT==osvi.dwPlatformId && osvi.dwMajorVersion > 4 )
	{
		Safe::strcpy(pszOS, BUFSIZE, TEXT("Microsoft "));

      if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
      {
         if( osvi.wProductType == VER_NT_WORKSTATION )
             Safe::strcat(pszOS, BUFSIZE, TEXT("Windows 7 "));
         else 
			 Safe::strcat(pszOS, BUFSIZE, TEXT("Windows Server 2008 R2 " ));
      }

		// Test for the specific product.
		if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
		{
			if( osvi.wProductType == VER_NT_WORKSTATION )
				Safe::strcat(pszOS, BUFSIZE, TEXT("Windows Vista "));
			else 
				Safe::strcat(pszOS, BUFSIZE, TEXT("Windows Server 2008 " ));

			pGPI = (PGPI) GetProcAddress(GetModuleHandleA(TEXT("kernel32.dll")), "GetProductInfo");
			pGPI( 6, 0, 0, 0, &dwType);

			switch( dwType )
			{
			case PRODUCT_ULTIMATE:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Ultimate Edition" ));
				break;

			case PRODUCT_HOME_PREMIUM:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Home Premium Edition" ));
				break;

			case PRODUCT_HOME_BASIC:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Home Basic Edition" ));
				break;

			case PRODUCT_ENTERPRISE:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Enterprise Edition" ));
				break;

			case PRODUCT_BUSINESS:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Business Edition" ));
				break;

			case PRODUCT_STARTER:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Starter Edition" ));
				break;

			case PRODUCT_CLUSTER_SERVER:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Cluster Server Edition" ));
				break;

			case PRODUCT_DATACENTER_SERVER:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Datacenter Edition" ));
				break;

			case PRODUCT_DATACENTER_SERVER_CORE:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Datacenter Edition (core installation)" ));
				break;

			case PRODUCT_ENTERPRISE_SERVER:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Enterprise Edition" ));
				break;

			case PRODUCT_ENTERPRISE_SERVER_CORE:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Enterprise Edition (core installation)" ));
				break;

			case PRODUCT_ENTERPRISE_SERVER_IA64:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems" ));
				break;

			case PRODUCT_SMALLBUSINESS_SERVER:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Small Business Server" ));
				break;
				
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Small Business Server Premium Edition" ));
				break;

			case PRODUCT_STANDARD_SERVER:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Standard Edition" ));
				break;

			case PRODUCT_STANDARD_SERVER_CORE:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Standard Edition (core installation)" ));
				break;

			case PRODUCT_WEB_SERVER:
				Safe::strcat(pszOS, BUFSIZE, TEXT("Web Server Edition" ));
				break;
			}

			if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
				Safe::strcat(pszOS, BUFSIZE, TEXT( ", 64-bit" ));
			else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
				Safe::strcat(pszOS, BUFSIZE, TEXT(", 32-bit"));
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
		{
			if( GetSystemMetrics(SM_SERVERR2) )
				Safe::strcat(pszOS, BUFSIZE, TEXT( "Windows Server 2003 R2, "));
			else if ( osvi.wSuiteMask==VER_SUITE_STORAGE_SERVER )
				Safe::strcat(pszOS, BUFSIZE, TEXT( "Windows Storage Server 2003"));
			else if ( osvi.wSuiteMask==VER_SUITE_WH_SERVER )
				Safe::strcat(pszOS, BUFSIZE, TEXT( "Windows Home Server"));
			else if( osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
				Safe::strcat(pszOS, BUFSIZE, TEXT( "Windows XP Professional x64 Edition"));
			else 
				Safe::strcat(pszOS, BUFSIZE, TEXT("Windows Server 2003, "));

			// Test for the server type.
			if ( osvi.wProductType != VER_NT_WORKSTATION )
			{
				if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
				{
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Datacenter Edition for Itanium-based Systems" ));
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					Safe::strcat(pszOS, BUFSIZE, TEXT( "Enterprise Edition for Itanium-based Systems" ));
				}
				else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
				{
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Datacenter x64 Edition" ));
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Enterprise x64 Edition" ));
					else 
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Standard x64 Edition" ));
				}
				else
				{
					if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Compute Cluster Edition" ));
					else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Datacenter Edition" ));
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Enterprise Edition" ));
					else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Web Edition" ));
					else 
						Safe::strcat(pszOS, BUFSIZE, TEXT( "Standard Edition" ));
				}
			}
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
		{
			Safe::strcat(pszOS, BUFSIZE, TEXT("Windows XP "));

			if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
				Safe::strcat(pszOS, BUFSIZE, TEXT( "Home Edition" ));
			else 
				Safe::strcat(pszOS, BUFSIZE, TEXT( "Professional" ));
		}

		if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
		{
			Safe::strcat(pszOS, BUFSIZE, TEXT("Windows 2000 "));

			if ( osvi.wProductType == VER_NT_WORKSTATION )
			{
				Safe::strcat(pszOS, BUFSIZE, TEXT( "Professional" ));
			}
			else 
			{
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					Safe::strcat(pszOS, BUFSIZE, TEXT( "Datacenter Server" ));
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					Safe::strcat(pszOS, BUFSIZE, TEXT( "Advanced Server" ));
				else 
					Safe::strcat(pszOS, BUFSIZE, TEXT( "Server" ));
			}
		}

		// Include service pack (if any) and build number.
		if ( osvi.szCSDVersion[0] != '0' )
		{
			Safe::strcat(pszOS, BUFSIZE, TEXT(" ") );
			Safe::strcat(pszOS, BUFSIZE, osvi.szCSDVersion);
		}

		char buf[80];
		Safe::snprintf(buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
		Safe::strcat(pszOS, BUFSIZE, buf);

		Safe::strcpy(dest, destSize, pszOS);
	}
	else
	{  
		Safe::strcpy(dest, destSize, "Non Supported version of windows.\n");
	}
}

}
}