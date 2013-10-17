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

#ifndef DESURA_SERVICECORE_H
#define DESURA_SERVICECORE_H
#ifdef _WIN32
#pragma once
#endif

#include "ServiceCoreI.h"
#include "IPCPipeServer.h"

namespace IPC
{
	class PipeServer;
}

class ServiceCore : public ServiceCoreI
{
public:
	ServiceCore();
	~ServiceCore();


	virtual void startPipe();
	virtual void stopPipe();

	virtual bool stopService(const char* serviceName);

	virtual void destroy();

	virtual void setCrashSettingCallback(CrashSettingFn crashSettingFn);
	virtual void setDisconnectCallback(DisconnectFn disconnectFn);

	void setCrashSettings(const wchar_t* user, bool upload);

protected:
	void onDisconnect(uint32 &);

private:
	IPC::PipeServer *m_pIPCServer;

	CrashSettingFn m_pCrashSetting;
	DisconnectFn m_pDisconnect;

};

#endif //DESURA_SERVICECORE_H
