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

#ifndef DESURA_WTWORKERTHREAD_H
#define DESURA_WTWORKERTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "MCFServerCon.h"
#include "mcfcore/MCFI.h"

class webControl;
class OutToBlock;

namespace MCFCore
{
namespace Misc
{
	class ProviderManager;
	class MCFServerCon;
}

namespace Thread
{
class WGTControllerI;

namespace Misc
{
	class WGTSuperBlock;
	class WGTBlock;
}

//! Web get thread worker. Does all the work in regards to downloading mcf files
//!
class WGTWorker : public ::Thread::BaseThread, protected MCFCore::Misc::OutBufferI
{
public:
	//! Constuctor
	//! 
	//! @param controller Parent class
	//! @param id Worker id
	//! @param pProvMng Provider manager
	//! @param pFileAuth Mcf download auth
	//!
	WGTWorker(WGTControllerI* controller, uint16 id, MCFCore::Misc::ProviderManager *pProvMng, MCFCore::Misc::GetFile_s* pFileAuth);
	~WGTWorker();

	//! Reports a download error from controller
	//!
	void reportError(gcException &e, gcString provider);

protected:
	void run();
	void onStop();

	//! Does the download block work
	//!
	void doDownload();

	//! On progress event callback
	//!
	//! @param prog Current progress
	//!
	void onProgress(uint32& prog);

	void takeProgressOff();
	void requestNewUrl(gcException& e);

	bool isGoodSocketError(uint32 errId);

	virtual bool writeData(char* data, uint32 size);
	virtual void reset();

	bool checkBlock(MCFCore::Thread::Misc::WGTBlock *block);

private:
	gcString m_szUrl;
	MCFCore::Misc::GetFile_s* m_pFileAuth;

	uint32 m_iAttempt;
	uint32 m_uiId;

	::Thread::Mutex m_DeleteMutex;

	MCFCore::Thread::WGTControllerI *m_pCT;
	MCFCore::Thread::Misc::WGTSuperBlock *m_pCurBlock;
	MCFCore::Misc::MCFServerCon *m_pMcfCon;
	MCFCore::Misc::ProviderManager *m_pProvMng;


	::Thread::Mutex m_ErrorMutex;
	bool m_bError;
	gcException m_Error;
};

}
}

#endif //DESURA_WTWORKERTHREAD_H
