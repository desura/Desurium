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

#include "Common.h"
#include "IPCComplexLaunch.h"
#include "IPCManager.h"

#ifndef DESURA_CLIENT
	#include "ComplexLaunchProcess.h"
#endif


REG_IPC_CLASS( IPCComplexLaunch );


IPCComplexLaunch::IPCComplexLaunch(IPC::IPCManager* mang, uint32 id, DesuraId itemId) : IPC::IPCClass(mang, id, itemId)
{
	registerFunctions();
	m_pThread = NULL;
}

IPCComplexLaunch::~IPCComplexLaunch()
{
#ifndef DESURA_CLIENT
	safe_delete(m_pThread);
#endif
}

void IPCComplexLaunch::registerFunctions()
{
#ifndef DESURA_CLIENT
	REG_FUNCTION_VOID( IPCComplexLaunch, startRemove );
	REG_FUNCTION_VOID( IPCComplexLaunch, startInstall );

	REG_FUNCTION_VOID( IPCComplexLaunch, stop );

	LINK_EVENT( onProgressIPCEvent, uint64 );
	LINK_EVENT( onErrorEvent, gcException );
	LINK_EVENT_VOID( onCompleteEvent );
#else
	REG_EVENT( onProgressIPCEvent );
	REG_EVENT( onErrorEvent );
	REG_EVENT_VOID( onCompleteEvent );

	onProgressIPCEvent += delegate(this, &IPCComplexLaunch::onProgress);
#endif
}

#ifdef DESURA_CLIENT

void IPCComplexLaunch::startRemove(const char* modMcfPath, const char* backupMcfPath, const char* installPath, const char* installScript)
{
	IPC::functionCallV(this, "startRemove", modMcfPath, backupMcfPath, installPath, installScript);
}	

void IPCComplexLaunch::startInstall(const char* mcfPath, const char* installPath, const char* installScript)
{
	IPC::functionCallV(this, "startInstall", mcfPath, installPath, installScript);
}

void IPCComplexLaunch::onProgress(uint64& prog)
{
	MCFCore::Misc::ProgressInfo pi(prog);
	onProgressEvent(pi);
}


#else

void IPCComplexLaunch::startRemove(const char* modMcfPath, const char* backupMcfPath, const char* installPath, const char* installScript)
{
	safe_delete(m_pThread);

	m_pThread = new ComplexLaunchProcess();
	
	m_pThread->setRemove(modMcfPath, backupMcfPath, installPath, installScript);
	m_pThread->onErrorEvent += delegate(&onErrorEvent);
	m_pThread->onCompleteEvent += delegate(&onCompleteEvent);
	m_pThread->onProgressEvent += delegate(&onProgressIPCEvent);

	m_pThread->start();
}	

void IPCComplexLaunch::startInstall(const char* mcfPath, const char* installPath, const char* installScript)
{
	safe_delete(m_pThread);

	m_pThread = new ComplexLaunchProcess();
	
	m_pThread->setInstall(mcfPath, installPath, installScript);
	m_pThread->onErrorEvent += delegate(&onErrorEvent);
	m_pThread->onCompleteEvent += delegate(&onCompleteEvent);
	m_pThread->onProgressEvent += delegate(&onProgressIPCEvent);

	m_pThread->start();
}

void IPCComplexLaunch::stop()
{
	if (m_pThread)
		m_pThread->nonBlockStop();
}

#endif
