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
#include "SMTController.h"
#include "SMTWorker.h"

#include "mcf/MCF.h"
#include "mcf/MCFFile.h"
#include "mcf/MCFHeader.h"

namespace MCFCore
{
namespace Thread
{

class tFile
{
public:
	tFile(uint64 s, uint32 p)
	{
		size=s;
		pos=p;
	}

	uint64 size;
	uint32 pos;
};

struct file_sortkey
{
	bool operator()(tFile *lhs, tFile *rhs )
	{
		return (lhs->size < rhs->size);
	}
};

//! Save MCF Worker container class
class SMTWorkerInfo
{
public:
	SMTWorkerInfo(SMTController* con, uint32 i, UTIL::FS::FileHandle* fileHandle, const char* f)
	{
		id = i;
		workThread = new SMTWorker(con, i, fileHandle);
		workThread->setPriority(::Thread::BaseThread::BELOW_NORMAL);
		curFile = NULL;
		ammountDone = 0;
		status = 0;
		file = f;
	}

	~SMTWorkerInfo()
	{
		safe_delete(workThread);
	}


	uint64 ammountDone;

	uint32 id;
	uint32 status;

	gcString file;

	MCFCore::MCFFile* curFile;
	SMTWorker* workThread;
	std::vector<uint32> vFileList;
};


SMTController::SMTController(uint16 num, MCFCore::MCF* caller) : MCFCore::Thread::BaseMCFThread(num, caller, "SaveMCF Thread")
{
	m_iRunningWorkers = 0;
}

SMTController::~SMTController()
{
	safe_delete(m_vWorkerList);
}

void SMTController::onPause()
{
	//get thread running again.
	m_WaitCond.notify();
	BaseMCFThread::onPause();
}

void SMTController::onStop()
{
	//get thread running again.
	m_WaitCond.notify();
	BaseMCFThread::onStop();
}

void SMTController::run()
{
	assert(m_uiNumber);
	assert(m_szFile);

	fillFileList();
	m_pUPThread->start();

	if (!makeThreads())
		return;

	for (size_t x=0; x<m_vWorkerList.size(); x++)
		m_vWorkerList[x]->workThread->start();

	while (true)
	{
		doPause();

		if (isStopped())
			break;

		//wait here as we have nothing else to do
		m_WaitCond.wait(2);

		if (m_iRunningWorkers==0)
			break;
	}

	m_pUPThread->stop();

	for (size_t x=0; x<m_vWorkerList.size(); x++)
		m_vWorkerList[x]->workThread->stop();

	if (!isStopped())
		postProcessing();

	safe_delete(m_vWorkerList);
}

bool SMTController::makeThreads()
{
	for (uint32 x=0; x<m_uiNumber; x++)
	{
		gcString file("{0}", m_szFile);

		UTIL::FS::FileHandle* fh = new UTIL::FS::FileHandle();

		if (x != 0)
			file += gcString(".part_{0}", x);

		try
		{
			fh->open(file.c_str(), UTIL::FS::FILE_WRITE);

			//due to the first thread being the proper MCF we have to allow for the header.
			if (x == 0)
				fh->seek(MCFCore::MCFHeader::getSizeS());
		}
		catch (gcException &except)
		{
			safe_delete(fh);
			onErrorEvent(except);
			return false;
		}

		SMTWorkerInfo* temp = new SMTWorkerInfo(this, x, fh, file.c_str());
		m_vWorkerList.push_back(temp);
		temp->workThread->start();

		m_iRunningWorkers++;
	}

	return true;
}

void SMTController::postProcessing()
{
	if (m_uiNumber == 1)
		return;

	UTIL::FS::FileHandle fhSource;
	UTIL::FS::FileHandle fhSink;

	UTIL::FS::Path path(m_szFile, "", true);

	uint64 sinkSize = UTIL::FS::getFileSize(path);

	try
	{
		fhSink.open(path, UTIL::FS::FILE_APPEND);
	}
	catch (gcException &)
	{
		return;
	}

	char buff[BLOCKSIZE];

	for (size_t x=1; x<m_vWorkerList.size(); x++)
	{
		SMTWorkerInfo *worker = m_vWorkerList[x];

		uint64 fileSize = UTIL::FS::getFileSize(UTIL::FS::PathWithFile(worker->file));
		uint64 done = 0;

		uint32 readSize = BLOCKSIZE;

		try
		{
			fhSource.open(worker->file.c_str(), UTIL::FS::FILE_READ);
			while (fileSize > done)
			{
				if ((fileSize-done) < (uint64)readSize)
					readSize = (uint32)(fileSize-done);

				fhSource.read(buff, readSize);
				fhSink.write(buff, readSize);

				done += readSize;
			}
			fhSource.close();
		}
		catch (gcException &)
		{
		}

		for (size_t y=0; y<worker->vFileList.size(); y++)
		{
			uint32 index = worker->vFileList[y];
			MCFCore::MCFFile *temp = m_rvFileList[index];

			if (!temp)
				continue;

			temp->setOffSet( temp->getOffSet() + sinkSize );
		}

		sinkSize += fileSize;
		UTIL::FS::delFile(UTIL::FS::PathWithFile(worker->file));
	}

	if (m_bCreateDiff == false)
		return;
}

void SMTController::fillFileList()
{
	uint64 sumSize = 0;

	std::vector<tFile*> vList;

	for (size_t x=0; x<m_rvFileList.size(); x++)
	{
		if (!m_rvFileList[x]->isSaved())
			continue;

		if (m_rvFileList[x]->getSize() == 0)
		{
			m_rvFileList[x]->addFlag(MCFCore::MCFFileI::FLAG_ZEROSIZE);
			continue;
		}
		
		sumSize += m_rvFileList[x]->getSize();
		

		if (m_bCompress && m_rvFileList[x]->shouldCompress())
			m_rvFileList[x]->addFlag(MCFCore::MCFFileI::FLAG_COMPRESSED);
		else
			m_rvFileList[x]->delFlag(MCFCore::MCFFileI::FLAG_COMPRESSED);

		vList.push_back(new tFile(m_rvFileList[x]->getSize(), (uint32)x));
	}


	std::sort(vList.begin(), vList.end(), file_sortkey());

	for (size_t x=0; x<vList.size(); x++)
	{
		m_vFileList.push_back(vList[x]->pos);
	}

	safe_delete(vList);

	m_pUPThread->setTotal(sumSize);
}

uint32 SMTController::getStatus(uint32 id)
{
	SMTWorkerInfo* worker = findWorker(id);
	assert(worker);

	if (isPaused())
		return SF_STATUS_PAUSE;

	if (isStopped())
		return SF_STATUS_STOP;

	return worker->status;
}

MCFCore::MCFFile* SMTController::newTask(uint32 id)
{
	SMTWorkerInfo* worker = findWorker(id);
	assert(worker);

	if (worker->status != SF_STATUS_NULL)
		return NULL;

	m_pFileMutex.lock();
	size_t listSize = m_vFileList.size();
	m_pFileMutex.unlock();

	if (listSize == 0)
	{
		m_pUPThread->stopThread(id);
		worker->status = SF_STATUS_STOP;

		m_iRunningWorkers--;
		//wake thread up
		m_WaitCond.notify();
		return NULL;
	}

	m_pFileMutex.lock();
	int index = m_vFileList.back();
	m_vFileList.pop_back();
	m_pFileMutex.unlock();

	MCFCore::MCFFile *temp = m_rvFileList[index];

	if (!temp)
		return newTask(id);

	worker->vFileList.push_back(index);
	worker->curFile = temp;
	worker->status = SF_STATUS_CONTINUE;
	return temp;
}

void SMTController::endTask(uint32 id)
{
	SMTWorkerInfo* worker = findWorker(id);
	assert(worker);

	worker->status = SF_STATUS_NULL;

	worker->ammountDone += worker->curFile->getSize();
}

SMTWorkerInfo* SMTController::findWorker(uint32 id)
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

void SMTController::reportError(uint32 id, gcException &e)
{
#ifdef WIN32
	SMTWorkerInfo* worker = findWorker(id);
	assert(worker);
#endif
	Warning(gcString("SMTControler {0} Error: {1}.\n", id, e));
	onErrorEvent(e);
}

void SMTController::reportProgress(uint32 id, uint64 ammount)
{
	SMTWorkerInfo* worker = findWorker(id);
	assert(worker);
	assert(m_pUPThread);

	m_pUPThread->reportProg(id, worker->ammountDone + ammount);
}

}
}
