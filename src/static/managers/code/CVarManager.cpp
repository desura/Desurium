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
#include "CVarManager.h"
#include "sqlite3x.hpp"

#ifdef WIN32
	#include <shlobj.h>
#endif

CVarManager* g_pCVarMang = NULL;


void InitCVarManger()
{
	if (!g_pCVarMang)
		g_pCVarMang = new CVarManager();

	g_pCVarMang->loadNormal();
	g_pCVarMang->loadWinUser();
}

void DestroyCVarManager()
{
	SaveCVars();
	safe_delete(g_pCVarMang);
}

void SaveCVars()
{
	g_pCVarMang->saveAll();
}



#define CREATE_CVARUSER "CREATE TABLE cvaruser(user TEXT, name TEXT, value TEXT, PRIMARY KEY (user, name));"
#define COUNT_CVARUSER "SELECT count(*) FROM sqlite_master WHERE name='cvaruser';"

#define CREATE_CVARWIN "CREATE TABLE cvarwin(user TEXT, name TEXT, value TEXT, PRIMARY KEY (user, name));"
#define COUNT_CVARWIN "SELECT count(*) FROM sqlite_master WHERE name='cvarwin';"

#define CREATE_CVAR "CREATE TABLE cvar(name TEXT PRIMARY KEY, value TEXT);"
#define COUNT_CVAR "SELECT count(*) FROM sqlite_master WHERE name='cvar';"


CVarManager::CVarManager() : BaseManager<CVar>(),
	m_uiUserId(-1),
	m_szCVarDb(UTIL::OS::getAppDataPath(L"settings_b.sqlite")),
	m_bUserLoaded(false),
	m_bWinUserLoaded(false),
	m_bNormalLoaded(false)
{
	UTIL::FS::recMakeFolder(UTIL::FS::Path(m_szCVarDb, "", true));

	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());

		if (db.executeint(COUNT_CVARUSER) == 0)
			db.executenonquery(CREATE_CVARUSER);

		if (db.executeint(COUNT_CVARWIN) == 0)
			db.executenonquery(CREATE_CVARWIN);

		if (db.executeint(COUNT_CVAR) == 0)
			db.executenonquery(CREATE_CVAR);
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to create cvar tables: {0}\n", e.what()));
	}
}

CVarManager::~CVarManager()
{
}

bool CVarManager::RegCVar(CVar* var)
{
	CVar* temp = findItem(var->getName());
	
	if (temp)
		return false;

	addItem(var);

	if ((var->getFlags() & CFLAG_USER) && m_bUserLoaded)
		loadUser(var);
	else if ((var->getFlags() & CFLAG_WINUSER) && m_bWinUserLoaded)
		loadWinUser(var);
	else if (m_bNormalLoaded)
		loadNormal(var);

	return true;
}

//if this screws up its too late any way
void  CVarManager::UnRegCVar(CVar* var)
{
	removeItem(var->getName());
}


void CVarManager::loadUser(CVar* var)
{
	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "SELECT value FROM cvaruser WHERE user=? AND name=?;");
		cmd.bind(1, (int)m_uiUserId);
		cmd.bind(2, var->getName());

		std::string value = cmd.executestring();
		var->setValueOveride(value.c_str());
	}
	catch (std::exception &)
	{
	}
}

std::wstring CVarManager::getWinUser()
{
	wchar_t username[255] = {0};
#ifdef WIN32
	DWORD size = 255;
	GetUserNameW(username, &size);
#endif
	return username;
}

void CVarManager::loadWinUser(CVar* var)
{
	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "SELECT value FROM cvarwin WHERE user=? AND name=?;");


		cmd.bind(1, getWinUser());
		cmd.bind(2, var->getName());

		std::string value = cmd.executestring();
		var->setValueOveride(value.c_str());
	}
	catch (std::exception &)
	{
	}
}

void CVarManager::loadNormal(CVar* var)
{
	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "SELECT value FROM cvar WHERE name=?;");
		cmd.bind(1, var->getName());

		std::string value = cmd.executestring();
		var->setValueOveride(value.c_str());
	}
	catch (std::exception &)
	{
	}
}



void CVarManager::loadUser(uint32 userid)
{
	m_uiUserId = userid;
	m_bUserLoaded = true;

	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "SELECT name, value FROM cvaruser WHERE user=?;");
		cmd.bind(1, (int)m_uiUserId);

		sqlite3x::sqlite3_reader cmdResults = cmd.executereader();
		loadFromDb(cmdResults);
	}
	catch (std::exception &)
	{
	}
}

void CVarManager::loadWinUser()
{
	m_bWinUserLoaded = true;

	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "SELECT name, value FROM cvarwin WHERE user=?;");

		cmd.bind(1, getWinUser());

		sqlite3x::sqlite3_reader cmdResults = cmd.executereader();
		loadFromDb(cmdResults);
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to load cvar win user: {0}\n", e.what()));
	}
}

void CVarManager::loadNormal()
{
	m_bNormalLoaded = true;

	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "SELECT name, value FROM cvar;");

		sqlite3x::sqlite3_reader cmdResults = cmd.executereader();
		loadFromDb(cmdResults);
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to load cvar normal: {0}\n", e.what()));
	}
}

void CVarManager::saveAll()
{
	saveUser();
	saveWinUser();
	saveNormal();
}

void CVarManager::saveUser()
{
	if (m_uiUserId == UINT_MAX)
		return;

	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());

		{
			sqlite3x::sqlite3_command cmd(db,"DELETE FROM cvaruser where user=?;");
			cmd.bind(1, (int)m_uiUserId);
			cmd.executenonquery();
		}

		sqlite3x::sqlite3_transaction trans(db);

		{
			sqlite3x::sqlite3_command cmd(db, "INSERT INTO cvaruser (name, value, user) VALUES (?,?,?);");
			cmd.bind(3, (int)m_uiUserId);
			saveToDb(cmd, CFLAG_USER);
		}

		trans.commit();
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to save cvar user: {0}\n", e.what()));
	}
}

void CVarManager::saveWinUser()
{
	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());

		{
			sqlite3x::sqlite3_command cmd(db,"DELETE FROM cvarwin where user=?;");
			cmd.bind(1, getWinUser());
			cmd.executenonquery();
		}

		sqlite3x::sqlite3_command cmd(db, "INSERT INTO cvarwin (name, value, user) VALUES (?,?,?);");
		cmd.bind(3, getWinUser());

		sqlite3x::sqlite3_transaction trans(db);
		saveToDb(cmd, CFLAG_WINUSER);
		trans.commit();
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to save cvar win user: {0}\n", e.what()));
	}
}

void CVarManager::saveNormal()
{
	try
	{
		sqlite3x::sqlite3_connection db(m_szCVarDb.c_str());
		db.executenonquery("DELETE FROM cvar;");

		sqlite3x::sqlite3_command cmd(db, "INSERT INTO cvar (name, value) VALUES (?,?);");

		sqlite3x::sqlite3_transaction trans(db);
		saveToDb(cmd, CFLAG_NOFLAGS);
		trans.commit();
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to save cvar normal: {0}\n", e.what()));
	}
}


void CVarManager::cleanUserCvars()
{
	for (uint32 x=0; x<getCount(); x++)
	{
		CVar* cvarNode = getItem(x);

		if (cvarNode->getFlags() & CFLAG_USER)
			cvarNode->setDefault();
	}

	m_uiUserId = -1;
}

void CVarManager::loadFromDb(sqlite3x::sqlite3_reader &reader)
{
	while (reader.read())
	{
		std::string name = reader.getstring(0);
		std::string value = reader.getstring(1);

		CVar* temp = findItem(name.c_str());

		if (temp)
			temp->setValueOveride(value.c_str());
	}
}

void CVarManager::saveToDb(sqlite3x::sqlite3_command &cmd, uint8 flags)
{
	for (uint32 x=0; x<getCount(); x++)
	{
		CVar* cvarNode = getItem(x);

		if (cvarNode->getFlags() & CFLAG_NOSAVE)
			continue;

		//dont save if user = true and usercvar = false || user = false and usercvar = true
		if ((flags & CFLAG_USER) ^ (cvarNode->getFlags() & CFLAG_USER))
			continue;

		if ((flags & CFLAG_WINUSER) ^ (cvarNode->getFlags() & CFLAG_WINUSER))
			continue;

		//dont save if setting hasnt changed
		if (!cvarNode->getExitString() || (strcmp(cvarNode->getExitString(), cvarNode->getDefault())) == 0)
			continue;

		cmd.bind(1, std::string(cvarNode->getName()));
		cmd.bind(2, std::string(cvarNode->getExitString()));
		cmd.executenonquery();
	}
}

CVar* CVarManager::findCVar(const char* name)
{
	return findItem(name);
}

void CVarManager::getCVarList(std::vector<CVar*> &vList)
{
	for (uint32 x=0; x<getCount(); x++)
		vList.push_back(getItem(x));
}
