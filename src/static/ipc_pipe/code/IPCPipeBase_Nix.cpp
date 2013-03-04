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

void print_buffer(const char* buff, size_t size)
{
	return;

	gcString out="\n";

	char b[10];

	for (size_t x=0; x<size; x++)
	{
		snprintf(b, 10, "%02X ", (char)buff[x]);

		out += b;

		if ((x+1)%10 == 0 && x!=0)
			out += "\n";
	}

	out += "\n";

	printf("%s", out.c_str());
}

namespace IPC
{



PipeBase::PipeBase(const char* pipeName, const char* threadName) : BaseThread(threadName)
{

}

PipeBase::~PipeBase()
{
	stop();

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
		
		if (itemsWaiting() == false)
			m_WaitCond.wait();
	} 
}

void PipeBase::processLoopback()
{
	m_LoopbackLock.lock();

	if (!m_vLoopback.empty())
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
	IPCManager* mng = getManager(0);	//TODO: Fix for multi instances
	
	PipeData* data = NULL;
	
	m_RecvLock.lock();
	
	if (m_vRecvBuffer.size() > 0)
	{
		data = m_vRecvBuffer.front();
		m_vRecvBuffer.pop_front();
	}
	
	m_RecvLock.unlock();
	
	
	if (data)
	{
		mng->recvMessage(data->buffer, data->size);
		safe_delete(data);
	}
	
	PipeData toSend;
	
	if (mng->getMessageToSend(toSend.buffer, BUFSIZE, toSend.size))
	{
		sendMsg(m_pSendObj, toSend.buffer, toSend.size);
	}
}

bool PipeBase::itemsWaiting()
{
	IPCManager* mng = getManager(0);	//TODO: Fix for multi instances
	
	uint32 num = 0;
	
	m_RecvLock.lock();
	num = m_vRecvBuffer.size();
	m_RecvLock.unlock();
	
	num += mng->getNumSendEvents();
	
	return num != 0;
}

void PipeBase::recvMessage(const char* buffer, size_t size)
{
	PipeData *data = new PipeData();
	
	data->size = size;
	memcpy(data->buffer, buffer, size);
	
	m_RecvLock.lock();
	m_vRecvBuffer.push_back(data);
	m_RecvLock.unlock();
	
	m_WaitCond.notify();
}
	
void PipeBase::setSendCallback(void* obj, SendFn funct)
{
	m_pSendObj = obj;
	sendMsg = funct;
}

void PipeBase::onStop()
{
	m_WaitCond.notify();
}

}
