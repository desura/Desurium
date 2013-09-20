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
#include "WGTWorker.h"
#include "WGTController.h"
#include "ProviderManager.h"


namespace MCFCore
{
namespace Thread
{

WGTWorker::WGTWorker(WGTControllerI* controller, uint16 id, MCFCore::Misc::ProviderManager *pProvMng, MCFCore::Misc::GetFile_s* pFileAuth) 
	: BaseThread( "WebGet Worker Thread" )
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

		m_pMcfCon->downloadRange(m_pCurBlock->offset + m_pCurBlock->done, m_pCurBlock->size - m_pCurBlock->done, this);
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

	if (m_pCurBlock->size == 0)
	{
		m_pCT->workerFinishedSuperBlock(m_uiId);
		m_pCurBlock = NULL;
	}
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


#ifdef WITH_GTEST

#include <gtest/gtest.h>

namespace UnitTest
{
	class StubMCFServerCon : public MCFCore::Misc::MCFServerConI
	{
	public:
		StubMCFServerCon(int nDownloadSize, int nMaxRequestSize)
			: m_nDownloadSize(nDownloadSize)
			, m_nMaxRequestSize(nMaxRequestSize)
		{
		}
		
		void connect(const char* host, MCFCore::Misc::GetFile_s* fileAuth) override
		{
		}

		void disconnect() override
		{
		}

		void setDPInformation(const char* name) override
		{
		}

		void downloadRange(uint64 offset, uint32 size, MCFCore::Misc::OutBufferI* buff) override
		{
			gcBuff temp(m_nDownloadSize);
			char* szBuff = temp.c_ptr();

			if (size > m_nMaxRequestSize)
				size = m_nMaxRequestSize;

			while (size > 0)
			{
				int todo = m_nDownloadSize;

				if (todo > size)
					todo = size;

				for (uint32 x = 0; x < todo; ++x)
					szBuff[x] = (char)(offset + x);

				buff->writeData(szBuff, todo);

				size -= todo;
				offset += todo;
			}
		}

		void onPause() override
		{
		}

		bool isConnected() override
		{
			return true;
		}

		const int m_nDownloadSize;
		const int m_nMaxRequestSize;
	};

	class StubWGTController : public MCFCore::Thread::WGTControllerI
	{
	public:
		StubWGTController()
			: m_bSuperCompleted(false)
		{
		}

		MCFCore::Thread::Misc::WGTSuperBlock* newTask(uint32 id, uint32 &status) override
		{
			return &m_SuperBlock;
		}

		uint32 getStatus(uint32 id) override
		{
			return MCFCore::Thread::BaseMCFThread::SF_STATUS_CONTINUE;
		}

		void reportError(uint32 id, gcException &e) override
		{
		}

		void reportProgress(uint32 id, uint64 ammount) override
		{
		}

		void reportNegProgress(uint32 id, uint64 ammount) override
		{
		}

		void workerFinishedBlock(uint32 id, MCFCore::Thread::Misc::WGTBlock* block) override
		{
			m_vCompletedBlocks.push_back(block);
		}

		void workerFinishedSuperBlock(uint32 id) override
		{
			m_bSuperCompleted = true;
		}

		void pokeThread() override
		{
		}

		bool m_bSuperCompleted;
		std::vector<MCFCore::Thread::Misc::WGTBlock*> m_vCompletedBlocks;
		MCFCore::Thread::Misc::WGTSuperBlock m_SuperBlock;
	};


	class TestWGTWorker : public MCFCore::Thread::WGTWorker
	{
	public:
		TestWGTWorker(MCFCore::Thread::WGTControllerI* controller, uint16 id, MCFCore::Misc::ProviderManager *pProvMng, MCFCore::Misc::GetFile_s* pFileAuth)
			: MCFCore::Thread::WGTWorker(controller, id, pProvMng, pFileAuth)
		{
			m_szUrl = "UnitTest";
		}

		void download(StubMCFServerCon *pMcfCon, int nRunCount)
		{
			safe_delete(m_pMcfCon);
			m_pMcfCon = pMcfCon;

			for (int x = 0; x < nRunCount; x++)
				doDownload();
		}
	};

	class WGTWorkerFixture : public ::testing::TestWithParam<std::pair<int, int>>
	{
	public:
		WGTWorkerFixture()
			: m_nParamOne(GetParam().first)
			, m_nParamTwo(GetParam().second)
			, Provider("", "", "", "")
			, ProviderManager(getProviderVector())
			, Worker(&Controller, 1, &ProviderManager, &FileAuth)
		{
		}

		void AddBlock(int nSize)
		{
			std::shared_ptr<MCFCore::Thread::Misc::WGTBlock> a(new MCFCore::Thread::Misc::WGTBlock());
			a->size = nSize;
			a->fileOffset = Controller.m_SuperBlock.size;
			a->webOffset = Controller.m_SuperBlock.size;

			unsigned char* szTemp = new unsigned char[nSize];

			for (int x = 0; x < nSize; x++)
				szTemp[x] = (unsigned char)(Controller.m_SuperBlock.size + x);

			a->crc = UTIL::MISC::CRC32(szTemp, nSize);
			a->index = m_vBlocks.size();

			safe_delete(szTemp);

			Controller.m_SuperBlock.size += nSize;
			Controller.m_SuperBlock.vBlockList.push_back(a.get());
			m_vBlocks.push_back(a);
		}

		void DoDownload(int nDownloadSize, int nMaxRequestSize=-1)
		{
			int nRunCount = (Controller.m_SuperBlock.size / nMaxRequestSize) + 1;

			Worker.download(new StubMCFServerCon(nDownloadSize, nMaxRequestSize), nRunCount);
		}

		void CheckCompletedBlocks()
		{
			ASSERT_TRUE(Controller.m_bSuperCompleted);
			ASSERT_EQ(m_vBlocks.size(), Controller.m_vCompletedBlocks.size());

			for (size_t x = 0; x < m_vBlocks.size(); x++)
			{
				unsigned long crc = UTIL::MISC::CRC32((unsigned char*)m_vBlocks[x]->buff, m_vBlocks[x]->size);
				ASSERT_EQ(m_vBlocks[x]->crc, crc);
			}
		}

		int m_nParamOne;
		int m_nParamTwo;

		std::vector<MCFCore::Misc::DownloadProvider*>& getProviderVector()
		{
			Providers.push_back(&Provider);
			return Providers;
		}

		std::vector<MCFCore::Misc::DownloadProvider*> Providers;
		MCFCore::Misc::DownloadProvider Provider;
		MCFCore::Misc::ProviderManager ProviderManager;
		
		std::vector<std::shared_ptr<MCFCore::Thread::Misc::WGTBlock>> m_vBlocks;
		TestWGTWorker Worker;
		StubWGTController Controller;

		MCFCore::Misc::GetFile_s FileAuth;	
	};


	INSTANTIATE_TEST_CASE_P(DownloadBufferSize,
		WGTWorkerFixture,
		::testing::Values(std::make_pair(1, 1), std::make_pair(5, 1), std::make_pair(10, 1),
				std::make_pair(1, 5), std::make_pair(5, 5), std::make_pair(10, 1),
				std::make_pair(1, 10), std::make_pair(5, 10), std::make_pair(10, 10)));

	TEST_P(WGTWorkerFixture, Download_SuperBlock_VarDownloadSize)
	{
		for (int x = 0; x < 5; x++)
			AddBlock(5);

		DoDownload(GetParam().first, GetParam().second);
		CheckCompletedBlocks();
	}

	TEST_P(WGTWorkerFixture, Download_SuperBlock_VarBlockSize)
	{
		for (int x = 0; x < 5; x++)
			AddBlock(GetParam().first);

		DoDownload(5, GetParam().second);
		CheckCompletedBlocks();
	}

	TEST_P(WGTWorkerFixture, Download_SuperBlock_VarBlockCount)
	{
		for (int x = 0; x < GetParam().first; x++)
			AddBlock(5);

		DoDownload(5, GetParam().second);
		CheckCompletedBlocks();
	}
}

#endif