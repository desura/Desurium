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

#ifndef DESURA_IPCMAIN_H
#define DESURA_IPCMAIN_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCClass.h"
#include "IPCParameter.h"

class IPCUpdateApp;
class IPCUninstallMcf;
class IPCInstallMcf;
class IPCComplexLaunch;
class IPCUninstallBranch;
class ServiceMainThread;

class IPCServiceMain : public IPC::IPCClass
{
public:
	IPCServiceMain(IPC::IPCManager* mang, uint32 id, DesuraId itemId);
	~IPCServiceMain();

	void warning(const char* msg);
	void message(const char* msg);
	void debug(const char* msg);

	void updateRegKey(const char* key, const char* value);

#ifdef DESURA_CLIENT
	void updateBinaryRegKey(const char* key, const char* value, size_t size);
#else
	void updateBinaryRegKeyBlob(const char* key, IPC::PBlob blob);
#endif

	void removeUninstallRegKey(uint64 id);
	void setUninstallRegKey(uint64 id, uint64 installSize);
	void updateAllUninstallRegKey();

	void addDesuraToGameExplorer();
	void addItemGameToGameExplorer(const char* name, const char* dllPath);
	void removeGameFromGameExplorer(const char* dllPath, bool deleteDll = true);

	void updateShortCuts();

	void runInstallScript(const char* file, const char* installPath, const char* function);
	IPC::PBlob getSpecialPath(int32 key);

#ifdef DESURA_CLIENT
	IPCUpdateApp* newUpdateApp();
	IPCUninstallMcf* newUninstallMcf();
	IPCInstallMcf* newInstallMcf();
	IPCComplexLaunch* newComplexLaunch();
	IPCUninstallBranch* newUninstallBranch();
#endif

	void setAppDataPath(const char* path);
	void setCrashSettings(const char* user, bool upload);
	void dispVersion();

	void fixFolderPermissions(const char* dir);

private:
	void registerFunctions();

#ifndef DESURA_CLIENT
	void startThread();
	ServiceMainThread* m_pServiceThread;
#endif
};

#endif //DESURA_IPCMAIN_H
