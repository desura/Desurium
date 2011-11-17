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
#include "sqlite3x.hpp"

#include "sql/ItemInfoSql.h"

#ifdef NIX
	#define localtime_s(a, b) localtime_r( (b), (a) )
#endif

gcString g_szAppDataPath;

void SetAppDataPath(const char* path)
{
	g_szAppDataPath = path;
}

class UninstallInfo
{
public:
	DesuraId id;

	gcString version;
	gcString displayName;
	gcString installDir;
	gcString icon;
	gcString developer;
	gcString profile;

	uint32 build;
	uint32 branch;
};

#ifdef WIN32
void CreateIco(DesuraId id, std::string &icon)
{
	std::string desuraInstallPath = UTIL::OS::getConfigValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\InstallPath");

	UTIL::FS::Path path(desuraInstallPath, "", false);

	if (!UTIL::FS::isValidFolder(path))
		path = UTIL::FS::Path(g_szAppDataPath.c_str(), "", false);

	path += "icons";
	path += UTIL::FS::File(gcString("{0}.ico", id.toInt64()));

	UTIL::FS::recMakeFolder(path);

	if (UTIL::MISC::convertToIco(icon, path.getFullPath()))
		icon = path.getFullPath();
}
#endif

bool GetUninstallInfo(DesuraId id, UninstallInfo &info)
{
	info.id = id;

	try
	{
		sqlite3x::sqlite3_connection db(getItemInfoDb(g_szAppDataPath.c_str()).c_str());

		{
			sqlite3x::sqlite3_command cmd(db, "SELECT developer, name, profile, icon, installpath, iprimpath, ibranch, ibuild FROM iteminfo WHERE internalid=?;");
			cmd.bind(1, (long long int)id.toInt64());
			sqlite3x::sqlite3_reader reader = cmd.executereader();
	
			reader.read();

			info.developer	= gcString(reader.getstring(0));
			info.displayName= gcString(reader.getstring(1));
			info.profile	= gcString(reader.getstring(2));
			info.icon		= gcString(reader.getstring(3));
			info.installDir	= gcString(reader.getstring(4));
			
			std::string primePath = gcString(reader.getstring(5));

			if (primePath.size() > 0)
				info.installDir = primePath;

			info.branch		= MCFBranch::BranchFromInt(reader.getint(6));
			info.build		= MCFBuild::BuildFromInt(reader.getint(7));
		}

		if (info.branch == 0 || info.build == 0)
			return false;

#ifdef WIN32
		if (UTIL::FS::isValidFile(info.icon))
			CreateIco(id, info.icon);
#endif

		{
			sqlite3x::sqlite3_command cmd(db, "SELECT name FROM branchinfo WHERE branchid=? AND internalid=?;");
			cmd.bind(1, (int)info.branch);
			cmd.bind(2, (long long int)id.toInt64());

			sqlite3x::sqlite3_reader reader = cmd.executereader();
	
			reader.read();
			info.version	= gcString(reader.getstring(0));
		}
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to get item {1} for uninstall update: {0}\n", e.what(), id.toInt64()));
	}


	return true;
}

void GetListOfItems(std::vector<DesuraId> &list)
{
	try
	{
		sqlite3x::sqlite3_connection db(getItemInfoDb(g_szAppDataPath.c_str()).c_str());

		sqlite3x::sqlite3_command cmd(db, "SELECT internalid FROM iteminfo;");
		sqlite3x::sqlite3_reader reader = cmd.executereader();
	
		while (reader.read())
		{
			list.push_back(DesuraId(reader.getint64(0)));
		}
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to get list of items for uninstall update: {0}\n", e.what()));
	}
}



void SetUninstallRegKey(UninstallInfo &info, uint64 installSize)
{
	gcString base("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura_{0}\\", info.id.toInt64());

	gcString name		= base + "DisplayName";
	gcString version	= base + "DisplayVersion";
	gcString publisher	= base + "Publisher";

	gcString vMinor		= base + "VersionMinor";
	gcString vMajor		= base + "VersionMajor";

	gcString about		= base + "URLInfoAbout";
	gcString help		= base + "HelpLink";
	gcString path		= base + "InstallLocation";
	gcString size		= base + "EstimatedSize";

	gcString uninstall	= base + "UninstallString";
	gcString verify		= base + "ModifyPath";
	gcString noModify	= base + "NoModify";

	gcString company	= base + "RegCompany";
	gcString icon		= base + "DisplayIcon";

	gcString date		= base + "InstallDate";

	time_t rawtime;
	struct tm timeinfo;

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	gcString today("{0}{1}{2}", (1900 + timeinfo.tm_year), timeinfo.tm_mon+1, timeinfo.tm_mday);

	std::string desuraExe = UTIL::OS::getConfigValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\InstallPath");

#ifdef WIN32
#ifdef DEBUG
	desuraExe += "\\desura-d.exe";
#else
	desuraExe += "\\desura.exe";
#endif
#else
	desuraExe += "/desura";
#endif

	if (!UTIL::FS::isValidFile(info.icon))
		info.icon = desuraExe;

	gcString uninstallExe("\"{0}\" desura://uninstall/{1}/{2}", desuraExe, info.id.getTypeString(), info.id.getItem());
	gcString verifyExe("\"{0}\" desura://verify/{1}/{2}", desuraExe, info.id.getTypeString(), info.id.getItem());

	UTIL::OS::setConfigValue(name, "Desura: " + info.displayName);
	UTIL::OS::setConfigValue(version, info.version);
	UTIL::OS::setConfigValue(publisher, info.developer);

	UTIL::OS::setConfigValue(vMinor, info.build);
	UTIL::OS::setConfigValue(vMajor, info.branch);

	UTIL::OS::setConfigValue(about, info.profile);
	UTIL::OS::setConfigValue(help, "http://www.desura.com/groups/desura/forum");
	UTIL::OS::setConfigValue(path, info.installDir);

	if (installSize != 0)
		UTIL::OS::setConfigValue(size, (uint32)(installSize/1024));

	UTIL::OS::setConfigValue(uninstall, uninstallExe, true);
	UTIL::OS::setConfigValue(verify, verifyExe, true);
	UTIL::OS::setConfigValue(noModify, 1);

	UTIL::OS::setConfigValue(company, info.developer);
	UTIL::OS::setConfigValue(icon, info.icon);

	UTIL::OS::setConfigValue(date, today);
}



void RemoveUninstallRegKey(DesuraId id)
{
#ifdef WIN32
	gcString regKey("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Desura_{0}", id.toInt64());
	UTIL::WIN::delRegTree(regKey.c_str());
#endif
}

bool SetUninstallRegKey(DesuraId id, uint64 installSize)
{
#ifdef WIN32
	UninstallInfo info;

	if (!GetUninstallInfo(id, info))
		return false;

	SetUninstallRegKey(info, installSize);
	return true;
#else
	return false;
#endif
}

void UpdateAllUninstallRegKey()
{
#ifdef WIN32
	std::vector<DesuraId> list;
	GetListOfItems(list);

	std::vector<std::string> regKeys;
	UTIL::WIN::getAllRegKeys("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", regKeys);

	std::vector<DesuraId> processed;

	UTIL::MISC::loadImgLib();

	for (size_t x=0; x<regKeys.size(); x++)
	{
		std::string key = regKeys[x];

		size_t pos = key.find("Desura_");

		if (pos != 0)
			continue;

		std::string szId = key.substr(7, -1);

		DesuraId id( UTIL::MISC::atoll(szId.c_str()) );

		if (!id.isOk())
			continue;

		processed.push_back(id);
		bool del = true;

		for (size_t y=0; y<list.size(); y++)
		{
			if (id == list[y])
			{
				del = false;
				break;
			}
		}

		if (!del)
			del = !SetUninstallRegKey(id, 0);

		if (del)
			RemoveUninstallRegKey(id);	
	}


	for (size_t x=0; x<list.size(); x++)
	{
		bool skip = false;

		for (size_t y=0; y<processed.size(); y++)
		{
			if (list[x] == processed[y])
			{
				skip = true;
				break;
			}
		}

		if (skip)
			continue;

		SetUninstallRegKey(list[x], 0);
	}

	UTIL::MISC::unloadImgLib();
#endif

}
