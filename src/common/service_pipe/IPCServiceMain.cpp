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
#include "IPCServiceMain.h"
#include "IPCManager.h"

#include "IPCUninstallBranch.h"
#include "IPCInstallMcf.h"
#include "IPCUninstallMcf.h"
#include "IPCComplexLaunch.h"
#include "IPCUpdateApp.h"
#include <branding/servicecore_version.h>

#ifndef DESURA_CLIENT
#include "InstallScriptRunTime.h"
#else
#include "Color.h"
#endif

#include "SharedObjectLoader.h"

#ifdef WIN32
typedef BOOL (WINAPI* SetDllDirectoryFunc)(LPCTSTR);
#endif

IPCServiceMain* servicemain = NULL;

#ifndef DESURA_CLIENT

#include "ServiceMainThread.h"

bool g_bLogEnabled = true;

void StopLogging()
{
	g_bLogEnabled = false;
}

gcString GetSpecialPath(int32 key)
{
	if (servicemain)
	{
		IPC::PBlob str = servicemain->getSpecialPath(key);
		return std::string(str.getData(), str.getSize());
	}

	return "SERVICE CORE IS NULL";
}

void PrintfMsg(const char* format, ...)
{
	if (!servicemain || !g_bLogEnabled)
		return;

	va_list args;
	va_start(args, format);

	gcString str;
	str.vformat(format, args);
	servicemain->message(str.c_str());

	va_end(args);
}

void LogMsg(int type, std::string msg, Color* col)
{
	if (!servicemain || !g_bLogEnabled)
		return;

	switch (type)
	{
	case MT_MSG:
	case MT_MSG_COL:
		servicemain->message(msg.c_str());
		break;

	case MT_WARN:
		servicemain->warning(msg.c_str());
		break;
	};
}

void LogMsg(int type, std::wstring msg, Color* col)
{
	if (!servicemain || !g_bLogEnabled)
		return;

	switch (type)
	{
	case MT_MSG:
	case MT_MSG_COL:
		servicemain->message(gcString(msg).c_str());
		break;

	case MT_WARN:
		servicemain->warning(gcString(msg).c_str());
		break;
	};
}





#endif


REG_IPC_CLASS( IPCServiceMain );


IPCServiceMain::IPCServiceMain(IPC::IPCManager* mang, uint32 id, DesuraId itemId) : IPC::IPCClass(mang, id, itemId)
{
	registerFunctions();
	servicemain = this;

#ifndef DESURA_CLIENT
	m_pServiceThread = NULL;
#endif
}

IPCServiceMain::~IPCServiceMain()
{
#ifndef DESURA_CLIENT
	if (m_pServiceThread)
		m_pServiceThread->stop();

	safe_delete(m_pServiceThread);
#endif
}

void IPCServiceMain::registerFunctions()
{
#ifndef DESURA_CLIENT
	REG_FUNCTION_VOID( IPCServiceMain, updateRegKey );
	REG_FUNCTION_VOID( IPCServiceMain, updateBinaryRegKeyBlob);

	REG_FUNCTION_VOID( IPCServiceMain, dispVersion );
	
	REG_FUNCTION_VOID( IPCServiceMain, setAppDataPath );
	REG_FUNCTION_VOID( IPCServiceMain, setCrashSettings );

	REG_FUNCTION_VOID( IPCServiceMain, removeUninstallRegKey);
	REG_FUNCTION_VOID( IPCServiceMain, setUninstallRegKey);
	REG_FUNCTION_VOID( IPCServiceMain, updateAllUninstallRegKey);

	REG_FUNCTION_VOID( IPCServiceMain, addDesuraToGameExplorer);
	REG_FUNCTION_VOID( IPCServiceMain, addItemGameToGameExplorer);
	REG_FUNCTION_VOID( IPCServiceMain, removeGameFromGameExplorer);

	REG_FUNCTION_VOID( IPCServiceMain, updateShortCuts);
	REG_FUNCTION_VOID( IPCServiceMain, fixFolderPermissions);

	REG_FUNCTION_VOID( IPCServiceMain, runInstallScript);

#else

	REG_FUNCTION_VOID( IPCServiceMain, warning );
	REG_FUNCTION_VOID( IPCServiceMain, message );
	REG_FUNCTION_VOID( IPCServiceMain, debug );

	REG_FUNCTION( IPCServiceMain, getSpecialPath );

#endif
}



#ifdef DESURA_CLIENT

#ifdef WIN32
#include <shlobj.h>
#endif

#ifdef NIX
#define MAX_PATH 255
#endif

IPC::PBlob IPCServiceMain::getSpecialPath(int32 key)
{
	char path[MAX_PATH] = "NOT SUPPORTED";

#ifdef WIN32
	switch (key)
	{
	case 1:
		SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
	case 2:
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
	};

#endif

	return IPC::PBlob(path, strlen(path));
}

void IPCServiceMain::warning(const char* msg)
{
	Color c(0xFF, 0x99, 0x33);
	MsgCol(&c, gcString("DS: {0}", msg));
}

void IPCServiceMain::message(const char* msg)
{
	Color c(0x00, 0x33, 0x00);
	MsgCol(&c, gcString("DS: {0}", msg));
}

void IPCServiceMain::debug(const char* msg)
{
	Color c(0x00, 0x33, 0x66);
	MsgCol(&c, gcString("DS: {0}", msg));
}	

void IPCServiceMain::updateRegKey(const char* key, const char* value)
{
	IPC::functionCallV(this, "updateRegKey", key, value);
}

void IPCServiceMain::updateBinaryRegKey(const char* key, const char* value, size_t size)
{
	IPC::PBlob blob(value, size);
	IPC::functionCallV(this, "updateBinaryRegKeyBlob", key, blob);
}

IPCUpdateApp* IPCServiceMain::newUpdateApp()
{
	return IPC::CreateIPCClass< IPCUpdateApp >(m_pManager, "IPCUpdateApp");
}

IPCUninstallMcf* IPCServiceMain::newUninstallMcf()
{
	return IPC::CreateIPCClass< IPCUninstallMcf >(m_pManager, "IPCUninstallMcf");
}

IPCInstallMcf* IPCServiceMain::newInstallMcf()
{
	return IPC::CreateIPCClass< IPCInstallMcf >(m_pManager, "IPCInstallMcf");
}

IPCComplexLaunch* IPCServiceMain::newComplexLaunch()
{
	return IPC::CreateIPCClass< IPCComplexLaunch >(m_pManager, "IPCComplexLaunch");
}

IPCUninstallBranch* IPCServiceMain::newUninstallBranch()
{
	return IPC::CreateIPCClass< IPCUninstallBranch >(m_pManager, "IPCUninstallBranch");
}

void IPCServiceMain::dispVersion()
{
	IPC::functionCallAsync(this, "dispVersion");
}

void IPCServiceMain::setAppDataPath(const char* path)
{
	IPC::functionCallAsync(this, "setAppDataPath", path);
}

void IPCServiceMain::setCrashSettings(const char* user, bool upload)
{
	IPC::functionCallAsync(this, "setCrashSettings", user, upload);
}

void IPCServiceMain::removeUninstallRegKey(uint64 id)
{
	IPC::functionCallAsync(this, "removeUninstallRegKey", id);
}

void IPCServiceMain::setUninstallRegKey(uint64 id, uint64 installSize)
{
	IPC::functionCallAsync(this, "setUninstallRegKey", id, installSize);
}

void IPCServiceMain::updateAllUninstallRegKey()
{
	IPC::functionCallAsync(this, "updateAllUninstallRegKey");
}

void IPCServiceMain::addDesuraToGameExplorer()
{
	IPC::functionCallAsync(this, "addDesuraToGameExplorer");
}

void IPCServiceMain::addItemGameToGameExplorer(const char* name, const char* dllPath)
{
	IPC::functionCallAsync(this, "addItemGameToGameExplorer", name, dllPath);
}

void IPCServiceMain::removeGameFromGameExplorer(const char* dllPath, bool deleteDll)
{
	IPC::functionCallAsync(this, "removeGameFromGameExplorer", dllPath, deleteDll);
}


void IPCServiceMain::updateShortCuts()
{
	IPC::functionCallAsync(this, "updateShortCuts");
}

void IPCServiceMain::fixFolderPermissions(const char* dir)
{
	IPC::functionCallV(this, "fixFolderPermissions", dir);
}

void IPCServiceMain::runInstallScript(const char* file, const char* installpath, const char* function)
{
	IPC::functionCallAsync(this, "runInstallScript", file, installpath, function);
}

#else

IPC::PBlob IPCServiceMain::getSpecialPath(int32 key)
{
	return IPC::functionCall<IPC::PBlob, int32>(this, "getSpecialPath", key);
}

void IPCServiceMain::warning(const char* msg)
{
	IPC::functionCallAsync(this, "warning", msg);
}

void IPCServiceMain::message(const char* msg)
{
	IPC::functionCallAsync(this, "message", msg);
}

void IPCServiceMain::debug(const char* msg)
{
	IPC::functionCallAsync(this, "debug", msg);
}





void IPCServiceMain::startThread()
{
	if (!m_pServiceThread)
	{
		m_pServiceThread = new ServiceMainThread();
		m_pServiceThread->start();
	}
}

void SetCrashSettings(const wchar_t* user, bool upload);
void SetAppDataPath(const char* path);

void IPCServiceMain::setAppDataPath(const char* path)
{
	SetAppDataPath(path);
}

void IPCServiceMain::setCrashSettings(const char* user, bool upload)
{
	gcWString wUser(user);
	SetCrashSettings(wUser.c_str(), upload);
}

void IPCServiceMain::dispVersion()
{
	Msg(gcString("Version: {0}.{1}.{2}.{3}\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND));
}

void IPCServiceMain::fixFolderPermissions(const char* dir)
{
	try
	{
		UTIL::FS::recMakeFolder(dir);
#ifdef WIN32
		UTIL::WIN::changeFolderPermissions(gcWString(dir));
#endif // LINUX TODO
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to fix folder permissions: {0}\n", e));
	}
}


void RemoveUninstallRegKey(DesuraId id);
bool SetUninstallRegKey(DesuraId id, uint64 installSize);
void UpdateAllUninstallRegKey();


#ifdef WIN32
void AddDesuraToWIndowsGameExplorer();
void AddGameToWindowsGameExplorer(const char* name, const char* dllPath);
void RemoveGameFromWindowsGameExplorer(const char* dllPath, bool deleteDll);
#else
void AddDesuraToWIndowsGameExplorer(){}
void AddGameToWindowsGameExplorer(const char* name, const char* dllPath){}
void RemoveGameFromWindowsGameExplorer(const char* dllPath, bool deleteDll){}
#endif

void UpdateShortCuts();

class RemoveUninstallTask : public TaskI
{
public:
	RemoveUninstallTask(uint64 id)
	{
		this->id = id;
	}

	void doTask()
	{
		RemoveUninstallRegKey(DesuraId(id));
	}

	void destroy()
	{
		delete this;
	}

	uint64 id;
};

class SetUninstallTask : public TaskI
{
public:
	SetUninstallTask(uint64 id, uint64 installSize)
	{
		this->id = id;
		this->installSize = installSize;
	}

	void doTask()
	{
		SetUninstallRegKey(DesuraId(id), installSize);
	}

	void destroy()
	{
		delete this;
	}

	uint64 id;
	uint64 installSize;
};

class UpdateUninstallTask : public TaskI
{
public:
	void doTask()
	{
		UpdateAllUninstallRegKey();
	}

	void destroy()
	{
		delete this;
	}
};

class AddDesuraToWGETask : public TaskI
{
public:
	void doTask()
	{
		AddDesuraToWIndowsGameExplorer();
	}

	void destroy()
	{
		delete this;
	}
};

class UpdateShortcutTask : public TaskI
{
public:
	void doTask()
	{
		UpdateShortCuts();
	}

	void destroy()
	{
		delete this;
	}
};




class AddToWGETask : public TaskI
{
public:
	AddToWGETask(const char* name, const char* dllPath)
	{
		m_szDllPath = dllPath;
		m_szName = name;
	}

	void doTask()
	{
		AddGameToWindowsGameExplorer(m_szName.c_str(), m_szDllPath.c_str());
	}

	void destroy()
	{
		delete this;
	}

	gcString m_szDllPath;
	gcString m_szName;
};


class RemoveFromWGETask : public TaskI
{
public:
	RemoveFromWGETask(const char* dllPath, bool deleteDll)
	{
		m_szDllPath = dllPath;
		m_bDelete = deleteDll;
	}

	void doTask()
	{
		RemoveGameFromWindowsGameExplorer(m_szDllPath.c_str(), m_bDelete);
	}

	void destroy()
	{
		delete this;
	}

	gcString m_szDllPath;
	bool m_bDelete;
};






void IPCServiceMain::updateRegKey(const char* key, const char* value)
{
	if (!key || !value)
		return;

	try
	{
		UTIL::OS::setConfigValue(key, value);
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to set reg key: {0}\n", e));
	}
}

void IPCServiceMain::updateBinaryRegKeyBlob(const char* key, IPC::PBlob blob)
{
	if (!key || !blob.getSize())
		return;

	try
	{
		UTIL::OS::setConfigBinaryValue(key, blob.getData(), blob.getSize(), false);
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to set reg key: {0}\n", e));
	}
}


void IPCServiceMain::removeUninstallRegKey(uint64 id)
{
	startThread();
	m_pServiceThread->addTask(new RemoveUninstallTask(id));
}

void IPCServiceMain::setUninstallRegKey(uint64 id, uint64 installSize)
{
	startThread();
	m_pServiceThread->addTask(new SetUninstallTask(id, installSize));	
}

void IPCServiceMain::updateAllUninstallRegKey()
{
	startThread();
	m_pServiceThread->addTask(new UpdateUninstallTask());	
}

void IPCServiceMain::addDesuraToGameExplorer()
{
	startThread();
	m_pServiceThread->addTask(new AddDesuraToWGETask());	
}

void IPCServiceMain::addItemGameToGameExplorer(const char* name, const char* dllPath)
{
	startThread();
	m_pServiceThread->addTask(new AddToWGETask(name, dllPath));		
}

void IPCServiceMain::removeGameFromGameExplorer(const char* dllPath, bool deleteDll)
{
	startThread();
	m_pServiceThread->addTask(new RemoveFromWGETask(dllPath, deleteDll));		
}

void IPCServiceMain::updateShortCuts()
{
	startThread();
	m_pServiceThread->addTask(new UpdateShortcutTask());	
}

void IPCServiceMain::runInstallScript(const char* file, const char* installpath, const char* function)
{
	try
	{
		InstallScriptRunTime isr(file, installpath);
		isr.run(function);
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to execute script {0}: {1}\n", file, e));
	}
}


#endif
