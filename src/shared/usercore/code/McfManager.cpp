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
#include "McfManager.h"

#include "sqlite3x.hpp"

#include "mcfcore/MCFHeaderI.h"
#include "mcfcore/MCFMain.h"

#include "usercore/ItemInfoI.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace bfs = boost::filesystem;

#define COUNT_MCFITEM "SELECT count(*) FROM sqlite_master WHERE name='mcfitem';"
#define CREATE_MCFITEM "CREATE TABLE mcfitem(internalid INTEGER, mcfbuild INTEGER, path TEXT, branch INTEGER, flags INTEGER, PRIMARY KEY (internalid, mcfbuild, branch));"

#define COUNT_MCFBACKUP "SELECT count(*) FROM sqlite_master WHERE name='mcfbackup';"
#define CREATE_MCFBACKUP "CREATE TABLE mcfbackup(gid INTEGER, mid INTEGER, path TEXTl, PRIMARY KEY (gid, mid));"


#define MCF_DB "mcfstoreb.sqlite"

enum
{
	FLAG_NONE = 0,
	FLAG_PATCH,
	FLAG_UNAUTHED,
};





bool McfInfoSort ( mcfInfo* elem1, mcfInfo* elem2 )
{
	if (!elem1)
		return false;

	if (!elem2)
		return true;

   return elem1->version > elem2->version;
}







namespace UserCore
{

MCFManager* g_pMCFManager = NULL;

void InitMCFManager(const char* appDataPath, const char* mcfDataPath)
{
	g_pMCFManager = new MCFManager(appDataPath, mcfDataPath);
	g_pMCFManager->init();
}

void DelMCFManager()
{
	safe_delete(g_pMCFManager);
}

MCFManager* GetMCFManager()
{
	return g_pMCFManager;
}





MCFManager::MCFManager(const char* appDataPath, const char* mcfDataPath)
	: m_szAppDataPath(appDataPath)
	, m_szMCFSavePath(mcfDataPath)
{
}

void MCFManager::init()
{
	createMcfDbTables(m_szAppDataPath.c_str());
	migrateOldFiles();
}

class MigrateInfo
{
public:
	DesuraId id;
	MCFBuild build;
	MCFBranch branch;
	gcString path;
	gcString newPath;
};

void MCFManager::getListOfBadMcfPaths(const gcString &szItemDb, std::vector<MigrateInfo> &delList, std::vector<MigrateInfo> &updateList)
{
	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		getListOfBadMcfPaths(db, delList, updateList);
	}
	catch (std::exception &)
	{
	}
}

void MCFManager::getListOfBadMcfPaths(sqlite3x::sqlite3_connection &db, std::vector<MigrateInfo> &delList, std::vector<MigrateInfo> &updateList)
{
	try
	{
		sqlite3x::sqlite3_command cmd(db, "SELECT * FROM mcfitem");
		sqlite3x::sqlite3_reader reader = cmd.executereader();

		while (reader.read())
		{
			gcString path = reader.getstring(2);

			if (path.find(m_szAppDataPath) != 0)
			{
				MigrateInfo mi;

				mi.id = DesuraId(reader.getint64(0));
				mi.build = MCFBuild::BuildFromInt(reader.getint(1));
				mi.branch = MCFBranch::BranchFromInt(reader.getint(3));

				gcString newPath = generatePath(mi.id, mi.branch, mi.build, false);

				if (newPath == path)
					continue;

				mi.path = path;
				mi.newPath = newPath;

				uint32 flags = reader.getint(4);

				if (HasAnyFlags(flags, FLAG_UNAUTHED))
					delList.push_back(mi);
				else
					updateList.push_back(mi);
			}
		}
	}
	catch (std::exception &)
	{
	}
}

void MCFManager::migrateOldFiles()
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());
	std::string res;

	std::vector<MigrateInfo> delList;
	std::vector<MigrateInfo> updateList;

	getListOfBadMcfPaths(szItemDb, delList, updateList);


	for (size_t x=0; x<delList.size(); x++)
	{
		delMcfPath(delList[x].id, delList[x].branch, delList[x].build);
		UTIL::FS::delFile(delList[x].path.c_str());
	}

	for (size_t x=0; x<updateList.size(); x++)
	{
		gcString newPath = updateList[x].newPath;

		UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(newPath));
		UTIL::FS::moveFile(updateList[x].path.c_str(), newPath.c_str());

		delMcfPath(updateList[x].id, updateList[x].branch, updateList[x].build);

		if (UTIL::FS::isValidFile(newPath.c_str()))
		{
			try
			{
				int flags = FLAG_NONE;

				sqlite3x::sqlite3_connection db(szItemDb.c_str());
				sqlite3x::sqlite3_command cmd(db, "INSERT INTO mcfitem VALUES (?,?,?,?,?);");
				cmd.bind(1, (long long int)updateList[x].id.toInt64());
				cmd.bind(2, (int)updateList[x].build);
				cmd.bind(3, UTIL::OS::getRelativePath(newPath));
				cmd.bind(4, (int)updateList[x].branch);
				cmd.bind(5, (int)flags);

				cmd.executenonquery();
			}
			catch (std::exception)
			{
			}			
		}
		else
		{
			UTIL::FS::delFile(updateList[x].path.c_str());
		}
	}
}

gcString MCFManager::getMcfDb(const char* dataPath)
{
	return gcString("{0}{1}{2}", dataPath, DIRS_STR, MCF_DB);
}

void MCFManager::createMcfDbTables(const char* dataPath)
{
	sqlite3x::sqlite3_connection db(getMcfDb(dataPath).c_str());

	try
	{
		if (db.executeint(COUNT_MCFITEM) == 0)
		{
			db.executenonquery(CREATE_MCFITEM);
			scanForMcf();
		}
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to create mcf item table: {0}\n", e.what()));
	}

	try
	{
		if (db.executeint(COUNT_MCFBACKUP) == 0)
			db.executenonquery(CREATE_MCFBACKUP);
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to create mcf backup table: {0}\n", e.what()));
	}
}

gcString MCFManager::getMcfPath(UserCore::Item::ItemInfoI* item, bool isUnAuthed)
{
	if (!item || !item->getCurrentBranch())
		return "";

	return getMcfPath(item->getId(), item->getCurrentBranch()->getBranchId(), item->getInstalledBuild(), isUnAuthed);
}

void MCFManager::getAllMcfPaths(DesuraId id, std::vector<McfPathData> &vList)
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());
	std::string res;

	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());

		sqlite3x::sqlite3_command cmd(db, "SELECT path, mcfbuild, branch FROM mcfitem WHERE internalid=?");
		cmd.bind(1, (long long int)id.toInt64());
		
		sqlite3x::sqlite3_reader reader = cmd.executereader();

		while (reader.read())
		{
			McfPathData mpd;

			mpd.path = UTIL::OS::getAbsPath(reader.getstring(0));
			mpd.build = MCFBuild::BuildFromInt(reader.getint(1));
			mpd.branch = MCFBranch::BranchFromInt(reader.getint(2));

			vList.push_back(mpd);
		}
	}
	catch (std::exception &)
	{
	}
}

gcString MCFManager::getMcfPath(DesuraId id, MCFBranch branch, MCFBuild build, bool isUnAuthed)
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());
	std::string res;

	try
	{
		int flags = isUnAuthed?FLAG_UNAUTHED:FLAG_NONE;

		sqlite3x::sqlite3_connection db(szItemDb.c_str());

		if (flags == FLAG_NONE)
		{
			sqlite3x::sqlite3_command cmd(db, "SELECT path FROM mcfitem WHERE internalid=? AND mcfbuild=? AND branch=?;");
			cmd.bind(1, (long long int)id.toInt64());
			cmd.bind(2, (int)build);
			cmd.bind(3, (int)branch);
			res = cmd.executestring();
		}
		else
		{
			sqlite3x::sqlite3_command cmd(db, "SELECT path FROM mcfitem WHERE internalid=? AND mcfbuild=? AND branch=? AND flags & ?;");
			cmd.bind(1, (long long int)id.toInt64());
			cmd.bind(2, (int)build);
			cmd.bind(3, (int)branch);
			cmd.bind(4, (int)flags);
			res = UTIL::OS::getAbsPath(cmd.executestring());
		}
	}
	catch (std::exception &)
	{
	}

	return res;
}

gcString MCFManager::newMcfPath(DesuraId id, MCFBranch branch, MCFBuild build, bool isUnAuthed)
{
	gcString curPath = getMcfPath(id, branch, build, isUnAuthed);

	if (curPath != "")
		return curPath;

	curPath = generatePath(id, branch, build, isUnAuthed);
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());

	try
	{
		int flags = isUnAuthed?FLAG_UNAUTHED:FLAG_NONE;

		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "INSERT INTO mcfitem VALUES (?,?,?,?,?);");
		cmd.bind(1, (long long int)id.toInt64());
		cmd.bind(2, (int)build);
		cmd.bind(3, UTIL::OS::getRelativePath(curPath));
		cmd.bind(4, (int)branch);
		cmd.bind(5, (int)flags);

		cmd.executenonquery();
	}
	catch (std::exception &e)
	{
		curPath = "";
		Warning(gcString("Failed to insert mcf item: {0}\n", e.what()));
	}

	return curPath;
}


void MCFManager::delMcfPath(DesuraId id, MCFBranch branch, MCFBuild build)
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());

	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "DELETE FROM mcfitem WHERE internalid=? AND mcfbuild=? AND branch=?;");
		cmd.bind(1, (long long int)id.toInt64());
		cmd.bind(2, (int)build);
		cmd.bind(3, (int)branch);
		cmd.executenonquery();
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to delete mcf item: {0}\n", e.what()));
	}
}


void MCFManager::delAllMcfPath(DesuraId id)
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());
	sqlite3x::sqlite3_connection db(szItemDb.c_str());

	try
	{
		sqlite3x::sqlite3_command cmd(db, "SELECT path FROM mcfitem WHERE internalid=?;");
		cmd.bind(1, (long long int)id.toInt64());
		sqlite3x::sqlite3_reader reader = cmd.executereader();

		while (reader.read())
		{
			std::string path = reader.getstring(0);
			UTIL::FS::delFile(UTIL::FS::PathWithFile(path));
		}
	}
	catch (std::exception &)
	{
	}

	try
	{
		sqlite3x::sqlite3_command cmd(db, "DELETE FROM mcfitem WHERE internalid=?;");
		cmd.bind(1, (long long int)id.toInt64());
		cmd.executenonquery();
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to delete mcf items: {0}\n", e.what()));
	}
}



void MCFManager::properDelMcfBackup(DesuraId gid, DesuraId mid)
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());
	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "DELETE FROM mcfbackup WHERE gid=? AND mid=?;");
		cmd.bind(1, (long long int)gid.toInt64());
		cmd.bind(2, (long long int)mid.toInt64());
		cmd.executenonquery();
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to delete mcf backup: {0}\n", e.what()));
	}
}

gcString MCFManager::getMcfBackup(DesuraId gid, DesuraId mid)
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());
	std::string res;

	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "SELECT path FROM mcfbackup WHERE gid=? AND mid=?;");
		cmd.bind(1, (long long int)gid.toInt64());
		cmd.bind(2, (long long int)mid.toInt64());
		res = UTIL::OS::getRelativePath(cmd.executestring());

		if (!UTIL::FS::isValidFile(UTIL::FS::PathWithFile(res)))
		{
			properDelMcfBackup(gid, mid);
			res = "";
		}
	}
	catch (std::exception &)
	{
	}

	return res;
}

gcString MCFManager::newMcfBackup(DesuraId gid, DesuraId mid)
{
	gcString curPath = getMcfBackup(gid, mid);

	if (curPath != "")
		return curPath;

	gcString parPath;
	gcString parFile;

	size_t x=0;


	do
	{


		if (x==0)
			parFile = gcString("backup_{0}.mcf", mid.toInt64());
		else
			parFile = gcString("backup_{0}_{1}.mcf", mid.toInt64(), x);

		parPath = gcString("{0}{1}{2}", m_szMCFSavePath, DIRS_STR, gid.getFolderPathExtension(parFile.c_str()));
		x++;
	}
	while ( UTIL::FS::isValidFile(UTIL::FS::PathWithFile(parPath)) );

	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());

	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "INSERT INTO mcfbackup VALUES (?,?,?);");
		cmd.bind(1, (long long int)gid.toInt64());
		cmd.bind(2, (long long int)mid.toInt64());
		cmd.bind(3, UTIL::OS::getRelativePath(parPath));

		cmd.executenonquery();
	}
	catch (std::exception &e)
	{
		Warning(gcString("Failed to update mcf backup: {0}\n", e.what()));
	}

	return parPath;
}

void MCFManager::delMcfBackup(DesuraId gid, DesuraId mid)
{
	gcString curPath = getMcfBackup(gid, mid);
	UTIL::FS::delFile(UTIL::FS::PathWithFile(curPath));

	properDelMcfBackup(gid, mid);
}




void MCFManager::getMCFFiles(std::vector<mcfInfo*> &validFiles, DesuraId id)
{
	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());

	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());

		sqlite3x::sqlite3_command cmd(db, "SELECT path, mcfbuild FROM mcfitem WHERE internalid=?;");
		cmd.bind(1, (long long int)id.toInt64());

		sqlite3x::sqlite3_reader reader = cmd.executereader();

		while (reader.read())
		{
			std::string p = UTIL::OS::getAbsPath(reader.getstring(0));
			uint32 v = reader.getint(1);

			validFiles.push_back(new mcfInfo(p.c_str(), v, MCFBranch()));
		}
	}
	catch (std::exception &)
	{
	}
}

gcString MCFManager::generatePath(DesuraId id, MCFBranch branch, MCFBuild build, bool isUnAuthed)
{
	gcString fileName("b{0}_m{1}.mcf", branch, build);

	if (isUnAuthed)
		fileName = gcString("b{0}_m{1}_unauthed.mcf", branch, build);

	return gcString("{0}{1}{2}", m_szMCFSavePath, DIRS_STR, id.getFolderPathExtension(fileName.c_str()));
}

void MCFManager::scanForMcf()
{
	recScanForMcf(gcString("{0}{1}mods", m_szMCFSavePath, DIRS_STR).c_str());
	recScanForMcf(gcString("{0}{1}games", m_szMCFSavePath, DIRS_STR).c_str());
}

void MCFManager::recScanForMcf(const char* root)
{
	if (!root)
		return;

	std::vector<UTIL::FS::Path> fileList;
	std::vector<std::string> extList;

	extList.push_back("mcf");

	UTIL::FS::getAllFiles(UTIL::FS::PathWithFile(root), fileList, &extList);

	for (size_t x=0; x<fileList.size(); x++)
	{
		validateMcf(fileList[x].getFullPath().c_str());
	}
}

void MCFManager::validateMcf(const char* mcf)
{
	if (!UTIL::FS::isValidFile(UTIL::FS::PathWithFile(mcf)))
		return;

	McfHandle mcfHandle;
	mcfHandle->setFile(mcf);

	try
	{
		mcfHandle->parseMCF();
	}
	catch (gcException &)
	{
		return;
	}

	MCFCore::MCFHeaderI *mcfHead = mcfHandle->getHeader();

	DesuraId id = mcfHead->getDesuraId();
	MCFBuild build = mcfHead->getBuild();

	bool isPatch = strstr(mcf, "patch")?true:false;
	bool isUnAuthed = strstr(mcf, "unauth")?true:false;
	int flags = (isPatch?FLAG_PATCH:FLAG_NONE) + (isUnAuthed?FLAG_UNAUTHED:FLAG_NONE);

	gcString szItemDb = getMcfDb(m_szAppDataPath.c_str());

	try
	{
		sqlite3x::sqlite3_connection db(szItemDb.c_str());
		sqlite3x::sqlite3_command cmd(db, "INSERT INTO mcfitem VALUES (?,?,?,?,?);");
		cmd.bind(1, (long long int)id.toInt64());
		cmd.bind(2, (int)build);
		cmd.bind(3, UTIL::OS::getRelativePath(mcf));
		cmd.bind(4, (int)flags);

		cmd.executenonquery();
	}
	catch (std::exception &)
	{
	}
}

gcString MCFManager::getMcfSavePath()
{
	return m_szMCFSavePath;
}

}

#ifdef WITH_GTEST

#include <gtest/gtest.h>

namespace UnitTest
{
	class MCFManagerFixture : public ::testing::Test
	{
	public:
		MCFManagerFixture()
			: m_MCFManager("appdata", "mcfroot")
		{
		}

		gcString generatePath(DesuraId id, MCFBranch branch, MCFBuild build, bool isUnAuthed)
		{
			return m_MCFManager.generatePath(id, branch, build, isUnAuthed);
		}

		void getListOfBadMcfPaths(sqlite3x::sqlite3_connection &db, std::vector<UserCore::MigrateInfo> &delList, std::vector<UserCore::MigrateInfo> &updateList)
		{
			m_MCFManager.getListOfBadMcfPaths(db, delList, updateList);
		}

		UserCore::MCFManager m_MCFManager;
	};

	TEST_F(MCFManagerFixture, generatePath_authed)
	{
		DesuraId id("1", "games");
		gcString strPath = generatePath(id, MCFBranch::BranchFromInt(2), MCFBuild::BuildFromInt(3), false);

#ifdef WIN32
		ASSERT_STREQ("mcfroot\\games\\1\\b2_m3.mcf", strPath.c_str());
#else
		ASSERT_STREQ("mcfroot/games/1/b2_m3.mcf", strPath.c_str());
#endif
	}

	TEST_F(MCFManagerFixture, generatePath_unauthed)
	{
		DesuraId id("1", "games");
		gcString strPath = generatePath(id, MCFBranch::BranchFromInt(2), MCFBuild::BuildFromInt(3), true);

#ifdef WIN32
		ASSERT_STREQ("mcfroot\\games\\1\\b2_m3_unauthed.mcf", strPath.c_str());
#else
		ASSERT_STREQ("mcfroot/games/1/b2_m3_unauthed.mcf", strPath.c_str());
#endif
	}

	TEST_F(MCFManagerFixture, getListOfBadMcfPaths)
	{
		sqlite3x::sqlite3_connection db;

		auto insertItem = [&db](DesuraId id, gcString strPath, bool bUnAuthed) -> void 
		{
			try
			{
				sqlite3x::sqlite3_command cmd(db, "INSERT INTO mcfitem VALUES (?,?,?,?,?);");
				cmd.bind(1, (long long int)id.toInt64());
				cmd.bind(2, 0);
				cmd.bind(3, strPath);
				cmd.bind(4, bUnAuthed ? FLAG_UNAUTHED:0);

				cmd.executenonquery();
			}
			catch (std::exception &)
			{
			}
		};

		insertItem(DesuraId("1", "games"), "mcfroot//test1.mcf", true);
		insertItem(DesuraId("2", "games"), "abcd//test2.mcf", false);
		insertItem(DesuraId("3", "games"), "appdata//test3.mcf", false);
		insertItem(DesuraId("4", "games"), "mcfroot//test4.mcf", false);

		std::vector<UserCore::MigrateInfo> delList;
		std::vector<UserCore::MigrateInfo> updateList;

		getListOfBadMcfPaths(db, delList, updateList);

		ASSERT_EQ(1, delList.size());
		ASSERT_EQ(DesuraId("1", "games"), delList[0].id);

		ASSERT_EQ(2, updateList.size());
		ASSERT_EQ(DesuraId("2", "games"), updateList[0].id);
		ASSERT_EQ(DesuraId("3", "games"), updateList[1].id);
	}
}


#endif
