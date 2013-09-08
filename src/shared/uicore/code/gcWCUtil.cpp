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

#include "Common.h"

#define CEF_IGNORE_FUNCTIONS 1
#include "cef_desura_includes/ChromiumBrowserI.h"

#include "mcfcore/UserCookies.h"
#include "MainApp.h"

#include "SharedObjectLoader.h"
#include "gcSchemeBase.h"
#include "gcJSBase.h"

#ifdef NIX
#include "managers/CVar.h"
#include <gtk/gtk.h>
#endif

#include <wx/app.h>

void RegisterJSBindings();
void RegisterSchemes();


typedef bool (*CEF_InitFn)(bool, const char*, const char*, const char*);
typedef void (*CEF_StopFn)();

typedef bool (*CEF_RegisterJSExtenderFn)(ChromiumDLL::JavaScriptExtenderI*);
typedef bool (*CEF_RegisterSchemeExtenderFn)(ChromiumDLL::SchemeExtenderI*);

typedef void (*CEF_DeleteCookieFn)(const char*, const char*);
typedef ChromiumDLL::CookieI* (*CEF_CreateCookieFn)();
typedef void (*CEF_SetCookieFn)(const char* ulr, ChromiumDLL::CookieI*);
typedef ChromiumDLL::ChromiumBrowserI* (*CEF_NewChromiumBrowserFn)(int*, const char *,  const char*);

typedef void (*CEF_DoWorkFn)();

typedef void (*CEF_PostCallbackFn)(ChromiumDLL::CallbackI*);

#ifdef NIX
guint m_timeoutSource = 0;
#endif

bool g_bLoaded = false;
SharedObjectLoader g_CEFDll;

CEF_InitFn CEF_Init = NULL;
CEF_StopFn CEF_Stop = NULL;
CEF_RegisterJSExtenderFn CEF_RegisterJSExtender = NULL;
CEF_RegisterSchemeExtenderFn CEF_RegisterSchemeExtender = NULL;
CEF_DeleteCookieFn CEF_DeleteCookie = NULL;
CEF_CreateCookieFn CEF_CreateCookie = NULL;
CEF_SetCookieFn CEF_SetCookie = NULL;
CEF_NewChromiumBrowserFn CEF_NewChromiumBrowser = NULL;
CEF_DoWorkFn CEF_DoWork = NULL;
CEF_PostCallbackFn CEF_PostCallback = NULL;


#ifdef NIX
gboolean onTimeout(gpointer data)
{
	if (!g_bLoaded || !CEF_DoWork)
	{
		m_timeoutSource = 0;
		return false;
	}
	
	CEF_DoWork();
	
	//if we dont do this here we could end up starving the pending que due to CEF_DoWork taking to long.
	if (wxTheApp)
		wxTheApp->ProcessPendingEvents();
	
	return true;
}

void RestartTimer(uint timeout)
{
	if (! g_bLoaded)
		return;
	
	if (m_timeoutSource != 0)
		g_source_remove(m_timeoutSource);
	m_timeoutSource = g_timeout_add(timeout, onTimeout, NULL);
}

bool RestartTimerCB(const CVar* hook, const char* newval)
{
	if (! newval)
		return false;

	RestartTimer(atoi(newval));
	
	return true;
}
	
CVar gc_cef_timeout("gc_cef_timeout", "75", 0, &RestartTimerCB);
#endif

void UnloadCEFDll()
{
	CEF_Init = NULL;
	CEF_Stop = NULL;
	CEF_RegisterJSExtender = NULL;
	CEF_RegisterSchemeExtender = NULL;
	CEF_DeleteCookie = NULL;
	CEF_CreateCookie = NULL;
	CEF_SetCookie = NULL;
	CEF_NewChromiumBrowser = NULL;
	CEF_DoWork = NULL;
	CEF_PostCallback = NULL;

	g_CEFDll.unload();
}

#ifdef WIN32
const char* szCefDLL = "cef_desura.dll";
#else
const char* szCefDLL = "libcef_desura.so";
#endif

bool LoadCEFDll()
{
	if (!g_CEFDll.load(szCefDLL))
	{
		Warning(gcString("Failed to load cef library: {0}\n", GetLastError()));
		return false;
	}

	CEF_Init = g_CEFDll.getFunction<CEF_InitFn>("CEF_Init");
	CEF_Stop = g_CEFDll.getFunction<CEF_StopFn>("CEF_Stop");
	CEF_RegisterJSExtender = g_CEFDll.getFunction<CEF_RegisterJSExtenderFn>("CEF_RegisterJSExtender");
	CEF_RegisterSchemeExtender = g_CEFDll.getFunction<CEF_RegisterSchemeExtenderFn>("CEF_RegisterSchemeExtender");
	CEF_DeleteCookie = g_CEFDll.getFunction<CEF_DeleteCookieFn>("CEF_DeleteCookie");
	CEF_CreateCookie = g_CEFDll.getFunction<CEF_CreateCookieFn>("CEF_CreateCookie");
	CEF_SetCookie = g_CEFDll.getFunction<CEF_SetCookieFn>("CEF_SetCookie");
	CEF_NewChromiumBrowser = g_CEFDll.getFunction<CEF_NewChromiumBrowserFn>("CEF_NewChromiumBrowser");
	CEF_DoWork = g_CEFDll.getFunction<CEF_DoWorkFn>("CEF_DoMsgLoop");
	CEF_PostCallback = g_CEFDll.getFunction<CEF_PostCallbackFn>("CEF_PostCallback");

	if (g_CEFDll.hasFailed())
	{
		Warning("Failed to find cef library exports\n");
		UnloadCEFDll();
		return false;
	}

	return true;
}

bool InitWebControl()
{
	if (g_bLoaded)
		return true;

	if (!LoadCEFDll())
		return false;

	UserAgentFN userAgent = (UserAgentFN)WebCore::FactoryBuilder(WEBCORE_USERAGENT);
	gcString ua;

	//stupid hack cause crappy paypal does useragent sniffing. FFFFFFFFFFFFUUUUUUUUUUUUUUUUUUUUUUU
#ifdef WIN32
	ua += "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.220 Safari/535.1";
#else
	ua += "Mozilla/5.0 (X11; Linux i686) AppleWebKit/535.1 (KHTML, like Gecko) Chrome/13.0.782.220 Safari/535.1";
#endif

	ua += " " + userAgent();

	UTIL::FS::Path path(UTIL::OS::getTempInternetPath(L"desura"), L"", false);
	UTIL::FS::recMakeFolder(path);

	gcString logPath;

#ifdef DEBUG
	logPath = "Chromium_log.txt";
#endif

#ifdef WIN32
	bool multiThreaded = true;
#else
	bool multiThreaded = false;
#endif

	if (!CEF_Init(multiThreaded, gcString(path.getFolderPath()).c_str(), logPath.c_str(), ua.c_str()))
	{
		Warning("Failed to init cef.\n");
		return false;
	}

	RegisterJSBindings();
	RegisterSchemes();

#ifdef NIX
	m_timeoutSource = g_timeout_add(50, onTimeout, NULL);
#endif
	
	g_bLoaded = true;
	return true;
}

void ShutdownWebControl()
{
	g_bLoaded = false;

#ifdef NIX
	if (m_timeoutSource != 0)
	{
		g_source_remove(m_timeoutSource);
		m_timeoutSource = 0;
	}
#endif

	if (CEF_Stop)
		CEF_Stop();

	UnloadCEFDll();
}

void DeleteCookies()
{
	if (!CEF_DeleteCookie)
		return;

	CEF_DeleteCookie("http://www.desura.com", "freeman");
	CEF_DeleteCookie("http://www.desura.com", "masterchief");
}

void SetCookies()
{
	if (!g_bLoaded && !InitWebControl() && !GetWebCore())
		return;

	if (!CEF_CreateCookie)
	{
		Warning("CEF_CreateCookie is NULL. Failed to set cookies. :(\n");
		return;
	}

	if (!CEF_SetCookie)
	{
		Warning("CEF_SetCookie is NULL. Failed to set cookies. :(\n");
		return;
	}

	ChromiumDLL::CookieI* cookie = CEF_CreateCookie();

	if (!cookie)
	{
		Warning("Cef failed to create cookie. Failed to set cookies. :(\n");
		return;
	}

	cookie->SetDomain(".desura.com");
	cookie->SetPath("/");

	MCFCore::Misc::UserCookies uc;
	GetWebCore()->setMCFCookies(&uc);

	gcString fD(uc.getIdCookie()); //= UTIL::STRING::urlDecode
	gcString mD(uc.getSessCookie()); //= UTIL::STRING::urlDecode

	cookie->SetName("freeman");
	cookie->SetData(fD.c_str());

	CEF_SetCookie("http://www.desura.com", cookie);

	cookie->SetName("masterchief");
	cookie->SetData(mD.c_str());

	CEF_SetCookie("http://www.desura.com", cookie);

	cookie->destroy();
}

#ifdef WIN32
//ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(HWND hwnd, const char* name, const char* loadUrl)
ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(HWND hwnd, const char* name, const char* loadUrl)
#else
ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(int* hwnd, const char* name, const char* loadUrl)
#endif
{
	if (!g_bLoaded && !InitWebControl())
		return NULL;

	return CEF_NewChromiumBrowser((int*)hwnd, name, loadUrl);
}



std::vector<ChromiumDLL::JavaScriptExtenderI*> *g_vJSExtenderList=NULL;

void RegisterJSExtender( ChromiumDLL::JavaScriptExtenderI* scheme )
{
	if (!g_vJSExtenderList)
		g_vJSExtenderList = new std::vector<ChromiumDLL::JavaScriptExtenderI*>();

	g_vJSExtenderList->push_back(scheme);
}

void RegisterJSBindings()
{
	if (!g_vJSExtenderList)
		return;

	for (size_t x=0; x<g_vJSExtenderList->size(); x++)
		CEF_RegisterJSExtender((*g_vJSExtenderList)[x]);
	
	g_vJSExtenderList->clear();
	safe_delete(g_vJSExtenderList);
}



std::vector<ChromiumDLL::SchemeExtenderI*> *g_vSchemeList = NULL;

void RegisterScheme( ChromiumDLL::SchemeExtenderI* scheme )
{
	if (!g_vSchemeList)
		g_vSchemeList = new std::vector<ChromiumDLL::SchemeExtenderI*>();

	g_vSchemeList->push_back(scheme);
}

void RegisterSchemes()
{
	if (!g_vSchemeList)
		return;

	for (size_t x=0; x<g_vSchemeList->size(); x++)
		CEF_RegisterSchemeExtender((*g_vSchemeList)[x]);
	
	g_vSchemeList->clear();
	safe_delete(g_vSchemeList);
}

void BrowserUICallback(ChromiumDLL::CallbackI* callback)
{
	if (CEF_PostCallback)
		CEF_PostCallback(callback);
}
