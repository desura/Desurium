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
#include "ItemHandleEvents.h"



namespace UserCore
{
namespace Item
{

typedef void (ItemHandleEvents::*onCompleteIntFn)(uint32&);
typedef void (ItemHandleEvents::*onCompleteStrFn)(gcString&);

class EventItemI
{
public:
	virtual void post(Helper::ItemHandleHelperI* helper)=0;
	virtual ~EventItemI(){}
};

class VoidEventItem : public EventItemI
{
public:
	typedef void (Helper::ItemHandleHelperI::*EventVoidFn)();

	VoidEventItem(EventVoidFn fn)
	{
		m_EVFn = fn;
	}

	virtual void post(Helper::ItemHandleHelperI* helper)
	{
		if (m_EVFn)
			(*helper.*m_EVFn)();
	}

	EventVoidFn m_EVFn;
};

template <typename T>
class GenericEventItem : public EventItemI
{
public:
	typedef void (Helper::ItemHandleHelperI::*EventRefFn)(T&);
	typedef void (Helper::ItemHandleHelperI::*EventFn)(T);

	GenericEventItem(EventRefFn fn, T& t)
	{
		m_bIsRef = true;

		m_ERFn = fn;
		m_t = t;
	}

	GenericEventItem(EventFn fn, T t)
	{
		m_bIsRef = false;

		m_EFn = fn;
		m_t = t;
	}

	virtual void post(Helper::ItemHandleHelperI* helper)
	{
		if (m_bIsRef)
			(*helper.*m_ERFn)(m_t);
		else
			(*helper.*m_EFn)(m_t);
	}

	bool m_bIsRef;

	EventRefFn m_ERFn;
	EventFn m_EFn;

	T m_t;
};

void CallEvent(std::vector<Helper::ItemHandleHelperI*> &vList, VoidEventItem::EventVoidFn Fn)
{
	for (size_t x=0; x<vList.size(); x++)
	{
		if (vList[x])
			(*vList[x].*Fn)();
	}
}

template <typename T>
void CallEvent(std::vector<Helper::ItemHandleHelperI*> &vList, void (Helper::ItemHandleHelperI::*Fn)(T&), T& t)
{
	for (size_t x=0; x<vList.size(); x++)
	{
		if (vList[x])
			(*vList[x].*Fn)(t);
	}
}

template <typename T>
void CallEvent(std::vector<Helper::ItemHandleHelperI*> &vList, void (Helper::ItemHandleHelperI::*Fn)(T), T t)
{
	for (size_t x=0; x<vList.size(); x++)
	{
		if (vList[x])
			(*vList[x].*Fn)(t);
	}
}


void CallEvent(std::vector<EventItemI*> &eventHistory, std::vector<Helper::ItemHandleHelperI*> &vList, VoidEventItem::EventVoidFn Fn)
{
	eventHistory.push_back(new VoidEventItem(Fn));
	CallEvent(vList, Fn);
}

template <typename T>
void CallEvent(std::vector<EventItemI*> &eventHistory, std::vector<Helper::ItemHandleHelperI*> &vList, void (Helper::ItemHandleHelperI::*Fn)(T&), T& t)
{
	eventHistory.push_back(new GenericEventItem<T>(Fn, t));
	CallEvent(vList, Fn, t);
}

template <typename T>
void CallEvent(std::vector<EventItemI*> &eventHistory, std::vector<Helper::ItemHandleHelperI*> &vList, void (Helper::ItemHandleHelperI::*Fn)(T), T t)
{
	eventHistory.push_back(new GenericEventItem<T>(Fn, t));
	CallEvent(vList, Fn, t);
}

ItemHandleEvents::ItemHandleEvents(std::vector<Helper::ItemHandleHelperI*> &vHelperList) : m_vHelperList(vHelperList)
{
}

ItemHandleEvents::~ItemHandleEvents()
{
	reset();
}

void ItemHandleEvents::reset()
{
	m_LastProg = MCFCore::Misc::ProgressInfo();
	m_LastProg.percent = -1;
	m_LastProg.flag = -1;

	safe_delete(m_EventHistory);
}

void ItemHandleEvents::postAll(Helper::ItemHandleHelperI* helper)
{
	for (size_t x=0; x<m_EventHistory.size(); x++)
	{
		if (m_EventHistory[x])
			m_EventHistory[x]->post(helper);
	}
}

void ItemHandleEvents::registerTask(UserCore::ItemTask::BaseItemTask* task)
{
	if (!task)
		return;

	task->onCompleteEvent		+= delegate(this, (onCompleteIntFn)&ItemHandleEvents::onComplete);
	task->onProgUpdateEvent		+= delegate(this, &ItemHandleEvents::onProgressUpdate);
	task->onErrorEvent			+= delegate(this, &ItemHandleEvents::onError);
	task->onNeedWCEvent			+= delegate(this, &ItemHandleEvents::onNeedWildCard);

	task->onMcfProgressEvent	+= delegate(this, &ItemHandleEvents::onMcfProgress);
	task->onNewProviderEvent	+= delegate(this, &ItemHandleEvents::onDownloadProvider);

	task->onCompleteStrEvent	+= delegate(this, (onCompleteStrFn)&ItemHandleEvents::onComplete);
	task->onVerifyCompleteEvent += delegate(this, &ItemHandleEvents::onVerifyComplete);
}

void ItemHandleEvents::deregisterTask(UserCore::ItemTask::BaseItemTask* task)
{
	if (!task)
		return;

	task->onCompleteEvent		-= delegate(this, (onCompleteIntFn)&ItemHandleEvents::onComplete);
	task->onProgUpdateEvent		-= delegate(this, &ItemHandleEvents::onProgressUpdate);
	task->onErrorEvent			-= delegate(this, &ItemHandleEvents::onError);
	task->onNeedWCEvent			-= delegate(this, &ItemHandleEvents::onNeedWildCard);

	task->onMcfProgressEvent	-= delegate(this, &ItemHandleEvents::onMcfProgress);
	task->onNewProviderEvent	-= delegate(this, &ItemHandleEvents::onDownloadProvider);

	task->onCompleteStrEvent	-= delegate(this, (onCompleteStrFn)&ItemHandleEvents::onComplete);
	task->onVerifyCompleteEvent -= delegate(this, &ItemHandleEvents::onVerifyComplete);
}

void ItemHandleEvents::onComplete(uint32& status)
{
	CallEvent<uint32>(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onComplete, status);
}

void ItemHandleEvents::onProgressUpdate(uint32& progress)
{
	CallEvent(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onProgressUpdate, progress);
}

void ItemHandleEvents::onError(gcException& e)
{
	CallEvent(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onError, e);
}

void ItemHandleEvents::onNeedWildCard(WCSpecialInfo& info)
{
	CallEvent(m_vHelperList, &Helper::ItemHandleHelperI::onNeedWildCard, info);
}

void ItemHandleEvents::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	if (info.percent != m_LastProg.percent || info.flag != m_LastProg.flag)
	{
		m_LastProg = info;
		CallEvent(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onMcfProgress, info);
	}
	else
	{
		CallEvent(m_vHelperList, &Helper::ItemHandleHelperI::onMcfProgress, info);
	}
}

void ItemHandleEvents::onComplete(gcString& str)
{
	CallEvent(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onComplete, str);
}

void ItemHandleEvents::onDownloadProvider(UserCore::Misc::GuiDownloadProvider& provider)
{
	CallEvent(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onDownloadProvider, provider);
}

void ItemHandleEvents::onVerifyComplete(UserCore::Misc::VerifyComplete& info)
{
	CallEvent(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onVerifyComplete, info);
}

void ItemHandleEvents::onPause(bool state)
{
	CallEvent(m_EventHistory, m_vHelperList, &Helper::ItemHandleHelperI::onPause, state);
}


}
}
