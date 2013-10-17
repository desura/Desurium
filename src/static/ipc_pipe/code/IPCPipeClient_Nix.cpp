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
#include "IPCPipeClient.h"

namespace IPC
{


PipeClient::PipeClient(const char* name, LoopbackProcessor* loopbackProcessor, uint32 managerId) : PipeBase(name, gcString("{0}- IPC Client",name).c_str()), IPCManager(loopbackProcessor, managerId, name)
{
	setSendEvent(&m_WaitCond);
}

PipeClient::~PipeClient()
{
	Thread::BaseThread::stop();
}

IPCManager* PipeClient::getManager(uint32 index)
{
	return this;
}

void PipeClient::disconnectAndReconnect(uint32 i) 
{ 
	informClassesOfDisconnect();
	disconnect();
} 

}
