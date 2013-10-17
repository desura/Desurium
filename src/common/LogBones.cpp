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

#include "Log.h"
#include "LogCallback.h"

LogCallback* g_pLogCallback = NULL;


void PrintfMsg(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	if (!g_pLogCallback)
	{
		vprintf(format, args);
	}
	else
	{
		gcString str;
		str.vformat(format, args);
		g_pLogCallback->Msg(str.c_str());
	}

	va_end(args);
}

void LogMsg(int type, std::string msg, Color* col)
{
	if (!g_pLogCallback)
		return;

	switch (type)
	{
	case MT_MSG:
		g_pLogCallback->Msg(msg.c_str());
		break;

	case MT_MSG_COL:
		g_pLogCallback->Msg(msg.c_str(), col);
		break;

	case MT_WARN:
		g_pLogCallback->Warn(msg.c_str());
		break;

	case MT_DEBUG:
		g_pLogCallback->Debug(msg.c_str());
		break;
	};
}

void LogMsg(int type, std::wstring msg, Color* col)
{
	if (!g_pLogCallback)
		return;

	switch (type)
	{
	case MT_MSG:
		g_pLogCallback->Msg_W(msg.c_str());
		break;

	case MT_MSG_COL:
		g_pLogCallback->Msg_W(msg.c_str(), col);
		break;

	case MT_WARN:
		g_pLogCallback->Warn_W(msg.c_str());
		break;

	case MT_DEBUG:
		g_pLogCallback->Debug_W(msg.c_str());
		break;
	};
}
