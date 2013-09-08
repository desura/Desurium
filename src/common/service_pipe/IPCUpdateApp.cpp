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

#pragma warning( disable : 4627 )

#ifdef DESURA_EXE
	#include "stdafx.h"
#endif

#include "Common.h"
#include "IPCUpdateApp.h"
#include "IPCManager.h"


#ifndef DESURA_CLIENT
	#include "UpdateProcess.h"
#endif

REG_IPC_CLASS( IPCUpdateApp );


IPCUpdateApp::IPCUpdateApp(IPC::IPCManager* mang, uint32 id, DesuraId itemId) : IPC::IPCClass(mang, id, itemId)
{
	registerFunctions();
	m_pThread = NULL;
}

IPCUpdateApp::~IPCUpdateApp()
{
#ifndef DESURA_CLIENT
	safe_delete(m_pThread);
#endif
}


void IPCUpdateApp::registerFunctions()
{
#ifndef DESURA_CLIENT
	REG_FUNCTION_VOID( IPCUpdateApp, stop );
	REG_FUNCTION_VOID( IPCUpdateApp, start );

	LINK_EVENT( onProgressEvent, uint32 );
	LINK_EVENT( onErrorEvent, gcException );
	LINK_EVENT_VOID( onCompleteEvent );

	REG_FUNCTION( IPCUpdateApp, updateService);
	REG_FUNCTION( IPCUpdateApp, fixDataDir);
#else
	REG_EVENT( onProgressEvent );
	REG_EVENT( onErrorEvent );
	REG_EVENT_VOID( onCompleteEvent );
#endif
}


#ifdef DESURA_CLIENT
void IPCUpdateApp::start(const char* mcfpath, const char* inspath, bool testMode)
{
	IPC::functionCallV(this, "start", mcfpath, inspath, testMode);
}

void IPCUpdateApp::stop()
{
	IPC::functionCallV(this, "stop");
}

bool IPCUpdateApp::updateService(const char* installPath)
{
	return IPC::functionCall<bool>(this, "updateService", installPath);
}

bool IPCUpdateApp::fixDataDir()
{
	return IPC::functionCall<bool>(this, "fixDataDir");
}

#else

void IPCUpdateApp::start(const char* mcfpath, const char* inspath, bool testMode)
{
	if (m_pThread)
		return;

	m_pThread = new GCUpdateProcess(mcfpath, inspath, testMode);
	m_pThread->onErrorEvent += delegate(&onErrorEvent);
	m_pThread->onCompleteEvent += delegate(&onCompleteEvent);
	m_pThread->onProgressEvent += delegate(&onProgressEvent);

	m_pThread->start();
}

void IPCUpdateApp::stop()
{
	if (m_pThread)
		m_pThread->nonBlockStop();
}

bool IPCUpdateApp::updateService(const char* installPath)
{
	return GCUpdateProcess::updateService(installPath);
}

bool IPCUpdateApp::fixDataDir()
{
	return GCUpdateProcess::fixDataDir();
}

#endif

