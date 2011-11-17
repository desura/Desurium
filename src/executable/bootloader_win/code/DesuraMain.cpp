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

#include "windows.h"
#include "resource.h"
#include "UtilBootloader.h"

#include "UpdateMFCForm.h"
#include "UpdateFunctions.h"
#include "MiniDumpGenerator.h"

#include "../../branding/desura_exe_version.h"
#include "util/gcDDE.h"
#include "UICoreI.h"

#include "SharedObjectLoader.h"
#include "AppUpdateInstall.h"

#include <map>
#include <string>
#include <shlobj.h>

#define STR( t ) #t

bool g_bRestart = false;
char* g_szArgs = NULL;


extern void InstallService();
extern void SetRegValues();
extern bool checkCert();
extern void cleanUpIPC();

INT_PTR DisplayUpdateWindow(int updateType);

void checkForBadUninstaller()
{
	char exePath[255];
	GetModuleFileName(NULL, exePath, 255);

	size_t exePathLen = strlen(exePath);
	for (size_t x=exePathLen; x>0; x--)
	{
		char c = exePath[x];

		exePath[x] = '\0';
		exePathLen = x;

		if (c == '\\')
			break;
	}

	char * folder = exePath;

	for (size_t x=exePathLen; x>0; x--)
	{
		if (exePath[x] == '\\')
		{
			folder = &exePath[x+1];
			break;
		}
	}	

	if (strcmp(folder, "Desura") != 0)
		DeleteFile("Desura_Uninstall.exe");
}

void UiCoreRestart(const char* args)
{
	g_bRestart = true;

	if (args)
	{
		delete [] g_szArgs;
		size_t len = strlen(args);
		g_szArgs = new char[len+1];
		strcpy_s(g_szArgs, len+1, args);
		g_szArgs[len] = '\0';
	}
}

FILE* g_pUpdateLog = NULL;

void InitUpdateLog()
{
	if (g_pUpdateLog)
		return;

	char path[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

	char file[255];

	_snprintf_s(file, 255, "%s\\Desura", path);
	CreateDirectory(file, NULL);

	_snprintf_s(file, 255, "%s\\Desura\\DesuraApp", path);
	CreateDirectory(file, NULL);

	_snprintf_s(file, 255, "%s\\Desura\\DesuraApp\\update_log.txt", path);
	fopen_s(&g_pUpdateLog, file, "a");
}

void StopUpdateLog()
{
	if (!g_pUpdateLog)
		return;

	fclose(g_pUpdateLog);
}

void Log(const char* format, ...)
{
	if (!g_pUpdateLog)
		return;

	time_t rawtime;
	struct tm timeinfo;
	char buffer[255];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, 255, "%c:", &timeinfo);
	fprintf(g_pUpdateLog, buffer);

	va_list args;
	va_start(args, format);
	vfprintf(g_pUpdateLog, format, args);
	va_end(args);
}


const char* g_UpdateReasons[] =
{
	"None",
	"Mcf Update",
	"Files",
	"Xml",
	"Service",
	"Cert",
	"Forced",
	"Service Path needs updating",
	"Service needs change of location",
	"User data path is incorrect",
	"Service Hash is bad",
	NULL
};

class BootLoader : public CWinApp
{
public:
	BootLoader();
	~BootLoader();

	BOOL InitInstance();
	int ExitInstance();
	BOOL PreTranslateMessage(MSG *msg);
	BOOL OnIdle(LONG lCount);

	MiniDumpGenerator m_MDumpHandle;

protected:
	bool sendArgs();
	void loadUICore();

	void restartAsAdmin(int needupdate);
	void preReadImages();

private:
	SharedObjectLoader m_hUICore;
	UICoreI* m_pUICore;

	bool m_bRetCode;
	int m_iRetCode;
	bool hasAdminRights;
};



BootLoader theApp;
bool g_bLockDump = false;

void SetDumpArgs(const wchar_t* user, bool upload)
{
	theApp.m_MDumpHandle.setUser(user);
	theApp.m_MDumpHandle.setUpload(upload);
}

void SetDumpLevel(unsigned char level)
{
	if (!g_bLockDump)
		theApp.m_MDumpHandle.setDumpLevel(level);
}

BootLoader::BootLoader()
{
	m_MDumpHandle.showMessageBox(true);

	AfxEnableMemoryTracking(FALSE);
	InitCommonControls();

#ifdef DEBUG
#if 0
	BootLoaderUtil::WaitForDebugger();
#endif
#endif

	hasAdminRights = false;
	m_pUICore = NULL;

	InitUpdateLog();
	m_bRetCode = false;
}

BootLoader::~BootLoader()
{
	StopUpdateLog();

	cleanUpIPC();

	if (g_szArgs)
		delete [] g_szArgs;
}


UINT DownloadFilesForTest();
UINT InstallFilesForTest();

BOOL BootLoader::InitInstance()
{
	BootLoaderUtil::CMDArgs args(m_lpCmdLine);
	
	if (args.hasArg("waitfordebugger"))
	{
		BootLoaderUtil::WaitForDebugger();
	}

	BootLoaderUtil::SetCurrentDir();
	CWinApp::InitInstance();

	checkForBadUninstaller();

	if (args.hasArg("urllink"))
	{
		std::string a(m_lpCmdLine);

		size_t pos = a.find("-urllink");
		a.replace(pos, 8, "");

		BootLoaderUtil::Restart(a.c_str(), false);
		return FALSE;
	}

	if (args.hasArg("testinstall"))
	{
		m_bRetCode = true;
		m_iRetCode = InstallFilesForTest();
		return FALSE;
	}

	if (args.hasArg("testdownload"))
	{
		m_bRetCode = true;
		m_iRetCode = DownloadFilesForTest();
		return FALSE;
	}

	if (args.hasArg("dumplevel"))
	{
		SetDumpLevel(args.getInt("dumplevel"));
		g_bLockDump = true;
	}

	if (args.hasArg("autostart"))
	{
		//need to wait for service to start
		Sleep(15*1000);
		BootLoaderUtil::RestartAsNormal("-wait");
		return FALSE;
	}

#ifdef DEBUG
	if (args.hasArg("debugupdater"))
	{
		INT_PTR nResponse = DisplayUpdateWindow(-1);
		return FALSE;
	}

	if (args.hasArg("debuginstall"))
	{
		McfUpdate();
		return FALSE;
	}

	if (args.hasArg("debugdownload"))
	{
		FullUpdate();
		return FALSE;
	}

	if (args.hasArg("debugcheck"))
	{
		CheckInstall();
		return FALSE;
	}
#endif

	if (args.hasArg("testcrash"))
	{
		AppUpdateInstall *ai = NULL;
		ai->run();
	}

	unsigned int osid = BootLoaderUtil::GetOSId();

	if (osid == WINDOWS_PRE2000)
	{
		::MessageBox(NULL, "Desura needs Windows xp or better to run.", "Desura Error: Old Windows", MB_OK);
		return FALSE;
	}
	else if (osid == WINDOWS_XP || osid == WINDOWS_XP64)
	{
		hasAdminRights = true;
	}

	if (args.hasArg("admin"))
	{
		hasAdminRights = true;
	}

	//if the wait command is parsed in then we need to wait for all other instances of desura to exit.
	if (args.hasArg("wait"))
	{
		BootLoaderUtil::WaitForOtherInstance(m_hInstance);
	}
	else
	{
		if (BootLoaderUtil::CheckForOtherInstances(m_hInstance))
		{
			sendArgs();
			return FALSE;
		}
		else
		{
			//if windows uninstall software launches desura it will disable its window till it quits.
			//Work around for existing clients
			std::string a(m_lpCmdLine);
			size_t pos = a.find("desura://uninstall/");

			if (pos != std::string::npos)
			{
				a.replace(pos+9, 9, "remove");
				BootLoaderUtil::RestartAsNormal(a.c_str());
				return FALSE;
			}
		}
	}

	if (args.hasArg("forceupdate"))
	{
		if (!hasAdminRights)
		{
			Log("Force updating, restarting as admin.\n");
			restartAsAdmin(UPDATE_FORCED);
			return FALSE;
		}
		else
		{
			Log("Force updating.\n");
			FullUpdate();
			BootLoaderUtil::RestartAsNormal("-wait");
			return FALSE;
		}
	}
	
	

#ifdef _DEBUG
	SetRegValues();
	InstallService();
#else
	int nu = NeedUpdate();

	if (nu != UPDATE_NONE)
	{
		if (nu == UPDATE_MCF)
		{
			Log("Updating from MCF.\n");
			McfUpdate();
			BootLoaderUtil::RestartAsNormal("-wait");
			return FALSE;
		}
		else if (nu == UPDATE_SERVICE_PATH)
		{
			Log("Service update path [%s].\n", g_UpdateReasons[nu]);

			if (ServiceUpdate(true))
				nu = UPDATE_NONE;
		}
	}

	if (nu != UPDATE_NONE)
	{
		if (!hasAdminRights)
		{
			restartAsAdmin(nu);
			return FALSE;
		}
		else if (nu == UPDATE_SERVICE_LOCATION || nu == UPDATE_SERVICE_HASH)
		{
			Log("Service update location [%s].\n", g_UpdateReasons[nu]);
			ServiceUpdate(false);
		}
		else if (nu == UPDATE_SERVICE_DISABLED)
		{
			if (FixServiceDisabled())
				BootLoaderUtil::RestartAsNormal("-wait");

			return FALSE;
		}
		else
		{
			Log("Full update [%s].\n", g_UpdateReasons[nu]);
			FullUpdate();
			BootLoaderUtil::RestartAsNormal("-wait");
			return FALSE;
		}
	}

	if (hasAdminRights && !(osid == WINDOWS_XP || osid == WINDOWS_XP64))
	{
		BootLoaderUtil::RestartAsNormal("-wait");
		return FALSE;
	}
#endif

	loadUICore();

	if (!m_pUICore)
		return FALSE;

	bool res = m_pUICore->initWxWidgets(m_hInstance, m_nCmdShow, args.getArgc(), const_cast<char**>(args.getArgv()));

	if (res)
		m_pMainWnd = new BootLoaderUtil::CDummyWindow(m_pUICore->getHWND());

	return res?TRUE:FALSE;
}

void BootLoader::restartAsAdmin(int needupdate)
{
	const char* args = "-wait -admin";

	if (needupdate == UPDATE_FORCED)
		args = "-wait -admin -forceupdate";

	if (!BootLoaderUtil::RestartAsAdmin(args))
	{
		char msg[255];
		_snprintf_s(msg, 255, _TRUNCATE, "Failed to restart Desura with admin rights [Update: %d].", needupdate);
		::MessageBox(NULL, msg, "Desura Critical Error", MB_OK);
	}
}

bool BootLoader::sendArgs()
{
	gcDDEClient* client = new gcDDEClient();
	gcDDEConnection *con = client->makeConnection("Desura", "link");

	bool res = false;

	if (con)
	{
		res = con->poke("desura", m_lpCmdLine);
		delete con;
	}

	if (client)
		delete client;

	return res;
}

int BootLoader::ExitInstance()
{
	delete m_pMainWnd;

	int ret = CWinApp::ExitInstance();

	if (m_pUICore)
		m_pUICore->exitApp(&ret);

	if (g_bRestart)
	{
		if (g_szArgs && strstr(g_szArgs, "-setcachedir"))
#ifdef DEBUG
			BootLoaderUtil::StartProcess("utility-d.exe", g_szArgs);
#else
			BootLoaderUtil::StartProcess("utility.exe", g_szArgs);
#endif
		else
			BootLoaderUtil::Restart(g_szArgs);
	}

	if (m_bRetCode)
		return m_iRetCode;

	return ret;
}

// Override this to provide wxWidgets message loop compatibility
BOOL BootLoader::PreTranslateMessage(MSG *msg)
{
	if (m_pUICore && m_pUICore->preTranslateMessage(msg) )
		return TRUE;

	return CWinApp::PreTranslateMessage(msg);
}

BOOL BootLoader::OnIdle(LONG lCount)
{
	if (m_pUICore)
		return m_pUICore->onIdle();

	return FALSE;
}

void BootLoader::preReadImages()
{
#ifdef DEBUG
	BootLoaderUtil::PreReadImage(".\\bin\\uicore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\webcore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\usercore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\mcfcore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\wxmsw290ud_vc_desura.dll");
#else
	BootLoaderUtil::PreReadImage(".\\bin\\uicore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\webcore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\usercore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\mcfcore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\wxmsw290u_vc_desura.dll");	
#endif
}

void BootLoader::loadUICore()
{
	if (!BootLoaderUtil::SetDllDir(".\\bin"))
	{
		::MessageBox(NULL, "Failed to set the dll path to the bin folder.", "Desura: ERROR!",  MB_OK);
		exit(-100);			
	}

	preReadImages();

#ifdef DEBUG
	const char* dllname = "uicore-d.dll";
#else
	const char* dllname = "uicore.dll";
#endif

	if (!m_hUICore.load(dllname))
	{
		DWORD err = GetLastError();
		::MessageBox(NULL, "Failed to load uicore.dll", "Desura: ERROR!",  MB_OK);
		exit(-200);
	}
	
	UICoreFP UICoreGetInterface = m_hUICore.getFunction<UICoreFP>("GetInterface");

	if (!UICoreGetInterface)
	{
		::MessageBox(NULL, "Failed to load wxWidgets mappings in uicore.dll", "Desura: ERROR!", MB_OK);
		exit(-500);
	} 

	m_pUICore = UICoreGetInterface();

	char version[100] = {0};
	_snprintf_s(version, 100, _TRUNCATE, "%d.%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND);

	m_pUICore->setDesuraVersion(version);
	m_pUICore->setRestartFunction(&UiCoreRestart);
	m_pUICore->setCrashDumpSettings(&SetDumpArgs);
	m_pUICore->setCrashDumpLevel(&SetDumpLevel);
}



