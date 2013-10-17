/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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
#include "Log.h"
#include "LogCallback.h"
#include "Managers.h"

#include "MainApp.h"
#include "Console.h"

#include "LogBones.cpp"


static Console* logForm;

extern bool admin_cb(CVar* var, const char* val);

CVar admin_developer("admin_developer", "0", CFLAG_ADMIN|CFLAG_NOSAVE, (CVarCallBackFn)&admin_cb);


#ifdef DEBUG
	CVar gc_debug("gc_debug", "1", CFLAG_USER); 
	CVar gc_showerror("gc_showerror", "1", CFLAG_USER);
#else
	CVar gc_debug("gc_debug", "0", CFLAG_USER); 
	CVar gc_showerror("gc_showerror", "0", CFLAG_USER);
#endif




void DESURA_Msg(const char* msg, Color *col = NULL)
{
	if (logForm)
		logForm->appendText(msg, col?*col:Color(0));
}

void DESURA_Msg(const wchar_t* msg, Color *col = NULL)
{
	if (logForm)
		logForm->appendText(msg, col?*col:Color(0));
}

void DESURA_Warn(const char* msg)
{
	if (logForm)
		logForm->appendText(msg, Color(0xFF000000));

	if (gc_showerror.getBool())
		ShowLogForm(false);
}

void DESURA_Warn(const wchar_t* msg)
{
	if (logForm)
		logForm->appendText(msg, Color(0xFF000000));

	if (gc_showerror.getBool())
		ShowLogForm(false);
}

void DESURA_Debug(const char* msg)
{
	if (logForm && gc_debug.getBool())
		logForm->appendText(msg, Color(0x0000FF00));
}

void DESURA_Debug(const wchar_t* msg)
{
	if (logForm && gc_debug.getBool())
		logForm->appendText(msg, Color(0x0000FF00));
}










void HideLogForm()
{
	if (!logForm)
		 return;
	
	logForm->Show(false);
}

void ShowLogForm(bool forced)
{
	if (!logForm)
		 return;
	
	if (!g_pMainApp->isLoggedIn() && !g_pMainApp->isOffline())
		return;

	if (!forced && logForm->IsShownOnScreen())
		return;

	logForm->postShowEvent();
}

extern "C"
{
	void CIMPORT RegDLLCB_MCF(LogCallback* cb);
	void CIMPORT RegDLLCB_WEBCORE(LogCallback* cb);
	void CIMPORT RegDLLCB_USERCORE(LogCallback* cb);
}



void LoggingapplyTheme()
{
	if (!logForm)
		 return;

	logForm->applyTheme();
}

void InitLogging()
{
	if (logForm)
		safe_delete(logForm);

	logForm = new Console(NULL);
	

	Msg("UICore Logging Started\n");

	safe_delete(g_pLogCallback);
	g_pLogCallback = new LogCallback();

	g_pLogCallback->RegMsg((MsgCallBackFn)&DESURA_Msg);
	g_pLogCallback->RegMsg((MsgCallBackWFn)&DESURA_Msg);
	g_pLogCallback->RegWarn((SpecialCallBackFn)&DESURA_Warn);
	g_pLogCallback->RegWarn((SpecialCallBackWFn)&DESURA_Warn);
	g_pLogCallback->RegDebug((SpecialCallBackFn)&DESURA_Debug);
	g_pLogCallback->RegDebug((SpecialCallBackWFn)&DESURA_Debug);

	RegDLLCB_MCF(g_pLogCallback);
	RegDLLCB_WEBCORE(g_pLogCallback);
	RegDLLCB_USERCORE(g_pLogCallback);
}

void RegLogWithWindow()
{
	GetWindowManager().registerWindow(logForm);
}

void DestroyLogging()
{
	logForm->Destroy();
	logForm = NULL;

	RegDLLCB_MCF(NULL);
	RegDLLCB_WEBCORE(NULL);
	RegDLLCB_USERCORE(NULL);

	safe_delete(g_pLogCallback);
}





