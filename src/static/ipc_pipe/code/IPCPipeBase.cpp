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
#include "IPCPipeBase.h"
#include "IPCManager.h"

//prints ipc stuff to a file
#ifdef DEBUG
//#define IPC_DEBUG
#endif

void printBuffer(FILE* fh, char* buf, uint32 size)
{
	fprintf(fh, "\n");

	for (uint32 x=0; x< size; x++)
	{
		fprintf(fh, "%02X ", buf[x]&0xFF);


			if ((x+1)%4 == 0 && x != 0)
				fprintf(fh, " ");

			if ((x+1)%24 == 0 && x != 0)
				fprintf(fh, "\n");

	}
	fprintf(fh, "\n\n");
}


namespace IPC
{

#ifdef IPC_DEBUG
FILE* fh = NULL;
#endif

PipeBase::PipeBase(const char* pipeName, const char* threadName) : BaseThread(threadName),
	m_szRecvName("\\\\.\\pipe\\{0}-r", pipeName),
	m_szSendName("\\\\.\\pipe\\{0}-s", pipeName)
{
	for (size_t x=0; x<512; x++)
	{
		m_hEventsArr[x] = INVALID_HANDLE_VALUE;
	}

#ifdef IPC_DEBUG
	fh = Safe::fopen(gcString("{0}_out.txt", threadName).c_str(), "a");

	if (fh)
		fprintf(fh, "---------------\nOpen File\n---------------\n");
#endif
}

PipeBase::~PipeBase()
{
	stop();

	for (size_t x=0; x<512; x++)
	{
		if (m_hEventsArr[x] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hEventsArr[x]);
			m_hEventsArr[x] = INVALID_HANDLE_VALUE;
		}
	}

	m_LoopbackLock.lock();
	safe_delete(m_vLoopback);
	m_LoopbackLock.unlock();
}

void PipeBase::loopbackMessage(const char* message, uint32 size, uint32 managerId)
{
	if (isStopped())
		return;

	LoopbackInfo* i = new LoopbackInfo();

	i->size = size;
	i->id = managerId;

	i->buffer = new char[size];
	memcpy(i->buffer, message, size);

	m_LoopbackLock.lock();
	m_vLoopback.push_back(i);
	m_LoopbackLock.unlock();
}

void PipeBase::run()
{
	while (!isStopped()) 
	{ 
		processEvents();
		processLoopback();
	} 
}

void PipeBase::processLoopback()
{
	m_LoopbackLock.lock();

	if (m_vLoopback.size() > 0)
	{
		LoopbackInfo* info = m_vLoopback.front();
		m_vLoopback.pop_front();

		if (getManager(info->id))
		{
			getManager(info->id)->recvMessage(info->buffer, info->size);
		}

		safe_delete(info);
	}

	m_LoopbackLock.unlock();
}

void PipeBase::processEvents()
{
	DWORD dwWait = WaitForMultipleObjects(getNumEvents(), m_hEventsArr, FALSE, 500);

	if (isStopped())
		return;

	if (dwWait == WAIT_TIMEOUT)
		return;

	if (dwWait == WAIT_FAILED)
	{
		DWORD lErr = GetLastError();
		return;
	}

	// dwWait shows which pipe completed the operation. 
	size_t i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
	if (i < 0 || i > (getNumEvents() - 1)) 
	{
		printf("Index out of range.\n"); 
		return;
	}

	PipeData* data = getData(i);
	DWORD cbRet = 0;

#ifdef IPC_DEBUG
	if (fh)
	{
		fprintf(fh, "Triggered even0328t %d\n", i);
		fflush(fh);
	}
#endif
	//printf("Event %d, P: %d\n", i, data->fPendingIO);


	// Get the result if the operation was pending. 
	if (data->fPendingIO) 
	{
		BOOL fSuccess = GetOverlappedResult(data->hPipe, &data->oOverlap, &cbRet, FALSE);

		//printf("Pending S: %d A: %d P: %d\n", fSuccess, cbRet, data->pendingConnection);

		if (data->pendingConnection)
		{
			if (!fSuccess) 
				throw gcException(ERR_PIPE, GetLastError(), gcString("Error {0}.\n", GetLastError()));

			data->pendingConnection = false; 
			data->fPendingIO = FALSE;
		}
		else
		{	
			DWORD err = GetLastError();

			//Buffer is full. Wait for space
			if (err == ERROR_IO_INCOMPLETE)
				fSuccess = GetOverlappedResult(data->hPipe, &data->oOverlap, &cbRet, TRUE);

			if (!fSuccess || (cbRet == 0 && data->sender == false) || (cbRet != data->size && data->sender == true))
			{
				disconnectAndReconnect(i);
				ResetEvent(m_hEventsArr[i]);
				printf("Disconnect pending!\n");
				return;
			}
				
			if (!data->sender)
			{
				data->size = cbRet;
				finishRead(data, getManager(i));
			}
		}
	}

	bool res = false;

	// The pipe state determines which operation to do next. 
	if (data->sender) 
		res = performWrite(data, getManager(i));
	else
		res = performRead(data, getManager(i));

	if (res)
		ResetEvent(m_hEventsArr[i]);
}


void PipeBase::finishRead(PipeData* data, IPCManager* mng)
{
#ifdef IPC_DEBUG
	if (fh)
	{
		fprintf(fh, "Read Data %d\n", data->size);
		printBuffer(fh, data->buffer, data->size);
		fprintf(fh, "\n\n");
		fflush(fh);
	}
#endif

	mng->recvMessage(data->buffer, data->size);
	data->size = 0;
	data->fPendingIO = FALSE; 
}


bool PipeBase::performRead(PipeData* data, IPCManager* mng)
{
	if (!data || !mng)
		return true;

	DWORD read;
	BOOL fSuccess = ReadFile(data->hPipe, data->buffer, BUFSIZE, &read, &data->oOverlap); 

	if (fSuccess) 
	{
		data->size = read;
		finishRead(data, mng);
	} 
	else if (!fSuccess && (GetLastError() == ERROR_IO_PENDING)) 
	{ 
		data->fPendingIO = TRUE;
	}
	else
	{
		//error
	}

	return false;
}

bool PipeBase::performWrite(PipeData* data, IPCManager* mng)
{
	if (!data || !mng)
		return true;

	if (!mng->getMessageToSend(data->buffer, BUFSIZE, data->size))
		return true;
	
#ifdef IPC_DEBUG
	if (fh)
	{
		fprintf(fh, "Sending Data %d\n", data->size);
		printBuffer(fh, data->buffer, data->size);
		fprintf(fh, "\n\n");
		fflush(fh);
	}
#endif

	BOOL fSuccess = WriteFile( data->hPipe, data->buffer, data->size, NULL, &data->oOverlap); 

	if (fSuccess) 
	{ 
		data->size = 0;
		data->fPendingIO = FALSE; 
	} 
	else if (!fSuccess && (GetLastError() == ERROR_IO_PENDING)) 
	{ 
		data->fPendingIO = TRUE;
	} 
	else
	{
		//error
	}

	return false;
}




}
