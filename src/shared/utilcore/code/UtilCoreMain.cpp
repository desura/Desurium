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

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "Log.h"
#include <shobjidl.h>

#include "wx_controls/gcManagers.h"
#include "managers\Managers.h"
#include "managers\CVar.h"

#include "ControlerForm.h"
#include "util_thread/BaseThread.h"

wxFrame* g_pMainApp = NULL;
HINSTANCE g_hInstDLL;

bool InitWebControl();
void ShutdownWebControl();

// TODO: Fix this to use UTIL::OS::getDataPath instead of data/.

#ifdef WIN32
	#define LANGFOLDER	".\\data\\language\\"
	#define THEMEFOLDER	".\\data\\themes"
	#define LANG_DEF	".\\data\\language\\english.xml"
	#define LANG_EXTRA	".\\data\\language\\english_utility.xml"
#else
	#define LANGFOLDER	"data/language/"
	#define THEMEFOLDER	"data/themes"
	#define LANG_DEF	"data/language/english.xml"
	#define LANG_EXTRA	"data/language/english_utility.xml"
#endif

CVar gc_theme("gc_theme", "default", CFLAG_SAVEONEXIT);

struct ITaskbarList3 * g_pITBL3 = NULL;

void InitManagers();
void DestroyManagers();

void InitLocalManagers()
{
	GetLanguageManager().loadFromFile(LANG_DEF);
	GetLanguageManager().loadFromFile(LANG_EXTRA);

	GetGCThemeManager()->loadFromFolder(THEMEFOLDER);
	GetGCThemeManager()->loadTheme(gc_theme.getString());
}

bool LangChanged(CVar* var, const char* val)
{
	gcString lan("{0}{1}.xml", LANGFOLDER, val);
	gcString extra("{0}{1}_utility.xml", LANGFOLDER, val);
	
	if (GetLanguageManager().loadFromFile(lan.c_str()))
	{
		GetLanguageManager().loadFromFile(extra.c_str());
		
		Msg(gcString("Loaded Language file: {0}\n", val));
		return true;
	}
	else
	{
		Warning(gcString("Failed to Load Language file: {0}\n", val));
		return false;
	}

	return true;
}

uint64 g_uiMainThreadId = 0;

uint64 GetMainThreadId()
{
	return g_uiMainThreadId;
}

CVar gc_language("gc_language", "english", CFLAG_SAVEONEXIT, (CVarCallBackFn)&LangChanged);

// This is where windows wants to start a DLL
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInstDLL = hinstDLL;
	return TRUE;
}


bool WindowsShutdown(wxWindowMSW *win, WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	if (nMsg == WM_QUERYENDSESSION)
	{
		return true;
	}
	else if (nMsg == WM_ENDSESSION)
	{
		if (wParam && g_pMainApp)
			g_pMainApp->Close();

		return true;
	}

	return false;
}

wxWindow* GetMainWindow()
{
	return g_pMainApp;
}



class Desura : public wxApp
{
public:
	bool OnInit()
	{
		g_uiMainThreadId = Thread::BaseThread::GetCurrentThreadId();

		//needed for mfc loading otherwise it asserts out
		SetExitOnFrameDelete(true);

		wxWindow::MSWRegisterMessageHandler(WM_QUERYENDSESSION, &WindowsShutdown);
		wxWindow::MSWRegisterMessageHandler(WM_ENDSESSION, &WindowsShutdown);

		wxInitAllImageHandlers();

		InitManagers();
		InitLocalManagers();
		InitLogging();

		ControllerForm* cf = new ControllerForm();

		if (!cf->init(argc, argv))
			return false;

		g_pMainApp = cf;
		//SetTopWindow(g_pMainApp);

		return true;
	}

	int OnExit()
	{
		DestroyLogging();
		
		wxWindow::MSWUnregisterMessageHandler(WM_ENDSESSION, &WindowsShutdown);
		wxWindow::MSWUnregisterMessageHandler(WM_QUERYENDSESSION, &WindowsShutdown);

		g_pMainApp = NULL;

		DestroyManagers();
		return wxApp::OnExit();
	}

	void ExitMainLoop()
	{
		// instead of existing wxWidgets main loop, terminate the MFC one
		::PostQuitMessage(0);
	}

private:
	DWORD m_wmTBC;
};

IMPLEMENT_APP_NO_MAIN(Desura)

void ExitApp()
{
	if (g_pMainApp)
		g_pMainApp->Close();
}
