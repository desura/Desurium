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
#include "WGTController.h"
#include "WGTWorker.h"

#include "mcf/MCFFile.h"
#include "mcf/MCF.h"

#include "ProviderManager.h"

#define MAX_BLOCK_SIZE (50*1024*1024)

#ifndef min
template <typename A>
A min(A a, A b)
{
	return a<b?a:b;
}

#endif

namespace MCFCore
{
namespace Thread
{

struct block_sortkey
{
	bool operator()(Misc::WGTBlock *lhs, Misc::WGTBlock *rhs )
	{
		return (lhs->webOffset < rhs->webOffset);
	}
};


class WGTWorkerInfo
{
public:
	WGTWorkerInfo(WGTControllerI* con, uint32 i, MCFCore::Misc::ProviderManager *pProvMng, MCFCore::Misc::GetFile_s* pFileAuth)
	{
		id = i;
		workThread = new WGTWorker(con, i, pProvMng, pFileAuth);
		workThread->setPriority(::Thread::BaseThread::BELOW_NORMAL);
		curBlock = NULL;
		ammountDone = 0;
		status = MCFCore::Thread::BaseMCFThread::SF_STATUS_CONTINUE;
	}

	~WGTWorkerInfo()
	{
		safe_delete(workThread);
		safe_delete(vBuffer);
	}

	uint64 ammountDone;

	uint32 id;
	uint32 status;

	Misc::WGTSuperBlock* curBlock;
	WGTWorker* workThread;

	::Thread::Mutex mutex;
	std::deque<Misc::WGTBlock*> vBuffer;
};


WGTController::WGTController(std::vector<MCFCore::Misc::DownloadProvider*> &source, uint16 numWorkers, MCFCore::MCF* caller, bool checkMcf) : MCFCore::Thread::BaseMCFThread(numWorkers, caller, "WebGet Controller Thread")
{
#ifdef DEBUG
#if 0
	source.clear();
	source.push_back(new MCFCore::Misc::DownloadProvider("us1", "mcf://10.0.0.2:62003", "http://images.wikia.com/mariokart/images/4/45/Mkdd_giant_banana.jpg", ""));
	/*std::vector<MCFCore::Misc::DownloadProvider*> list;

	for (size_t x=0; x<source.size(); x++)
	{
		source[x]->m_szUrl = "";

		if (gcString("mcf://us20.dl.desura.com:62001") == source[x]->getUrl())
		{
			list.push_back(source[x]);
			break;
		}
	}

	if (list.size() == 0)
		list.push_back(source[0]);

	source.clear();
	source = list;*/
#endif
	m_uiSaved = 0;
#endif

	m_pProvManager = new MCFCore::Misc::ProviderManager(source);

	if (m_uiNumber > source.size())
		m_uiNumber = source.size();

	m_pProvManager->onProviderEvent += delegate(&onProviderEvent);

	setPriority(BELOW_NORMAL);
	m_bCheckMcf = checkMcf;

	m_iRunningWorkers=0;
	m_iAvailbleWork=0;

	m_pFileAuth = caller->getAuthInfo();

	m_bDoingStop = false;
}

WGTController::~WGTController()
{
	join();

	if (m_bDoingStop)
		gcSleep(500);

	safe_delete(m_pProvManager);
	safe_delete(m_vWorkerList);

	m_pFileMutex.lock();
	safe_delete(m_vSuperBlockList);
	m_pFileMutex.unlock();
}

void WGTController::pokeThread()
{
	m_WaitCondition.notify();
}


void WGTController::run()
{
	if (m_pProvManager->getVector().size() == 0)
	{
		gcException newE(ERR_INVALID, "There are no download providers to download mcf.");
		onErrorEvent(newE);
		return;
	}

	UTIL::FS::FileHandle fh;

	if (!fillBlockList())
		return;

	try
	{
		//header should be saved all ready so append to it
		fh.open(m_szFile, UTIL::FS::FILE_APPEND);
	}
	catch (gcException &except)
	{
		onErrorEvent(except);
		return;
	}

	m_pUPThread->start();
	createWorkers();

	//notify that init is done. :P
	MCFCore::Misc::ProgressInfo pi;
	pi.flag = MCFCore::Misc::ProgressInfo::FLAG_INITFINISHED;
	pi.percent = 0;
	onProgressEvent(pi);

	while (!isStopped())
	{
		doPause();

		saveBuffers(fh);

		if (m_iRunningWorkers == 0 && !isQuedBlocks())
			break;

		if (!isQuedBlocks() && m_iRunningWorkers > 0 && !isStopped())
		{
			if (!m_pProvManager->hasValidAgents())
				break;

			m_WaitCondition.wait(5);
		}
	}

	m_pUPThread->stop();

	for (uint16 x=0; x<m_uiNumber; x++)
		m_vWorkerList[x]->workThread->stop();

	saveBuffers(fh, true);

	if (m_iAvailbleWork == 0)
	{
		//notify that download is done. :P
		MCFCore::Misc::ProgressInfo pi;
		pi.flag = MCFCore::Misc::ProgressInfo::FLAG_FINALIZING;
		pi.percent = 100;
		onProgressEvent(pi);

		for (size_t x=0; x<m_vDlFiles.size(); x++)
		{
			m_rvFileList[m_vDlFiles[x]]->addFlag(MCFCore::MCFFileI::FLAG_COMPLETE);
			m_rvFileList[m_vDlFiles[x]]->delFlag(MCFCore::MCFFileI::FLAG_STARTEDDL);
		}
	}
	else
	{
		if (!isStopped())
		{
			gcException newE(ERR_SOCKET, "All server connections failed. \n\n"
								"Please check you are connected to the internet\n"
								"and try again or check console for more info.");
			onErrorEvent(newE);
		}
	}
}

void WGTController::createWorkers()
{
	for (uint16 x=0; x<m_uiNumber; x++)
	{
		WGTWorkerInfo* temp = new WGTWorkerInfo(this, (uint32)x, m_pProvManager, m_pFileAuth);
		m_vWorkerList.push_back(temp);

		temp->workThread->start();
		m_iRunningWorkers++;
	}
}

bool WGTController::isQuedBlocks()
{
	for (size_t x=0; x<m_vWorkerList.size(); x++)
	{
		if (m_vWorkerList[x]->vBuffer.size() > 0)
			return true;
	}

	return false;
}

void WGTController::saveBuffers(UTIL::FS::FileHandle& fileHandle, bool allBlocks)
{
	//dont eat all the cpu speed
	if (m_vWorkerList.size() == 0)
		return;

	for (size_t x=0; x<m_vWorkerList.size(); x++)
	{
		do
		{
			if (m_vWorkerList[x]->vBuffer.size() == 0)
				break;

			m_vWorkerList[x]->mutex.lock();
			Misc::WGTBlock *block = m_vWorkerList[x]->vBuffer.front();
			m_vWorkerList[x]->vBuffer.pop_front();
			m_vWorkerList[x]->mutex.unlock();

			assert(block);

			if (!checkBlock(block, m_vWorkerList[x]->id))
				continue;

			try
			{
				fileHandle.seek(block->fileOffset);
				fileHandle.write(block->buff, block->size);

#ifdef DEBUG
				m_uiSaved += block->size;
#endif
			}
			catch (gcException &e)
			{
				onErrorEvent(e);
				safe_delete(block);
				break;
			}

			safe_delete(block);
		}
		while (allBlocks); //if all blocks is true it will keep looping until buff size is zero else it will run once
	}
}

bool WGTController::checkBlock(Misc::WGTBlock *block, uint32 workerId)
{
	if (!block)
		return true;

	bool crcFail = false;
	bool sizeFail = block->dlsize != block->size;

	uint32 crc = 0;

	if (!sizeFail)
	{
		crc = UTIL::MISC::CRC32((unsigned char*)block->buff, block->size);
		crcFail = (block->crc != 0 && block->crc != crc);
	}

	if (!sizeFail && !crcFail)
		return true;


	reportNegProgress(workerId, block->dlsize);

	gcString appData = UTIL::OS::getAppDataPath(L"crc");

	UTIL::FS::Path logFile(appData, "crc_log.txt", false);
	UTIL::FS::Path outFile(appData, gcString("block_{0}_{1}.bin", block->fileOffset, time(NULL)), false);

	outFile += m_pHeader->getDesuraId().toString();

	gcString log("Crc of block didnt match expected.\nFile: {0}\nWebOffset: {1}\nFileOffset: {2}\nExpected Crc: {3}\nCrc: {4}\nOut File: {5}\n", block->file->getName(), block->webOffset, block->fileOffset, block->crc, crc, outFile.getFullPath());
	log += gcString("Size: {3}\nMcf: {0}\nBuild: {1}\nBranch: {2}\nProvider: {4}\n\n", m_pHeader->getId(), m_pHeader->getBuild(), m_pHeader->getBranch(), block->size, block->provider);

	UTIL::FS::recMakeFolder(logFile);
	UTIL::FS::recMakeFolder(outFile);

	try
	{
		UTIL::FS::FileHandle fh(outFile, UTIL::FS::FILE_WRITE);
		fh.write(block->buff, block->size);
		fh.close();

		fh.open(logFile, UTIL::FS::FILE_APPEND);
		fh.write(log.c_str(), log.size());
		fh.close();

		Warning(gcString("Failed crc check. Writen to log file [{0}], block saved to file [{1}].\n", logFile.getFullPath(), outFile.getFullPath()));
	}
	catch (gcException)
	{
	}

	Misc::WGTSuperBlock* super = new Misc::WGTSuperBlock();

	super->vBlockList.push_back(block);
	super->offset = block->webOffset;
	super->size = block->size;

	m_pFileMutex.lock();
	m_vSuperBlockList.push_back(super);
	m_iAvailbleWork++;
	m_pFileMutex.unlock();

	//get thread running again.
	m_WaitCondition.notify();

	gcException e(ERR_INVALIDDATA);

	if (sizeFail)
		e = gcException(ERR_INVALIDDATA, "Size of the download chunk didnt match what was expected");
	else if (crcFail)
		e = gcException(ERR_INVALIDDATA, "Crc of the download chunk didnt match what was expected");

	m_vWorkerList[workerId]->workThread->reportError(e, block->provider);


	return false;
}

static bool WGTBlockSort(Misc::WGTBlock* a, Misc::WGTBlock* b)
{
	return a->webOffset < b->webOffset;
}

static bool SortByOffset(MCFCore::MCFFile* a, MCFCore::MCFFile* b)
{
	return a->getOffSet() < b->getOffSet();
}

bool WGTController::fillBlockList()
{
	MCFCore::Misc::ProgressInfo pi;
	MCFCore::MCF webMcf(m_pProvManager->getVector(), m_pFileAuth);

	try
	{
		webMcf.dlHeaderFromWeb();
	}
	catch (gcException &e)
	{
		onErrorEvent(e);
		return false;
	}

	pi.percent = 5;
	onProgressEvent(pi);

	webMcf.sortFileList();

	uint64 mcfOffset = m_pHeader->getSize();
	uint64 downloadSize = 0;
	uint64 done = 0;

	size_t fsSize = m_rvFileList.size();

	std::vector<MCFFile*> tempFileList;
	tempFileList.reserve(fsSize);

	//find the last files offset
	for (size_t x=0; x< fsSize; x++)
	{
		MCFFile *file = m_rvFileList[x];

		if (!file->isSaved())
			continue;

		tempFileList.push_back(file);

		if ((file->isComplete() || file->hasStartedDL()) && file->getOffSet() > mcfOffset)
		{
			mcfOffset = file->getOffSet() + file->getCurSize();
		}
	}

	fsSize = tempFileList.size();

	UTIL::FS::FileHandle fh;
	try
	{
		fh.open(m_szFile, UTIL::FS::FILE_READ);
	}
	catch (gcException &)
	{
	}

	std::deque<Misc::WGTBlock*> vBlockList;
	std::sort(tempFileList.begin(), tempFileList.end(), SortByOffset);

	for (size_t x=0; x< fsSize; x++)
	{
		MCFFile* file = tempFileList[x];

		if (isStopped())
		{
			safe_delete(vBlockList);
			return false;
		}

		uint32 p = 5 + x*95/fsSize;

		//dont go crazy on sending progress events
		if (pi.percent != p)
		{
			pi.percent = p;
			onProgressEvent(pi);
		}

		//dont download all ready downloaded items
		if (file->isComplete())
		{
			done += file->getCurSize();
			continue;
		}

		//skip files that arnt "saved" in the MCF
		if (!file->isSaved())
			continue;

		if (file->isZeroSize())
		{
			file->setOffSet(0);
			continue;
		}

		uint64 size = file->getCurSize();
		bool started = file->hasStartedDL();

		uint32 index = webMcf.findFileIndexByHash(file->getHash());
		MCFCore::MCFFile *webFile = webMcf.getFile(index);

		if (index == UNKNOWN_ITEM || !webFile || !webFile->isSaved())
		{
			Warning(gcString("File {0} is not in web MCF. Skipping download.\n", file->getName()));
			if (!started)
				file->delFlag(MCFCore::MCFFileI::FLAG_SAVE);
			continue;
		}

		file->copyBorkedSettings(webFile);

		m_vDlFiles.push_back(x);
		file->addFlag(MCFCore::MCFFileI::FLAG_STARTEDDL);

		if (!started && file->getOffSet() != 0)
		{
			bool check1 = false;
			bool check2 = false;

			if (fsSize > 1 && x != (fsSize-1))
				check1 = (file->getOffSet() + file->getCurSize()) > tempFileList[x+1]->getOffSet();

			if (fsSize > 1 && x != 0 )
				check2 = file->getOffSet() < (tempFileList[x-1]->getOffSet() + tempFileList[x-1]->getCurSize());

			if (!check1 && !check2)
				started = true;
		}

		if (!started)
		{
			file->setOffSet(mcfOffset);
			mcfOffset += size;
		}

		uint32 blocksize = file->getBlockSize();
		uint32 offset = 0;
		uint32 y = 0;

		while (offset < size)
		{
			Misc::WGTBlock* temp = new Misc::WGTBlock;

			temp->file =  file;
			temp->index = y;

			temp->webOffset = webFile->getOffSet() + offset;
			temp->fileOffset = file->getOffSet() + offset;

			if (webFile->getCRCCount() > y)
				temp->crc = webFile->getCRC(y);

			//make sure we dont read past end of the file
			if (size-offset < blocksize)
				temp->size = (uint32)(size-offset);
			else
				temp->size = blocksize;

			if (started && m_bCheckMcf && fh.isValidFile() && file->crcCheck(y, fh))
			{
				done += temp->size;
				safe_delete(temp);
			}
			else
			{
				vBlockList.push_back(temp);
				downloadSize += temp->size;
			}

			offset+=blocksize;
			y++;
		}
	}

	m_pUPThread->setDone(done);
	m_pUPThread->setTotal(downloadSize+done);

	std::sort(vBlockList.begin(), vBlockList.end(), &WGTBlockSort);
	uint64 maxSize = min((uint32)(downloadSize / m_uiNumber), (uint32)(256*1024*1024));

	while (vBlockList.size() > 0)
	{
		Misc::WGTSuperBlock* sb = new Misc::WGTSuperBlock();
		sb->offset = vBlockList[0]->webOffset;

		bool size = false;
		bool offset = false;
		bool maxsize = false;

		do
		{
			Misc::WGTBlock* block = vBlockList.front();

			uint64 t = (uint64)block->size + (uint64)sb->size;

			//if we go over the 32 bit size limit break
			if (t >> 32)
				break;

			vBlockList.pop_front();

			sb->size += block->size;
			sb->vBlockList.push_back(block);

			size = (vBlockList.size() > 0);
			offset = size && (vBlockList[0]->webOffset == (sb->offset + sb->size));
			maxsize = (sb->size < maxSize);
		}
		while (size && offset && maxsize);

		m_vSuperBlockList.push_back(sb);
	}

	m_iAvailbleWork = m_vSuperBlockList.size();

	//make sure we hit 100
	pi.percent = 100;
	onProgressEvent(pi);
	return true;
}

bool WGTController::stealBlocks()
{
	//Disable this for now as it causes mcf download issues
	return false;

	size_t largestIndex = -1;
	size_t largestCount = 0;

	for (size_t x=0; x<m_vWorkerList.size(); x++)
	{
		::Thread::AutoLock al(m_vWorkerList[x]->mutex);

		if (!m_vWorkerList[x]->curBlock)
			continue;

		size_t count = m_vWorkerList[x]->curBlock->vBlockList.size();

		if (count > largestCount)
		{
			largestCount = count;
			largestIndex = x;
		}
	}

	if (largestCount < 3 || largestIndex == (size_t)-1)
		return false;

	WGTWorkerInfo* worker = m_vWorkerList[largestIndex];
	worker->mutex.lock();

	Misc::WGTSuperBlock* curBlock = worker->curBlock;
	Misc::WGTSuperBlock* superBlock = new Misc::WGTSuperBlock();

	size_t halfWay = largestCount/2;

	curBlock->m_Lock.lock();
	superBlock->offset = curBlock->offset;

	for (size_t x=0; x<curBlock->vBlockList.size(); x++)
	{
		if (x <= halfWay)
		{
			superBlock->offset += curBlock->vBlockList[x]->size;
		}
		else
		{
			curBlock->size -= curBlock->vBlockList[x]->size;
			superBlock->size += curBlock->vBlockList[x]->size;
			superBlock->vBlockList.push_back(curBlock->vBlockList[x]);
		}
	}

	for (size_t x=curBlock->vBlockList.size()-1; x>halfWay; x--)
	{
		curBlock->vBlockList.erase(curBlock->vBlockList.begin()+x);
	}

	curBlock->m_Lock.unlock();
	worker->mutex.unlock();

	m_pFileMutex.lock();
	m_vSuperBlockList.push_back(superBlock);
	m_iAvailbleWork++;
	m_pFileMutex.unlock();

	return true;
}

Misc::WGTSuperBlock* WGTController::newTask(uint32 id, uint32 &status)
{
	WGTWorkerInfo* worker = findWorker(id);
	assert(worker);

	status = worker->status;

	if (worker->status != SF_STATUS_CONTINUE)
		return NULL;

	if (worker->curBlock)
		return worker->curBlock;

	m_pFileMutex.lock();
	size_t listSize = m_vSuperBlockList.size();
	m_pFileMutex.unlock();

	if (listSize == 0 && !stealBlocks())
	{
		m_pUPThread->stopThread(id);
		worker->status = SF_STATUS_STOP;
		status = SF_STATUS_STOP;

		m_iRunningWorkers--;
		//get thread running again.
		m_WaitCondition.notify();

		return NULL;
	}

	m_pFileMutex.lock();
	Misc::WGTSuperBlock* temp = m_vSuperBlockList.front();
	m_vSuperBlockList.pop_front();
	m_pFileMutex.unlock();

	if (!temp)
		return newTask(id, status);

	worker->curBlock = temp;
	worker->status = SF_STATUS_CONTINUE;
	status = SF_STATUS_CONTINUE;

	return temp;
}

void WGTController::workerFinishedSuperBlock(uint32 id)
{
	WGTWorkerInfo* worker = findWorker(id);
	assert(worker);

	Misc::WGTSuperBlock* block = NULL;

	worker->mutex.lock();
	block = worker->curBlock;
	worker->curBlock = NULL;
	worker->status = SF_STATUS_CONTINUE;
	worker->mutex.unlock();

	if (!block)
	{
		m_WaitCondition.notify();
		return;
	}

	if (block->vBlockList.size() == 0)
	{
		if (block->size != 0)
			Warning("WGControler: Block still has data to download but all blocks consumed. Expect trouble ahead\n");

		safe_delete(block);

		m_pFileMutex.lock();
		m_iAvailbleWork--;
		m_pFileMutex.unlock();
	}
	else
	{
		m_pFileMutex.lock();
		m_vSuperBlockList.push_back(block);
		m_pFileMutex.unlock();
	}

	//get thread running again.
	m_WaitCondition.notify();
}

void WGTController::workerFinishedBlock(uint32 id, Misc::WGTBlock* block)
{
	WGTWorkerInfo* worker = findWorker(id);
	assert(worker);

	if (!block || !worker)
		return;

	worker->mutex.lock();
	worker->vBuffer.push_back(block);
	worker->mutex.unlock();

	//get thread running again.
	m_WaitCondition.notify();
}

uint32 WGTController::getStatus(uint32 id)
{
	WGTWorkerInfo* worker = findWorker(id);
	assert(worker);

	if (isPaused())
		return SF_STATUS_PAUSE;

	if (isStopped())
		return SF_STATUS_STOP;

	return worker->status;
}

void WGTController::reportError(uint32 id, gcException &e)
{
	WGTWorkerInfo* worker = findWorker(id);
	assert(worker);

	Warning(gcString("WebGet: {0} Error: {1}.\n", id, e));

	m_pUPThread->stopThread(id);
	worker->status = SF_STATUS_STOP;

	m_iRunningWorkers--;

	//get thread running again.
	m_WaitCondition.notify();
}

void WGTController::reportProgress(uint32 id, uint64 ammount)
{
	WGTWorkerInfo* worker = findWorker(id);
	assert(worker);
	assert(m_pUPThread);

	worker->ammountDone += ammount;
	m_pUPThread->reportProg(id, worker->ammountDone);
}

void WGTController::reportNegProgress(uint32 id, uint64 ammount)
{
	WGTWorkerInfo* worker = findWorker(id);
	assert(worker);
	assert(m_pUPThread);

	worker->ammountDone -= ammount;
	m_pUPThread->reportProg(id, worker->ammountDone);
}

WGTWorkerInfo* WGTController::findWorker(uint32 id)
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

void WGTController::onStop()
{
	m_bDoingStop = true;

	BaseMCFThread::onStop();

	for (size_t x=0; x<m_vWorkerList.size(); x++)
	{
		if (m_vWorkerList[x] && m_vWorkerList[x]->workThread)
			m_vWorkerList[x]->workThread->stop();
	}

	//get thread running again.
	m_WaitCondition.notify();

	m_bDoingStop = false;
}

}
}
