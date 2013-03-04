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

#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <mscoree.h>

#include "SharedObjectLoader.h"

// In case the machine this is compiled on does not have the most recent platform SDK
// with these values defined, define them here
#ifndef SMABLETPC
	#define SMABLETPC     86
#endif

#ifndef SM_MEDIACENTER
	#define SM_MEDIACENTER  87
#endif

#define CountOf(x) sizeof(x)/sizeof(*x)

// Constants that represent registry key names and value names
// to use for detection
const char *g_szNetfx10RegKeyName = ("Software\\Microsoft\\.NETFramework\\Policy\\v1.0");
const char *g_szNetfx10RegKeyValue = ("3705");
const char *g_szNetfx10SPxMSIRegKeyName = ("Software\\Microsoft\\Active Setup\\Installed Components\\{78705f0d-e8db-4b2d-8193-982bdda15ecd}");
const char *g_szNetfx10SPxOCMRegKeyName = ("Software\\Microsoft\\Active Setup\\Installed Components\\{FDC11A6F-17D1-48f9-9EA3-9051954BAA24}");
const char *g_szNetfx11RegKeyName = ("Software\\Microsoft\\NET Framework Setup\\NDP\\v1.1.4322");
const char *g_szNetfx20RegKeyName = ("Software\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727");
const char *g_szNetfx30RegKeyName = ("Software\\Microsoft\\NET Framework Setup\\NDP\\v3.0\\Setup");
const char *g_szNetfx30SpRegKeyName = ("Software\\Microsoft\\NET Framework Setup\\NDP\\v3.0");
const char *g_szNetfx30RegValueName = ("InstallSuccess");
const char *g_szNetfx35RegKeyName = ("Software\\Microsoft\\NET Framework Setup\\NDP\\v3.5");
const char *g_szNetfx40ClientRegKeyName = ("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Client");
const char *g_szNetfx40FullRegKeyName = ("Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full");
const char *g_szNetfx40SPxRegValueName = ("Servicing");
const char *g_szNetfxStandardRegValueName = ("Install");
const char *g_szNetfxStandardSPxRegValueName = ("SP");
const char *g_szNetfxStandardVersionRegValueName = ("Version");

// Version information for final release of .NET Framework 3.0
const int g_iNetfx30VersionMajor = 3;
const int g_iNetfx30VersionMinor = 0;
const int g_iNetfx30VersionBuild = 4506;
const int g_iNetfx30VersionRevision = 26;

// Version information for final release of .NET Framework 3.5
const int g_iNetfx35VersionMajor = 3;
const int g_iNetfx35VersionMinor = 5;
const int g_iNetfx35VersionBuild = 21022;
const int g_iNetfx35VersionRevision = 8;

// Version information for final release of .NET Framework 4
const int g_iNetfx40VersionMajor = 4;
const int g_iNetfx40VersionMinor = 0;
const int g_iNetfx40VersionBuild = 30319;
const int g_iNetfx40VersionRevision = 0;

// Constants for known .NET Framework versions used with the GetRequestedRuntimeInfo API
const char *g_szNetfx10VersionString = ("v1.0.3705");
const char *g_szNetfx11VersionString = ("v1.1.4322");
const char *g_szNetfx20VersionString = ("v2.0.50727");
const char *g_szNetfx40VersionString = ("v4.0.30319");

// Function prototypes
bool CheckNetfxBuildNumber(const char*, const char*, const int, const int, const int, const int);
bool CheckNetfxVersionUsingMscoree(const char*);
DWORD GetProcessorArchitectureFlag();
bool IsCurrentOSTabletMedCenter();
bool IsNetfx10Installed();
bool IsNetfx11Installed();
bool IsNetfx20Installed();
bool IsNetfx30Installed();
bool IsNetfx35Installed();
bool IsNetfx40ClientInstalled();
bool IsNetfx40FullInstalled();
bool RegistryGetValue(HKEY, const char*, const char*, DWORD, LPBYTE, DWORD);

namespace UTIL
{
namespace WIN
{

bool isDotNetVersionInstalled(uint32 major, uint32 minor, bool clientProfile)
{
	if (major == 1 && minor == 0)
	{
		return (IsNetfx10Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx10VersionString));
	}
	else if (major == 1 && minor == 1)
	{
		return (IsNetfx11Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx11VersionString));
	}
	else if (major == 2 && minor == 0)
	{
		return (IsNetfx20Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString));
	}
	else if (major == 3 && minor == 0)
	{
		// The .NET Framework 3.0 is an add-in that installs
		// on top of the .NET Framework 2.0.  For this version
		// check, validate that both 2.0 and 3.0 are installed.
		return (IsNetfx20Installed() && IsNetfx30Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString));
	}
	else if (major == 3 && minor == 5)
	{
		// The .NET Framework 3.5 is an add-in that installs
		// on top of the .NET Framework 2.0 and 3.0.  For this version
		// check, validate that 2.0, 3.0 and 3.5 are installed.
		return (IsNetfx20Installed() && IsNetfx30Installed() && IsNetfx35Installed() && CheckNetfxVersionUsingMscoree(g_szNetfx20VersionString));
	}
	else if (major == 4 && minor == 0 && clientProfile)
	{
		return (IsNetfx40ClientInstalled() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	}
	else if (major == 4 && minor == 0 && !clientProfile)
	{
		return (IsNetfx40FullInstalled() && CheckNetfxVersionUsingMscoree(g_szNetfx40VersionString));
	}

	return false;
}

}
}

typedef HRESULT (STDAPICALLTYPE *GETCORVERSION)(LPWSTR szBuffer, DWORD cchBuffer, DWORD* dwLength);
typedef HRESULT (STDAPICALLTYPE *CORBINDTORUNTIME)(LPCWSTR pwszVersion, LPCWSTR pwszBuildFlavor, REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv);
typedef HRESULT (STDAPICALLTYPE *GETREQUESTEDRUNTIMEINFO)(LPCWSTR pExe, LPCWSTR pwszVersion, LPCWSTR pConfigurationFile, DWORD startupFlags, DWORD runtimeInfoFlags, LPWSTR pDirectory, DWORD dwDirectory, DWORD *dwDirectoryLength, LPWSTR pVersion, DWORD cchBuffer, DWORD* dwlength);

/******************************************************************
Function Name:  CheckNetfxVersionUsingMscoree
Description:    Uses the logic described in the sample code at
                http://msdn2.microsoft.com/library/ydh6b3yb.aspx
                to load mscoree.dll and call its APIs to determine
                whether or not a specific version of the .NET
                Framework is installed on the system
Inputs:         pszNetfxVersionToCheck - version to look for
Results:        true if the requested version is installed
                false otherwise
******************************************************************/
bool CheckNetfxVersionUsingMscoree(const char *pszNetfxVersionToCheck)
{
	bool bFoundRequestedNetfxVersion = false;
	HRESULT hr = S_OK;

	// Check input parameter
	if (NULL == pszNetfxVersionToCheck)
		return false;


	SharedObjectLoader sol;

	if (!sol.load("mscoree.dll"))
		return false;

	
	GETCORVERSION pfnGETCORVERSION = sol.getFunction<GETCORVERSION>("GetCORVersion");


	// Some OSs shipped with a placeholder copy of mscoree.dll. The existence of mscoree.dll
	// therefore does NOT mean that a version of the .NET Framework is installed.
	// If this copy of mscoree.dll does not have an exported function named GetCORVersion
	// then we know it is a placeholder DLL.
	if (!pfnGETCORVERSION)
		return false;

	CORBINDTORUNTIME pfnCORBINDTORUNTIME = sol.getFunction<CORBINDTORUNTIME>("CorBindToRuntime");

	if (!pfnCORBINDTORUNTIME)
		return false;

	GETREQUESTEDRUNTIMEINFO pfnGETREQUESTEDRUNTIMEINFO = sol.getFunction<GETREQUESTEDRUNTIMEINFO>("GetRequestedRuntimeInfo");

	wchar_t szRetrievedVersion[50];
	DWORD dwLength = CountOf(szRetrievedVersion);

	if (!pfnGETREQUESTEDRUNTIMEINFO)
	{
		// Having CorBindToRuntimeHost but not having GetRequestedRuntimeInfo means that
		// this machine contains no higher than .NET Framework 1.0, but the only way to
		// 100% guarantee that the .NET Framework 1.0 is installed is to call a function
		// to exercise its functionality
		if (gcString(pszNetfxVersionToCheck) == g_szNetfx10VersionString)
		{
			hr = pfnGETCORVERSION(szRetrievedVersion, dwLength, &dwLength);

			if (SUCCEEDED(hr))
				return (gcString(szRetrievedVersion) == g_szNetfx10VersionString);

			return false;
		}
	}
	else
	{
		// Set error mode to prevent the .NET Framework from displaying
		// unfriendly error dialogs
		UINT uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);


		gcWString version(pszNetfxVersionToCheck);
		wchar_t szDirectory[MAX_PATH];
		DWORD dwDirectoryLength = 0;
		DWORD dwRuntimeInfoFlags = RUNTIME_INFO_DONT_RETURN_DIRECTORY | GetProcessorArchitectureFlag();

		// Check for the requested .NET Framework version
		hr = pfnGETREQUESTEDRUNTIMEINFO(NULL, version.c_str(), NULL, STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST, NULL, szDirectory, CountOf(szDirectory), &dwDirectoryLength, szRetrievedVersion, CountOf(szRetrievedVersion), &dwLength);

		// Restore the previous error mode
		SetErrorMode(uOldErrorMode);

		if (SUCCEEDED(hr))
			return true;
	}
	
	return false;
}




/******************************************************************
Function Name:  GetProcessorArchitectureFlag
Description:    Determine the processor architecture of the
                system (x86, x64, ia64)
Inputs:         NONE
Results:        DWORD processor architecture flag
******************************************************************/
DWORD GetProcessorArchitectureFlag()
{
	HMODULE hmodKernel32 = NULL;
	typedef void (WINAPI *PFnGetNativeSystemInfo) (LPSYSTEM_INFO);
	PFnGetNativeSystemInfo pfnGetNativeSystemInfo;

	SYSTEM_INFO sSystemInfo;
	memset(&sSystemInfo, 0, sizeof(sSystemInfo));

	bool bRetrievedSystemInfo = false;

	// Attempt to load kernel32.dll
	hmodKernel32 = LoadLibraryA("Kernel32.dll");
	if (NULL != hmodKernel32)
	{
		// If the DLL loaded correctly, get the proc address for GetNativeSystemInfo
		pfnGetNativeSystemInfo = (PFnGetNativeSystemInfo) GetProcAddress(hmodKernel32, "GetNativeSystemInfo");
		if (NULL != pfnGetNativeSystemInfo)
		{
			// Call GetNativeSystemInfo if it exists
			(*pfnGetNativeSystemInfo)(&sSystemInfo);
			bRetrievedSystemInfo = true;
		}
		FreeLibrary(hmodKernel32);
	}

	if (!bRetrievedSystemInfo)
	{
		// Fallback to calling GetSystemInfo if the above failed
		GetSystemInfo(&sSystemInfo);
		bRetrievedSystemInfo = true;
	}

	if (bRetrievedSystemInfo)
	{
		switch (sSystemInfo.wProcessorArchitecture) 
		{
			case PROCESSOR_ARCHITECTURE_INTEL:
				return RUNTIME_INFO_REQUEST_X86;
			case PROCESSOR_ARCHITECTURE_IA64:
				return RUNTIME_INFO_REQUEST_IA64;
			case PROCESSOR_ARCHITECTURE_AMD64:
				return RUNTIME_INFO_REQUEST_AMD64;
			default:
				return 0;
		}
	}

	return 0;
}


/******************************************************************
Function Name:	CheckNetfxBuildNumber
Description:	Retrieves the .NET Framework build number from
                the registry and validates that it is not a pre-release
                version number
Inputs:         NONE
Results:        true if the build number in the registry is greater
				than or equal to the passed in version; false otherwise
******************************************************************/
bool CheckNetfxBuildNumber(const char *pszNetfxRegKeyName, const char *pszNetfxRegKeyValue, const int iRequestedVersionMajor, const int iRequestedVersionMinor, const int iRequestedVersionBuild, const int iRequestedVersionRevision)
{
	char szRegValue[MAX_PATH];
	char *pszToken = NULL;
	char *pszNextToken = NULL;
	int iVersionPartCounter = 0;
	int iRegistryVersionMajor = 0;
	int iRegistryVersionMinor = 0;
	int iRegistryVersionBuild = 0;
	int iRegistryVersionRevision = 0;
	bool bRegistryRetVal = false;

	// Attempt to retrieve the build number registry value
	bRegistryRetVal = RegistryGetValue(HKEY_LOCAL_MACHINE, pszNetfxRegKeyName, pszNetfxRegKeyValue, NULL, (LPBYTE)szRegValue, MAX_PATH);

	if (bRegistryRetVal)
	{
		std::vector<std::string> outList;
		UTIL::STRING::tokenize(szRegValue, outList, ".");

		// This registry value should be of the format
		// #.#.#####.##.  Try to parse the 4 parts of
		// the version here

		if (!outList.empty() && !outList[0].empty())
			iRegistryVersionMajor = atoi(outList[0].c_str());

		if (outList.size() >= 2 && !outList[1].empty())
			iRegistryVersionMinor = atoi(outList[1].c_str());

		if (outList.size() >= 3 && !outList[2].empty())
			iRegistryVersionBuild = atoi(outList[2].c_str());

		if (outList.size() >= 4 && !outList[3].empty())
			iRegistryVersionRevision = atoi(outList[3].c_str());
	}

	// Compare the version number retrieved from the registry with
	// the version number of the final release of the .NET Framework
	// that we are checking
	if (iRegistryVersionMajor > iRequestedVersionMajor)
	{
		return true;
	}
	else if (iRegistryVersionMajor == iRequestedVersionMajor)
	{
		if (iRegistryVersionMinor > iRequestedVersionMinor)
		{
			return true;
		}
		else if (iRegistryVersionMinor == iRequestedVersionMinor)
		{
			if (iRegistryVersionBuild > iRequestedVersionBuild)
			{
				return true;
			}
			else if (iRegistryVersionBuild == iRequestedVersionBuild)
			{
				if (iRegistryVersionRevision >= iRequestedVersionRevision)
				{
					return true;
				}
			}
		}
	}

	// If we get here, the version in the registry must be less than the
	// version of the final release of the .NET Framework we are checking,
	// so return false
	return false;
}


/******************************************************************
Function Name:  IsCurrentOSTabletMedCenter
Description:    Determine if the current OS is a Windows XP
                Tablet PC Edition or Windows XP Media Center
                Edition system
Inputs:         NONE
Results:        true if the OS is Tablet PC or Media Center
                false otherwise
******************************************************************/
bool IsCurrentOSTabletMedCenter()
{
	// Use GetSystemMetrics to detect if we are on a Tablet PC or Media Center OS  
	return ( (GetSystemMetrics(SMABLETPC) != 0) || (GetSystemMetrics(SM_MEDIACENTER) != 0) );
}


/******************************************************************
Function Name:  IsNetfx10Installed
Description:    Uses the detection method recommended at
                http://msdn.microsoft.com/library/ms994349.aspx
                to determine whether the .NET Framework 1.0 is
                installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 1.0 is installed
                false otherwise
******************************************************************/
bool IsNetfx10Installed()
{
	char szRegValue[MAX_PATH];
	return (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx10RegKeyName, g_szNetfx10RegKeyValue, NULL, (LPBYTE)szRegValue, MAX_PATH));
}


/******************************************************************
Function Name:  IsNetfx11Installed
Description:    Uses the detection method recommended at
                http://msdn.microsoft.com/library/ms994339.aspx
                to determine whether the .NET Framework 1.1 is
                installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 1.1 is installed
                false otherwise
******************************************************************/
bool IsNetfx11Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx11RegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx20Installed
Description:	Uses the detection method recommended at
                http://msdn2.microsoft.com/library/aa480243.aspx
                to determine whether the .NET Framework 2.0 is
                installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 2.0 is installed
                false otherwise
******************************************************************/
bool IsNetfx20Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx20RegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	return bRetValue;
}


/******************************************************************
Function Name:	IsNetfx30Installed
Description:	Uses the detection method recommended at
                http://msdn.microsoft.com/library/aa964979.aspx
                to determine whether the .NET Framework 3.0 is
                installed on the machine
Inputs:	        NONE
Results:        true if the .NET Framework 3.0 is installed
                false otherwise
******************************************************************/
bool IsNetfx30Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	// Check that the InstallSuccess registry value exists and equals 1
	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx30RegKeyName, g_szNetfx30RegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 3.0 can
	// have the InstallSuccess value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx30RegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx30VersionMajor, g_iNetfx30VersionMinor, g_iNetfx30VersionBuild, g_iNetfx30VersionRevision));
}


/******************************************************************
Function Name:	IsNetfx35Installed
Description:	Uses the detection method recommended at
                http://msdn.microsoft.com/library/cc160716.aspx
                to determine whether the .NET Framework 3.5 is
                installed on the machine
Inputs:	        NONE
Results:        true if the .NET Framework 3.5 is installed
                false otherwise
******************************************************************/
bool IsNetfx35Installed()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	// Check that the Install registry value exists and equals 1
	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx35RegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 3.5 can
	// have the Install value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx35RegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx35VersionMajor, g_iNetfx35VersionMinor, g_iNetfx35VersionBuild, g_iNetfx35VersionRevision));
}


/******************************************************************
Function Name:	IsNetfx40ClientInstalled
Description:	Uses the detection method recommended at
                http://msdn.microsoft.com/library/ee942965(v=VS.100).aspx
                to determine whether the .NET Framework 4 Client is
                installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4 Client is installed
                false otherwise
******************************************************************/
bool IsNetfx40ClientInstalled()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx40ClientRegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 4 can
	// have the Install value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx40ClientRegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx40VersionMajor, g_iNetfx40VersionMinor, g_iNetfx40VersionBuild, g_iNetfx40VersionRevision));
}


/******************************************************************
Function Name:	IsNetfx40FullInstalled
Description:	Uses the detection method recommended at
                http://msdn.microsoft.com/library/ee942965(v=VS.100).aspx
                to determine whether the .NET Framework 4 Full is
                installed on the machine
Inputs:         NONE
Results:        true if the .NET Framework 4 Full is installed
                false otherwise
******************************************************************/
bool IsNetfx40FullInstalled()
{
	bool bRetValue = false;
	DWORD dwRegValue=0;

	if (RegistryGetValue(HKEY_LOCAL_MACHINE, g_szNetfx40FullRegKeyName, g_szNetfxStandardRegValueName, NULL, (LPBYTE)&dwRegValue, sizeof(DWORD)))
	{
		if (1 == dwRegValue)
			bRetValue = true;
	}

	// A system with a pre-release version of the .NET Framework 4 can
	// have the Install value.  As an added verification, check the
	// version number listed in the registry
	return (bRetValue && CheckNetfxBuildNumber(g_szNetfx40FullRegKeyName, g_szNetfxStandardVersionRegValueName, g_iNetfx40VersionMajor, g_iNetfx40VersionMinor, g_iNetfx40VersionBuild, g_iNetfx40VersionRevision));
}


/******************************************************************
Function Name:  RegistryGetValue
Description:    Get the value of a reg key
Inputs:         HKEY hk - The hk of the key to retrieve
                char *pszKey - Name of the key to retrieve
                char *pszValue - The value that will be retrieved
                DWORD dwType - The type of the value that will be retrieved
                LPBYTE data - A buffer to save the retrieved data
                DWORD dwSize - The size of the data retrieved
Results:        true if successful, false otherwise
******************************************************************/
bool RegistryGetValue(HKEY hk, const char * pszKey, const char * pszValue, DWORD dwType, LPBYTE data, DWORD dwSize)
{
	HKEY hkOpened;

	// Try to open the key
	if (RegOpenKeyExA(hk, pszKey, 0, KEY_READ, &hkOpened) != ERROR_SUCCESS)
	{
		return false;
	}

	// If the key was opened, try to retrieve the value
	if (RegQueryValueExA(hkOpened, pszValue, 0, &dwType, (LPBYTE)data, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hkOpened);
		return false;
	}
	
	// Clean up
	RegCloseKey(hkOpened);

	return true;
}

