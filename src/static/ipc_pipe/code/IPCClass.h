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

#ifndef DESURA_IPCClass_H
#define DESURA_IPCClass_H
#ifdef _WIN32
#pragma once
#endif

#include "Common.h"
#include "IPCClass.h"
#include "IPCParameterI.h"
#include "IPCParameter.h"
#include "IPCLockable.h"
#include "IPCMessage.h"

namespace IPC
{

class IPCManager;

#ifdef WIN32
#define REG_FUNCTION_VOID( class, name ) registerFunction( networkFunctionV(this, &class##::##name ), #name );
#define REG_FUNCTION( class, name ) registerFunction( networkFunction(this, &class##::##name ), #name );
#else
#define REG_FUNCTION_VOID( class, name ) registerFunction( networkFunctionV(this, &class::name ), #name );
#define REG_FUNCTION( class, name ) registerFunction( networkFunction(this, &class::name ), #name );
#endif

char* serializeList(std::vector<IPCParameterI*> &list, uint32 &size);
uint32 deserializeList(std::vector<IPCParameterI*> &list, const char* buffer, uint32 size);


class NetworkFunctionI
{
public:
	virtual ~NetworkFunctionI(){;}

	virtual IPCParameterI* call(char* buff, uint32 size, uint8 numP)=0;
	virtual IPCParameterI* returnVal(char* buff, uint32 size)=0;
};


template <class TObj, typename R, typename A = PVoid, typename B = PVoid, typename C = PVoid, typename D = PVoid, typename E = PVoid, typename F = PVoid>
class NetworkFunction : public NetworkFunctionI
{
public:
	typedef R (TObj::*TFunct0)();
	typedef R (TObj::*TFunct1)(A);
	typedef R (TObj::*TFunct2)(A, B);
	typedef R (TObj::*TFunct3)(A, B, C);
	typedef R (TObj::*TFunct4)(A, B, C, D);
	typedef R (TObj::*TFunct5)(A, B, C, D, E);
	typedef R (TObj::*TFunct6)(A, B, C, D, E, F);

	NetworkFunction(TObj* t, TFunct0 f)
	{
		init();
		m_pObj = t;
		m_pFunct0 = f;
		m_uiNumParams = 0;
	}	

	NetworkFunction(TObj* t, TFunct1 f)
	{
		init();
		m_pObj = t;
		m_pFunct1 = f;
		m_uiNumParams = 1;
	}	

	NetworkFunction(TObj* t, TFunct2 f)
	{
		init();
		m_pObj = t;
		m_pFunct2 = f;
		m_uiNumParams = 2;
	}

	NetworkFunction(TObj* t, TFunct3 f)
	{
		init();
		m_pObj = t;
		m_pFunct3 = f;
		m_uiNumParams = 3;
	}

	NetworkFunction(TObj* t, TFunct4 f)
	{
		init();
		m_pObj = t;
		m_pFunct4 = f;
		m_uiNumParams = 4;
	}

	NetworkFunction(TObj* t, TFunct5 f)
	{
		init();
		m_pObj = t;
		m_pFunct5 = f;
		m_uiNumParams = 5;
	}

	NetworkFunction(TObj* t, TFunct6 f)
	{
		init();
		m_pObj = t;
		m_pFunct6 = f;
		m_uiNumParams = 6;
	}

	void init()
	{
		m_pFunct0 = NULL;
		m_pFunct1 = NULL;
		m_pFunct2 = NULL;
		m_pFunct3 = NULL;
		m_pFunct4 = NULL;
		m_pFunct5 = NULL;
		m_pFunct6 = NULL;
	}

	IPCParameterI* call(char* buff, uint32 size, uint8 numP)
	{
		if (numP != m_uiNumParams)
			return IPC::getParameter<R>();

		IPCParameterI* a = IPC::getParameter<A>();
		IPCParameterI* b = IPC::getParameter<B>();
		IPCParameterI* c = IPC::getParameter<C>();
		IPCParameterI* d = IPC::getParameter<D>();
		IPCParameterI* e = IPC::getParameter<E>();
		IPCParameterI* f = IPC::getParameter<F>();
	
		std::vector<IPCParameterI*> vPList;

		vPList.push_back(a);
		vPList.push_back(b);
		vPList.push_back(c);
		vPList.push_back(d);
		vPList.push_back(e);
		vPList.push_back(f);
	
		char* buffPos = buff;
		IPCParameter* msg = NULL;

		for (uint8 x=0; x<m_uiNumParams; x++)
		{
			msg = (IPCParameter*)buffPos;
			vPList[x]->deserialize(&msg->data, msg->size);
			buffPos += sizeofStruct(msg);
		}
	
		IPCParameterI* ret = NULL;

		try
		{
			switch (m_uiNumParams)
			{
			case 0: 
				ret = IPC::getParameter( (*m_pObj.*m_pFunct0)()  , true); 
				break;

			case 1: 
				ret = IPC::getParameter( (*m_pObj.*m_pFunct1)(getParameterValue<A>(a)) , true); 
				break;

			case 2: 
				ret = IPC::getParameter( (*m_pObj.*m_pFunct2)(getParameterValue<A>(a), getParameterValue<B>(b)) , true); 
				break;

			case 3: 
				ret = IPC::getParameter( (*m_pObj.*m_pFunct3)(getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c)) , true); 
				break;

			case 4: 
				ret = IPC::getParameter( (*m_pObj.*m_pFunct4)(getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c), getParameterValue<D>(d)) , true); 
				break;

			case 5: 
				ret = IPC::getParameter( (*m_pObj.*m_pFunct5)(getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c), getParameterValue<D>(d), getParameterValue<E>(e)) , true); 
				break;

			case 6: 
				ret = IPC::getParameter( (*m_pObj.*m_pFunct6)(getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c), getParameterValue<D>(d), getParameterValue<E>(e), getParameterValue<F>(f)) , true); 
				break;
			};

		}
		catch (gcException &e)
		{
			ret = IPC::newParameter( e );
		}

		safe_delete(vPList);

		if (!ret)
			return IPC::getParameter<R>();
		else
			return ret;
	}


	IPCParameterI* returnVal(char* buff, uint32 size)
	{
		IPCParameterI *r = IPC::getParameter<R>();
		r->deserialize(buff, size);
		return r;
	}



private:
	// pointer to object
	TObj* m_pObj;     

	// pointer to member function
	TFunct0 m_pFunct0;
	TFunct1 m_pFunct1;
	TFunct2 m_pFunct2;  
	TFunct3 m_pFunct3;  
	TFunct4 m_pFunct4;  
	TFunct5 m_pFunct5;  
	TFunct6 m_pFunct6;  
	
	uint8 m_uiNumParams;
};


template <class TObj, typename A = PVoid, typename B = PVoid, typename C = PVoid, typename D = PVoid, typename E = PVoid, typename F = PVoid>
class NetworkFunctionVoid : public NetworkFunctionI
{
public:
	typedef void (TObj::*TFunct0)();
	typedef void (TObj::*TFunct1)(A);
	typedef void (TObj::*TFunct2)(A, B);
	typedef void (TObj::*TFunct3)(A, B, C);
	typedef void (TObj::*TFunct4)(A, B, C, D);
	typedef void (TObj::*TFunct5)(A, B, C, D, E);
	typedef void (TObj::*TFunct6)(A, B, C, D, E, F);

	NetworkFunctionVoid(TObj* t, TFunct0 f)
	{
		init();
		m_pObj = t;
		m_pFunct0 = f;
		m_uiNumParams = 0;
	}	

	NetworkFunctionVoid(TObj* t, TFunct1 f)
	{
		init();
		m_pObj = t;
		m_pFunct1 = f;
		m_uiNumParams = 1;
	}	

	NetworkFunctionVoid(TObj* t, TFunct2 f)
	{
		init();
		m_pObj = t;
		m_pFunct2 = f;
		m_uiNumParams = 2;
	}

	NetworkFunctionVoid(TObj* t, TFunct3 f)
	{
		init();
		m_pObj = t;
		m_pFunct3 = f;
		m_uiNumParams = 3;
	}

	NetworkFunctionVoid(TObj* t, TFunct4 f)
	{
		init();
		m_pObj = t;
		m_pFunct4 = f;
		m_uiNumParams = 4;
	}

	NetworkFunctionVoid(TObj* t, TFunct5 f)
	{
		init();
		m_pObj = t;
		m_pFunct5 = f;
		m_uiNumParams = 5;
	}

	NetworkFunctionVoid(TObj* t, TFunct6 f)
	{
		init();
		m_pObj = t;
		m_pFunct6 = f;
		m_uiNumParams = 6;
	}

	void init()
	{
		m_pFunct0 = NULL;
		m_pFunct1 = NULL;
		m_pFunct2 = NULL;
		m_pFunct3 = NULL;
		m_pFunct4 = NULL;
		m_pFunct5 = NULL;
		m_pFunct6 = NULL;
	}

	IPCParameterI* call(char* buff, uint32 size, uint8 numP)
	{
		if (numP != m_uiNumParams)
			return new IPC::PVoid();
		
		IPCParameterI* a = IPC::getParameter<A>();
		IPCParameterI* b = IPC::getParameter<B>();
		IPCParameterI* c = IPC::getParameter<C>();
		IPCParameterI* d = IPC::getParameter<D>();
		IPCParameterI* e = IPC::getParameter<E>();
		IPCParameterI* f = IPC::getParameter<F>();
	
		std::vector<IPCParameterI*> vPList;

		vPList.push_back(a);
		vPList.push_back(b);
		vPList.push_back(c);
		vPList.push_back(d);
		vPList.push_back(e);
		vPList.push_back(f);
	
		char* buffPos = buff;
		IPCParameter* msg = NULL;

		for (uint8 x=0; x<m_uiNumParams; x++)
		{
			msg = (IPCParameter*)buffPos;
			vPList[x]->deserialize(&msg->data, msg->size);
			buffPos += sizeofStruct(msg);
		}
	
	
		try
		{

			switch (m_uiNumParams)
			{
			case 0: 
				(*m_pObj.*m_pFunct0)(); 
				break;

			case 1: 
				(*m_pObj.*m_pFunct1)( getParameterValue<A>(a) ); 
				break;

			case 2: 
				(*m_pObj.*m_pFunct2)( getParameterValue<A>(a), getParameterValue<B>(b)); 
				break;

			case 3: 
				(*m_pObj.*m_pFunct3)( getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c)); 
				break;

			case 4: 
				(*m_pObj.*m_pFunct4)( getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c), getParameterValue<D>(d)); 
				break;

			case 5: 
				(*m_pObj.*m_pFunct5)( getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c), getParameterValue<D>(d), getParameterValue<E>(e)); 
				break;

			case 6: 
				(*m_pObj.*m_pFunct6)( getParameterValue<A>(a), getParameterValue<B>(b), getParameterValue<C>(c), getParameterValue<D>(d), getParameterValue<E>(e), getParameterValue<F>(f)); 
				break;
			};

		}
		catch (gcException &e)
		{
			safe_delete(vPList);
			return IPC::newParameter( e );
		}

		safe_delete(vPList);
		return new IPC::PVoid();
	}


	IPCParameterI* returnVal(char* buff, uint32 size)
	{
		return new PVoid();
	}

private:
	// pointer to object
	TObj* m_pObj;     

	// pointer to member function
	TFunct0 m_pFunct0;
	TFunct1 m_pFunct1;
	TFunct2 m_pFunct2;  
	TFunct3 m_pFunct3;  
	TFunct4 m_pFunct4;  
	TFunct5 m_pFunct5;  
	TFunct6 m_pFunct6;  
	
	uint8 m_uiNumParams;
};







template <class TObj>
NetworkFunctionI* networkFunctionV(TObj* pObj, void (TObj::*func)())
{
    return new NetworkFunctionVoid<TObj>(pObj, func);
}

template <class TObj, typename A>
NetworkFunctionI* networkFunctionV(TObj* pObj, void (TObj::*func)(A))
{
    return new NetworkFunctionVoid<TObj, A>(pObj, func);
}

template <class TObj, typename A, typename B>
NetworkFunctionI* networkFunctionV(TObj* pObj, void (TObj::*func)(A, B))
{
    return new NetworkFunctionVoid<TObj, A, B>(pObj, func);
}

template <class TObj, typename A, typename B, typename C>
NetworkFunctionI* networkFunctionV(TObj* pObj, void (TObj::*func)(A, B, C))
{
    return new NetworkFunctionVoid<TObj, A, B, C>(pObj, func);
}

template <class TObj, typename A, typename B, typename C, typename D>
NetworkFunctionI* networkFunctionV(TObj* pObj, void (TObj::*func)(A, B, C, D))
{
    return new NetworkFunctionVoid<TObj, A, B, C, D>(pObj, func);
}

template <class TObj, typename A, typename B, typename C, typename D, typename E>
NetworkFunctionI* networkFunctionV(TObj* pObj, void (TObj::*func)(A, B, C, D, E))
{
    return new NetworkFunctionVoid<TObj, A, B, C, D, E>(pObj, func);
}

template <class TObj, typename A, typename B, typename C, typename D, typename E, typename F>
NetworkFunctionI* networkFunctionV(TObj* pObj, void (TObj::*func)(A, B, C, D, E, F))
{
    return new NetworkFunctionVoid<TObj, A, B, C, D, E, F>(pObj, func);
}





template <class TObj, typename R>
NetworkFunctionI* networkFunction(TObj* pObj, R (TObj::*func)())
{
    return new NetworkFunction<TObj, R, PVoid, PVoid, PVoid, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A>
NetworkFunctionI* networkFunction(TObj* pObj, R (TObj::*func)(A))
{
    return new NetworkFunction<TObj, R, A, PVoid, PVoid, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B>
NetworkFunctionI* networkFunction(TObj* pObj, R (TObj::*func)(A, B))
{
    return new NetworkFunction<TObj, R, A, B, PVoid, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C>
NetworkFunctionI* networkFunction(TObj* pObj, R (TObj::*func)(A, B, C))
{
    return new NetworkFunction<TObj, R, A, B, C, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C, typename D>
NetworkFunctionI* networkFunction(TObj* pObj, R (TObj::*func)(A, B, C, D))
{
    return new NetworkFunction<TObj, R, A, B, C, D, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C, typename D, typename E>
NetworkFunctionI* networkFunction(TObj* pObj, R (TObj::*func)(A, B, C, D, E))
{
    return new NetworkFunction<TObj, R, A, B, C, D, E, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C, typename D, typename E, typename F>
NetworkFunctionI* networkFunction(TObj* pObj, R (TObj::*func)(A, B, C, D, E, F))
{
    return new NetworkFunction<TObj, R, A, B, C, D, E, F>(pObj, func);
}










class IPCEventI;




//! Represents a Inter process communication class
//!
class IPCClass : public IPCLockable
{
public:
	//! Constuctor
	//! 
	//! @param mang Manager
	//! @param id Class id
	//! @param itemId Active item id (used for filtering)
	//!
	IPCClass(IPCManager* mang, uint32 id, DesuraId itemId);

	//! deconstructor
	//!
	~IPCClass();

	//! Gets the class id
	//!
	//! @return Class id
	//!
	uint32 getId(){return m_uiId;}

	//! Send a message to the other side. Needs to be public due to event delegate
	//!
	//! @param type Message type
	//! @param buff Message buffer
	//! @param size Message size
	//!
	void sendMessage(uint8 type, const char* buff, uint32 size);

	//! Send a message to this side from IPC thread. Needs to be public due to event delegate
	//!
	//! @param type Message type
	//! @param buff Message buffer
	//! @param size Message size
	//!
	void sendLoopbackMessage(uint8 type, const char* buff, uint32 size);

	//! Gets the item id for filtering
	//!
	//! @return Item Id
	//!
	DesuraId getItemId(){return m_uiItemId;}

	//! Deletes this class and the same class on the other side
	//!
	void destroy();


	//! Calls a function on the other side of the IPC. Needs to be public due to helper function. Shouldnt be called directly
	//!
	//! @param name Function name
	//! @param async Do an async function call (immediate void return)
	//! @param a Paramater one
	//! @param b Paramater two
	//! @param c Paramater three
	//! @param d Paramater four
	//! @param e Paramater five
	//! @param f Paramater six
	//! @return Result
	//!
	IPCParameterI* callFunction(const char* name, bool async=false, IPCParameterI* a = NULL, IPCParameterI* b = NULL, IPCParameterI* c = NULL, IPCParameterI* d = NULL, IPCParameterI* e = NULL, IPCParameterI* f = NULL);
	
	//! Calls a function on the other side of the IPC. Needs to be public due to helper function. Shouldnt be called directly
	//!
	//! @param name Function name
	//! @param async Do an async function call (immediate void return)
	//! @param pList Parameter list
	//! @return Result
	//!
	virtual IPCParameterI* callFunction(const char* name, bool async, std::vector<IPCParameterI*> &pList);


	//! Calls a function on this side from IPC thread. Needs to be public due to helper function. Shouldnt be called directly
	//!
	//! @param name Function name
	//! @param async Do an async function call (immediate void return)
	//! @param a Paramater one
	//! @param b Paramater two
	//! @param c Paramater three
	//! @param d Paramater four
	//! @param e Paramater five
	//! @param f Paramater six
	//! @return Result
	//!
	IPCParameterI* callLoopback(const char* name, bool async=false, IPCParameterI* a = NULL, IPCParameterI* b = NULL, IPCParameterI* c = NULL, IPCParameterI* d = NULL, IPCParameterI* e = NULL, IPCParameterI* f = NULL);
	
	//! Calls a function on this side from IPC thread. Needs to be public due to helper function. Shouldnt be called directly
	//!
	//! @param name Function name
	//! @param async Do an async function call (immediate void return)
	//! @param pList Parameter list
	//! @return Result
	//!
	virtual IPCParameterI* callLoopback(const char* name, bool async, std::vector<IPCParameterI*> &pList);

	

protected:
	//! Registers a IPC callable function
	//!
	//! @param funct Function callback handle
	//! @param name Function name
	//!
	virtual void registerFunction(NetworkFunctionI* funct, const char* name);


	//! Registers a IPC event
	//!
	//! @param e Event callback handle
	//! @param name Event name
	//!
	void registerEvent(IPCEventI* e, const char* name);


	//! Sets the item id for filtering. Passes this onto every constucted class
	//!
	//! @param itemId Item id
	//!
	void setItemId(DesuraId itemId){m_uiItemId = itemId;}
	
	IPCManager* m_pManager;	//!< Manager


	//! Handles a new message from other side
	//!
	//! @param type Message type
	//! @param buff Message buffer
	//! @param size Message size
	//!
	virtual void messageRecived(uint8 type, const char* buff, uint32 size);

	//! Handles a function call from the other side
	//!
	//! @param buff Message buffer
	//! @param size Message size
	//! @param async Do an async function call (no return)
	//!
	virtual void handleFunctionCall(const char* buff, uint32 size, bool async = false);

	//! Handles a function return from the other side
	//!
	//! @param buff Message buffer
	//! @param size Message size
	//!
	virtual void handleFunctionReturn(const char* buff, uint32 size);

	//! Handles a event trigger from the other side
	//!
	//! @param buff Message buffer
	//! @param size Message size
	//!
	virtual void handleEventTrigger(const char* buff, uint32 size);


private:
	uint32 m_uiId;
	DesuraId m_uiItemId;

	std::map<uint32, NetworkFunctionI*> m_mFunc;
	std::map<uint32, IPCEventI*> m_mEvent;

	friend class IPCManager;
	friend class ProcessThread;
};








//need to reg the event on the reciever end
#define REG_EVENT( event ) registerEvent( IPC::IPCEventHandle( &event ), #event );
#define REG_EVENT_VOID( event ) registerEvent( new IPC::IPCEventV( &event ), #event );

//need to link the event on the sender end. Do not link on both ends other wise it will get in a loop
#define LINK_EVENT( event, type ) event += new IPC::IPCDelegate< type >( this , #event );
#define LINK_EVENT_VOID( event ) event += new IPC::IPCDelegateV( this , #event );


class IPCEventI
{
public:
	virtual ~IPCEventI(){;}
	virtual void trigger(char* buff, uint32 size)=0;
};


template <typename E>
class IPCEvent : public IPCEventI
{
public:
	IPCEvent(Event<E>* e)
	{
		m_pEvent = e;
	}	

	void trigger(char* buff, uint32 size)
	{
		IPCParameterI *r = IPC::getParameter<E>();
		r->deserialize(buff, size);

		E ret = IPC::getParameterValue<E>(r);
		m_pEvent->operator()(ret);

		safe_delete(r);
	}

private:
	Event<E> *m_pEvent;
};


class IPCEventV : public IPCEventI
{
public:
	IPCEventV(EventV* e)
	{
		m_pEvent = e;
	}	

	void trigger(char* buff, uint32 size)
	{
		m_pEvent->operator()();
	}

private:
	EventV *m_pEvent;
};

template <typename T>
IPCEventI* IPCEventHandle( Event<T> *e )
{
	return new IPC::IPCEvent<T>( e );
}



template <typename TArg>
class IPCDelegate : public DelegateI<TArg>
{
public:
	IPCDelegate(IPCClass* c, const char* name)
	{
		m_uiHash = UTIL::MISC::RSHash_CSTR(name);
		m_pClass = c;
	}

	IPCDelegate(IPCClass* c, uint32 hash)
	{
		m_uiHash = hash;
		m_pClass = c;
	}

	virtual bool equals(DelegateI<TArg>* di)
	{
		IPCDelegate<TArg> *d = dynamic_cast<IPCDelegate<TArg>*>(di);

		if (!d)
			return false;

		return ((m_uiHash == d->m_uiHash) && (m_pClass == d->m_pClass));
	}

	virtual DelegateI<TArg>* clone()
	{
		return new IPCDelegate(m_pClass, m_uiHash);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void operator()(TArg& a)
	{
		IPCParameterI* p = IPC::getParameter( a );

		uint32 size;
		const char* data = p->serialize(size);

		char* buff = new char[size + IPCEventTriggerSIZE];
		IPCEventTrigger *et = (IPCEventTrigger*)buff;

		et->size = size;
		et->eventHash = m_uiHash;

		memcpy(&et->data, data, size);

		m_pClass->sendMessage(MT_EVENTTRIGGER, (const char*)et, sizeofStruct(et) );

		safe_delete(buff);
		safe_delete(data);
		safe_delete(p);
	}

	uint32 m_uiHash;
	IPCClass* m_pClass;
};


class IPCDelegateV : public DelegateVI
{
public:
	IPCDelegateV(IPCClass* c, const char* name)
	{
		m_uiHash = UTIL::MISC::RSHash_CSTR(name);
		m_pClass = c;
	}

	IPCDelegateV(IPCClass* c, uint32 hash)
	{
		m_uiHash = hash;
		m_pClass = c;
	}

	virtual bool equals(DelegateVI* di)
	{
		IPCDelegateV *d = dynamic_cast<IPCDelegateV*>(di);

		if (!d)
			return false;

		return ((m_uiHash == d->m_uiHash) && (m_pClass == d->m_pClass));
	}

	virtual DelegateVI* clone()
	{
		return new IPCDelegateV(m_pClass, m_uiHash);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void operator()()
	{
		char* buff = new char[IPCEventTriggerSIZE];
		IPCEventTrigger *et = (IPCEventTrigger*)buff;

		et->size = 0;
		et->eventHash = m_uiHash;

		m_pClass->sendMessage(MT_EVENTTRIGGER, (const char*)et, sizeofStruct(et) );

		safe_delete(buff);
	}

	uint32 m_uiHash;
	IPCClass* m_pClass;
};









template <typename R>
R handleReturn(IPC::IPCParameterI* r)
{
	if (r->getType() == PException::getTypeS())
	{
		gcException gce((gcException*)r->getValue());
		safe_delete(r);
		throw gce;
	}

	if (r->getType() == PVoid::getTypeS())
		throw gcException();

	R temp = IPC::getParameterValue<R>(r, true);
	safe_delete(r);
	return temp;
}

inline void handleReturnV(IPC::IPCParameterI* r)
{
	if (r->getType() == PException::getTypeS())
	{
		gcException gce((gcException*)r->getValue());
		safe_delete(r);
		throw gce;
	}

	safe_delete(r);
	return;
}


template <typename R>
R functionCall(IPC::IPCClass* c, const char* name)
{
	IPC::IPCParameterI* r = c->callFunction( name, false );
	return handleReturn<R>(r);
}

template <typename R, typename A>
R functionCall(IPC::IPCClass* cl, const char* name, A a)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a));
	return handleReturn<R>(r);
}

template <typename R, typename A, typename B>
R functionCall(IPC::IPCClass* cl, const char* name, A a, B b)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b) );
	return handleReturn<R>(r);
}

template <typename R, typename A, typename B, typename C>
R functionCall(IPC::IPCClass* cl, const char* name, A a, B b, C c)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c) );
	return handleReturn<R>(r);
}

template <typename R, typename A, typename B, typename C, typename D>
R functionCall(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d) );
	return handleReturn<R>(r);
}

template <typename R, typename A, typename B, typename C, typename D, typename E>
R functionCall(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e) );
	return handleReturn<R>(r);
}

template <typename R, typename A, typename B, typename C, typename D, typename E, typename F>
R functionCall(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e, F f)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e), IPC::getParameter(f) );
	return handleReturn<R>(r);
}








inline void functionCallV(IPC::IPCClass* cl, const char* name)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false );
	handleReturnV(r);
}

template <typename A>
void functionCallV(IPC::IPCClass* cl, const char* name, A a)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a));
	handleReturnV(r);
}

template <typename A, typename B>
void functionCallV(IPC::IPCClass* cl, const char* name, A a, B b)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b) );
	handleReturnV(r);
}

template <typename A, typename B, typename C>
void functionCallV(IPC::IPCClass* cl, const char* name, A a, B b, C c)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D>
void functionCallV(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D, typename E>
void functionCallV(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D, typename E, typename F>
void functionCallV(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e, F f)
{
	IPC::IPCParameterI* r = cl->callFunction( name, false, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e), IPC::getParameter(f) );
	handleReturnV(r);
}










inline void functionCallAsync(IPC::IPCClass* cl, const char* name)
{
	IPC::IPCParameterI* r = cl->callFunction( name, true );
	handleReturnV(r);
}

template <typename A>
void functionCallAsync(IPC::IPCClass* cl, const char* name, A a)
{
	IPC::IPCParameterI* r = cl->callFunction( name, true, IPC::getParameter(a));
	handleReturnV(r);
}

template <typename A, typename B>
void functionCallAsync(IPC::IPCClass* cl, const char* name, A a, B b)
{
	IPC::IPCParameterI* r = cl->callFunction( name, true, IPC::getParameter(a), IPC::getParameter(b) );
	handleReturnV(r);
}

template <typename A, typename B, typename C>
void functionCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c)
{
	IPC::IPCParameterI* r = cl->callFunction( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D>
void functionCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d)
{
	IPC::IPCParameterI* r = cl->callFunction( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D, typename E>
void functionCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e)
{
	IPC::IPCParameterI* r = cl->callFunction( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D, typename E, typename F>
void functionCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e, F f)
{
	IPC::IPCParameterI* r = cl->callFunction( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e), IPC::getParameter(f) );
	handleReturnV(r);
}




inline void loopbackCallAsync(IPC::IPCClass* cl, const char* name)
{
	IPC::IPCParameterI* r = cl->callLoopback( name, true );
	handleReturnV(r);
}

template <typename A>
void loopbackCallAsync(IPC::IPCClass* cl, const char* name, A a)
{
	IPC::IPCParameterI* r = cl->callLoopback( name, true, IPC::getParameter(a));
	handleReturnV(r);
}

template <typename A, typename B>
void loopbackCallAsync(IPC::IPCClass* cl, const char* name, A a, B b)
{
	IPC::IPCParameterI* r = cl->callLoopback( name, true, IPC::getParameter(a), IPC::getParameter(b) );
	handleReturnV(r);
}

template <typename A, typename B, typename C>
void loopbackCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c)
{
	IPC::IPCParameterI* r = cl->callLoopback( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D>
void loopbackCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d)
{
	IPC::IPCParameterI* r = cl->callLoopback( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D, typename E>
void loopbackCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e)
{
	IPC::IPCParameterI* r = cl->callLoopback( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e) );
	handleReturnV(r);
}

template <typename A, typename B, typename C, typename D, typename E, typename F>
void loopbackCallAsync(IPC::IPCClass* cl, const char* name, A a, B b, C c, D d, E e, F f)
{
	IPC::IPCParameterI* r = cl->callLoopback( name, true, IPC::getParameter(a), IPC::getParameter(b), IPC::getParameter(c), IPC::getParameter(d), IPC::getParameter(e), IPC::getParameter(f) );
	handleReturnV(r);
}



}

#endif //DESURA_IPCClass_H
