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

#ifndef DESURA_IPCPIPEHELPER_H
#define DESURA_IPCPIPEHELPER_H
#ifdef _WIN32
#pragma once
#endif



#define BUFSIZE 4096
#define PIPE_ACCESS	(PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED) //|FILE_FLAG_FIRST_PIPE_INSTANCE
#define	PIPE_MODE	(PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT)
#define PIPE_TIMEOUT 5000

#include "IPCManager.h"

namespace IPC
{



class PipeData
{
public:
	PipeData()
	{
#ifdef WIN32
		oOverlap.hEvent = INVALID_HANDLE_VALUE;
		hPipe = INVALID_HANDLE_VALUE;
		sender = false;
#endif
		reset();
	}

	void reset()
	{
		size = 0;	
		
#ifdef WIN32
		pendingConnection = true;

		fPendingIO = FALSE;
		
		oOverlap.Internal = 0;
		oOverlap.InternalHigh = 0;
		oOverlap.Offset = 0;
		oOverlap.OffsetHigh = 0;
		oOverlap.hEvent = INVALID_HANDLE_VALUE;
#endif
	}
	
#ifdef WIN32
	OVERLAPPED oOverlap; 
	HANDLE hPipe;

	BOOL fPendingIO;

	bool pendingConnection;
	bool sender;	
#endif
	
	char buffer[BUFSIZE]; 
	uint32 size;
};


class PipeInst 
{ 
public:
	PipeInst()
	{
#ifdef WIN32
		pipes[0] = &send;
		pipes[1] = &recv;

		send.sender = true;
		recv.sender = false;
#endif
	}

	~PipeInst()
	{
		disconnect();
		safe_delete(pIPC);
	}

	PipeData send;
	PipeData recv;

#ifdef WIN32
	PipeData *pipes[2];
#endif
	
	IPCManager* pIPC;

	void disconnect()
	{
#ifdef WIN32
		ResetEvent(send.oOverlap.hEvent);
		ResetEvent(recv.oOverlap.hEvent);

		CancelIo(send.hPipe);
		CancelIo(recv.hPipe);

		DisconnectNamedPipe(send.hPipe);
		DisconnectNamedPipe(recv.hPipe);

		send.reset();
		recv.reset();
#endif
	}
}; 


}

#endif //DESURA_IPCPIPEHELPER_H
