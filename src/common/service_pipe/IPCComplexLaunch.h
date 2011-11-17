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

#ifndef DESURA_IPCCOMPLEXLAUNCH_H
#define DESURA_IPCCOMPLEXLAUNCH_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCClass.h"
#include "mcfcore/ProgressInfo.h"

class ComplexLaunchProcess;

class IPCComplexLaunch : public IPC::IPCClass
{
public:
	IPCComplexLaunch(IPC::IPCManager* mang, uint32 id, DesuraId itemId);
	~IPCComplexLaunch();

	void startRemove(const char* modMcfPath, const char* backupMcfPath, const char* installPath, const char* installScript);
	void startInstall(const char* mcfPath, const char* installPath, const char* installScript);
	void stop();

	EventV onCompleteEvent;
	Event<gcException> onErrorEvent;

#ifdef DESURA_CLIENT
public:
	Event<MCFCore::Misc::ProgressInfo> onProgressEvent;
protected:
	void onProgress(uint64& prog);
#endif

private:
	Event<uint64> onProgressIPCEvent;

	ComplexLaunchProcess* m_pThread;
	void registerFunctions();
};

#endif //DESURA_IPCCOMPLEXLAUNCH_H
