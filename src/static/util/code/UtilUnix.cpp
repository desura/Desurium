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

#include "UtilUnix.h"

#include <sqlite3x.hpp>

#define COUNT_CONFIGTABLE_STRING "SELECT count(*) FROM sqlite_master WHERE name='config_string';"
#define CREATE_CONFIGTABLE_STRING "CREATE TABLE config_string(key TEXT primary key, value TEXT);"

// the non official build will use generated files by cmake, so we don't need a version file at all
// see UtilOs_cmake.cpp.inc
#ifdef DESURA_OFFICAL_BUILD
	extern std::string GetAppBuild()
	{
		FILE* fh = fopen("version", "r");

		if (!fh)
			return "";
	
		int appid = 0;
		int build = 0;

		fscanf(fh, "BRANCH=%d\nBUILD=%d", &appid, &build);
		fclose(fh);

		return gcString("{0}", build);
	}

	extern std::string GetAppBranch()
	{
		FILE* fh = fopen("version", "r");

		if (!fh)
			return "";
	
		int appid = 0;
		int build = 0;

		fscanf(fh, "BRANCH=%d\nBUILD=%d", &appid, &build);
		fclose(fh);

		return gcString("{0}", appid);
	}
#else
std::string GetAppBuild();
std::string GetAppBranch();
#endif

inline const wchar_t* CONFIG_DB(void)
{
	return UTIL::OS::getAppDataPath(L"linux_registry.sqlite").c_str();
}

static void dbCreateTables()
{
	try
	{
		sqlite3x::sqlite3_connection db(CONFIG_DB());
		
		if (db.executeint(COUNT_CONFIGTABLE_STRING) == 0)
			db.executenonquery(CREATE_CONFIGTABLE_STRING);

/*		if (db.executeint(COUNT_CONFIGTABLE_BLOB) == 0)
			db.executenonquery(CREATE_CONFIGTABLE_BLOB);*/
	}
	catch (std::exception &e)
	{
		ERROR_OUTPUT(gcString("Failed to create config table: \"{0}\"!", e.what()).c_str());
	}
}

namespace UTIL
{
namespace UNIX
{
void SetAppBuild(const std::string &val)
{
	std::string branch = GetAppBranch();

	FILE* fh = fopen("version", "w");

	if (!fh)
		return;

	fprintf(fh, "BRANCH=%s\nBUILD=%s", branch.c_str(), val.c_str());
	fclose(fh);
}

void SetAppBranch(const std::string &val)
{
	std::string build = GetAppBuild();

	FILE* fh = fopen("version", "w");

	if (!fh)
		return;

	fprintf(fh, "BRANCH=%s\nBUILD=%s", val.c_str(), build.c_str());
	fclose(fh);
}

std::string getConfigValue(const std::string &configKey)
{
	if (configKey == APPBUILD)
		return GetAppBuild();
	else if (configKey == APPID)
		return GetAppBranch();
	else if (configKey.find("HKEY_") != std::string::npos)
		return "";

	std::string result;

	try
	{
		dbCreateTables();
		sqlite3x::sqlite3_connection db(CONFIG_DB());	
		
		sqlite3x::sqlite3_command cmd(db, "SELECT value FROM config_string WHERE key=?;");
		cmd.bind(1, configKey);
		result = cmd.executestring();
	}
	catch(std::exception &e)
	{
		result = "";
		ERROR_OUTPUT(gcString("Failed to select value for key \"{0}\": \"{1}\"!", configKey.c_str(), e.what()).c_str());
	}

	return result;
}

void setConfigValue(const std::string &configKey, uint32 value)
{
	setConfigValue(configKey, gcString("{0}", value));
}

void setConfigValue(const std::string &configKey, const std::string &value)
{
	if (configKey == APPBUILD)
	{
		UTIL::UNIX::SetAppBuild(value);
		return;
	}
	else if (configKey == APPID)
	{
		UTIL::UNIX::SetAppBranch(value);
		return;	
	}

	dbCreateTables();

	sqlite3x::sqlite3_connection db(CONFIG_DB());

	ERROR_OUTPUT(gcString("Setting key \"{0}\" to value \"{1}\"", configKey, value).c_str());

	if(getConfigValue(configKey).length() > 0)
	{
		try
		{
			sqlite3x::sqlite3_command cmd(db, "UPDATE config_string SET value=? WHERE key=?;");
			cmd.bind(1, value);
			cmd.bind(2, configKey);
			cmd.executenonquery();
		}
		catch(std::exception &e)
		{
			ERROR_OUTPUT(gcString("Failed to update key \"{0}\" with value \"{1}\": \"{2}\"!\n", configKey, value, e.what()).c_str());
		}
	}
	else
	{
		try
		{
			sqlite3x::sqlite3_command cmd(db, "INSERT INTO config_string VALUES(?,?);");
			cmd.bind(1, configKey);
			cmd.bind(2, value);
			cmd.executenonquery();
		}
		catch(std::exception &e)
		{
			ERROR_OUTPUT(gcString("Failed to insert key \"{0}\" with value \"{1}\": \"{2}\"!\n", configKey, value, e.what()).c_str());
		}
	}
}

}
}
