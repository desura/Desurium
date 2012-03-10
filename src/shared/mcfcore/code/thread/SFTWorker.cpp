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
#include "util_thread/BaseThread.h"
#include "thread/SFTWorker.h"
#include "thread/SFTController.h"
#include "mcf/MCFFile.h"

#include <time.h>
#include <time.h>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/date_time/posix_time/conversion.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

namespace bfs = boost::filesystem;
namespace bpt = boost::posix_time;

#ifdef NIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


namespace MCFCore 
{
namespace Thread
{

SFTWorker::SFTWorker(SFTController* controller, uint32 id) : BaseThread( "SafeFiles Thread" )
{
	m_uiId = id;
	m_pCT = controller;

	m_pCurFile = NULL;
	m_pBzs = NULL;
}

SFTWorker::~SFTWorker()
{
	stop();
	safe_delete(m_pBzs);
}

void SFTWorker::run()
{
	assert(m_pCT);

	while (true)
	{
		uint32 status = m_pCT->getStatus(m_uiId);
		int32 res = BZ_OK;

		while (status == BaseMCFThread::SF_STATUS_PAUSE)
		{
			gcSleep(500);
			status = m_pCT->getStatus(m_uiId);
		}

		if (status == BaseMCFThread::SF_STATUS_STOP)
		{
			break;
		}
		else if (status == BaseMCFThread::SF_STATUS_NULL)
		{
			if (!newTask())
				gcSleep(500);

			continue;
		}
		else if (status == BaseMCFThread::SF_STATUS_ENDFILE)
		{
			do
			{
				res = doWork();
				if (bzErrorCheck(res))
					break;
			}
			while (res != BZ_STREAM_END);
		}
		else if (status == BaseMCFThread::SF_STATUS_CONTINUE)
		{
			res = doWork();
			bzErrorCheck(res);
		}

		if (res == BZ_STREAM_END || status == BaseMCFThread::SF_STATUS_SKIP)
			finishFile();
	}
}

void SFTWorker::finishFile()
{
	m_hFh.close();

	std::string file = m_pCurFile->getFullPath();
	gcString str("{0}", m_pCurFile->getTimeStamp());

	try
	{
		ptime t(from_iso_string(str));
		bfs::wpath spath(gcWString(file), bfs::native);

		tm pt_tm = to_tm(t);
		last_write_time(spath, mktime(&pt_tm));
	}
	catch (...)
	{
		Warning(gcString("Failed to change {0} time stamp to {1}.\n", m_pCurFile->getName(), str));
	}

	safe_delete(m_pBzs);

	std::string hash = "Failed to generate hash";

#ifdef NIX
	//struct stat s;
	bool isWinExe = false;
	
	if (file.size() > 4)
	{
		std::string lastFour(file.end() - 4, file.end());
		std::transform(lastFour.begin(), lastFour.end(), lastFour.begin(), ::toupper);
		if (lastFour == ".EXE")
			isWinExe = true;
	}
	
	//( && stat(file.c_str(), &s) == 0) 
	
	bool isExecutable = HasAnyFlags(m_pCurFile->getFlags(), MCFCore::MCFFileI::FLAG_XECUTABLE);

	if (isExecutable || isWinExe)
		chmod(file.c_str(), (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH));
#endif

	if (m_pCurFile->isZeroSize() || m_pCurFile->hashCheckFile(&hash))
	{
		m_pCT->endTask(m_uiId, BaseMCFThread::SF_STATUS_COMPLETE);
	}
	else
	{
		Warning(gcString("Hash check failed for file [{0}]: Cur: {1} !=  Should: {2}\n", m_pCurFile->getName(), hash, m_pCurFile->getCsum()));
		m_pCT->endTask(m_uiId, BaseMCFThread::SF_STATUS_HASHMISSMATCH);
	}
}

int32 SFTWorker::doWork()
{
	if (m_pCurFile->isZeroSize())
		return BZ_STREAM_END;

	uint32 status = 0;
	AutoDelete<SFTWorkerBuffer> temp(m_pCT->getBlock(m_uiId, status));

	bool endFile = (status == BaseMCFThread::SF_STATUS_ENDFILE);

	//if temp is null we are waiting on data to be read. Lets nap for a bit
	if (!temp.handle())
	{
		if (endFile)
		{
			if (m_pCurFile->isCompressed())
				return doDecompression(NULL, 0, true);

			return BZ_STREAM_END;
		}
		else
		{
			//gcSleep(100);
			return BZ_OK;
		}
	}

	if (m_pCurFile->isCompressed())
		return doDecompression(temp->buff, temp->size, false);

	return doWrite(temp->buff, temp->size);
}

int SFTWorker::doDecompression(const char* buff, uint32 buffSize, bool endFile)
{
	if (!m_pBzs)
	{
		gcException e(ERR_BZ2, 0, "Bzip2 handle was NULL");
		return reportError(BZ_STREAM_END, e);
	}

	m_pBzs->write(buff, buffSize, endFile);

	try
	{
		m_pBzs->doWork();
	}
	catch (gcException &e)
	{
		return reportError(BZ_STREAM_END, e);
	}

	size_t outBuffSize = m_pBzs->getReadSize();

	if (outBuffSize == 0)
		return m_pBzs->getLastStatus();

	AutoDelete<char> outBuff(new char[outBuffSize]);

	m_pBzs->read(outBuff, outBuffSize);
	int32 res = doWrite(outBuff, outBuffSize);

	if (res == BZ_OK)
		return m_pBzs->getLastStatus();

	return res;
}

int32 SFTWorker::doWrite(const char* buff, uint32 buffSize)
{
	try
	{
		m_hFh.write(buff, buffSize);
	}
	catch (gcException &e)
	{
		return reportError(BZ_STREAM_END, e);
	}

	m_pCT->reportProgress(m_uiId, buffSize);
	m_pCT->pokeThread();

	return BZ_OK;
}

int32 SFTWorker::reportError(int32 bz2Code, gcException &e)
{
	m_pCT->reportError(m_uiId, e);
	m_pCT->pokeThread();

	m_hFh.close();
	safe_delete(m_pBzs);

	return bz2Code;
}

bool SFTWorker::newTask()
{
	m_pCurFile = m_pCT->newTask(m_uiId);

	if (!m_pCurFile)
		return false;

	Debug(gcString("{0}: starting Decompressing file {1}.\n", m_uiId, m_pCurFile->getName()));
	std::string file = m_pCurFile->getFullPath();

	safe_delete(m_pBzs);
	m_pBzs = new UTIL::MISC::BZ2Worker(UTIL::MISC::BZ2_DECOMPRESS);

	int res = 0;
	if (m_pBzs->isInit(res) == false)
	{
		gcException e(ERR_BZ2DFAIL, res, gcString("Failed to init bzip2 decompression for file {0}", file));
		return reportError(false, e)?true:false;
	}

	UTIL::FS::Path path(file, "", true);
	UTIL::FS::recMakeFolder(path);

	try
	{
		m_hFh.open(path, UTIL::FS::FILE_WRITE);
	}
	catch (gcException &e)
	{
		return reportError(false, e)?true:false;
	}

	return true;
}

bool SFTWorker::bzErrorCheck(int32 bzStatus)
{
	if (bzStatus == BZ_OK || bzStatus == BZ_STREAM_END)
		return false;

	gcException e(ERR_BZ2, bzStatus, gcString("Failed bz2 error check for file {0}", m_pCurFile->getFullPath()));
	return reportError(true, e)?true:false;
}


}
}
