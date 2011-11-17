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

#ifndef DESURA_INSTALLTHREAD_H
#define DESURA_INSTALLTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "mcfcore/ProgressInfo.h"

namespace MCFCore{ class MCFI; }

class InstallProcess : public Thread::BaseThread
{
public:
	InstallProcess(const char* mcfpath, const char* inspath, const char* installScript, uint8 workercount, bool delfiles, bool makeWriteable);
	~InstallProcess();

	EventV onCompleteEvent;
	EventV onFinishEvent;
	Event<gcException> onErrorEvent;
	Event<uint64> onProgressEvent;

protected:
	void run();
	void onStop();
	void onPause();
	void onUnpause();

	void onError(gcException& e);
	void onProgress(MCFCore::Misc::ProgressInfo& p);

private:
	bool m_bDelFiles;
	bool m_bHashMissMatch;
	bool m_bMakeWriteable;
	bool m_bHasHadError;

	uint32 m_uiWorkerCount;
	uint64 m_uiLastProg;

	gcString m_szIPath;
	gcString m_szMCFPath;
	gcString m_szInstallScript;

	MCFCore::MCFI* m_pMcfHandle;
};

#endif //DESURA_INSTALLTHREAD_H
