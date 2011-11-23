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
#include "UICoreI.h"
#include "MainApp.h"
#include "managers/CVar.h"
#include "managers/ConCommand.h"
#include <wx/wx.h>
#include <wx/snglinst.h>
#include <wx/evtloop.h>

extern "C" CEXPORT UICoreI* GetInterface();

#ifdef NIX
#include "util/UtilLinux.h"
#endif

#ifdef WIN32
#include <shlobj.h>

WXDLLIMPEXP_BASE void wxSetInstance(HINSTANCE hInst);


HHOOK hIISHook = NULL;


LRESULT CALLBACK CallWndProc(int nCode,WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* msg = (CWPSTRUCT*)lParam;

	//this is needed here as it doesnt process this event properly.
	if (msg->message == WM_QUERYENDSESSION)
	{
		GetInterface()->closeMainWindow();

		if (hIISHook != NULL)
			UnhookWindowsHookEx(hIISHook);

		hIISHook = NULL;
	}

	return CallNextHookEx(hIISHook, nCode, wParam, lParam);
}
#endif


#ifndef WIN32

#include <gtk/gtk.h>

void gtkMessageBox(const char* text, const char* title)
{
	int argc=0;
	char** argv=NULL;

	gtk_init(&argc, &argv);
	GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", text);
	gtk_window_set_title(GTK_WINDOW(dialog), title);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
} 
#endif

class UICore : public UICoreI
{
public:
	UICore()
	{
		m_szAppVersion = NULL;
		m_pDumpSettings = NULL;
		m_pDumpLevel = NULL;
		m_pRestart = NULL;

		m_bExitCodeSet = false;
		m_iExitCode = 0;
	}

	~UICore()
	{
		safe_delete(m_szAppVersion);

#ifdef WIN32
		if (hIISHook != NULL)
			UnhookWindowsHookEx(hIISHook);

		hIISHook = NULL;
#else
		safe_delete(m_pChecker);
#endif
	}

	void setDesuraVersion(const char* version)
	{
		Safe::strcpy(&m_szAppVersion, version, 255);
	}

#ifdef WIN32
	bool initWxWidgets(HINSTANCE hInst, int CmdShow, int argc, char** argv)
	{
		if (needForceUpdate())
			return false;

		hIISHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, 0, GetCurrentThreadId());

		wxSetInstance(hInst);
		wxApp::m_nCmdShow = CmdShow;
		wxEntry(argc, argv);

		if ( !wxTheApp || !wxTheApp->CallOnInit() )
			return false;

		return true;
	}
#else
	bool initWxWidgets(int argc, char** argv)
	{
		if (needForceUpdate())
			return false;

		wxEntry(argc, argv);

		if ( !wxTheApp || !wxTheApp->CallOnInit() )
			return false;

		return true;
	}
#endif

	void exitApp(int* result)
	{
		if ( wxTheApp )
			wxTheApp->OnExit();

		wxEntryCleanup();

		if (m_bExitCodeSet)
			*result = m_iExitCode;
	}

#ifdef WIN32
	bool preTranslateMessage(MSG *msg)
	{
		wxEventLoop *evtLoop = static_cast<wxEventLoop*>(wxEventLoop::GetActive());

		if ( evtLoop && evtLoop->PreProcessMessage(msg))
			return true;

		return false;
	}

	BOOL onIdle()
	{
		return (wxTheApp && wxTheApp->ProcessIdle());
	}

	HWND getHWND()
	{
		return (HWND)wxTheApp->GetTopWindow()->GetHWND();
	}
#endif // LINUX TODO
	void setRestartFunction(RestartFP rfp)
	{
		m_pRestart = rfp;
	}

	void setCrashDumpSettings(DumpSettingsFP dsfp)
	{
		m_pDumpSettings = dsfp;
	}

	void setCrashDumpLevel(DumpLevelFP dlfp)
	{
		m_pDumpLevel = dlfp;
	}

	void setDumpSettings(const wchar_t* user, bool upload)
	{
#ifdef WIN32
		if (m_pDumpSettings)
			m_pDumpSettings(user, upload);
#else
		if (m_pDumpSettings)
			m_pDumpSettings(gcString(user).c_str(), upload);
#endif
	}

	void setDumpLevel(unsigned char level)
	{
		if (m_pDumpLevel)
			m_pDumpLevel(level);
	}

	void closeMainWindow()
	{
		if (g_pMainApp)
			g_pMainApp->Close(true);
	}

	void restart(const char* args = NULL)
	{
		if (m_pRestart)
			m_pRestart(args);

		g_pMainApp->Close(true);
	}

	const char* getAppVersion()
	{
		return m_szAppVersion;
	}

	void setExitCode(int32 exitCode)
	{
		m_bExitCodeSet = true;
		m_iExitCode = exitCode;
	}

#ifndef WIN32
	bool singleInstantCheck(const char* args)
	{
		std::string path = UTIL::LIN::expandPath("$XDG_RUNTIME_DIR/desura");
		
		UTIL::FS::recMakeFolder(path.c_str());
		m_pChecker = new wxSingleInstanceChecker("applock", path);

		if (m_pChecker->IsAnotherRunning())
			return false;

		return true;
	}
	
	void destroySingleInstanceCheck()
	{
		safe_delete(m_pChecker);
	}
	
	void disableSingleInstanceLock()
	{
		m_pChecker = NULL;
	}
#endif

protected:
	bool needForceUpdate()
	{
		return false;
	}


private:
	DumpSettingsFP m_pDumpSettings;
	DumpLevelFP m_pDumpLevel;

	RestartFP m_pRestart;
	char* m_szAppVersion;

	bool m_bExitCodeSet;
	int32 m_iExitCode;

#ifndef WIN32
	wxSingleInstanceChecker* m_pChecker;
#endif
};


UICore g_pUICore;

extern "C"
{
	CEXPORT UICoreI* GetInterface()
	{
		return &g_pUICore;
	}
}

const char* GetAppVersion()
{
	return g_pUICore.getAppVersion();
}

const char* GetUserCoreVersion()
{
	UserCoreVersionFN ver = (UserCoreVersionFN)UserCore::FactoryBuilderUC(USERCORE_VER);

	if (!ver)
		return "Unable To Load Version";

	return ver();
}


const char* GetWebCoreVersion()
{
	WebCoreVersionFN ver = (WebCoreVersionFN)WebCore::FactoryBuilder(WEBCORE_VER);

	if (!ver)
		return "Unable To Load Version";

	return ver();
}

bool setDumpLevel(const CVar* cvar, const char* value)
{
	int level = atoi(value);

	if (level != 0)
		g_pUICore.setDumpLevel(level);
	
	return true;
}

CVar gc_dumplevel("gc_dumplevel", "0", 0, &setDumpLevel);

CONCOMMAND(cc_restart, "restart")
{
	g_pUICore.restart();
}

CONCOMMAND(cc_restart_st, "restart_st")
{
	g_pUICore.restart("-servicetest");
}

CONCOMMAND(cc_restart_wait, "restart_wait")
{
	g_pUICore.restart("-restartwait");
}

CONCOMMAND(cc_restart_forceupdate, "restart_update")
{
	g_pUICore.restart("-forceupdate");
}

void RestartAndSetMCFCache(const char* dir)
{
	if (!dir)
		return;

	gcString args("-setcachedir -wait -dir \"{0}\"", dir);
	g_pUICore.restart(args.c_str());
}

void SetCrashDumpSettings(const wchar_t* user, bool upload)
{
	g_pUICore.setDumpSettings(user, upload);
}
