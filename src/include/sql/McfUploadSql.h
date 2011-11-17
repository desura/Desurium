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

#ifndef DESURA_MCFUPLOAD_H
#define DESURA_MCFUPLOAD_H
#ifdef _WIN32
#pragma once
#endif


#define CREATE_MCFUPLOAD "CREATE TABLE mcfupload(key TEXT PRIMARY KEY, file TEXT, internalid INTEGER);"
#define COUNT_MCFUPLOAD "SELECT count(*) FROM sqlite_master WHERE name='mcfupload';"


#define MCFUPLOAD_DB "mcf_uploads.sqlite"

inline gcString getMcfUploadDb(const char* appDataPath)
{
	return gcString("{0}{2}{1}", appDataPath, MCFUPLOAD_DB, DIRS_STR);
}

inline void createMcfUploadDbTables(const char* appDataPath)
{
	sqlite3x::sqlite3_connection db(getMcfUploadDb(appDataPath).c_str());

	if (db.executeint(COUNT_MCFUPLOAD) == 0)
		db.executenonquery(CREATE_MCFUPLOAD);
}

#endif //DESURA_MCFUPLOAD_H
