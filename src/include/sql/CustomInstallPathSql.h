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

#ifndef DESURA_CIP_H
#define DESURA_CIP_H
#ifdef _WIN32
#pragma once
#endif

//This is also defined in GetItemListThread in usercore
#define CREATE_CIPITEMINFO "create table cipiteminfo(internalid INTEGER PRIMARY KEY, name TEXT);"
#define COUNT_CIPITEMINFO "select count(*) from sqlite_master where name='cipiteminfo';"

#define CREATE_CIP "create table cip(internalid INTEGER PRIMARY KEY, path TEXT);"
#define COUNT_CIP "select count(*) from sqlite_master where name='cip';"

#define CIP_DB "cip.sqlite"

inline gcString getCIBDb(const char* appDataPath)
{
	return gcString("{0}{2}{1}", appDataPath, CIP_DB, DIRS_STR);
}

inline void createCIPDbTables(const char* appDataPath)
{
	sqlite3x::sqlite3_connection db(getCIBDb(appDataPath).c_str());

	if (db.executeint(COUNT_CIPITEMINFO) == 0)
		db.executenonquery(CREATE_CIPITEMINFO);

	if (db.executeint(COUNT_CIP) == 0)
		db.executenonquery(CREATE_CIP);
}



#endif //DESURA_CIP_H
