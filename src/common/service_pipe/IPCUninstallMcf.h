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

#ifndef DESURA_IPCUNINSTALLMCF_H
#define DESURA_IPCUNINSTALLMCF_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCClass.h"
#include "mcfcore/ProgressInfo.h"

class UninstallProcess;

class IPCUninstallMcf : public IPC::IPCClass
{
public:
	IPCUninstallMcf(IPC::IPCManager* mang, uint32 id, DesuraId itemId);
	~IPCUninstallMcf();

	void start(const char* mcfpath, const char* inspath, const char* installScript);
	void pause();
	void unpause();
	void stop();

	EventV onCompleteEvent;
	Event<gcException> onErrorEvent;


#ifdef DESURA_CLIENT
public:
	Event<MCFCore::Misc::ProgressInfo> onProgressEvent;
protected:
	void onProgress(uint64& prog);
#endif

protected:
	UninstallProcess* m_pThread;
	Event<uint64> onProgressIPCEvent;

private:
	void registerFunctions();
};

#endif //DESURA_IPCUNINSTALLMCF_H
