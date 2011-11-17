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

#ifndef DESURA_COMPLEXLAUNCHPROCESS_H
#define DESURA_COMPLEXLAUNCHPROCESS_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "mcfcore/ProgressInfo.h"

namespace MCFCore{ class MCFI; }

class ComplexLaunchProcess : public Thread::BaseThread
{
public:
	ComplexLaunchProcess();
	~ComplexLaunchProcess();

	void setRemove(const char* modMcfPath, const char* backupMcfPath, const char* installPath, const char* installScript);
	void setInstall(const char* mcfPath, const char* installPath, const char* installScript);

	EventV onCompleteEvent;
	Event<gcException> onErrorEvent;
	Event<uint64> onProgressEvent;

protected:
	void run();
	void onStop();

	void onError(gcException& e);
	void onProgress(MCFCore::Misc::ProgressInfo& p);

	void doRemove();
	void doInstall();

private:
	gcString m_szIPath;
	gcString m_szInsMCFPath;
	gcString m_szRemMCFPath;

	gcString m_szInstallScript;

	bool m_bHashMissMatch;
	bool m_iFirstStage;
	uint8 m_iMode;
	uint64 m_uiLastProg;
};

#endif //DESURA_COMPLEXLAUNCHPROCESS_H
