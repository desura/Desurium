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

#ifndef DESURA_BASEUSERTHREAD_H
#define DESURA_BASEUSERTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "webcore/WebCoreI.h"
#include "usercore/UserCoreI.h"
#include "usercore/UserThreadManagerI.h"

#include "ItemInfo.h"

namespace UserCore
{
namespace Thread
{

template<class Interface, class Base>
class BaseUserThread : public Interface, public Base
{
public:
	BaseUserThread(const char* name, DesuraId id) : Base(name)
	{
		m_iId = id;
		m_pWebCore = NULL;
		m_pUserCore = NULL;
		m_pThreadManager = NULL;
	}

	virtual ~BaseUserThread()
	{	
		stop();
		if (m_pThreadManager)
			m_pThreadManager->delist(this);
	}

	DesuraId getItemId()
	{
		return m_iId;
	}

	WebCore::WebCoreI* getWebCore()
	{
		return m_pWebCore;
	}

	UserCore::UserI* getUserCore()
	{
		return m_pUserCore;
	}

	void setThreadManager(UserCore::UserThreadManagerI* tm)
	{
		m_pThreadManager = tm;

		if (m_pThreadManager)
			m_pThreadManager->enlist(this);
	}

	void setWebCore(WebCore::WebCoreI *wc)
	{
		m_pWebCore = wc;
	}

	void setUserCore(UserCore::UserI *uc)
	{
		m_pUserCore = uc;
	}

	Event<uint32>* getCompleteEvent()
	{
		return &onCompleteEvent;
	}

	Event<uint32>* getProgressEvent()
	{
		return &onProgUpdateEvent;
	}

	Event<gcException>* getErrorEvent()
	{
		return &onErrorEvent;
	}

	Event<WCSpecialInfo>* getNeedWCEvent()
	{
		return &onNeedWCEvent;
	}

	Event<UserCore::Misc::GuiDownloadProvider>* getDownloadProviderEvent()
	{
		return &onNewProviderEvent;
	}

	void start()
	{
		Base::start();
	}

	void stop()
	{
		Base::stop();
	}

	void nonBlockStop()
	{
		Base::nonBlockStop();
	}

	void stopAndDelete(bool nonBlock = false)
	{
		Base::stopAndDelete(nonBlock);
	}

	void unpause()
	{
		Base::unpause();
	}

	void pause()
	{
		Base::pause();
	}


	UserCore::Item::ItemInfo* getItemInfo()
	{
		if (!m_pUserCore || !m_iId.isOk())
			return NULL;

		return dynamic_cast<UserCore::Item::ItemInfo*>(m_pUserCore->getItemManager()->findItemInfo(m_iId));
	}

	UserCore::Item::ItemInfo* getParentItemInfo()
	{
		UserCore::Item::ItemInfo* item = getItemInfo();

		if (!m_pUserCore || !item)
			return NULL;

		return dynamic_cast<UserCore::Item::ItemInfo*>(m_pUserCore->getItemManager()->findItemInfo(item->getParentId()));
	}

protected:
	virtual void run()
	{
		if (!m_pWebCore || !m_pUserCore)
		{
			gcException e(ERR_BADCLASS);
			onErrorEvent(e);
			return;
		}

		try
		{
			doRun();
		}
		catch (gcException& e)
		{
			onErrorEvent(e);
		}
	}

	Event<uint32> onCompleteEvent;
	Event<uint32> onProgUpdateEvent;
	Event<gcException> onErrorEvent;
	Event<WCSpecialInfo> onNeedWCEvent;

	//download mcf
	Event<UserCore::Misc::GuiDownloadProvider> onNewProviderEvent;

	virtual void doRun()=0;

private:
	DesuraId m_iId;

	WebCore::WebCoreI* m_pWebCore;
	UserCore::UserI* m_pUserCore;
	UserCore::UserThreadManagerI* m_pThreadManager;

	friend class CreateMCFThread; //so we can access usercore
};


}
}

#endif //DESURA_BASEUSERTHREAD_H
