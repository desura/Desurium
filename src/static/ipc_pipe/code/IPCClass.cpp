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

#include "util_thread/BaseThread.h"

#include "IPCClass.h"
#include "IPCMessage.h"
#include "IPCManager.h"
#include "IPCParameter.h"


namespace IPC
{

typedef struct
{
	char* data;
	uint32 size;
} PS_s;

char* serializeList(std::vector<IPCParameterI*> &pList, uint32 &rsize)
{
	uint32 tsize = 0;
	std::vector<IPCParameter*> vIPCP;

	for (size_t x=0; x<pList.size(); x++)
	{
		uint32 size = 0;
		char* data = pList[x]->serialize( size );

		char* buff = new char[size + IPCParameterSIZE];
		IPCParameter* p = (IPCParameter*)buff;
	
		memcpy(&p->data, data, size);
		p->size = size;
		p->type = pList[x]->getType();

		vIPCP.push_back(p);
		tsize += sizeofStruct(p);

		safe_delete(data);
	}

	char* buff = new char[tsize];
	char* temp = buff;

	for (size_t x=0; x<pList.size(); x++)
	{
		memcpy(temp, vIPCP[x], sizeofStruct(vIPCP[x]) );
		temp += sizeofStruct(vIPCP[x]);

		delete [] (char*)vIPCP[x];
	}

	rsize = tsize;
	return buff;
}




uint32 deserializeList(std::vector<IPCParameterI*> &list, const char* buffer, uint32 size)
{
	uint32 sizeLeft = size;
	const char* tempB = buffer;
	IPCParameter* tempP;

	while (sizeLeft > 0)
	{
		tempP = (IPCParameter*)tempB;
		IPCParameterI* p = getParameter(tempP->type, &tempP->data, tempP->size);

		list.push_back(p);

		tempB += tempP->size + IPCParameterSIZE;
		sizeLeft -= tempP->size + IPCParameterSIZE;
	}

	return size - sizeLeft;
}







IPCClass::IPCClass(IPCManager* mang, uint32 id, DesuraId itemId)
:	m_uiItemId(itemId),
	m_pManager(mang)
{
	m_uiId = id;
}

IPCClass::~IPCClass()
{
	safe_delete(m_mFunc);
	safe_delete(m_mEvent);
}



IPCParameterI* IPCClass::callFunction(const char* name, bool async, IPCParameterI* a, IPCParameterI* b, IPCParameterI* c, IPCParameterI* d, IPCParameterI* e, IPCParameterI* f)
{
	std::vector<IPCParameterI*> pList;

	if (a)
		pList.push_back(a);
	if (b)
		pList.push_back(b);
	if (c)
		pList.push_back(c);
	if (d)
		pList.push_back(d);
	if (e)
		pList.push_back(e);
	if (f)
		pList.push_back(f);

	IPCParameterI* res = callFunction(name, async, pList);
	safe_delete(pList);

	return res;
}

IPCParameterI* IPCClass::callFunction(const char* name, bool async, std::vector<IPCParameterI*> &pList)
{
	IPCFunctionCall *fch;

	uint32 tsize;
	char* data = serializeList(pList, tsize);

	char* buff = new char[tsize + IPCFunctionCallSIZE];
	fch = (IPCFunctionCall*)buff;

	fch->size = tsize;
	fch->functionHash = UTIL::MISC::RSHash_CSTR(name);
	fch->numP = pList.size();
	memcpy(&fch->data, data, tsize );
	
	safe_delete(data);

	IPCParameterI* ret = NULL;

	if (async)
	{
		fch->id = 0;
		this->sendMessage(MT_FUNCTIONCALL_ASYNC, (const char*)fch, sizeofStruct(fch) );
		safe_delete(buff);

		ret = new PVoid();
	}
	else
	{	
		IPCScopedLock<IPCClass> lock(this, newLock());

		fch->id = lock->id;

		this->sendMessage(MT_FUNCTIONCALL, (const char*)fch, sizeofStruct(fch) );
		safe_delete(buff);

		//wait on mutex
		if (lock->wait(30, 0))
			throw gcException(ERR_IPC, "Waited too long with no response");

		ret = lock->result;
	}

	return ret;
}


IPCParameterI* IPCClass::callLoopback(const char* name, bool async, IPCParameterI* a, IPCParameterI* b, IPCParameterI* c, IPCParameterI* d, IPCParameterI* e, IPCParameterI* f)
{
	std::vector<IPCParameterI*> pList;

	if (a)
		pList.push_back(a);
	if (b)
		pList.push_back(b);
	if (c)
		pList.push_back(c);
	if (d)
		pList.push_back(d);
	if (e)
		pList.push_back(e);
	if (f)
		pList.push_back(f);

	IPCParameterI* res = callLoopback(name, async, pList);
	safe_delete(pList);

	return res;
}

IPCParameterI* IPCClass::callLoopback(const char* name, bool async, std::vector<IPCParameterI*> &pList)
{
	IPCFunctionCall *fch;

	uint32 tsize;
	char* data = serializeList(pList, tsize);

	char* buff = new char[tsize + IPCFunctionCallSIZE];
	fch = (IPCFunctionCall*)buff;

	fch->size = tsize;
	fch->functionHash = UTIL::MISC::RSHash_CSTR(name);
	fch->numP = pList.size();
	memcpy(&fch->data, data, tsize );
	
	safe_delete(data);

	IPCParameterI* ret = NULL;

	if (async)
	{
		fch->id = 0;
		this->sendLoopbackMessage(MT_FUNCTIONCALL_ASYNC, (const char*)fch, sizeofStruct(fch) );
		safe_delete(buff);

		ret = new PVoid();
	}
	else
	{	
		IPCScopedLock<IPCClass> lock(this, newLock());

		fch->id = lock->id;

		this->sendLoopbackMessage(MT_FUNCTIONCALL, (const char*)fch, sizeofStruct(fch) );
		safe_delete(buff);

		//wait on mutex
		lock->wait();
		ret = lock->result;
	}

	return ret;
}

void IPCClass::registerFunction(NetworkFunctionI* funct, const char* name)
{
	uint32 hash = UTIL::MISC::RSHash_CSTR(name);

	std::map<uint32,NetworkFunctionI*>::iterator it = m_mFunc.find(hash);

	if (it == m_mFunc.end())
	{
		m_mFunc[hash] = funct;
	}
	else
	{
		safe_delete(funct);
	}	
}


void IPCClass::registerEvent(IPCEventI* e, const char* name)
{
	uint32 hash = UTIL::MISC::RSHash_CSTR(name);

	std::map<uint32,IPCEventI*>::iterator it = m_mEvent.find(hash);

	if (it == m_mEvent.end())
	{
		m_mEvent[hash] = e;
	}
	else
	{
		safe_delete(e);
	}	
}

void IPCClass::messageRecived(uint8 type, const char* buff, uint32 size)
{
	if (type == MT_FUNCTIONCALL)
	{
		handleFunctionCall(buff, size, false);
	}
	else if (type == MT_FUNCTIONCALL_ASYNC)
	{
		handleFunctionCall(buff, size, true);
	}
	else if (type == MT_FUNCTIONRETURN)
	{
		handleFunctionReturn(buff, size);
	}
	else if (type == MT_EVENTTRIGGER)
	{
		handleEventTrigger(buff,size);
	}
}

void IPCClass::handleFunctionCall(const char* buff, uint32 size, bool async)
{
	IPCFunctionCall *fch = (IPCFunctionCall*)buff;

	std::map<uint32,NetworkFunctionI*>::iterator it;
	it = m_mFunc.find(fch->functionHash);

	IPCParameterI* ret = NULL;

	if (it != m_mFunc.end())
	{
		ret = it->second->call(&fch->data, fch->size, fch->numP);
	}
	else
	{
		gcException errFindFailed(ERR_IPC, gcString("Failed to find function [H:{0}, NP:{1}]!", fch->functionHash, fch->numP));
		ret = new PException(errFindFailed);
		Warning("Failed to find function for function call \n");
	}

	//dont worry about return for async calls.
	if (async)
	{
		safe_delete(ret);
		return;
	}

	if (!ret)
		ret = new PVoid();

	uint32 dsize = 0;
	char* data = ret->serialize(dsize);

	uint32 bsize = IPCFunctionCallSIZE + dsize + IPCParameterSIZE;
	char* nbuff = new char[bsize];

	memset(nbuff, 1, bsize);

	IPCFunctionCall *fchr = (IPCFunctionCall*)nbuff;
	fchr->id = fch->id;
	fchr->numP = 1;
	fchr->functionHash = fch->functionHash;
	fchr->size = dsize + IPCParameterSIZE;

	IPCParameter *p = (IPCParameter*)&fchr->data;

	p->size = dsize;
	p->type = ret->getType();
	memcpy(&p->data, data, dsize);

	sendMessage( MT_FUNCTIONRETURN, (const char*)fchr, sizeofStruct(fchr) );

	safe_delete(data);
	safe_delete(nbuff);
	safe_delete(ret);
}

void IPCClass::handleFunctionReturn(const char* buff, uint32 size)
{
	IPCFunctionCall *fch = (IPCFunctionCall*)buff;
	IPCLock* lock = findLock(fch->id);

	if (lock)
	{
		IPCParameter* par = (IPCParameter*)&fch->data;
		lock->result = getParameter(par->type, &par->data, par->size);
		lock->trigger();
	}
	else
	{
		Warning("Failed to process function return.\n");
	}
}

void IPCClass::handleEventTrigger(const char* buff, uint32 size)
{
	if (IPCEventTriggerSIZE > size)
	{
		Warning("Trigger size is too small \n");
		return;
	}

	IPCEventTrigger *fch = (IPCEventTrigger*)buff;

	std::map<uint32,IPCEventI*>::iterator it;
	it = m_mEvent.find(fch->eventHash);

	if (it != m_mEvent.end())
	{
		it->second->trigger(&fch->data, fch->size);
	}
	else
	{
		Warning("Failed to find event for Event Trigger \n");
	}
}


void IPCClass::sendMessage(uint8 type, const char* buff, uint32 size)
{
	//const char* buff, uint32 size, uint32 id, uint8 type
	m_pManager->sendMessage(buff, size, m_uiId, type);
}

void IPCClass::sendLoopbackMessage(uint8 type, const char* buff, uint32 size)
{
	//const char* buff, uint32 size, uint32 id, uint8 type
	m_pManager->sendLoopbackMessage(buff, size, m_uiId, type);
}

void IPCClass::destroy()
{
	if (!m_pManager->isDisconnected())
		sendMessage( MT_KILL, NULL, 0 );

	m_pManager->destroyClass(this);
}

}
