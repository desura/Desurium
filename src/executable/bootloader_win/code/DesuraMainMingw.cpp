/*
Desura is the leading indie game distribution platform
Copyright (C) Karol Herbst <git@karolherbst.de>

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

#include <windows.h>

#include <Common.h>

#include <branding/desura_exe_version.h>

#include "SharedObjectLoader.h"
#include "UICoreI.h"
#include "UtilBootloader.h"

class BootLoader
{
public:
	BootLoader(int argc, char** argv);
	BOOL InitInstance();

protected:
//	bool sendArgs();
	void loadUICore();

//	void restartAsAdmin(int needupdate);
	void preReadImages();

private:
	SharedObjectLoader m_hUICore;
	UICoreI* m_pUICore;

	bool m_bRetCode;
	int m_iRetCode;
	bool hasAdminRights;
	
	int argc;
	char** argv;
};

BootLoader::BootLoader(int argc, char** argv)
:	argc(argc), argv(argv){};

BOOL BootLoader::InitInstance()
{
	// TODO: parse arguments
	unsigned int osid = BootLoaderUtil::GetOSId();
	// TODO: implement os checks, arguments, admin mode, etc...
	
	loadUICore();
	
	if (!m_pUICore)
		return FALSE;
	
	bool res = m_pUICore->initWxWidgets(GetModuleHandle(NULL), 0, argc, argv);

	/*if (res)
		m_pMainWnd = new BootLoaderUtil::CDummyWindow(m_pUICore->getHWND());

	return res?TRUE:FALSE;*/
	return TRUE;	
}

void BootLoader::preReadImages()
{
#ifdef DEBUG
	BootLoaderUtil::PreReadImage(".\\bin\\uicore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\webcore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\usercore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\mcfcore-d.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\wxmsw293ud_gcc_custom.dll");
#else
	BootLoaderUtil::PreReadImage(".\\bin\\uicore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\webcore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\usercore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\mcfcore.dll");
	BootLoaderUtil::PreReadImage(".\\bin\\wxmsw293u_gcc_custom.dll");	
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

#ifdef DEBUG
	const char* dllname = "libuicore-d.dll";
#else
	const char* dllname = "libuicore.dll";
#endif

	if (!m_hUICore.load(dllname))
	{
		DWORD err = GetLastError();
		std::cout << std::hex << HRESULT_FROM_WIN32(err) << std::endl;
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
	//m_pUICore->setRestartFunction(&UiCoreRestart);
	//m_pUICore->setCrashDumpSettings(&SetDumpArgs);
	//m_pUICore->setCrashDumpLevel(&SetDumpLevel);
}

int main(int argc, char** argv)
{
	BootLoader bootloader(argc, argv);
	bootloader.InitInstance();
}
