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

#include "stdafx.h"
#include "UpdateFunctions.h"
#include <branding/branding.h>

#include <shlobj.h>
#include <sys/stat.h> 

#define UTILWEB_INCLUDE

#include "Common.h"
#include "AppUpdateInstall.h"
#include "IPCPipeClient.h"
#include "IPCManager.h"
#include "IPCUpdateApp.h"

void SetRegValues();
void InstallService();

#ifdef DESURA_OFFICAL_BUILD
int NeedUpdateNonGpl();
#endif


bool FolderExists(const char* strFolderName)
{   
	if (!strFolderName)
		return false;

	return GetFileAttributes(strFolderName) != INVALID_FILE_ATTRIBUTES;   
}

bool FolderExists(const wchar_t* strFolderName)
{   
	if (!strFolderName)
		return false;

	return GetFileAttributesW(strFolderName) != INVALID_FILE_ATTRIBUTES;   
}

bool FileExists(const wchar_t* fileName) 
{
	struct _stat64i32 stFileInfo;
	int intStat;

	// Attempt to get the file attributes
	intStat = _wstat(fileName, &stFileInfo);

	if (intStat == 0) 
		return true;
	else 
		return false;
}

#ifdef DESURA_OFFICAL_BUILD
extern FILE* g_pUpdateLog;
#endif

int IsServiceInstalled()
{
	uint32 sres = UTIL::WIN::queryService(SERVICE_NAME);

	if (sres == SERVICE_STATUS_NOTINSTALLED || sres == SERVICE_STATUS_UNKNOWN)
		return UPDATE_SERVICE;

	char regname[MAX_PATH] = {0};
	char commonFiles[MAX_PATH] = {0};


	gcWString localPath = UTIL::OS::getCurrentDir(L"desura_service.exe");
	gcWString servicePath = UTIL::OS::getCommonProgramFilesPath(L"desura_service.exe");

	Safe::snprintf(regname,	255, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\%s\\ImagePath", SERVICE_NAME);
	gcWString regValue = UTIL::WIN::getRegValue(regname);
	
	bool regMatchesPath = (regValue == servicePath);
	bool regMatchesLocal = (regValue == localPath);

	if (FileExists(servicePath.c_str()) == false)
	{
		if (regMatchesLocal)
			return UPDATE_SERVICE_PATH;
		else
			return UPDATE_SERVICE_LOCATION;
	}
	else if (!regMatchesPath)
	{
		if (FileExists(servicePath.c_str()))
			return UPDATE_SERVICE_PATH;
		else
			return UPDATE_SERVICE_LOCATION;
	}

	std::string serviceHash = UTIL::MISC::hashFile(gcString(servicePath));
	std::string localHash = UTIL::MISC::hashFile(gcString(localPath));

	if (serviceHash != localHash)
		return UPDATE_SERVICE_HASH;

	Safe::snprintf(regname, 255, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s\\Start", SERVICE_NAME);
	int res = UTIL::WIN::getRegValueInt(regname, true);

#ifdef DESURA_OFFICAL_BUILD
	if (g_pUpdateLog)
		fprintf(g_pUpdateLog, "Status: %d\n", res);
#endif

	if (res == 4)
		return UPDATE_SERVICE_DISABLED;

	return UPDATE_NONE;
}

int NeedUpdate()
{
#ifdef DESURA_OFFICAL_BUILD
	int res = NeedUpdateNonGpl();
	
	if (res != UPDATE_NONE)
		return res;
#endif

	return IsServiceInstalled();
}

void SetRegValues()
{
	char mod[255];
	GetModuleFileName(NULL, mod, 255);

	UTIL::FS::Path path = UTIL::FS::PathWithFile(mod);

	std::string command = gcString("\"{0}\\desura.exe\" \"{1}\" -urllink", path.getFullPath(), "%1");
	std::string exePath = path.getFolderPath();

	UTIL::WIN::setRegValue("HKEY_CLASSES_ROOT\\Desura\\", "URL:Desura Protocol");
	UTIL::WIN::setRegValue("HKEY_CLASSES_ROOT\\Desura\\URL Protocol", "");
	UTIL::WIN::setRegValue("HKEY_CLASSES_ROOT\\Desura\\shell\\open\\command\\", command);
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\InstallPath", exePath);
	UTIL::WIN::delRegKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\DesuraNET");

	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\DisplayName", PRODUCT_NAME);
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\UninstallString", gcString("{0}\\Desura_Uninstaller.exe", exePath));
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\InstallLocation", exePath);
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\DisplayIcon", gcString("{0}\\desura.exe", exePath));
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\HelpLink", "http://www.desura.com");
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\URLInfoAbout", "http://www.desura.com/about");
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\NoRepair", "0");
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\NoModify", "1");
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\RegCompany", "Desura");
	UTIL::WIN::setRegValue("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura\\Publisher", "Desura");
}

void InstallService()
{
	unsigned int res = UTIL::WIN::queryService(SERVICE_NAME);

	char servicePath[255];

	char curDir[255];
	GetCurrentDirectory(255, curDir);
	Safe::snprintf(servicePath, 255, "%s\\desura_service.exe", curDir);

	if (res == SERVICE_STATUS_NOTINSTALLED)
	{
		UTIL::WIN::installService(SERVICE_NAME, servicePath, SERVICE_DESC);

		//this is a hack for a crash that happens in xp when you try and change the service access rights.
		//As xp has no uac its not needed.
		unsigned int osid = UTIL::WIN::getOSId();
		if (!(osid == WINDOWS_XP || osid == WINDOWS_XP64))
			UTIL::WIN::changeServiceAccess(SERVICE_NAME);
	}
	else
	{
		//make sure it is stopped
		UTIL::WIN::stopService(SERVICE_NAME);

		char regname[255];
		Safe::snprintf(regname, 255, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s\\ImagePath", SERVICE_NAME);
		
		//update its path to the new location
		UTIL::WIN::setRegValue(regname, servicePath, false, true);
	}
}

class ServiceInstaller : public AppUpdateInstall
{
public:
	ServiceInstaller() : AppUpdateInstall(NULL, false)
	{
	}

	virtual int run()
	{
		try
		{
			startService();
			IPC::PipeClient pc("DesuraIS");

			pc.setUpPipes();
			pc.start();

			IPCUpdateApp* ipua = IPC::CreateIPCClass<IPCUpdateApp>(&pc, "IPCUpdateApp");

			if (!ipua)
				return -1;

			char curDir[255];
			GetCurrentDirectory(255, curDir);

			if (!ipua->updateService(curDir))
				return -3;
		}
		catch (gcException)
		{
			return -2;
		}

		return 0;
	}
};

bool ServiceUpdate(bool validService)
{
	if (validService)
	{
		ServiceInstaller si;

		if (si.run() != 0)
			return false;
	}
	else
	{
		std::wstring appPath = UTIL::OS::getCommonProgramFilesPath();

		if (!FolderExists(appPath.c_str()))
			CreateDirectoryW(appPath.c_str(), NULL);

		std::wstring newService = UTIL::OS::getCommonProgramFilesPath(L"desura_service.exe");
		std::wstring curService = UTIL::OS::getCurrentDir(L"desura_service.exe");

		char regname[255];
		Safe::snprintf(regname, 255, "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s\\ImagePath", SERVICE_NAME);

		CopyFileW(curService.c_str(), newService.c_str(), FALSE);
		UTIL::WIN::setRegValue(regname, gcString(newService).c_str());
	}

	return true;
}

class DataMover : public AppUpdateInstall
{
public:
	DataMover() : AppUpdateInstall(NULL, false)
	{
	}

	virtual int run()
	{
		try
		{
			startService();
			IPC::PipeClient pc("DesuraIS");

			pc.setUpPipes();
			pc.start();

			IPCUpdateApp* ipua = IPC::CreateIPCClass<IPCUpdateApp>(&pc, "IPCUpdateApp");

			if (!ipua)
				return -1;

			if (!ipua->fixDataDir())
				return -3;
		}
		catch (gcException)
		{
			return -2;
		}

		return 0;
	}
};

bool MoveDataFolder()
{
	DataMover dm;

	if (dm.run() != 0)
		return false;

	return true;
}

bool FixServiceDisabled()
{
	try
	{
		UTIL::WIN::enableService(SERVICE_NAME);
	}
	catch (gcException)
	{
		MessageBox(NULL, "The service desura needs to use to install content is disabled. \n\nPlease enable it via msconfig (\"" PRODUCT_NAME " Install Service\").", PRODUCT_NAME ": Failed to enable service", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}

#ifndef DESURA_OFFICAL_BUILD
void McfUpdate() {}
void FullUpdate() {}
#endif
