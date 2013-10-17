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

#ifndef DESURA_GAMEEXPLORERSQL_H
#define DESURA_GAMEEXPLORERSQL_H
#ifdef _WIN32
#pragma once
#endif


#define COUNT_GEINFO "select count(*) from sqlite_master where name='gameexplorer';"
#define CREATE_GEINFO "create table gameexplorer(internalid INTEGER PRIMARY KEY, "		\
												"dllpath TEXT,"							\
												"guid INTEGER,"							\
												"flags TEXT"							\
												");"


#define GEINFO_DB "gameexplorer.sqlite"

inline gcString getGameExplorerDb(const char* appDataPath)
{
	return gcString("{0}{2}{1}", appDataPath, GEINFO_DB, DIRS_STR);
}

inline void createGameExplorerDbTables(const char* appDataPath)
{
	sqlite3x::sqlite3_connection db(getGameExplorerDb(appDataPath).c_str());

	if (db.executeint(COUNT_GEINFO) == 0)
		db.executenonquery(CREATE_GEINFO);
}

#endif //DESURA_GAMEEXPLORERSQL_H
