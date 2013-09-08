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

#ifndef DESURA_IPCINSTALLMCF_H
#define DESURA_IPCINSTALLMCF_H
#ifdef _WIN32
#pragma once
#endif


#include "IPCClass.h"
#include "mcfcore/ProgressInfo.h"

class InstallProcess;


class IPCInstallMcf : public IPC::IPCClass
{
public:
	IPCInstallMcf(IPC::IPCManager* mang, uint32 id, DesuraId itemId);
	~IPCInstallMcf();

	void start(const char* mcfpath, const char* inspath, const char* installScript, uint8 workercount = 1, bool delfiles = false, bool makeWriteable=false);
	void pause();
	void unpause();
	void stop();

	EventV onCompleteEvent;
	EventV onFinishEvent;
	Event<gcException> onErrorEvent;

#ifdef DESURA_CLIENT
public:
	Event<MCFCore::Misc::ProgressInfo> onProgressEvent;
protected:
	void onProgress(uint64& prog);
#endif

protected:
	InstallProcess* m_pThread;
	Event<uint64> onProgressIPCEvent;

private:
	void registerFunctions();
};



#endif //DESURA_IPCINSTALLMCF_H
