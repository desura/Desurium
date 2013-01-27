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
#include "SFTController.h"
#include "SFTWorker.h"
#include "mcf/MCFFile.h"

namespace MCFCore
{
namespace Thread
{



class SFTWorkerInfo
{
public:
	SFTWorkerInfo(SFTController* con, uint32 i)
	{
		id = i;
		workThread = new SFTWorker(con, i);
		workThread->setPriority(::Thread::BaseThread::BELOW_NORMAL);
		curFile = NULL;
		status = 0;
		offset = 0;
		ammountDone = 0;
	}

	~SFTWorkerInfo()
	{
		safe_delete(vBuffer);
		safe_delete(workThread);
	}

	uint64 offset;
	uint64 ammountDone;

	uint32 id;
	uint32 status;

	::Thread::Mutex mutex;

	MCFCore::MCFFile* curFile;
	SFTWorker* workThread;
	std::vector<SFTWorkerBuffer*> vBuffer;
};



SFTController::SFTController(uint16 num, MCFCore::MCF* caller, const char* path) : MCFCore::Thread::BaseMCFThread(num, caller, "SaveFiles Thread")
{
	m_szPath = path;
}

SFTController::~SFTController()
{
	safe_delete(m_vWorkerList);
}


void SFTController::run()
{
	assert(m_uiNumber);
	assert(m_szFile);

	UTIL::FS::FileHandle fh;

	try
	{
		fh.open(m_szFile, UTIL::FS::FILE_READ, m_uiFileOffset);
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
		return;
	}

	fillFileList();

	//no work to do this finish up.
	if (m_vFileList.size() == 0)
		return;

	m_pUPThread->start();


	for (uint32 x=0; x<m_uiNumber; x++)
		m_vWorkerList.push_back(new SFTWorkerInfo(this, x));

	for (size_t x=0; x<m_vWorkerList.size(); x++)
		m_vWorkerList[x]->workThread->start();

	while (true)
	{
		doPause();

		if (isStopped())
			break;

		if (!fillBuffers(fh))
			m_WaitCond.wait(0, 500);

		if (workersDone())
			break;
	}

	for (size_t x=0; x<m_vWorkerList.size(); x++)
		m_vWorkerList[x]->workThread->stop();

	safe_delete(m_vWorkerList);
}

bool SFTController::workersDone()
{
	for (size_t x=0; x<m_vWorkerList.size(); x++)
	{
		if (m_vWorkerList[x]->status != SF_STATUS_STOP)
			return false;
	}

	return true;
}

void SFTController::fillFileList()
{
	uint64 totSize = 0;
	size_t count = m_rvFileList.size();

	MCFCore::Misc::ProgressInfo p;
	p.flag = 1;

	for (size_t x=0; x<count; x++)
	{
		p.percent = x*100/count;
		onProgressEvent(p);

		if (!m_rvFileList[x]->isSaved())
			continue;

		bool Skip = false;

		m_rvFileList[x]->setDir(m_szPath.c_str());
		UTIL::FS::Path path = UTIL::FS::PathWithFile(m_rvFileList[x]->getFullPath());

		if (gcString("%%EMPTYFOLDER%%") == m_rvFileList[x]->getName())
		{
			UTIL::FS::recMakeFolder(path);
			continue;
		}

		if (UTIL::FS::isValidFile(path))
		{
			if (m_rvFileList[x]->isZeroSize())
			{
				if (UTIL::FS::getFileSize(path) != 0)
				{
					UTIL::FS::delFile(path);

					try
					{
						UTIL::FS::FileHandle fh(path, UTIL::FS::FILE_WRITE);
						fh.close();
					}
					catch (gcException &)
					{
					}
				}

				Skip = true;
			}
			else
			{
				try
				{
					uint64 size = UTIL::FS::getFileSize(path);
					UTIL::FS::FileHandle fh(path.getFullPath().c_str(), UTIL::FS::FILE_READ);
	
					std::string md5 = UTIL::MISC::hashFile(fh.getHandle(), size);
					std::string fileMd5 = m_rvFileList[x]->getCsum();

					//if file exists and hash matchs
					if (md5 == fileMd5)
						Skip = true;
				}
				catch (...)
				{
				}
			}
		}

		if (Skip)
			continue;

		totSize += m_rvFileList[x]->getSize();
		m_vFileList.push_back((uint32)x);
	}

	p.percent = 100;
	onProgressEvent(p);

	m_pUPThread->setTotal(totSize);
}

bool SFTController::fillBuffers(UTIL::FS::FileHandle& fileHandle)
{
	bool processed = false;

	for (size_t x=0; x<m_vWorkerList.size(); x++)
	{
		if (m_vWorkerList[x]->status != SF_STATUS_CONTINUE || !m_vWorkerList[x]->curFile)
			continue;

		if (m_vWorkerList[x]->vBuffer.size() >= 4 || m_vWorkerList[x]->status == SF_STATUS_ENDFILE)
			continue;

		processed = true;

		MCFCore::MCFFile *file = m_vWorkerList[x]->curFile;

		if (!file || file->isZeroSize())
			continue;

		bool endFile = false;

		uint64 diff = file->getCurSize() - m_vWorkerList[x]->offset;
		uint32 buffSize = BLOCKSIZE;

		if (diff <= BLOCKSIZE)
		{
			endFile = true;
			buffSize = (uint32)diff;
		}

		char* buff = new char[buffSize];

		try
		{
			fileHandle.seek(file->getOffSet() + m_vWorkerList[x]->offset);
			fileHandle.read(buff, buffSize);
		}
		catch (gcException &except)
		{
			safe_delete(buff);
			reportError((uint32)x, except);
			continue;
		}

		m_vWorkerList[x]->mutex.lock();

		m_vWorkerList[x]->vBuffer.push_back(new SFTWorkerBuffer(buffSize, buff));

		if (endFile)
			m_vWorkerList[x]->status = SF_STATUS_ENDFILE;

		m_vWorkerList[x]->offset += buffSize;

		m_vWorkerList[x]->mutex.unlock();
	}

	return processed;
}

uint32 SFTController::getStatus(uint32 id)
{
	SFTWorkerInfo* worker = findWorker(id);
	assert(worker);

	if (isPaused())
		return SF_STATUS_PAUSE;

	if (isStopped())
		return SF_STATUS_STOP;

	return worker->status;
}

SFTWorkerBuffer* SFTController::getBlock(uint32 id, uint32 &status)
{
	SFTWorkerInfo* worker = findWorker(id);
	assert(worker);

	SFTWorkerBuffer* temp = NULL;

	worker->mutex.lock();
		status = worker->status;

		if (!worker->vBuffer.empty())
		{
			temp = worker->vBuffer.front();
			worker->vBuffer.erase(worker->vBuffer.begin());
		}
	worker->mutex.unlock();

	m_WaitCond.notify();
	return temp;
}

void SFTController::pokeThread()
{
	m_WaitCond.notify();
}

MCFCore::MCFFile* SFTController::newTask(uint32 id)
{
	SFTWorkerInfo* worker = findWorker(id);
	assert(worker);

	if (worker->status != SF_STATUS_NULL)
		return NULL;

	worker->status = SF_STATUS_WAITTASK;

	m_pFileMutex.lock();
	size_t listSize = m_vFileList.size();
	m_pFileMutex.unlock();

	if (listSize == 0)
	{
		m_pUPThread->stopThread(id);
		worker->status = SF_STATUS_STOP;
		return NULL;
	}

	m_pFileMutex.lock();
	int index = m_vFileList.back();
	m_vFileList.pop_back();
	m_pFileMutex.unlock();

	MCFCore::MCFFile *temp = m_rvFileList[index];

	if (!temp)
		return newTask(id);

	worker->curFile = temp;
	worker->offset = 0;
	worker->status = SF_STATUS_CONTINUE;

	m_WaitCond.notify();

	return temp;
}

void SFTController::endTask(uint32 id, uint32 status, gcException e)
{
	SFTWorkerInfo* worker = findWorker(id);
	assert(worker);

	if (status == SF_STATUS_HASHMISSMATCH)
	{
		if (worker->curFile)
			Warning(gcString("\t{0}: Hash mismatch found in file {1}.\n", id, worker->curFile->getName()));
		else
			Warning(gcString("\t{0}: Hash mismatch in unknown file.\n", id));
	}

	if (status != BaseMCFThread::SF_STATUS_COMPLETE)
	{
		gcException err = e;

		if (err.getErrId() == 0)
		{
			if (worker->curFile)
				err = gcException(ERR_HASHMISSMATCH, status, gcString("Hash mismatch found in file {0}", worker->curFile->getName()));
			else
				err = gcException(ERR_HASHMISSMATCH, status, gcString("Hash mismatch found in unknown file"));
		}

		onErrorEvent(e);
	}

	worker->status = SF_STATUS_NULL;
	worker->curFile = NULL;

	worker->mutex.lock();
	safe_delete(worker->vBuffer);
	worker->mutex.unlock();
}


SFTWorkerInfo* SFTController::findWorker(uint32 id)
{
	if (id >= m_vWorkerList.size())
		return NULL;

	for (size_t x=0; x<m_vWorkerList.size(); x++)
	{
		if (m_vWorkerList[x]->id == id)
			return m_vWorkerList[x];
	}

	return NULL;
}


void SFTController::reportError(uint32 id, gcException &e)
{
#ifdef WIN32
	SFTWorkerInfo* worker = findWorker(id);
	assert(worker);
#endif
	Warning(gcString("SFTControler: {0} Error: {1}.\n", id, e));
	endTask(id, SF_STATUS_ERROR, e);
}

void SFTController::reportProgress(uint32 id, uint64 ammount)
{
	SFTWorkerInfo* worker = findWorker(id);
	assert(worker);
	assert(m_pUPThread);

	worker->ammountDone += ammount;

	m_pUPThread->reportProg(id, worker->ammountDone);
}

}
}
