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
#include "ChangeDirThread.h"


ChangeDirThread::ChangeDirThread(const char* dest, UserCore::UserI* user) : ::Thread::BaseThread("ChangeDir Thread")
{
	m_szDest = dest;
	m_szCurDir = UTIL::WIN::getRegValue(MCFCACHE);

	if (m_szCurDir.size() == 0)
		m_szCurDir = UTIL::OS::getAppDataPath();

	m_bStopped = false;
	m_bCompleted = false;
	m_pUser = user;
}

ChangeDirThread::~ChangeDirThread()
{
	if (m_bStopped && !m_bCompleted)
	{
		UTIL::FS::delFile(m_szDest);
	}
}

void ChangeDirThread::run()
{
	if (!m_pUser)
	{
		onErrorEvent(gcException(ERR_NULLHANDLE, "User Core is null!"));
		onCompleteEvent();
		return;
	}

	UTIL::FS::recMakeFolder(m_szDest);

	if (!UTIL::FS::isValidFolder(m_szDest))
	{
		onErrorEvent(gcException(ERR_BADPATH, "Could not create destination folder"));
		onCompleteEvent();
		return;
	}

	FileList vFileList;

	updateDb(vFileList);
	copyFiles(vFileList);
	
	if (!m_bStopped)
	{
		m_pUser->updateRegKey(MCFCACHE, m_szDest.c_str());
		m_bCompleted = true;
	}

	onCompleteEvent();
}

void ChangeDirThread::onStop()
{
	m_bStopped = true;
}

void ChangeDirThread::fixFilePath(UTIL::FS::Path &file)
{
	UTIL::FS::Path fdest(m_szDest, "", false);
	UTIL::FS::Path path(m_szCurDir, "", false);

	for (size_t y=path.getFolderCount(); y<file.getFolderCount(); y++)
		fdest += file.getFolder(y);

	fdest += file.getFile();
	file = fdest;
}

void ChangeDirThread::copyFiles(FileList &list)
{
	uint32 stage = 2;
	onStageEvent(stage);

	for (size_t x=0; x<list.size(); x++)
	{
		uint32 prog = x*100/list.size();
		onProgressEvent(prog);
		
		UTIL::FS::recMakeFolder(list[x].second);
		UTIL::FS::moveFile(list[x].first, list[x].second);

		if (isStopped())
			break;
	}

	delLeftOverMcf();
	UTIL::FS::delEmptyFolders(m_szCurDir);
}

void ChangeDirThread::delLeftOverMcf()
{
	UTIL::FS::Path path(m_szCurDir, "", false);
	std::vector<std::string> extList;

	extList.push_back("mcf");

	for (size_t x=0; x<20; x++)
		extList.push_back(gcString("part_{0}", x));

	delLeftOverMcf(path, extList);
}

void ChangeDirThread::delLeftOverMcf(UTIL::FS::Path path, std::vector<std::string> &extList)
{
	std::vector<UTIL::FS::Path> outList;
	UTIL::FS::getAllFiles(path, outList, &extList);

	for (size_t x=0; x<outList.size(); x++)
		UTIL::FS::delFile(outList[x]);

	outList.clear();

	UTIL::FS::getAllFolders(path, outList);

	for (size_t x=0; x<outList.size(); x++)
		delLeftOverMcf(outList[x], extList);
}

void ChangeDirThread::updateDb(FileList &list)
{
	if (isStopped())
		return;

	uint32 prog = 0;
	onProgressEvent(prog);

	uint32 stage = 1;
	onStageEvent(stage);

	gcString path = UTIL::OS::getAppDataPath();
	UTIL::FS::Path dbDir(path, "", false);

	fixMcfUploads(dbDir, list);
	fixMcfStore(dbDir, list);
}

void ChangeDirThread::fixMcfUploads(UTIL::FS::Path dest, FileList &list)
{
	dest += UTIL::FS::File("mcf_uploads.sqlite");
	sqlite3x::sqlite3_connection db(dest.getFullPath().c_str());
	sqlite3x::sqlite3_transaction trans(db);

	try
	{
		sqlite3x::sqlite3_command cmd(db, "SELECT key, file FROM mcfupload;");
		sqlite3x::sqlite3_reader reader = cmd.executereader();

		while (reader.read())
		{
			std::string id = reader.getstring(0);
			UTIL::FS::Path file = UTIL::FS::PathWithFile(reader.getstring(1));
			UTIL::FS::Path dest = file;

			fixFilePath(dest);
			list.push_back(FilePair(file, dest));

			try
			{
				sqlite3x::sqlite3_command cmd(db, "UPDATE mcfupload SET file=? WHERE key=?;");
				cmd.bind(1, dest.getFullPath());
				cmd.bind(2, id);

				cmd.executenonquery();
			}
			catch (std::exception)
			{
			}
		}
	}
	catch (std::exception)
	{
	}

	trans.commit();
}

void ChangeDirThread::fixMcfStore(UTIL::FS::Path dest, FileList &list)
{
	dest += UTIL::FS::File("mcfstoreb.sqlite");
	sqlite3x::sqlite3_connection db(dest.getFullPath().c_str());
	sqlite3x::sqlite3_transaction trans(db);

	try
	{
		sqlite3x::sqlite3_command cmd(db, "SELECT internalid, path FROM mcfitem;");
		sqlite3x::sqlite3_reader reader = cmd.executereader();

		while (reader.read())
		{
			std::string id = reader.getstring(0);
			UTIL::FS::Path file = UTIL::FS::PathWithFile(reader.getstring(1));
			UTIL::FS::Path dest = file;

			fixFilePath(dest);
			list.push_back(FilePair(file, dest));


			try
			{
				sqlite3x::sqlite3_command cmd(db, "UPDATE mcfitem SET path=? WHERE internalid=?;");
				cmd.bind(1, dest.getFullPath());
				cmd.bind(2, id);

				cmd.executenonquery();
			}
			catch (std::exception)
			{
			}
		}
	}
	catch (std::exception)
	{
	}


	try
	{
		sqlite3x::sqlite3_command cmd(db, "SELECT gid, mid, path FROM mcfbackup;");
		sqlite3x::sqlite3_reader reader = cmd.executereader();

		while (reader.read())
		{
			std::string gid = reader.getstring(0);
			std::string mid = reader.getstring(1);

			UTIL::FS::Path file = UTIL::FS::PathWithFile(reader.getstring(2));
			UTIL::FS::Path dest = file;

			fixFilePath(dest);
			list.push_back(FilePair(file, dest));

			try
			{
				sqlite3x::sqlite3_command cmd(db, "UPDATE mcfitem SET path=? WHERE gid=? AND mid=?;");
				cmd.bind(1, dest.getFullPath());
				cmd.bind(2, gid);
				cmd.bind(3, mid);

				cmd.executenonquery();
			}
			catch (std::exception)
			{
			}
		}
	}
	catch (std::exception)
	{
	}

	trans.commit();
}
