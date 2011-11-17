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

#include "log.h"

FILE* g_pUpdateLog = NULL;

void InitUpdateLog()
{
	if (g_pUpdateLog)
		return;

	char file[] = "update_log.txt";
	g_pUpdateLog = fopen(file, "a");
}

void StopUpdateLog()
{
	if (!g_pUpdateLog)
		return;

	fclose(g_pUpdateLog);
}

void Log(const char* format, ...)
{
	if (!g_pUpdateLog)
		return;

	time_t rawtime;
	tm timeinfo;
	char buffer[255];

	time(&rawtime);
	localtime_r(&rawtime, &timeinfo);

	strftime(buffer, 255, "%c:", &timeinfo);
	fprintf(g_pUpdateLog, "%s", buffer);

	va_list args;
	va_start(args, format);
	vfprintf(g_pUpdateLog, format, args);
	va_end(args);
}
