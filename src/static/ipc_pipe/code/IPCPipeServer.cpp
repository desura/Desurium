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

#include "Common.h"
#include "IPCPipeServer.h"
#include "IPCManager.h"
#include <aclapi.h>

namespace IPC
{

PipeServer::PipeServer(const char* name, uint8 numPipes, bool changeAccess) : PipeBase(name, gcString("{0}- IPC Server", name).c_str())
{
	m_szName = name;

	m_uiNumPipes = numPipes;
	m_bChangeAccess = changeAccess;
}

PipeServer::~PipeServer()
{	
	stop();
	safe_delete(m_vPipeInst);
}

void PipeServer::run()
{
	try
	{
		setUpPipes();
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to start pipe: {0}\n", e));
		return;
	}

	PipeBase::run();
}

void PipeServer::setUpPipes()
{
	PACL pNewAcl = NULL;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;

	if (m_bChangeAccess)
		createAccessRights(pNewAcl, sa, sd);


	// The initial loop creates several instances of a named pipe 
	// along with an event object for each instance.  An 
	// overlapped ConnectNamedPipe operation is started for 
	// each instance. 
	for (int i = 0; i < m_uiNumPipes*2; i+=2) 
	{ 
		// Create an event object for this instance. 
		m_hEventsArr[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
		m_hEventsArr[i+1] = CreateEvent(NULL, TRUE, TRUE, NULL);

		if (m_hEventsArr[i] == NULL || m_hEventsArr[i+1] == NULL) 
			throw gcException(ERR_IPC, gcString("CreateEvent failed with {0}.\n", GetLastError()));

		PipeInst *p = new PipeInst();
		m_vPipeInst.push_back(p);

		p->send.oOverlap.hEvent = m_hEventsArr[i];
		p->recv.oOverlap.hEvent = m_hEventsArr[i+1];

		p->send.hPipe = CreateNamedPipe(m_szSendName.c_str(), PIPE_ACCESS, PIPE_MODE, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, PIPE_TIMEOUT, m_bChangeAccess?&sa:NULL);
		DWORD sErr = GetLastError();

		p->recv.hPipe = CreateNamedPipe(m_szRecvName.c_str(), PIPE_ACCESS, PIPE_MODE, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, PIPE_TIMEOUT, m_bChangeAccess?&sa:NULL);
		DWORD rErr = GetLastError();

		// Call the subroutine to connect to the new client
		initNewClient(i, p, m_hEventsArr[i]); 
	} 

	if (pNewAcl)
		LocalFree((HLOCAL)pNewAcl);

	printf("Finished setting pipe up...\n");
}


void PipeServer::createAccessRights(PACL pNewAcl, SECURITY_ATTRIBUTES &sa, SECURITY_DESCRIPTOR &sd)
{
	EXPLICIT_ACCESS ea;
	
	DWORD SidSize;
	PSID TheSID;

	SidSize = SECURITY_MAX_SID_SIZE;
	// Allocate enough memory for the largest possible SID.
	if(!(TheSID = LocalAlloc(LMEM_FIXED, SidSize)))
	{    
		LocalFree(TheSID);
		throw gcException(ERR_PIPE, GetLastError(), "LocalAlloc failed");
	}

	if(!CreateWellKnownSid(WinWorldSid, NULL, TheSID, &SidSize))
	{
		LocalFree(TheSID);
		throw gcException(ERR_PIPE, GetLastError(), "CreateWellKnownSid failed");
	}

	char everyone[255];
	char domain[255];
	DWORD eSize = 255;
	DWORD dSize = 255;
	SID_NAME_USE rSidNameUse;

	if (!LookupAccountSid(NULL, TheSID, everyone, &eSize, domain, &dSize, &rSidNameUse))
	{
		LocalFree(TheSID);
		throw gcException(ERR_PIPE, GetLastError(), "LookupAccountSid failed");
	}

	LocalFree(TheSID);

	// Build the ACE.
	BuildExplicitAccessWithName(&ea, everyone, GENERIC_READ|GENERIC_WRITE, SET_ACCESS, NO_INHERITANCE);

	if (SetEntriesInAcl(1, &ea, NULL, &pNewAcl) != ERROR_SUCCESS)
		throw gcException(ERR_PIPE, GetLastError(), "SetEntriesInAcl failed");

	// Initialize a NEW Security Descriptor.
	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))			
		throw gcException(ERR_PIPE, GetLastError(), "InitializeSecurityDescriptor failed");

	// Set the new DACL in the Security Descriptor.
	if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))						
		throw gcException(ERR_PIPE, GetLastError(), "SetSecurityDescriptorDacl failed");

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE;
}



void PipeServer::initNewClient(uint32 index, PipeInst *p, HANDLE e) 
{ 
	if (!p)
		return;

	uint32 i = index;
	onConnectEvent(i);

	if (p->send.hPipe == INVALID_HANDLE_VALUE || p->recv.hPipe == INVALID_HANDLE_VALUE) 
		throw gcException(ERR_PIPE, GetLastError(), "Failed to create pipe");

	p->pIPC = new IPCManager(this, index, m_szName.c_str(), true);
	p->pIPC->onNeedAuthEvent += delegate(&onNeedAuthEvent);
	p->pIPC->setSendEvent(e);

	connectNamedPipe(index, &p->send);
	connectNamedPipe(index, &p->recv);
}

void PipeServer::connectNamedPipe(uint32 index, PipeData* data)
{
	if (!data)
		return;

	BOOL fConnected = ConnectNamedPipe(data->hPipe, &data->oOverlap);
	DWORD err = GetLastError();

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected) 
		throw gcException(ERR_PIPE, GetLastError(), gcString("ConnectNamedPipe failed with {0}.\n", GetLastError()));

	switch (GetLastError()) 
	{ 
		// The overlapped connection in progress. 
		case ERROR_IO_PENDING: 
			data->fPendingIO = TRUE; 
			break; 

		// Client is already connected, so signal an event. 
		case ERROR_PIPE_CONNECTED:
			onConnectEvent(index);
			if (SetEvent(data->oOverlap.hEvent)) 
				break; 

		// If an error occurs during the connect operation... 
		default: 
			throw gcException(ERR_PIPE, GetLastError(), gcString("ConnectNamedPipe failed with {0}.\n", GetLastError()));
	} 

	data->pendingConnection = data->fPendingIO ? true : false;
}


PipeData* PipeServer::getData(uint32 i)
{
	size_t index = i/2;
	size_t sorr = i%2;

	return m_vPipeInst[index]->pipes[sorr];
}

IPCManager* PipeServer::getManager(uint32 i)
{
	size_t index = i/2;
	size_t sorr = i%2;

	return m_vPipeInst[index]->pIPC;
}



void PipeServer::disconnectAndReconnect(uint32 i) 
{ 
	onDisconnectEvent(i);

	size_t index = i/2;
	size_t sorr = i%2;

	m_vPipeInst[index]->disconnect();
	safe_delete(m_vPipeInst[index]->pIPC);

	try
	{
		initNewClient(index, m_vPipeInst[index], m_hEventsArr[index*2] ); 
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to reconnect ipc pipe: {0}\n", e));
	}
}


}