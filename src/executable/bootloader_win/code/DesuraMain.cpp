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
#include "DesuraWinApp.h"
#include "UtilBootloader.h"

#ifdef DESURA_OFFICIAL_BUILD
	#include "AppUpdateInstall.h"
	
	extern UINT DownloadFilesForTest();
	extern UINT InstallFilesForTest();
	
	extern INT_PTR DisplayUpdateWindow(int updateType);
	
	extern bool CheckCert();
	extern void CheckForBadUninstaller();
#endif

#include "UpdateFunctions.h"
#include "MiniDumpGenerator.h"

#include <branding/branding.h>
#include <branding/desura_exe_version.h>
#include "util/gcDDE.h"

#include "UICoreI.h"
#include "SharedObjectLoader.h"

#include <map>
#include <string>
#include <shlobj.h>

#define STR( t ) #t

#define Log( s ) ;

bool g_bRestart = false;
char* g_szArgs = NULL;


extern void InstallService();
extern void SetRegValues();
extern void cleanUpIPC();


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

class BootLoader : public Desurium::CDesuraWinApp
{
public:
	BootLoader();
	~BootLoader();

	void InitInstance() override;
	int ExitInstance() override;

	MiniDumpGenerator m_MDumpHandle;

protected:
	bool sendArgs();
	void loadUICore();

	void restartAsAdmin(int needupdate);
	void preReadImages();

	bool preLaunchCheck(BootLoaderUtil::CMDArgs &args);

private:
	SharedObjectLoader m_hUICore;
	UICoreI* m_pUICore;

	bool m_bRetCode;
	int m_iRetCode;
	bool m_bHasAdminRights;
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

	//AfxEnableMemoryTracking(FALSE);
	InitCommonControls();

	m_bHasAdminRights = false;
	m_pUICore = NULL;
	m_bRetCode = false;
}

BootLoader::~BootLoader()
{
	cleanUpIPC();

	if (g_szArgs)
		delete [] g_szArgs;
}

void BootLoader::InitInstance()
{
	BootLoaderUtil::CMDArgs args(m_lpCmdLine);
	
	if (args.hasArg("waitfordebugger"))
		BootLoaderUtil::WaitForDebugger();

	BootLoaderUtil::SetCurrentDir();

	bool bRunUnitTests = args.hasArg("unittests");

	if (!bRunUnitTests && !preLaunchCheck(args))
		return;

	loadUICore();

	if (!m_pUICore)
		return;

	if (bRunUnitTests)
	{
		m_iRetCode = m_pUICore->runUnitTests(args.getArgc(), const_cast<char**>(args.getArgv()));
		m_bRetCode = true;
		return;
	}

	m_pUICore->initWxWidgets(m_hInstance, m_nCmdShow, args.getArgc(), const_cast<char**>(args.getArgv()));
}

bool BootLoader::preLaunchCheck(BootLoaderUtil::CMDArgs &args)
{
#ifdef DESURA_OFFICAL_BUILD
	CheckForBadUninstaller();
#endif

	if (args.hasArg("urllink"))
	{
		std::string a(m_lpCmdLine);

		size_t pos = a.find("-urllink");
		a.replace(pos, 8, "");

		BootLoaderUtil::Restart(a.c_str(), false);
		return false;
	}

#ifdef DESURA_OFFICIAL_BUILD
	if (args.hasArg("testinstall"))
	{
		m_bRetCode = true;
		m_iRetCode = InstallFilesForTest();
		return false;
	}

	if (args.hasArg("testdownload"))
	{
		m_bRetCode = true;
		m_iRetCode = DownloadFilesForTest();
		return false;
	}
#endif

	if (args.hasArg("dumplevel"))
	{
		SetDumpLevel(args.getInt("dumplevel"));
		g_bLockDump = true;
	}

	if (args.hasArg("autostart"))
	{
		//need to wait for service to start
		Sleep(15 * 1000);
		BootLoaderUtil::RestartAsNormal("-wait");
		return false;
	}

#ifdef DESURA_OFFICIAL_BUILD
#ifdef DEBUG
	if (args.hasArg("debugupdater"))
	{
		INT_PTR nResponse = DisplayUpdateWindow(-1);
		return false;
	}

	if (args.hasArg("debuginstall"))
	{
		McfUpdate();
		return false;
	}

	if (args.hasArg("debugdownload"))
	{
		FullUpdate();
		return false;
	}

	if (args.hasArg("debugcheck"))
	{
		CheckInstall();
		return false;
	}
#endif
#endif

	if (args.hasArg("testcrash"))
	{
		BootLoader *ai = NULL;
		//ai->AssertValid();
	}

	unsigned int osid = BootLoaderUtil::GetOSId();

	if (osid == WINDOWS_PRE2000)
	{
		::MessageBox(NULL, PRODUCT_NAME " needs Windows XP or better to run.", PRODUCT_NAME " Error: Old Windows", MB_OK);
		return false;
	}
	else if (osid == WINDOWS_XP || osid == WINDOWS_XP64)
	{
		m_bHasAdminRights = true;
	}

	if (args.hasArg("admin"))
	{
		m_bHasAdminRights = true;
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
			return false;
		}
		else
		{
			//if windows uninstall software launches desura it will disable its window till it quits.
			//Work around for existing clients
			std::string a(m_lpCmdLine);
			size_t pos = a.find("desura://uninstall/");

			if (pos != std::string::npos)
			{
				a.replace(pos + 9, 9, "remove");
				BootLoaderUtil::RestartAsNormal(a.c_str());
				return false;
			}
		}
	}

#ifdef DESURA_OFFICIAL_BUILD
	if (args.hasArg("forceupdate"))
	{
		if (!m_bHasAdminRights)
		{
			restartAsAdmin(UPDATE_FORCED);
			return false;
		}
		else
		{
			FullUpdate();
			BootLoaderUtil::RestartAsNormal("-wait");
			return false;
		}
	}
#endif


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
			return false;
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
		if (!m_bHasAdminRights)
		{
			restartAsAdmin(nu);
			return false;
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

			return false;
		}
		else
		{
			Log("Full update [%s].\n", g_UpdateReasons[nu]);
			FullUpdate();
			BootLoaderUtil::RestartAsNormal("-wait");
			return false;
		}
	}

	if (m_bHasAdminRights && !(osid == WINDOWS_XP || osid == WINDOWS_XP64))
	{
		BootLoaderUtil::RestartAsNormal("-wait");
		return false;
	}
#endif

	return true;
}

void BootLoader::restartAsAdmin(int needupdate)
{
	const char* args = "-wait -admin";

	if (needupdate == UPDATE_FORCED)
		args = "-wait -admin -forceupdate";

	if (!BootLoaderUtil::RestartAsAdmin(args))
	{
		char msg[255];
		_snprintf_s(msg, 255, _TRUNCATE, "Failed to restart " PRODUCT_NAME " with admin rights [Update: %d].", needupdate);
		::MessageBox(NULL, msg, PRODUCT_NAME " Critical Error", MB_OK);
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
	int ret = 0;

	if (m_pUICore)
		m_pUICore->exitApp(&ret);

	if (g_bRestart)
	{
		if (g_szArgs && strstr(g_szArgs, "-setcachedir"))
			BootLoaderUtil::StartProcess("utilhelper.exe", g_szArgs);
		else
			BootLoaderUtil::Restart(g_szArgs);
	}

	if (m_bRetCode)
		return m_iRetCode;

	return ret;
}

void BootLoader::preReadImages()
{
	BootLoaderUtil::PreReadImage(".\\bin\\uicore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\webcore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\usercore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\mcfcore.dll");
#ifdef DEBUG
	BootLoaderUtil::PreReadImage(".\\bin\\wxmsw293ud_vc_desura.dll");
#else
	BootLoaderUtil::PreReadImage(".\\bin\\wxmsw293u_vc_desura.dll");
#endif
}

void BootLoader::loadUICore()
{
	if (!BootLoaderUtil::SetDllDir(".\\bin"))
	{
		::MessageBox(NULL, "Failed to set the DLL path to the bin folder.", PRODUCT_NAME ": ERROR!",  MB_OK);
		exit(-100);			
	}

	preReadImages();

	const char* dllname = "uicore.dll";

	if (!m_hUICore.load(dllname))
	{
		DWORD err = GetLastError();
		::MessageBox(NULL, "Failed to load uicore.dll", PRODUCT_NAME ": ERROR!",  MB_OK);
		exit(-200);
	}
	
	UICoreFP UICoreGetInterface = m_hUICore.getFunction<UICoreFP>("GetInterface");

	if (!UICoreGetInterface)
	{
		::MessageBox(NULL, "Failed to load wxWidgets mappings in uicore.dll", PRODUCT_NAME ": ERROR!", MB_OK);
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



