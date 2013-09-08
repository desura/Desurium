/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_CEFPIPECLIENT_H
#define DESURA_CEFPIPECLIENT_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCPipeClient.h"
#include "browser_pipe/IPCBrowser.h"

class CefIPCPipeClient : public IPC::PipeClient
{
public:
	CefIPCPipeClient(const char* pipeId, const char* hostName);
	~CefIPCPipeClient();

	void start();

	IPCBrowser* createBrowser(HWND hwnd, const char* defaultUrl);

protected:
	void stopService();
	void startService();
	void onDisconnect();

private:
	std::string m_szPipeId;
	std::string m_szHostName;
};


#endif //DESURA_CEFPIPECLIENT_H
