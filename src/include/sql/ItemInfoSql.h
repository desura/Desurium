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


#define COUNT_ITEMINFO "select count(*) from sqlite_master where name='iteminfo';"
#define CREATE_ITEMINFO "create table iteminfo(	internalid INTEGER PRIMARY KEY, "		\
												"parentid INTEGER DEFAULT 0,"			\
												"percent INTEGER,"						\
												"statusflags INTEGER,"					\
												"rating TEXT,"							\
																						\
												"developer TEXT,"						\
												"name TEXT,"							\
												"shortname TEXT,"						\
												"profile TEXT,"							\
												"devprofile TEXT,"						\
																						\
												"icon TEXT,"							\
												"iconurl TEXT,"							\
												"logo TEXT,"							\
												"logourl TEXT,"							\
																						\
												"publisher TEXT,"						\
												"pubprofile TEXT,"						\
																						\
												"ibranch INTEGER,"						\
												"lastbranch INTEGER"					\
												");"


#define COUNT_BRANCHINFO "select count(*) from sqlite_master where name='branchinfo';"
#define CREATE_BRANCHINFO "create table branchinfo(	branchid INTEGER PRIMARY KEY, "		\
												"internalid INTEGER,"					\
												"name TEXT,"							\
												"flags INTEGER,"						\
												"eula TEXT,"							\
												"euladate TEXT,"						\
												"preorderdate TEXT,"					\
												"cdkey TEXT,"							\
												"installscript TEXT,"					\
												"installscriptCRC INTEGER,"				\
												"globalid INTEGER,"						\
												"biid INTEGER"							\
												");"

#define COUNT_RECENT "select count(*) from sqlite_master where name='recent';"
#define CREATE_RECENT "create table recent(	internalid INTEGER, "						\
												"userid INTEGER,"						\
												"time DATETIME,"						\
												"PRIMARY KEY (internalid, userid, time)"\
												");"

#define COUNT_NEWITEMS "select count(*) from sqlite_master where name='newItems';"
#define CREATE_NEWITEMS "create table newItems(	internalid INTEGER, "						\
												"userid INTEGER,"						\
												"time DATETIME,"						\
												"PRIMARY KEY (internalid, userid)"\
												");"

#define COUNT_FAVORITE "select count(*) from sqlite_master where name='favorite';"
#define CREATE_FAVORITE "create table favorite(	internalid INTEGER, "					\
												"userid INTEGER,"						\
												"PRIMARY KEY (internalid, userid)"		\
												");"


#define COUNT_TOOLS "select count(*) from sqlite_master where name='tools';"
#define CREATE_TOOLS "create table tools(	branchid INTEGER, "						\
												"toolid INTEGER,"						\
												"PRIMARY KEY (branchid, toolid)"		\
												");"

#define COUNT_EXE "select count(*) from sqlite_master where name='exe';"
#define CREATE_EXE "create table exe(	itemid INTEGER, "								\
												"biid INTEGER,"							\
												"name TEXT,"							\
												"exe TEXT,"								\
												"exeargs TEXT,"							\
												"userargs TEXT,"						\
												"rank INTEGER,"							\
												"PRIMARY KEY (itemid, biid, name)"		\
												");"

#define COUNT_INSTALLINFO "select count(*) from sqlite_master where name='installinfo';"
#define CREATE_INSTALLINFO "create table installinfo(	itemid INTEGER, "				\
												"biid INTEGER,"							\
												"installpath TEXT,"						\
												"installcheck TEXT,"					\
												"iprimpath TEXT,"						\
												"imod INTEGER,"							\
												"ibuild INTEGER,"						\
												"lastbuild INTEGER,"					\
												"PRIMARY KEY (itemid, biid)"			\
												");"


#define COUNT_INSTALLINFOEX "select count(*) from sqlite_master where name='installinfoex';"
#define CREATE_INSTALLINFOEX "create table installinfoex(itemid INTEGER, "				\
												"biid INTEGER,"							\
												"installcheck TEXT,"					\
												"PRIMARY KEY (itemid, biid, installcheck)"	\
												");"


#define ITEMINFO_DB "iteminfo_d.sqlite"

inline gcString getItemInfoDb(const char* appDataPath)
{
	return gcString("{0}{2}{1}", appDataPath, ITEMINFO_DB, DIRS_STR);
}

inline void trycatch(sqlite3x::sqlite3_connection &db, const char* sql)
{
	try
	{
		db.executenonquery(sql);
	}
	catch (std::exception)
	{
	}
}

inline void createItemInfoDbTables(const char* appDataPath)
{
	sqlite3x::sqlite3_connection db(getItemInfoDb(appDataPath).c_str());

	if (db.executeint(COUNT_ITEMINFO) == 0)
		db.executenonquery(CREATE_ITEMINFO);

	if (db.executeint(COUNT_BRANCHINFO) == 0)
		db.executenonquery(CREATE_BRANCHINFO);

	if (db.executeint(COUNT_RECENT) == 0)
		db.executenonquery(CREATE_RECENT);

	if (db.executeint(COUNT_NEWITEMS) == 0)
		db.executenonquery(CREATE_NEWITEMS);

	if (db.executeint(COUNT_FAVORITE) == 0)
		db.executenonquery(CREATE_FAVORITE);

	if (db.executeint(COUNT_TOOLS) == 0)
		db.executenonquery(CREATE_TOOLS);

	if (db.executeint(COUNT_EXE) == 0)
		db.executenonquery(CREATE_EXE);

	if (db.executeint(COUNT_INSTALLINFO) == 0)
		db.executenonquery(CREATE_INSTALLINFO);

	if (db.executeint(COUNT_INSTALLINFOEX) == 0)
		db.executenonquery(CREATE_INSTALLINFOEX);
}

#endif //DESURA_CIP_H
