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

//   Description :
//      Event Class is a multi purpose event system where by other classes/functions or events
//		can register interest in the event (by delegates) and receive notifaction when the
//		event triggers.
//
//		To create a new event just use the template class Event for paramater events or EventV
//		for void events.
//
//		I.e. Event<int> myIntEvent; Has one int parameter
//			 EventV myVoidEvent;	Has no parameters
//
//		To register intrest in an event use the overloaded += opperatior. I.e. for:
//			*Object		event += delegate( this, &Class::Function );
//			*Function	event += delegate( &Function );
//			*Event		event += delegate( &otherEvent );
//
//		Functions must be in the form: void Name([Type]& [pname]) where type is the same type
//		used to define the event or for void Events: void Name()
//
//		To trigger an event use the operator(). I.e. myIntEvent( intVar ) or myVoidEvent()


#ifndef DESURA_EVENT_H
#define DESURA_EVENT_H
#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include <algorithm>

#ifndef UNKNOWN_ITEM
#define UNKNOWN_ITEM 0xFFFFFFFF
#endif

#ifdef NIX
#include <typeinfo>
#endif

#include <functional>

#ifdef WIN32
	#include <WinBase.h>
#else
	 #include <pthread.h>
#endif
 
class VoidEventArg
{
};
 
template <typename TArg>
class DelegateI
{
public:
	virtual void operator()(TArg& a)=0;
	virtual bool equals(DelegateI<TArg>* d)=0;
	virtual DelegateI<TArg>* clone()=0;
	virtual void destroy()=0;
	virtual ~DelegateI(){}
};

class DelegateVI
{
public:
	virtual void operator()()=0;
	virtual bool equals(DelegateVI* d)=0;
	virtual DelegateVI* clone()=0;
	virtual void destroy()=0;
	virtual ~DelegateVI(){}
};


class InvokeI
{
public:
	virtual ~InvokeI(){;}
	virtual void invoke()=0;
	virtual void cancel()=0;
};

template <typename TArg, typename TDel>
class EventBase
{
public:
	EventBase()
	{
		assertType();

		m_bCancel = false;
		m_pCurDelegate = NULL;
	}

	EventBase(const EventBase<TArg, TDel>& e)
	{
		assertType();

		for (size_t x=0; x<e.m_vDelegates.size(); x++)
			m_vDelegates.push_back(e.m_vDelegates[x]->clone());

		m_bCancel = false;
		m_pCurDelegate = NULL;
	}

	~EventBase()
	{
		reset();
	}

	void assertType()
	{
		//shouldnt be using char* and wchar_t* in events as they can leak memory. Use gcString instead
		assert( typeid(TArg) != typeid(char*) );
		assert( typeid(TArg) != typeid(wchar_t*) );
		assert( typeid(TArg) != typeid(const char*) );
		assert( typeid(TArg) != typeid(const wchar_t*) );
	}

	void operator()(TArg& a)
	{
		//cant use this with void event
		assert( typeid(TArg) != typeid(VoidEventArg) );

		for (size_t x=0; x<m_vDelegates.size(); x++)
		{
			if (!m_vDelegates[x])
				continue;

			m_pCurDelegate = m_vDelegates[x];
			m_pCurDelegate->operator()(a);
			m_pCurDelegate = NULL;
	
			if (m_bCancel)
				break;
		}
	}

	void operator()()
	{
		for (size_t x=0; x<m_vDelegates.size(); x++)
		{
			if (!m_vDelegates[x])
				continue;

			m_pCurDelegate = m_vDelegates[x];
			m_pCurDelegate->operator()();
			m_pCurDelegate = NULL;

			if (m_bCancel)
				break;
		}
	}

	EventBase<TArg, TDel>& operator=(const EventBase<TArg, TDel>& e)
	{
		m_vDelegates = e.m_vDelegates;

		std::for_each(e.m_vDelegates.begin(), e.m_vDelegates.end(), [&](TDel* pDel)
		{
			this->m_vDelegates.push_back(pDel->clone());
		});


		return *this;
	}

	EventBase<TArg, TDel>& operator+=(const EventBase<TArg, TDel>& e)
	{
		for (size_t x=0; x<e.m_vDelegates.size(); x++)
		{
			TDel* d = e.m_vDelegates[x];

			if (findInfo(d) == UNKNOWN_ITEM)
				m_vDelegates.push_back(d->clone());
		}

		return *this;
	}

	EventBase<TArg, TDel>& operator-=(const EventBase<TArg, TDel>& e)
	{
		std::vector<size_t> del;

		for (size_t x=0; x<e.m_vDelegates.size(); x++)
		{
			TDel* d = e.m_vDelegates[x];
			size_t index = findInfo(d);

			if (index == UNKNOWN_ITEM)
				continue;

			del.push_back(index);
		}

		std::sort(del.begin(), del.end());

		for (size_t x=del.size(); x>0; x--)
			m_vDelegates.erase(m_vDelegates.begin()+del[x]);

		return *this;
	}


	EventBase<TArg, TDel>& operator+=(TDel* d)
	{
		if (!d)
			return *this;

		if (findInfo(d) == UNKNOWN_ITEM)
			m_vDelegates.push_back(d->clone());

		d->destroy();
		return *this;
	}

	EventBase<TArg, TDel>& operator-=(TDel* d)
	{
		if (!d)
			return *this;

		for (auto it=m_vDelegates.begin(); it != m_vDelegates.end(); ++it)
		{
			if ((*it)->equals(d) == false)
				continue;

			(*it)->destroy();
			m_vDelegates.erase(it);
			break;
		}

		d->destroy();
		return *this;
	}

	void reset()
	{
		if (!this || !&m_vDelegates)
			return;

		m_bCancel = true;
		InvokeI* i = dynamic_cast<InvokeI*>(m_pCurDelegate);

		if (i)
			i->cancel();

		for (size_t x=0; x<m_vDelegates.size(); x++)
		{
			if (m_vDelegates[x])
				m_vDelegates[x]->destroy();
		}

		m_vDelegates.clear();
		m_bCancel = false;
	}

protected:
	size_t findInfo(TDel* d)
	{
		for (size_t x=0; x<m_vDelegates.size(); x++)
		{
			if (m_vDelegates[x]->equals(d))
				return x;
		}

		return UNKNOWN_ITEM;
	}

private:
	std::vector<TDel*> m_vDelegates;
	bool m_bCancel;

	TDel* m_pCurDelegate;
};





template <typename A>
void CallFunction(void (*func)(), A &a)
{
}

template <typename A>
void CallFunction(void (*func)(A &a), A &a)
{
	if (func)
		(*func)(a);
}

template <typename A>
void CallFunction(void (*func)())
{
	if (func)
		(*func)();
}

template <typename A>
void CallFunction(void (*func)(A &a))
{
}


template <typename TObj>
void CallObjectFunction(TObj* obj, void (TObj::*func)())
{
	if (obj && func)
		(*obj.*func)();
}

template <typename TObj, typename A>
void CallObjectFunction(TObj* obj, void (TObj::*func)(A &a))
{
}

template <typename TObj, typename A>
void CallObjectFunction(TObj* obj, void (TObj::*func)(), A &a)
{
}

template <typename TObj, typename A>
void CallObjectFunction(TObj* obj, void (TObj::*func)(A &a), A &a)
{
	if (obj && func)
		(*obj.*func)(a);
}


template <typename A, typename TDel>
void CallEvent(EventBase<A, DelegateVI>* event, A &a)
{
}

template <typename A, typename TDel>
void CallEvent(EventBase<A, TDel>* event, A &a)
{
	if (event)
		(*event)(a);
}

template <typename A, typename TDel>
inline void CallEvent(EventBase<A, DelegateVI>* event)
{
	if (event)
		(*event)();
}

template <typename A, typename TDel>
void CallEvent(EventBase<A, TDel>* event)
{
}


template <typename TFunct, typename TDel, typename TArg>
class FunctDelegateBase : public TDel
{
public:
	FunctDelegateBase(TFunct f)
	{
		m_pFunct = f;
	}

	virtual bool equals(TDel* di)
	{
		FunctDelegateBase<TFunct, TDel, TArg> *d = dynamic_cast<FunctDelegateBase<TFunct, TDel, TArg>*>(di);

		if (!d)
			return false;

		return (m_pFunct == d->m_pFunct);
	}

	virtual TDel* clone()
	{
		return new FunctDelegateBase<TFunct, TDel, TArg>(m_pFunct);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void operator()(TArg& a)
	{
		//cant use this with void event
		assert( typeid(TArg) != typeid(VoidEventArg) );
		CallFunction<TArg>(m_pFunct, a);
	}

	virtual void operator()()
	{
		CallFunction<TArg>(m_pFunct);
	}

	TFunct m_pFunct;   // pointer to member function
};



template <typename TFunct, typename TDel, typename TObj, typename TArg>
class ObjDelegateBase : public TDel
{
public:
	ObjDelegateBase(TObj* t, TFunct f)
	{
		m_pObj = t;
		m_pFunct = f;
	}

	ObjDelegateBase(ObjDelegateBase<TFunct, TDel, TObj,TArg> *oDel)
	{
		assert(oDel);

		m_pObj = oDel->m_pObj;
		m_pFunct = oDel->m_pFunct;
	}

	virtual bool equals(TDel* di)
	{
		ObjDelegateBase<TFunct, TDel, TObj, TArg> *d = dynamic_cast<ObjDelegateBase<TFunct, TDel, TObj, TArg>*>(di);

		if (!d)
			return false;

		return ((m_pObj == d->m_pObj) && (m_pFunct == d->m_pFunct));
	}

	virtual TDel* clone()
	{
		return new ObjDelegateBase<TFunct, TDel, TObj, TArg>(m_pObj, m_pFunct);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void operator()(TArg& a)
	{
		//cant use this with void event
		assert( typeid(TArg) != typeid(VoidEventArg) );
		CallObjectFunction<TObj, TArg>(m_pObj, m_pFunct, a);
	}

	virtual void operator()()
	{
		CallObjectFunction<TObj>(m_pObj, m_pFunct);
	}

	TFunct m_pFunct;   // pointer to member function
	TObj* m_pObj;     // pointer to object

protected:
	ObjDelegateBase()
	{
		m_pObj = NULL;
		m_pFunct = NULL;
	}

	void init(TObj* t, TFunct f)
	{
		m_pObj = t;
		m_pFunct = f;
	}
};



template <typename TArg, typename TDel>
class ProxieDelegateBase : public TDel
{
public:
	ProxieDelegateBase(EventBase<TArg, TDel>* e)
	{
		m_pEvent = e;
	}

	virtual bool equals(TDel* di)
	{
		ProxieDelegateBase<TArg, TDel> *d = dynamic_cast<ProxieDelegateBase<TArg, TDel>*>(di);

		if (!d)
			return false;

		return (m_pEvent == d->m_pEvent);
	}

	virtual TDel* clone()
	{
		return new ProxieDelegateBase<TArg, TDel>(m_pEvent);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void operator()(TArg& a)
	{
		//cant use this with void event
		assert( typeid(TArg) != typeid(VoidEventArg) );
		CallEvent<TArg, TDel>(m_pEvent, a);
	}

	virtual void operator()()
	{
		CallEvent<TArg, TDel>(m_pEvent);
	}

	EventBase<TArg, TDel>* m_pEvent;   // pointer to member function
};



template <typename TLambda, typename TArg, typename TDel>
class LambdaDelegateBase : public TDel
{
public:
	LambdaDelegateBase(TLambda& f) : m_pFunct(f)
	{
		m_uiId = (size_t)this;
	}

	virtual bool equals(TDel* di)
	{
		LambdaDelegateBase<TLambda, TArg, TDel> *d = dynamic_cast<LambdaDelegateBase<TLambda, TArg, TDel>*>(di);

		if (!d)
			return false;

		return (m_uiId == d->m_uiId);
	}

	virtual DelegateVI* clone()
	{
		return new LambdaDelegateBase<TLambda, TArg, TDel>(m_pFunct, m_uiId);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void operator()()
	{
		m_pFunct();
	}

protected:
	LambdaDelegateBase(TLambda& f, size_t id) : m_pFunct(f)
	{
		m_uiId = id;
	}

	size_t m_uiId;
	TLambda m_pFunct;   // pointer to member function
};
































template <typename TArg>
class Event : public EventBase<TArg, DelegateI<TArg> >
{
public:
	Event() : EventBase<TArg, DelegateI<TArg> >()
	{
	}

	Event(const EventBase<TArg, DelegateI<TArg> >& e) : EventBase<TArg, DelegateI<TArg> >(e)
	{
	}
};

typedef EventBase<VoidEventArg, DelegateVI> EventV;



template <class TObj, class TArg>
DelegateI<TArg>* delegate(TObj* pObj, void (TObj::*NotifyMethod)(TArg&))
{
	return new ObjDelegateBase<void (TObj::*)(TArg&), DelegateI<TArg>, TObj, TArg>(pObj, NotifyMethod);
}

template <class TObj>
DelegateVI* delegate(TObj* pObj, void (TObj::*NotifyMethod)())
{
	return new ObjDelegateBase<void (TObj::*)(), DelegateVI, TObj, VoidEventArg>(pObj, NotifyMethod);
}



template <class TArg>
DelegateI<TArg>* delegate(void (*NotifyMethod)(TArg&))
{
	return new FunctDelegateBase<void (*)(TArg&), DelegateI<TArg>, TArg>(NotifyMethod);
}

inline DelegateVI* delegate(void (*NotifyMethod)())
{
	return new FunctDelegateBase<void (*)(), DelegateVI, VoidEventArg>(NotifyMethod);
}



template <class TArg>
DelegateI<TArg>* delegate(Event<TArg>* e)
{
	return new ProxieDelegateBase<TArg, DelegateI<TArg> >(e);
}

inline DelegateVI* delegate(EventV* e)
{
	return new ProxieDelegateBase<VoidEventArg, DelegateVI>(e);
}


//template <typename TArg>
//DelegateI<TArg>* delegateL(std::function<void (TArg&)> & f)
//{
//	return new LambdaDelegateBase<std::function<void (TArg&)>, TArg, DelegateI<TArg>>(f);
//}
//
//template <typename TArg>
//DelegateI<TArg>* delegateL(const std::function<void (TArg&)> &f)
//{
//	return new LambdaDelegateBase<const std::function<void (TArg&)>, TArg, DelegateI<TArg>>(f);
//}
//
//
//template <typename TLambda>
//DelegateVI* delegateLV(TLambda& f)
//{
//	return new LambdaDelegateBase<TLambda, VoidEventArg, DelegateVI>(f);
//}
//
//template <typename TLambda>
//DelegateVI* delegateLV(const TLambda& f)
//{
//	return new LambdaDelegateBase<const TLambda, VoidEventArg, DelegateVI>(f);
//}











template <typename TObj, typename TArg>
class ObjDelegate : public ObjDelegateBase<void (TObj::*)(TArg&), DelegateI<TArg>, TObj, TArg>
{
public:
	typedef void (TObj::*TFunct)(TArg&);

	ObjDelegate(TObj* t, TFunct f) : ObjDelegateBase<void (TObj::*)(TArg&), DelegateI<TArg>, TObj, TArg>(t, f)
	{
	}

	ObjDelegate(ObjDelegate<TObj,TArg> *oDel) : ObjDelegateBase<void (TObj::*)(TArg&), DelegateI<TArg>, TObj, TArg>(oDel)
	{
	}

protected:
	ObjDelegate()
	{
	}
};

template <typename TObj>
class ObjDelegateV : public ObjDelegateBase<void (TObj::*)(), DelegateVI, TObj, VoidEventArg>
{
public:
	typedef void (TObj::*TFunct)();

	ObjDelegateV(TObj* t, TFunct f) : ObjDelegateBase<void (TObj::*)(), DelegateVI, TObj, VoidEventArg>(t, f)
	{
	}

	ObjDelegateV(ObjDelegateV<TObj> *oDel) : ObjDelegateBase<void (TObj::*)(), DelegateVI, TObj, VoidEventArg>(oDel)
	{
	}

protected:
	ObjDelegateV()
	{
	}
};












#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4355 )
#endif

template <typename TObj, typename TArg, typename TExtra>
class ExtraDelegate : public ObjDelegate<ExtraDelegate<TObj, TArg, TExtra>, TArg>
{
public:
	typedef void (TObj::*TFunct)(TExtra, TArg&); 

	ExtraDelegate(TObj* t, TExtra e, TFunct f) : ObjDelegate<ExtraDelegate<TObj, TArg, TExtra>, TArg>(this, &ExtraDelegate::callBack), m_Extra(e)
	{
		m_pFunct = f;
		m_pObj = t;
	}

	void callBack(TArg& a)
	{
		if (m_pObj && m_pFunct)
		{
			(*m_pObj.*m_pFunct)(m_Extra, a);
		}
	}

	virtual DelegateI<TArg>* clone()
	{
		return new ExtraDelegate<TObj, TArg, TExtra>(m_pObj, m_Extra, m_pFunct);
	}

	virtual bool equals(DelegateI<TArg>* di)
	{
		ExtraDelegate<TObj, TArg, TExtra> *d = dynamic_cast<ExtraDelegate<TObj, TArg, TExtra>*>(di);

		if (!d)
			return false;

		return ((m_pObj == d->m_pObj) && (m_pFunct == d->m_pFunct) && m_Extra == d->m_Extra);
	}

	virtual void destroy()
	{
		delete this;
	}

	TFunct m_pFunct;   // pointer to member function
	TObj* m_pObj;     // pointer to object
	TExtra m_Extra;
};

template <class TObj, class TArg, class TExtra>
DelegateI<TArg>* extraDelegate(TObj* pObj, void (TObj::*NotifyMethod)(TExtra, TArg&), TExtra tExtra)
{
	return new ExtraDelegate<TObj, TArg, TExtra>(pObj, tExtra, NotifyMethod);
}

template <typename TObj, typename TExtra>
class ExtraDelegateV : public ObjDelegateV<ExtraDelegateV<TObj, TExtra> >
{
public:
	typedef void (TObj::*TFunct)(TExtra); 

	ExtraDelegateV(TObj* t, TExtra e, TFunct f) : ObjDelegateV<ExtraDelegateV<TObj, TExtra> >(this, &ExtraDelegateV::callBack)
	{
		m_Extra = e;
		m_pFunct = f;
		m_pObj = t;
	}

	void callBack()
	{
		if (m_pObj && m_pFunct)
		{
			(*m_pObj.*m_pFunct)(m_Extra);
		}
	}

	virtual DelegateVI* clone()
	{
		return new ExtraDelegateV<TObj, TExtra>(m_pObj, m_Extra, m_pFunct);
	}

	virtual bool equals(DelegateVI* di)
	{
		ExtraDelegateV<TObj, TExtra> *d = dynamic_cast<ExtraDelegateV<TObj, TExtra>*>(di);

		if (!d)
			return false;

		return ((m_pObj == d->m_pObj) && (m_pFunct == d->m_pFunct) && m_Extra == d->m_Extra);
	}

	virtual void destroy()
	{
		delete this;
	}

	TFunct m_pFunct;   // pointer to member function
	TObj* m_pObj;     // pointer to object
	TExtra m_Extra;
};

#pragma warning( pop )

template <class TObj, class TExtra>
DelegateVI* extraDelegate(TObj* pObj, void (TObj::*NotifyMethod)(TExtra), TExtra tExtra)
{
	return new ExtraDelegateV<TObj, TExtra>(pObj, tExtra, NotifyMethod);
}


#endif //DESURA_EVENT_H
