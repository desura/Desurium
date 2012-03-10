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
#include <branding/branding.h>
#include "XMLMacros.h"
#include "SharedObjectLoader.h"


enum GAME_INSTALL_SCOPE
{
	GIS_NOT_INSTALLED	= 1,
	GIS_CURRENT_USER	= 2,
	GIS_ALL_USERS		= 3,
};

typedef HRESULT (__stdcall *GameExplorerInstallWFn)(WCHAR* strGDFBinPath, WCHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope);
typedef HRESULT (__stdcall * GameExplorerUninstallWFn)(WCHAR* strGDFBinPath);

void ValidateGamesInGameExplorer();

void AddGameToWindowsGameExplorer(const char* name, const char* dllPath)
{
	if (!dllPath || !UTIL::FS::isValidFile(dllPath))
	{
		Warning(gcString("Not a valid dll for {0} to install into game explorer! [{1}]\n", name, dllPath));
		return;
	}

	SharedObjectLoader sol;

	if (!sol.load("GameuxInstallHelper.dll"))
	{
		Warning(gcString("Failed to open GameuxInstallHelper.dll: {0}\n", GetLastError()));
		return;
	}

	GameExplorerInstallWFn GameExplorerInstall = sol.getFunction<GameExplorerInstallWFn>("GameExplorerInstallW");
	GameExplorerUninstallWFn GameExplorerUninstall = sol.getFunction<GameExplorerUninstallWFn>("GameExplorerUninstallW");

	if (!GameExplorerInstall)
	{
		Warning("Failed to find install function in GameuxInstallHelper.dll\n");
		return;
	}

	if (!GameExplorerUninstall)
	{
		Warning("Failed to find uninstall function in GameuxInstallHelper.dll\n");
		return;
	}


	UTIL::FS::Path path = UTIL::FS::PathWithFile(dllPath);

	std::string fullPath = path.getFullPath();
	std::string folderPath = path.getFolderPath();

	gcWString wDllPath(dllPath);
	gcWString wFolderPath(folderPath);
	gcWString wFullPath(fullPath);

	GameExplorerUninstall(const_cast<wchar_t*>(wDllPath.c_str()));
	HRESULT res = GameExplorerInstall(const_cast<wchar_t*>(wFullPath.c_str()), const_cast<wchar_t*>(wFolderPath.c_str()), GIS_ALL_USERS);

	if (res != 0)
		Warning(gcString("Failed to install {0} for game explorer: {1} [{2}]\n", name, (uint32)res, dllPath));
	else
		Msg(gcString("Installed {0} for game explorer\n", name));
}

void AddDesuraToWIndowsGameExplorer()
{
	ValidateGamesInGameExplorer();

	std::string path = UTIL::WIN::getRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\InstallPath") + "\\bin\\Desura_GDF.dll";
	AddGameToWindowsGameExplorer(PRODUCT_NAME, path.c_str());
}

void RemoveGameFromWindowsGameExplorer(const char* dllPath, bool deleteDl)
{
	if (!dllPath)
		return;

	if (!UTIL::FS::isValidFile(dllPath))
		return;

	SharedObjectLoader sol;

	if (!sol.load("GameuxInstallHelper.dll"))
	{
		Warning(gcString("Failed to open GameuxInstallHelper.dll: {0}\n", GetLastError()));
		return;
	}

	GameExplorerUninstallWFn GameExplorerUninstall = sol.getFunction<GameExplorerUninstallWFn>("GameExplorerUninstallW");

	if (!GameExplorerUninstall)
	{
		Warning("Failed to find uninstall function in GameuxInstallHelper.dll\n");
		return;
	}

	gcWString wDllPath(dllPath);
	GameExplorerUninstall(const_cast<wchar_t*>(wDllPath.c_str()));

	if (deleteDl)
		UTIL::FS::delFile(dllPath);
}

void ValidateGamesInGameExplorer()
{
	std::vector<std::string> regKeys;
	UTIL::WIN::getAllRegKeys("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameUX\\Games", regKeys, true);

	for (size_t x=0; x<regKeys.size(); x++)
	{
		gcString regKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameUX\\Games\\{0}\\AppExePath", regKeys[x]);
		std::string value = UTIL::WIN::getRegValue(regKey, true);

		if (value.find("desura://") != 0)
			continue;

		regKey = gcString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameUX\\Games\\{0}\\ConfigGDFBinaryPath", regKeys[x]);
		value = UTIL::WIN::getRegValue(regKey, true);

		if (!UTIL::FS::isValidFile(value))
		{
			regKey = gcString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\GameUX\\Games\\{0}", regKeys[x]);
			UTIL::WIN::delRegTree(regKey.c_str(), true);
		}
	}
}
