///////////// Copyright 2008 Mark Chandler. All rights reserved. /////////////
//
//   Project     : desura
//   File        : main.cpp
//   Description :
//      [Write the purpose of main.cpp.]
//
//   Created On: 8/5/2008 9:51:48 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
/////////////////////////////////////////////////////////////////////////////////

#include "Common.h"

#include "windows.h"
#include "resource.h"

#include "UtilBootloader.h"
#include "util\UtilWindows.h"
#include "MiniDumpGenerator.h"

#include "SharedObjectLoader.h"
#include "UICoreI.h"

#include <vector>

#include "DesuraWinApp.h"


void ProcessDump(const char* commandLine);

class BootLoader : public Desurium::CDesuraWinApp
{
public:
	BootLoader();
	~BootLoader();

	void InitInstance() override;
	int ExitInstance() override;

protected:
	bool sendArgs();
	void loadUICore();

	void restartAsAdmin(int needupdate);

private:
	SharedObjectLoader m_hUICore;
	UICoreI* m_pUICore;

	MiniDumpGenerator m_MDumpHandle;
};

BootLoader theApp;



BootLoader::BootLoader()
{
	BootLoaderUtil::InitCommonControls();

#if 0
	WaitForDebugger();
#endif

	m_pUICore = NULL;
}

BootLoader::~BootLoader()
{
}

void BootLoader::InitInstance()
{
	BootLoaderUtil::SetCurrentDir();

	if (!BootLoaderUtil::SetDllDir(".\\bin"))
	{
		::MessageBox(NULL, "Failed to set the dll path to the bin folder.", "Desura: ERROR!", MB_OK);
		exit(-100);
	}

	if (strstr(m_lpCmdLine,"-crashreport")!=NULL)
	{
		ProcessDump(m_lpCmdLine);
		return;
	}

#ifndef DEBUG
	bool hasAdminRights = false;
	unsigned int osid = BootLoaderUtil::GetOSId();

	if (osid == WINDOWS_PRE2000)
	{
		::MessageBox(NULL, "Desura needs Windows xp or better to run.", "Desura Error: Old Windows", MB_OK);
		return;
	}
	else if (osid == WINDOWS_XP || osid == WINDOWS_XP64)
	{
		hasAdminRights = true;
	}

	if (strstr(m_lpCmdLine,"-admin")!=NULL)
	{
		hasAdminRights = true;
	}

	if (!hasAdminRights)
	{
		BootLoaderUtil::RestartAsAdmin("-admin");
		return;
	}
#endif

	loadUICore();

	if (!m_pUICore)
		return;

	char** argv = new char*[1];
	argv[0] = "-crashdump";

	m_pUICore->initWxWidgets(m_hInstance, m_nCmdShow, 1, argv);

	delete [] argv;
}

int BootLoader::ExitInstance()
{
	int ret = 0;

	if (m_pUICore)
		m_pUICore->exitApp(&ret);

	return ret;
}

void BootLoader::loadUICore()
{
	const char* dllname = "utilcore.dll";

	if (!m_hUICore.load(dllname))
	{
		DWORD err = GetLastError();
		::MessageBox(NULL, "Failed to load utilcore.dll", "Desura: ERROR!",  MB_OK);
		exit(-200);
	}
	
	UICoreFP UICoreGetInterface = m_hUICore.getFunction<UICoreFP>("GetInterface");

	if (!UICoreGetInterface)
	{
		::MessageBox(NULL, "Failed to load wxWidgets mappings in utilcore.dll", "Desura: ERROR!", MB_OK);
		exit(-500);
	} 

	m_pUICore = UICoreGetInterface();
}
