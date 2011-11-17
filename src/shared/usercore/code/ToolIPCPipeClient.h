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

#ifndef DESURA_TOOLIPCPIPECLIENT_H
#define DESURA_TOOLIPCPIPECLIENT_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCPipeClient.h"


class IPCToolMain;

namespace UserCore
{

class ToolIPCPipeClient : public IPC::PipeClient
{
public:
	ToolIPCPipeClient(const char* user, bool uploadDumps, const char* key, HWND handle);
	~ToolIPCPipeClient();

	void start();

	IPCToolMain* getToolMain();

protected:
	void stopHelper();
	void startHelper();

	void onDisconnect();

	void tryStart();

private:
	bool m_bUploadDumps;

	gcString m_szUser;
	gcString m_szKey;

	IPCToolMain *m_pServiceMain;

	HWND m_WinHandle;
};


}

#endif //DESURA_TOOLIPCPIPECLIENT_H
