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

#ifndef DESURA_IPCMANAGER_H
#define DESURA_IPCMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCParameterI.h"
#include "IPCMessage.h"
#include "IPCLockable.h"
#include "IPCPipeAuth.h"
#include "IPCServerI.h"

#include "util_thread/BaseThread.h"

namespace IPC
{
class IPCManager;
class IPCClass;
class PipeMessage;
class ProcessThread;
class LoopbackProcessor;

typedef IPCClass* (*newClassFunc)(IPC::IPCManager*, uint32, DesuraId);
void RegIPCClass(uint32 id, newClassFunc funct);

class AutoRegClass
{
public:
	AutoRegClass(const char* name, newClassFunc funct )
	{
		RegIPCClass( UTIL::MISC::RSHash_CSTR(name), funct);
	}
};

#define REG_IPC_CLASS( cn )	IPC::IPCClass* new_##cn (IPC::IPCManager* mang, uint32 id, DesuraId itemId){return new cn(mang, id, itemId);} IPC::AutoRegClass  arc_##cn( #cn, ::new_##cn );





//! Manages IPC Communication 
//!
class IPCManager : public IPCLockable
{
public:
	//! Constuctor
	//!
	//! @param isServer is this a server instance of the manager
	//!
	IPCManager(LoopbackProcessor* loopbackProcessor, uint32 managerId, const char* threadName, bool isServer = false);

	//! Constructor
	//!
	~IPCManager();



	//! Send a message to the other side
	//!
	//! @param buff Message buffer
	//! @param size Size of message
	//! @param id Process id
	//! @param type Message type
	//!
	void sendMessage(const char* buff, uint32 size, uint32 id, uint8 type);

	//! Send a message to this side from IPC thread
	//!
	//! @param buff Message buffer
	//! @param size Size of message
	//! @param id Process id
	//! @param type Message type
	//!
	void sendLoopbackMessage(const char* buff, uint32 size, uint32 id, uint8 type);

	//! Get a pending message to send
	//!
	//! @param buffer Buffer to copy message into
	//! @param buffSize Size of the buffer
	//! @param msgSize Size of message copied into the buffer
	//!
	bool getMessageToSend(char* buffer, uint32 buffSize, uint32& msgSize);

	//! Process a recived message
	//!
	//! @param buff Message buffer
	//! @param size Message size
	//!
	void recvMessage(const char* buff, uint32 size);

	//! Create a class to be used for IPC
	//!
	//! @param name Class name
	//! @return New class or Null if cant create it
	//!
	WeakPtr<IPCClass> createClass(const char* name);

	//! Destroy a class created with createClass
	//!
	//! @param obj Class to destory
	//!
	void destroyClass(IPCClass* obj);
	void destroyClass(uint32 id);
	
#ifdef WIN32
	//! Set the event to trigger when a new message arrives
	//!
	//! @param event Event handler
	//!
	void setSendEvent(HANDLE event){m_hEvent = event;}
#else
	void setSendEvent(::Thread::WaitCondition *waitCond){m_hEvent = waitCond;}
	
	uint32 getNumSendEvents();
#endif
	
	//! Disconnect from the pipe
	//!
	//! @param triggerEvent Trigger onDisconnectEvent
	//!
	void disconnect(bool triggerEvent = true);

	//! Sets the item id for filtering. Passes this onto every constucted class
	//!
	//! @param itemId Item id
	//!
	void setItemId(DesuraId itemId){m_uiItemId = itemId;}

	//! Gets the item id for filtering
	//!
	//! @return Item Id
	//!
	DesuraId getItemId(){return m_uiItemId;}

	//! Event that gets triggered on disconnect
	//!
	EventV onDisconnectEvent;

	//! Event that gets triggered when authentication is needed for pipe connect
	//!
	Event<PipeItemAuth> onNeedAuthEvent;


	//! Sets the item id for filtering. Should only be called server side
	//!
	//! @param itemHashId Item hash id found on Desura.com
	//! @return Item id
	//!
	DesuraId setItemHashId(const char* itemHashId);

	//! Cancels all pending function calls
	//!
	void informClassesOfDisconnect();

	void restart();

	bool isDisconnected()
	{
		return m_bDisconnected;
	}

protected:
	//! Process a recvied message
	//!
	//! @param msg Message to process
	//!
	void recvMessage(IPCMessage* msg);

	//! Process a internal message
	//!
	//! @param type Message type
	//! @param buff Message buffer
	//! @param size Message size
	//!
	void processInternalMessage(uint8 type, const char* buff, uint32 size);

	//! Create a class to match a class on the other side of the pipe
	//!
	//! @param hash Class hash
	//! @param id Message id
	//! @return Result
	//!
	IPCParameterI* createClass(uint32 hash, uint32 id);
	
	//! Disconnect from pipe
	//!
	void stop();


private:
	int32 m_mClassId;

	Thread::Mutex m_ClassMutex;
	Thread::Mutex m_mVectorMutex;

	std::vector<SmartPtr<IPCClass> > m_vClassList;
	std::vector<PipeMessage*> m_vPipeMsgs;

#ifdef WIN32
	HANDLE m_hEvent;
#else
	::Thread::WaitCondition *m_hEvent;
#endif
	
	bool m_bServer;
	bool m_bDisconnected;

	ProcessThread* m_pEventThread;
	ProcessThread* m_pCallThread;

	DesuraId m_uiItemId;

	uint32 m_uiManagerId;
	LoopbackProcessor* m_pLoopbackProcessor;

	gcString m_szThreadName;
};



template <class T>
T* CreateIPCClass(IPC::IPCManager* mng, const char* name)
{
	if (!mng || ! name)
		return NULL;

	WeakPtr<IPCClass> wClass = mng->createClass(name);

	if (wClass.expired())
		return NULL;

	SmartPtr<IPCClass> sClass = wClass.lock();
	return dynamic_cast<T*>(sClass.get());
}

}




#endif //DESURA_IPCMANAGER_H
