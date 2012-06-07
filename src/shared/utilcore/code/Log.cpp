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
#include "Log.h"
#include "LogCallback.h"

#include "LogBones.cpp"
//LogCallback* g_pLogCallback = NULL;
//void LogMsg(int type, std::string msg, Color* col);
//void LogMsg(int type, std::wstring msg, Color* col);

void DESURA_Msg(const char* msg, Color *col = NULL)
{
	if (!msg)
		return;

	LogMsg(MT_MSG, msg, col);
}

void DESURA_Msg_W(const wchar_t* msg, Color *col = NULL)
{
	if (!msg)
		return;

	LogMsg(MT_MSG, msg, col);
}

void DESURA_Warn(const char* msg)
{
	if (!msg)
		return;

	LogMsg(MT_WARN, msg, NULL);
}

void DESURA_Warn_W(const wchar_t* msg)
{
	if (!msg)
		return;

	LogMsg(MT_WARN, msg, NULL);
}

void DESURA_Debug(const char* msg)
{
#ifdef DEBUG
	if (!msg)
		return;

	LogMsg(MT_MSG, msg, NULL);
#endif
}

void DESURA_Debug_W(const wchar_t* msg)
{
#ifdef DEBUG
	if (!msg)
		return;

	LogMsg(MT_MSG, msg, NULL);
#endif
}


extern "C"
{
	void CIMPORT RegDLLCB_MCF( LogCallback* cb );
	void CIMPORT RegDLLCB_WEBCORE( LogCallback* cb );
	void CIMPORT RegDLLCB_USERCORE( LogCallback* cb );
}

void InitLogging()
{
	safe_delete(g_pLogCallback);
	g_pLogCallback = new LogCallback();

	g_pLogCallback->RegMsg(&DESURA_Msg);
	g_pLogCallback->RegMsg(&DESURA_Msg_W);
	g_pLogCallback->RegWarn(&DESURA_Warn);
	g_pLogCallback->RegWarn(&DESURA_Warn_W);
	g_pLogCallback->RegDebug(&DESURA_Debug);
	g_pLogCallback->RegDebug(&DESURA_Debug_W);
}

void DestroyLogging()
{
	RegDLLCB_MCF(NULL);
	RegDLLCB_WEBCORE(NULL);
	RegDLLCB_USERCORE(NULL);

	safe_delete(g_pLogCallback);
}





