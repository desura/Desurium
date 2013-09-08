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
////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "Log.h"
#include "LogCallback.h"

class Color;

LogCallback* g_pLogCallBack = NULL;

void DESURA_Msg(const char* msg, Color* col = NULL)
{
	Msg(msg);
}

void DESURA_Msg_W(const wchar_t* msg, Color* col = NULL)
{
	gcString m(msg);
	Msg(m.c_str());
}

void DESURA_Warn(const char* msg)
{
	Warning(msg);
}

void DESURA_Warn_W(const wchar_t* msg)
{
	gcString m(msg);
	Warning(m.c_str());
}

void DESURA_Debug(const char* msg)
{
	Debug(msg);
}

void DESURA_Debug_W(const wchar_t* msg)
{
	gcString m(msg);
	Debug(m.c_str());
}


void InitLogging(RegDLLCB_MCF cb)
{
	if (!g_pLogCallBack)
	{
		g_pLogCallBack = new LogCallback();

		g_pLogCallBack->RegMsg(&DESURA_Msg);
		g_pLogCallBack->RegMsg(&DESURA_Msg_W);
		g_pLogCallBack->RegWarn(&DESURA_Warn);
		g_pLogCallBack->RegWarn(&DESURA_Warn_W);
		g_pLogCallBack->RegDebug(&DESURA_Debug);
		g_pLogCallBack->RegDebug(&DESURA_Debug_W);
	}

	cb(g_pLogCallBack);
}

void DestroyLogging()
{
	safe_delete(g_pLogCallBack);
}
