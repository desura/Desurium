/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>
          (C) Mark Chandler <mark@moddb.com>
          (C) Wojciech Zylinski <voitek@boskee.co.uk>

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

#include "stdafx.h"

#define UTILWEB_INCLUDE

#include "Common.h"
#include "AppUpdateInstall.h"
#include "IPCPipeClient.h"
#include "IPCManager.h"
#include "IPCUpdateApp.h"

#include "util/UtilWindows.h"
#include <branding/branding.h>

class AppUpdateInstall::Privates
{
public:
	gcString m_szMcfPath;
	gcString m_szInsPath;

	ProgressReportI* m_pProgressReport;

	int m_iResult;
	bool m_bTestMode;
};

AppUpdateInstall::AppUpdateInstall(ProgressReportI* progressReport, bool testMode)
{
	m_pPrivates = new Privates();

	m_pPrivates->m_bTestMode = testMode;
	m_pPrivates->m_iResult = -4;

	m_pPrivates->m_pProgressReport = progressReport;

	char temp[255] = {0};
	GetCurrentDirectory(255, temp);

	m_pPrivates->m_szInsPath = temp;
	
	if (m_pPrivates->m_bTestMode)
		m_pPrivates->m_szMcfPath = "desura_update.mcf";
	else
		m_pPrivates->m_szMcfPath = UTIL::OS::getAppDataPath(UPDATEFILE_W);

#ifdef DEBUG
	m_pPrivates->m_szMcfPath = "desura_update.mcf";
	m_pPrivates->m_szInsPath = ".\\test_install\\";
#endif

	m_pEvent = CreateEvent(NULL, true, false, NULL);
}

AppUpdateInstall::~AppUpdateInstall()
{
	SetEvent(m_pEvent);
	CloseHandle(m_pEvent);

	try
	{
		stopService();
	}
	catch (gcException)
	{
	}

	safe_delete(m_pPrivates);
}

void AppUpdateInstall::startService()
{
	uint32 res = UTIL::WIN::queryService(SERVICE_NAME);

	if (res != SERVICE_STATUS_STOPPED)
	{
		try
		{
			UTIL::WIN::stopService(SERVICE_NAME);
			gcSleep(500);
		}
		catch (gcException &)
		{
		}
	}

	std::vector<std::string> args;

	args.push_back("-wdir");
	args.push_back(gcString(UTIL::OS::getCurrentDir()));

	UTIL::WIN::startService(SERVICE_NAME, args);

	uint32 count = 0;
	while (UTIL::WIN::queryService(SERVICE_NAME) != SERVICE_STATUS_RUNNING)
	{
		//wait 10 seconds
		if (count > 40)
			throw gcException(ERR_SERVICE, "Failed to start desura Service (PipeManager).");

		gcSleep(250);
		count++;
	}
}

void AppUpdateInstall::stopService()
{
	UTIL::WIN::stopService(SERVICE_NAME);
}

void AppUpdateInstall::onIPCDisconnect()
{
	m_pPrivates->m_iResult = 4;
	SetEvent(m_pEvent);
}

int AppUpdateInstall::run()
{
	m_pPrivates->m_iResult = -1;

	try
	{
		startService();

		IPC::PipeClient pc("DesuraIS");
		pc.onDisconnectEvent += delegate(this, &AppUpdateInstall::onIPCDisconnect);

		pc.setUpPipes();
		pc.start();

		IPCUpdateApp* ipua = IPC::CreateIPCClass< IPCUpdateApp >(&pc, "IPCUpdateApp");

		if (!ipua)
		{
			m_pPrivates->m_iResult = 2;
			throw gcException(ERR_NULLHANDLE, "Failed to create UpdateApp service.");
		}

		ipua->onCompleteEvent += delegate(this, &AppUpdateInstall::onComplete);
		ipua->onErrorEvent += delegate(this, &AppUpdateInstall::onError);
		ipua->onProgressEvent += delegate(this, &AppUpdateInstall::onProgress);

		ipua->start(m_pPrivates->m_szMcfPath.c_str(), m_pPrivates->m_szInsPath.c_str(), m_pPrivates->m_bTestMode);
		WaitForSingleObject(m_pEvent, INFINITE);


		ipua->onCompleteEvent -= delegate(this, &AppUpdateInstall::onComplete);
		ipua->onErrorEvent -= delegate(this, &AppUpdateInstall::onError);
		ipua->onProgressEvent -= delegate(this, &AppUpdateInstall::onProgress);

		pc.onDisconnectEvent -= delegate(this, &AppUpdateInstall::onIPCDisconnect);

		uint32 p = 100;

		if (m_pPrivates->m_pProgressReport)
			m_pPrivates->m_pProgressReport->reportProgress(p);
	}
	catch (gcException &e)
	{
		m_pPrivates->m_iResult = 3;
		onError(e);
	}

	try
	{
		stopService();
	}
	catch(...)
	{
	}

	return m_pPrivates->m_iResult;
}


void AppUpdateInstall::onError(gcException& e)
{
	if (m_pPrivates->m_bTestMode)
	{
		m_pPrivates->m_iResult = e.getErrId();
		return;
	}

	if (e.getErrId() != ERR_INVALIDFILE && e.getErrId() != ERR_FAILEDSEEK)
	{
		char mbmsg[255];
		Safe::snprintf(mbmsg, 255, PRODUCT_NAME " has had a critical error while updating.\n\n%s [%d.%d]", e.getErrMsg(), e.getErrId(), e.getSecErrId());
		::MessageBox(NULL, mbmsg, PRODUCT_NAME " Critical Update Error", MB_OK);
	}
	else
	{
		m_pPrivates->m_iResult = 0;
	}
}

void AppUpdateInstall::onProgress(uint32& p)
{
	if (m_pPrivates->m_pProgressReport)
		m_pPrivates->m_pProgressReport->reportProgress(p);
}

void AppUpdateInstall::onComplete()
{
	SetEvent(m_pEvent);
	m_pPrivates->m_iResult = 0;
}
