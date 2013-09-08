/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>
          (C) Mark Chandler <mark@moddb.com>

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

#ifndef DESURA_GUIDELEGATE_H
#define DESURA_GUIDELEGATE_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include "Event.h"
#include "util_thread/BaseThread.h"

#include "boost/shared_ptr.hpp"

class gcPanel;
class gcDialog;
class gcScrolledWindow;
class gcFrame;
class gcTaskBarIcon;


enum MODE
{
	MODE_PENDING,
	MODE_PENDING_WAIT,
	MODE_PROCESS,
};

uint64 GetMainThreadId();


class EventHelper
{
public:
	EventHelper()
	{
		m_bDone = false;
	}

	void done()
	{
		if (m_bDone)
			return;

		m_bDone = true;
		m_WaitCond.notify();
	}

	void wait()
	{
		while (!m_bDone)
			m_WaitCond.wait(0, 500);
	}

private:
	Thread::WaitCondition m_WaitCond;
	volatile bool m_bDone;
};


class wxGuiDelegateEvent : public wxNotifyEvent
{
public:
	wxGuiDelegateEvent();
	wxGuiDelegateEvent(boost::shared_ptr<InvokeI> invoker, int winId);
	wxGuiDelegateEvent(InvokeI* invoker, int winId);

	wxGuiDelegateEvent(const wxGuiDelegateEvent& event);

	~wxGuiDelegateEvent();

	virtual wxEvent *Clone() const;
	void invoke();

private:
	boost::shared_ptr<InvokeI> m_spDelegate;
	InvokeI* m_pDelegate;

	DECLARE_DYNAMIC_CLASS(wxGuiDelegateEvent);
};

wxDECLARE_EVENT(wxEVT_GUIDELEGATE, wxGuiDelegateEvent);

class wxDelegate
{
public:
	virtual void nullObject()=0;
};

template <typename T>
class wxGuiDelegateImplementation : public T
{
public:
	wxGuiDelegateImplementation(wxWindow *parent)
		: T(parent)
	{
		this->Bind(wxEVT_GUIDELEGATE, &wxGuiDelegateImplementation::onEventCallBack, this);
	}

	wxGuiDelegateImplementation(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
		: T(parent, id, pos, size, style)
	{
		this->Bind(wxEVT_GUIDELEGATE, &wxGuiDelegateImplementation::onEventCallBack, this);
	}

	wxGuiDelegateImplementation(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
		: T(parent, id, title, pos, size, style)
	{
		this->Bind(wxEVT_GUIDELEGATE, &wxGuiDelegateImplementation::onEventCallBack, this);
	}

	~wxGuiDelegateImplementation()
	{
		this->Unbind(wxEVT_GUIDELEGATE, &wxGuiDelegateImplementation::onEventCallBack, this);
		cleanUpEvents();
	}

	void registerDelegate(wxDelegate* d)
	{
		doDeregisterDelegate(d);

		m_ListLock.lock();
		m_vDelgateList.push_back(d);
		m_ListLock.unlock();
	}

	void deregisterDelegate(wxDelegate* d)
	{
		bool bFound = doDeregisterDelegate(d);
		assert( bFound );
	}

	void cleanUpEvents()
	{
		m_ListLock.lock();

		for (size_t x=0; x<m_vDelgateList.size(); x++)
			m_vDelgateList[x]->nullObject();

		m_vDelgateList.clear();

		m_ListLock.unlock();
	}

private:
	bool doDeregisterDelegate(wxDelegate* d)
	{
		bool bFound = false;
		m_ListLock.lock();

		for (size_t x=0; x<m_vDelgateList.size(); x++)
		{
			if (m_vDelgateList[x] == d)
			{
				m_vDelgateList.erase(m_vDelgateList.begin()+x);
				bFound = true;
				break;
			}
		}

		m_ListLock.unlock();	
		return bFound;
	}

	void onEventCallBack(wxGuiDelegateEvent& event)
	{
		event.invoke();
	}

	::Thread::Mutex m_ListLock;
	std::vector<wxDelegate*> m_vDelgateList;
};


















template <class TObj, typename TArg>
class Invoker : public InvokeI
{
public:
	typedef void (TObj::*TFunct)(TArg&); 

	Invoker(DelegateI<TArg> *oDel, TArg* a)
	{
		m_pDelegate = oDel;
		m_pArg = a;
	}

	~Invoker()
	{
		safe_delete(m_pDelegate);
		safe_delete(m_pArg);
	}

	void invoke()
	{
		if (m_pDelegate)
			m_pDelegate->operator()(*m_pArg);
	}

	void cancel()
	{
		printf("--- Cancel Invoker\n");
	}

	DelegateI<TArg> *m_pDelegate;
	TArg* m_pArg;
};






template <class TObj, typename TArg>
class PrimInvoker : public InvokeI
{
public:
	typedef void (TObj::*TFunct)(TArg&); 

	PrimInvoker(DelegateI<TArg> *oDel, TArg a) : m_Arg(a)
	{
		m_pDelegate = oDel;
	}

	~PrimInvoker()
	{
		m_pHelper.done();
		safe_delete(m_pDelegate);
	}

	void invoke()
	{
		if (m_pDelegate)
			m_pDelegate->operator()(m_Arg);
	
		m_pHelper.done();
	}

	void cancel()
	{
		m_pHelper.done();
	}

	void wait()
	{
		m_pHelper.wait();
	}

	EventHelper m_pHelper;
	DelegateI<TArg> *m_pDelegate;
	TArg m_Arg;
};


template <class TObj, typename TArg>
class GuiDelegate : public ObjDelegate<TObj, TArg>, public wxDelegate
{
public:
	typedef void (TObj::*TFunct)(TArg&); 

	GuiDelegate(TObj* t, TFunct f, MODE mode) : ObjDelegate<TObj, TArg>(t,f)
	{
		m_Mode = mode;
		m_pInvoker = NULL;

		if (ObjDelegate<TObj, TArg>::m_pObj)
			ObjDelegate<TObj, TArg>::m_pObj->registerDelegate(this);
	}

protected:
	GuiDelegate(MODE mode) : ObjDelegate<TObj, TArg>()
	{
		m_Mode = mode;
		m_pInvoker = NULL;
	}

public:
	~GuiDelegate()
	{
		if (ObjDelegate<TObj, TArg>::m_pObj)
			ObjDelegate<TObj, TArg>::m_pObj->deregisterDelegate(this);

	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void nullObject()
	{
		ObjDelegate<TObj, TArg>::m_pObj = NULL;
		ObjDelegate<TObj, TArg>::m_pFunct = NULL;
		
		m_InvokerMutex.lock();
		
		if (m_pInvoker)
			m_pInvoker->cancel();
			
		m_InvokerMutex.unlock();
	}

	virtual DelegateI<TArg>* clone()
	{
		return new GuiDelegate(ObjDelegate<TObj, TArg>::m_pObj, ObjDelegate<TObj, TArg>::m_pFunct, m_Mode);
	}

	void operator()(TArg& a)
	{	
		if (!ObjDelegate<TObj, TArg>::m_pObj || !ObjDelegate<TObj, TArg>::m_pFunct)
			return;

		if (m_Mode == MODE_PENDING)
		{
			InvokeI *i = new Invoker<TObj, TArg>(new ObjDelegate<TObj, TArg>(this), new TArg(a));

			wxGuiDelegateEvent event(boost::shared_ptr<InvokeI>(i), ObjDelegate<TObj, TArg>::m_pObj->GetId());
			ObjDelegate<TObj, TArg>::m_pObj->GetEventHandler()->AddPendingEvent(event);
		}
		else if (m_Mode == MODE_PROCESS || Thread::BaseThread::GetCurrentThreadId() == GetMainThreadId())
		{
			ObjDelegate<TObj, TArg>::operator()(a);
		}
		else if (m_Mode == MODE_PENDING_WAIT)
		{
			PrimInvoker<TObj, TArg> *i = new PrimInvoker<TObj, TArg>(new ObjDelegate<TObj, TArg>(this), a);
			boost::shared_ptr<InvokeI> invoker(i);

			wxGuiDelegateEvent event(invoker, ObjDelegate<TObj, TArg>::m_pObj->GetId());
			ObjDelegate<TObj, TArg>::m_pObj->GetEventHandler()->AddPendingEvent(event);

			setInvoker(i);
			i->wait();
			setInvoker(NULL);
			
			a = i->m_Arg;
		}
	}

	MODE m_Mode;

protected:
	void setInvoker(PrimInvoker<TObj, TArg> *i)
	{
		m_InvokerMutex.lock();
		m_pInvoker = i;
		m_InvokerMutex.unlock();	
	}

	void init(TObj* t, TFunct f)
	{
		ObjDelegate<TObj, TArg>::init(t, f);

		if (ObjDelegate<TObj, TArg>::m_pObj)
			ObjDelegate<TObj, TArg>::m_pObj->registerDelegate(this);
	}
	
	::Thread::Mutex m_InvokerMutex;
	PrimInvoker<TObj, TArg> *m_pInvoker;
};

template <class TObj, typename TArg>
class GuiPrimDelegate : public ObjDelegate<TObj, TArg>, public wxDelegate
{
public:
	typedef void (TObj::*TFunct)(TArg&); 

	GuiPrimDelegate(TObj* t, TFunct f, MODE mode) : ObjDelegate<TObj, TArg>(t,f)
	{
		m_Mode = mode;

		if (ObjDelegate<TObj, TArg>::m_pObj)
			ObjDelegate<TObj, TArg>::m_pObj->registerDelegate(this);

		m_pInvoker = NULL;
	}

	~GuiPrimDelegate()
	{
		if (ObjDelegate<TObj, TArg>::m_pObj)
			ObjDelegate<TObj, TArg>::m_pObj->deregisterDelegate(this);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void nullObject()
	{
		ObjDelegate<TObj, TArg>::m_pObj = NULL;
		ObjDelegate<TObj, TArg>::m_pFunct = NULL;

		m_InvokerMutex.lock();
		
		if (m_pInvoker)
			m_pInvoker->cancel();
			
		m_InvokerMutex.unlock();
	}

	virtual DelegateI<TArg>* clone()
	{
		return new GuiPrimDelegate(ObjDelegate<TObj, TArg>::m_pObj, ObjDelegate<TObj, TArg>::m_pFunct, m_Mode);
	}

	void operator()(TArg& a)
	{
		if (!ObjDelegate<TObj, TArg>::m_pObj || !ObjDelegate<TObj, TArg>::m_pFunct)
			return;

		if (m_Mode == MODE_PENDING)
		{
			InvokeI *i = new Invoker<TObj, TArg>(new ObjDelegate<TObj, TArg>(this), a);

			wxGuiDelegateEvent event(i, ObjDelegate<TObj, TArg>::m_pObj->GetId());
			ObjDelegate<TObj, TArg>::m_pObj->GetEventHandler()->AddPendingEvent(event);
		}
		else if (m_Mode == MODE_PROCESS || Thread::BaseThread::GetCurrentThreadId() == GetMainThreadId())
		{
			ObjDelegate<TObj, TArg>::operator()(a);
		}
		else if (m_Mode == MODE_PENDING_WAIT)
		{
			PrimInvoker<TObj, TArg> *i = new PrimInvoker<TObj, TArg>(new ObjDelegate<TObj, TArg>(this), a);
			boost::shared_ptr<InvokeI> invoker(i);

			wxGuiDelegateEvent event(invoker, ObjDelegate<TObj, TArg>::m_pObj->GetId());
			ObjDelegate<TObj, TArg>::m_pObj->GetEventHandler()->AddPendingEvent(event);

			setInvoker(i);
			i->wait();
			setInvoker(NULL);
			a = i->m_Arg;
		}
	}

private:
	MODE m_Mode;

	void setInvoker(PrimInvoker<TObj, TArg> *i)
	{
		m_InvokerMutex.lock();
		m_pInvoker = i;
		m_InvokerMutex.unlock();
	}

	::Thread::Mutex m_InvokerMutex;
	PrimInvoker<TObj, TArg> *m_pInvoker;
};





template <class TObj>
inline bool validateForm(TObj* pObj)
{
	gcPanel* pan = dynamic_cast<gcPanel*>(pObj);
	gcFrame* frm = dynamic_cast<gcFrame*>(pObj);
	gcDialog* dlg = dynamic_cast<gcDialog*>(pObj);
	gcScrolledWindow* swin = dynamic_cast<gcScrolledWindow*>(pObj);
	gcTaskBarIcon* gtbi = dynamic_cast<gcTaskBarIcon*>(pObj);

	return (pan || frm || dlg || swin || gtbi);
}


#define PRIMOVERIDEDELEGATE( type )	template <class TObj, type> DelegateI<type>* guiDelegate(TObj* pObj, void (TObj::*NotifyMethod)(type&), MODE mode = MODE_PENDING){if (!validateForm(pObj)){assert(false);return NULL;}return new GuiPrimDelegate<TObj, type>(pObj, NotifyMethod, mode);}

PRIMOVERIDEDELEGATE( bool );
PRIMOVERIDEDELEGATE( int8 );
PRIMOVERIDEDELEGATE( int16 );
PRIMOVERIDEDELEGATE( int32 );
PRIMOVERIDEDELEGATE( int64 );
PRIMOVERIDEDELEGATE( uint8 );
PRIMOVERIDEDELEGATE( uint16 );
PRIMOVERIDEDELEGATE( uint32 );
PRIMOVERIDEDELEGATE( uint64 );

template <class TObj, class TArg>
DelegateI<TArg>* guiDelegate(TObj* pObj, void (TObj::*NotifyMethod)(TArg&), MODE mode = MODE_PENDING)
{
	if (!validateForm(pObj))
	{
		assert(false);
		return NULL;
	}

	return new GuiDelegate<TObj, TArg>(pObj, NotifyMethod, mode);
}































template <class TObj>
class InvokerV : public InvokeI
{
public:
	typedef void (TObj::*TFunct)(); 

	InvokerV(DelegateVI *oDel)
	{
		m_pDelegate = oDel;
	}

	~InvokerV()
	{
		m_pEventHelper.done();
		safe_delete(m_pDelegate);
	}

	void invoke()
	{
		if (m_pDelegate)
			m_pDelegate->operator()();

		m_pEventHelper.done();
	}

	void cancel()
	{
		m_pEventHelper.done();
	}

	void wait()
	{
		m_pEventHelper.wait();
	}

	EventHelper m_pEventHelper;
	DelegateVI *m_pDelegate;
};


template <class TObj>
class GuiDelegateV : public ObjDelegateV<TObj>, public wxDelegate
{
public:
	typedef void (TObj::*TFunct)(); 

	GuiDelegateV(TObj* t, TFunct f, MODE mode) : ObjDelegateV<TObj>(t,f)
	{
		m_Mode = mode;
		m_pInvoker = NULL;

		if (ObjDelegateV<TObj>::m_pObj)
			ObjDelegateV<TObj>::m_pObj->registerDelegate(this);
	}

protected:
	GuiDelegateV(MODE mode) : ObjDelegateV<TObj>()
	{
		m_Mode = mode;
		m_pInvoker = NULL;
	}

public:
	~GuiDelegateV()
	{
		if (ObjDelegateV<TObj>::m_pObj)
			ObjDelegateV<TObj>::m_pObj->deregisterDelegate(this);
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void nullObject()
	{
		ObjDelegateV<TObj>::m_pObj = NULL;
		ObjDelegateV<TObj>::m_pFunct = NULL;
		
		m_InvokerMutex.lock();
		
		if (m_pInvoker)
			m_pInvoker->cancel();
			
		m_InvokerMutex.unlock();
	}

	virtual DelegateVI* clone()
	{
		return new GuiDelegateV(ObjDelegateV<TObj>::m_pObj, ObjDelegateV<TObj>::m_pFunct, m_Mode);
	}

	void operator()()
	{
		if (!ObjDelegateV<TObj>::m_pObj || !ObjDelegateV<TObj>::m_pFunct)
			return;

		if (m_Mode == MODE_PENDING)
		{
			InvokerV<TObj> *i = new InvokerV<TObj>(new ObjDelegateV<TObj>(this));

			wxGuiDelegateEvent event(boost::shared_ptr<InvokeI>(i), ObjDelegateV<TObj>::m_pObj->GetId());
			ObjDelegateV<TObj>::m_pObj->GetEventHandler()->AddPendingEvent(event);
		}
		else if (m_Mode == MODE_PROCESS || Thread::BaseThread::GetCurrentThreadId() == GetMainThreadId())
		{
			ObjDelegateV<TObj>::operator()();
		}
		else if (m_Mode == MODE_PENDING_WAIT)
		{
			InvokerV<TObj> *i = new InvokerV<TObj>(new ObjDelegateV<TObj>(this));
			boost::shared_ptr<InvokeI> invoker(i);

			wxGuiDelegateEvent event(invoker, ObjDelegateV<TObj>::m_pObj->GetId());
			ObjDelegateV<TObj>::m_pObj->GetEventHandler()->AddPendingEvent(event);

			setInvoker(i);
			i->wait();
			setInvoker(NULL);
		}
	}

	MODE m_Mode;

protected:
	void setInvoker(InvokerV<TObj> *i)
	{
		m_InvokerMutex.lock();
		m_pInvoker = i;
		m_InvokerMutex.unlock();
	}

	void init(TObj* t, TFunct f)
	{
		ObjDelegateV<TObj>::init(t, f);

		if (ObjDelegateV<TObj>::m_pObj)
			ObjDelegateV<TObj>::m_pObj->registerDelegate(this);
	}
	
	::Thread::Mutex m_InvokerMutex;
	InvokerV<TObj> *m_pInvoker;
};












template <class TObj>
DelegateVI* guiDelegate(TObj* pObj, void (TObj::*NotifyMethod)(), MODE mode = MODE_PENDING)
{
	if (!validateForm(pObj))
	{
		assert(false);
		return NULL;
	}

	return new GuiDelegateV<TObj>(pObj, NotifyMethod, mode);
}








































template <typename TObj, typename TArg, typename TExtra>
class GuiExtraDelegate : public GuiDelegate<GuiExtraDelegate<TObj, TArg, TExtra>, TArg>
{
public:
	typedef void (TObj::*TFunct)(TExtra, TArg&); 

	GuiExtraDelegate(TObj* t, TExtra e, TFunct f, MODE mode) : GuiDelegate<GuiExtraDelegate<TObj, TArg, TExtra>, TArg>::GuiDelegate(mode)
	{
		m_Extra = e;
		m_pFunct = f;
		m_pObj = t;

		this->init(this, &GuiExtraDelegate::callBack);
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
		return new GuiExtraDelegate<TObj, TArg, TExtra>(m_pObj, m_Extra, m_pFunct, GuiDelegate<GuiExtraDelegate<TObj, TArg, TExtra>, TArg>::m_Mode);
	}

	virtual bool equals(DelegateI<TArg>* di)
	{
		GuiExtraDelegate<TObj, TArg, TExtra> *d = dynamic_cast<GuiExtraDelegate<TObj, TArg, TExtra>*>(di);

		if (!d)
			return false;

		return ((m_pObj == d->m_pObj) && (m_pFunct == d->m_pFunct) && m_Extra == d->m_Extra);
	}

	virtual void destroy()
	{
		delete this;
	}

	wxEvtHandler* GetEventHandler()
	{
		return m_pObj->GetEventHandler();
	}

	void registerDelegate(wxDelegate* d)
	{
		m_pObj->registerDelegate(d);
	}

	void deregisterDelegate(wxDelegate* d)
	{
		m_pObj->deregisterDelegate(d);
	}

	int GetId()
	{
		return m_pObj->GetId();
	}

	TFunct m_pFunct;   // pointer to member function
	TObj* m_pObj;     // pointer to object
	TExtra m_Extra;
};

template <class TObj, class TArg, class TExtra>
DelegateI<TArg>* guiExtraDelegate(TObj* pObj, void (TObj::*NotifyMethod)(TExtra, TArg&), TExtra tExtra, MODE mode = MODE_PENDING)
{
	if (!validateForm(pObj))
	{
		assert(false);
		return NULL;
	}

	return new GuiExtraDelegate<TObj, TArg, TExtra>(pObj, tExtra, NotifyMethod, mode);
}


template <typename TObj, typename TExtra>
class GuiExtraDelegateV : public GuiDelegateV<GuiExtraDelegateV<TObj, TExtra>>
{
public:
	typedef void (TObj::*TFunct)(TExtra); 

	GuiExtraDelegateV(TObj* t, TExtra e, TFunct f, MODE mode) : GuiDelegateV<GuiExtraDelegateV<TObj, TExtra>>::GuiDelegateV(mode)
	{
		m_Extra = e;
		m_pFunct = f;
		m_pObj = t;

		this->init(this, &GuiExtraDelegateV::callBack);
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
		return new GuiExtraDelegateV<TObj, TExtra>(m_pObj, m_Extra, m_pFunct, GuiDelegateV<GuiExtraDelegateV<TObj, TExtra>>::m_Mode);
	}

	virtual bool equals(DelegateVI* di)
	{
		GuiExtraDelegateV<TObj, TExtra> *d = dynamic_cast<GuiExtraDelegateV<TObj, TExtra>*>(di);

		if (!d)
			return false;

		return ((m_pObj == d->m_pObj) && (m_pFunct == d->m_pFunct) && m_Extra == d->m_Extra);
	}

	virtual void destroy()
	{
		delete this;
	}

	wxEvtHandler* GetEventHandler()
	{
		return m_pObj->GetEventHandler();
	}

	void registerDelegate(wxDelegate* d)
	{
		m_pObj->registerDelegate(d);
	}

	void deregisterDelegate(wxDelegate* d)
	{
		m_pObj->deregisterDelegate(d);
	}

	int GetId()
	{
		return m_pObj->GetId();
	}

	TFunct m_pFunct;   // pointer to member function
	TObj* m_pObj;     // pointer to object
	TExtra m_Extra;
};

template <class TObj, class TExtra>
DelegateVI* guiExtraDelegate(TObj* pObj, void (TObj::*NotifyMethod)(TExtra), TExtra tExtra, MODE mode = MODE_PENDING)
{
	if (!validateForm(pObj))
	{
		assert(false);
		return NULL;
	}

	return new GuiExtraDelegateV<TObj, TExtra>(pObj, tExtra, NotifyMethod, mode);
}

#endif //DESURA_GUIDELEGATE_H
