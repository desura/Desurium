/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#ifndef DESURA_WEBGETTHREAD_H
#define DESURA_WEBGETTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseMCFThread.h"
#include "WGTControllerI.h"

#include "WGTExtras.h"
#include "mcfcore/DownloadProvider.h"
#include "mcfcore/MCFI.h"

namespace MCFCore
{
namespace Misc
{
	class ProviderManager;
}

namespace Thread
{
class WGTWorkerInfo;


//! Web Get Thread controller. Downloads mcf from mcf servers
//!
class WGTController : public MCFCore::Thread::BaseMCFThread, protected WGTControllerI
{
public:
	//! Constuctor
	//!
	//! @param source Download provider list
	//! @param numWorkers Number of children to spawn to download the mcf
	//! @param caller Parent Mcf
	//! @param checkMcf Check the Mcf for downloaded chunks before starting
	//!
	WGTController(std::vector<MCFCore::Misc::DownloadProvider*> &source, uint16 numWorkers, MCFCore::MCF* caller, bool checkMcf);
	~WGTController();
	
	//! Provider event
	//!
	Event<MCFCore::Misc::DP_s> onProviderEvent;

protected:
	//! Has blocks ready to written to the mcf file
	//!
	//! @return True if blocks are ready, false if not
	//!
	bool isQuedBlocks();

	//! Finds a Worker given a worker id
	//!
	//! @param id worker id
	//! @return Worker
	//!
	WGTWorkerInfo* findWorker(uint32 id);

	//! Fills the block list with all the blocks needed to be downloaded
	//!
	//! @return True if compelted, False if error occured
	//!
	bool fillBlockList();

	//! Creates the workers
	//!
	void createWorkers();

	//! Are all workers done
	//!
	//! @return True if all done, false if not
	//!
	bool workersDone();

	//! Save completed blocks to the mcf file
	//!
	//! @param fileHandle Mcf file to save to
	//! @param allBlocks Save all blocks instead of one
	//!
	void saveBuffers(UTIL::FS::FileHandle& fileHandle, bool allBlocks = false);

	//inhereted from BaseThread
	void run();
	void onStop();

	//! Checks a block for errors
	//!
	bool checkBlock(Misc::WGTBlock *block, uint32 workerId);

	//! Steals blocks from other workes
	//!
	//! @return true if blocks stolen
	//!
	bool stealBlocks();

	virtual Misc::WGTSuperBlock* newTask(uint32 id, uint32 &status);
	virtual uint32 getStatus(uint32 id);
	virtual void reportError(uint32 id, gcException &e);
	virtual void reportProgress(uint32 id, uint64 ammount);
	virtual void reportNegProgress(uint32 id, uint64 ammount);
	virtual void workerFinishedBlock(uint32 id, Misc::WGTBlock* block);
	virtual void workerFinishedSuperBlock(uint32 id);
	virtual void pokeThread();

private:
	MCFCore::Misc::GetFile_s* m_pFileAuth;
	MCFCore::Misc::ProviderManager* m_pProvManager;


	uint32 m_iAvailbleWork;
	uint32 m_iRunningWorkers;

	bool m_bCheckMcf;
	volatile bool m_bDoingStop;

	std::vector<WGTWorkerInfo*> m_vWorkerList;
	std::deque<Misc::WGTSuperBlock*> m_vSuperBlockList;
	std::vector<uint32> m_vDlFiles;

	::Thread::WaitCondition m_WaitCondition;

#ifdef DEBUG
	uint64 m_uiSaved;
#endif
};

}
}

#endif //DESURA_WEBGETTHREAD_H
