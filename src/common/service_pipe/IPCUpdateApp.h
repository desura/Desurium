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

#ifndef DESURA_IPCUPDATEAPP_H
#define DESURA_IPCUPDATEAPP_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCClass.h"

class GCUpdateProcess;

class IPCUpdateApp : public IPC::IPCClass
{
public:
	IPCUpdateApp(IPC::IPCManager* mang, uint32 id, DesuraId itemId);
	~IPCUpdateApp();

	void start(const char* mcfpath, const char* inspath, bool testMode);
	void stop();

	EventV onCompleteEvent;
	Event<gcException> onErrorEvent;
	Event<uint32> onProgressEvent;

	bool updateService(const char* installPath);
	bool fixDataDir();

protected:
	GCUpdateProcess* m_pThread;

private:
	void registerFunctions();
};

#endif //DESURA_IPCUPDATEAPP_H
