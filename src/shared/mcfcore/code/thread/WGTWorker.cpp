/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Mark Chandler <mark@moddb.com>

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
#include "WGTWorker.h"
#include "WGTController.h"
#include "ProviderManager.h"


namespace MCFCore
{
namespace Thread
{

WGTWorker::WGTWorker(WGTControllerI* controller, uint16 id, MCFCore::Misc::ProviderManager *pProvMng, MCFCore::Misc::GetFile_s* pFileAuth) : BaseThread( "WebGet Worker Thread" )
{
	m_pProvMng = pProvMng;
	m_pMcfCon = NULL;
	m_pCurBlock = NULL;

	m_uiId = id;
	m_pCT = controller;

	m_pFileAuth = pFileAuth;
	m_iAttempt = 0;

	m_bError = false;
}

WGTWorker::~WGTWorker()
{
	m_DeleteMutex.lock();
	safe_delete(m_pMcfCon);
	m_DeleteMutex.unlock();
}

void WGTWorker::reportError(gcException &e, gcString provider)
{
	if (m_pProvMng->getName(m_uiId) != provider)
		return;

	m_ErrorMutex.lock();
	m_bError = true;
	m_Error = e;
	m_ErrorMutex.unlock();
}

void WGTWorker::run()
{
	m_szUrl = m_pProvMng->getUrl(m_uiId);
	if (m_szUrl == "NULL")
	{
		Warning(gcString("Mcf Download Thread [{0}] failed to get valid url for download.\n", m_uiId));
		return;
	}

	gcString name = m_pProvMng->getName(m_uiId);

	m_pMcfCon = new MCFCore::Misc::MCFServerCon();
	m_pMcfCon->setDPInformation(name.c_str());
	m_pMcfCon->onProgressEvent += delegate(this, &WGTWorker::onProgress);

	while (!isStopped())
	{
		uint32 status = m_pCT->getStatus(m_uiId);

		uint32 pauseCount = 0;

		bool isPaused= (status == MCFCore::Thread::BaseMCFThread::SF_STATUS_PAUSE);

		if (isPaused)
			m_pMcfCon->onPause();

		while (status == MCFCore::Thread::BaseMCFThread::SF_STATUS_PAUSE)
		{
			if (pauseCount > 30000 && m_pMcfCon->isConnected())
				m_pMcfCon->disconnect();

			gcSleep(500);
			pauseCount += 500;

			status = m_pCT->getStatus(m_uiId);
		}

		if (isPaused)
		{
			gcString name = m_pProvMng->getName(m_uiId);
			m_pMcfCon->setDPInformation(name.c_str());
		}

		if (status == MCFCore::Thread::BaseMCFThread::SF_STATUS_STOP)
			break;

		m_ErrorMutex.lock();
		if (m_bError)
		{
			requestNewUrl(m_Error);
			m_bError = false;
		}
		m_ErrorMutex.unlock();

		if (status == MCFCore::Thread::BaseMCFThread::SF_STATUS_CONTINUE)
			doDownload();
	}

	m_pProvMng->removeAgent(m_uiId);

	m_DeleteMutex.lock();
	safe_delete(m_pMcfCon);
	m_DeleteMutex.unlock();

	//need to do this incase we are the last thread and the controller is stuck on the wait mutex
	m_pCT->pokeThread();
}

void WGTWorker::reset()
{
}

bool WGTWorker::writeData(char* data, uint32 size)
{
	if (isStopped())
		return false;

	if (m_bError)
		return false;

	if (size == 0)
		return true;

	if (!m_pCurBlock)
		return false;

	MCFCore::Thread::Misc::WGTBlock* block = NULL;

	m_pCurBlock->m_Lock.lock();

	if (m_pCurBlock->vBlockList.size() != 0)
		block = m_pCurBlock->vBlockList[0];

	m_pCurBlock->m_Lock.unlock();

	if (!block)
		return false;

	size_t done = m_pCurBlock->done;

	if (done == 0)
	{
		safe_delete(block->buff);
		block->buff =  new char[block->size];
	}

	size_t ds = block->size - done;

	if (size >= ds)
	{
		block->dlsize = block->size;
		block->provider = m_pProvMng->getName(m_uiId);

		memcpy(block->buff+done, data, ds);

#ifdef DEBUG
		//checkBlock(block);
#endif

		m_pCT->workerFinishedBlock(m_uiId, block);

		m_pCurBlock->m_Lock.lock();

			m_pCurBlock->vBlockList.pop_front();
			m_pCurBlock->done = 0;

			m_pCurBlock->offset += block->size;
			m_pCurBlock->size -= block->size;

		m_pCurBlock->m_Lock.unlock();
		
		writeData(data+ds, size-ds);
	}
	else
	{
		memcpy(block->buff+done, data, size);
		m_pCurBlock->done += size;
	}

	if (m_pCT->getStatus(m_uiId) == MCFCore::Thread::BaseMCFThread::SF_STATUS_PAUSE)
		return false;

	return true;
}


bool WGTWorker::checkBlock(Misc::WGTBlock *block)
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

	return false;
}


void WGTWorker::doDownload()
{
	assert(m_pMcfCon);

	if (!m_pCurBlock)
	{
		uint32 status;
		m_pCurBlock = m_pCT->newTask(m_uiId, status);

		if (!m_pCurBlock)
		{
			if (status != MCFCore::Thread::BaseMCFThread::SF_STATUS_STOP)
				Warning(gcString("The block was NULL for Mcf Download thread {0}\n", m_uiId));

			return;
		}
	}

	try
	{
		if (m_szUrl.size() == 0 || m_szUrl == "NULL")
			throw gcException(ERR_MCFSERVER, "No more download servers to use.");

		if (!m_pMcfCon->isConnected())
			m_pMcfCon->connect(m_szUrl.c_str(), m_pFileAuth);

		m_pMcfCon->downloadRange(m_pCurBlock->offset, m_pCurBlock->size, this);
	}
	catch (gcException &excep)
	{
		if (excep.getErrId() == ERR_MCFSERVER && excep.getSecErrId() == ERR_USERCANCELED)
		{
			//do nothing. Block errored out before or client paused.
		}
		else if (!isStopped())
		{
			Warning(gcString("Mcf Server error: {0} [{1}]\n", excep, m_szUrl));

			if (excep.getErrId() == ERR_LIBCURL)
			{
				//dont do any thing, just retry unless this is our third time
				if (isGoodSocketError(excep.getSecErrId()) && m_iAttempt < 3)
					m_iAttempt++;
				else
					requestNewUrl(excep);
			}
			else if (excep.getErrId() == ERR_MCFSERVER)
			{
				requestNewUrl(excep);
			}
			else
			{
				m_pCT->reportError(m_uiId, excep);
			}
		}

		takeProgressOff();
		return;
	}

	m_pCT->workerFinishedSuperBlock(m_uiId);
	m_pCurBlock = NULL;
}


bool WGTWorker::isGoodSocketError(uint32 errId)
{
	switch (errId)
	{
	case 28: //CURL_TIMEOUT
	case 42: //CURLE_ABORTED_BY_CALLBACK:
	case 81: //CURLE_AGAIN:
		return true;
	};

	return false;
}

void WGTWorker::onStop()
{
	m_DeleteMutex.lock();

	if (m_pMcfCon)
		m_pMcfCon->disconnect();

	m_DeleteMutex.unlock();

	join();
}

void WGTWorker::takeProgressOff()
{
	m_pCT->reportNegProgress(m_uiId, m_pCurBlock->done);
	m_pCurBlock->done = 0;

	m_pCT->workerFinishedSuperBlock(m_uiId);
	m_pCurBlock = NULL;
}

void WGTWorker::onProgress(uint32& prog)
{
	m_pCT->reportProgress(m_uiId, prog);
}

void WGTWorker::requestNewUrl(gcException& e)
{
	m_szUrl = m_pProvMng->requestNewUrl(m_uiId, e.getSecErrId(), e.getErrMsg());

	if (m_szUrl != "NULL")
	{
		m_iAttempt = 0;
		m_pMcfCon->disconnect();

		gcString name = m_pProvMng->getName(m_uiId);
		m_pMcfCon->setDPInformation(name.c_str());
	}
	else
	{
		gcException e(ERR_MCFSERVER, "No more download servers to use.");
		m_pCT->reportError(m_uiId, e);
		m_pCT->pokeThread();
	}
}



}
}
