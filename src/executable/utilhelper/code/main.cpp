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

#include "Common.h"

#include "Commctrl.h"

#include "UtilBootloader.h"
#include "MiniDumpGenerator.h"

#include "SharedObjectLoader.h"
#include "UICoreI.h"
#include <branding/branding.h>
#include "DesuraWinApp.h"
#include <vector>

class BootLoader : public Desurium::CDesuraWinApp
{
public:
	BootLoader();
	~BootLoader();

	void InitInstance();
	int ExitInstance();

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
	m_MDumpHandle.showMessageBox(true);

	InitCommonControls();

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
	UTIL::MISC::CMDArgs args(m_lpCmdLine);

	if (BootLoaderUtil::GetOSId() == WINDOWS_PRE2000)
	{
		::MessageBox(NULL, PRODUCT_NAME " needs Windows XP or better to run.", PRODUCT_NAME " Error: Old Windows", MB_OK);
		return;
	}

	if (args.hasArg("wait"))
	{
#ifdef DEBUG
		BootLoaderUtil::WaitForOtherInstance("desura-d.exe");
#else
		BootLoaderUtil::WaitForOtherInstance("desura.exe");
#endif
	}

	loadUICore();

	if (!m_pUICore)
		return;

	m_pUICore->initWxWidgets(m_hInstance, m_nCmdShow, args.getArgc(), const_cast<char**>(args.getArgv()));
}

int BootLoader::ExitInstance()
{
	int ret=0;

	if (m_pUICore)
		m_pUICore->exitApp(&ret);

	return ret;
}

void BootLoader::loadUICore()
{
#ifndef DEBUG
	BootLoaderUtil::SetCurrentDir();
#endif

	if (!BootLoaderUtil::SetDllDir(".\\bin"))
	{
		::MessageBox(NULL, "Failed to set the DLL path to the bin folder.", PRODUCT_NAME ": ERROR!",  MB_OK);
		exit(-100);			
	}

#ifdef DEBUG
	const char* dllname = "utilcore-d.dll";
#else
	const char* dllname = "utilcore.dll";
#endif

	if (!m_hUICore.load(dllname))
	{
		DWORD err = GetLastError();
		::MessageBox(NULL, "Failed to load utilcore.dll", PRODUCT_NAME ": ERROR!",  MB_OK);
		exit(-200);
	}
	
	UICoreFP UICoreGetInterface = m_hUICore.getFunction<UICoreFP>("GetInterface");

	if (!UICoreGetInterface)
	{
		::MessageBox(NULL, "Failed to load wxWidgets mappings in utilcore.dll", PRODUCT_NAME ": ERROR!", MB_OK);
		exit(-500);
	} 

	m_pUICore = UICoreGetInterface();
}
