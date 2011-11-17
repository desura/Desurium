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
#include "ItemTaskGroup.h"
#include "ItemHandle.h"
#include "ItemHandleEvents.h"
#include "ItemManager.h"

namespace UserCore
{
namespace Item
{

ItemTaskGroup::ItemTaskGroup(UserCore::ItemManager* manager, ACTION action, uint8 activeCount) 
{
	m_Action = action;

	m_uiLastActive = -1;
	m_uiActiveItem = -1;

	m_iActiveCount = activeCount;

	m_bStarted = false;
	m_bPaused = false;
	m_bFinal = false;

	m_uiId = 0;
	m_pItemManager = manager;
}

ItemTaskGroup::~ItemTaskGroup()
{

}

UserCore::Item::ItemTaskGroupI::ACTION ItemTaskGroup::getAction()
{
	return m_Action;
}

void ItemTaskGroup::getItemList(std::vector<UserCore::Item::ItemHandleI*> &list)
{
	m_ListLock.lock();
	for (size_t x=0; x<m_vWaitingList.size(); x++)
	{
		list.push_back(m_vWaitingList[x]);
	}
	m_ListLock.unlock();
}

void ItemTaskGroup::cancelAll()
{
	m_ListLock.lock();

	for (size_t x=0; x<m_vWaitingList.size(); x++)
		m_vWaitingList[x]->setTaskGroup(NULL);

	m_vWaitingList.clear();
	m_ListLock.unlock();

	if (m_bFinal)
		delete this;
}

bool ItemTaskGroup::addItem(UserCore::Item::ItemInfoI* item)
{
	return addItem(m_pItemManager->findItemHandle(item->getId()));
}

bool ItemTaskGroup::addItem(UserCore::Item::ItemHandleI* item)
{
	if (!item)
		return false;

	UserCore::Item::ItemHandle* handle = dynamic_cast<UserCore::Item::ItemHandle*>(item);

	if (!handle)
		return false;

	if (!handle->setTaskGroup(this))
		return false;

	m_ListLock.lock();
	for (size_t x=0; x<m_vWaitingList.size(); x++)
	{
		if (m_vWaitingList[x] == handle)
			return true;
	}

	m_vWaitingList.push_back(handle);
	m_ListLock.unlock();

	if (m_bStarted && m_uiActiveItem == UINT_MAX)
		nextItem();

	uint32 p=m_vWaitingList.size();

	m_TaskListLock.lock();
	for (size_t x=0; x<m_vTaskList.size(); x++)
	{
		if (m_vTaskList[x]->getItemHandle() != getActiveItem())
			m_vTaskList[x]->onProgUpdateEvent(p);
	}
	m_TaskListLock.unlock();

	return true;
}

bool ItemTaskGroup::removeItem(UserCore::Item::ItemHandleI* item)
{
	if (!item)
		return false;

	UserCore::Item::ItemHandle* handle = dynamic_cast<UserCore::Item::ItemHandle*>(item);

	if (!handle)
		return false;

	bool found = false;

	m_ListLock.lock();
	for (size_t x=0; x<m_vWaitingList.size(); x++)
	{
		if (m_vWaitingList[x] == handle)
		{
			found = true;
			m_vWaitingList.erase(m_vWaitingList.begin()+x);
			break;
		}
	}
	m_ListLock.unlock();

	if (found)
		handle->setTaskGroup(NULL);

	uint32 p=m_vWaitingList.size();
	m_TaskListLock.lock();
	for (size_t x=0; x<m_vTaskList.size(); x++)
	{
		if (m_vTaskList[x]->getItemHandle() != getActiveItem())
			m_vTaskList[x]->onProgUpdateEvent(p);
	}
	m_TaskListLock.unlock();

	return true;
}

UserCore::Item::ItemHandleI* ItemTaskGroup::getActiveItem()
{
	if (m_uiActiveItem >= m_vWaitingList.size())
		return NULL;

	return m_vWaitingList[m_uiActiveItem];
}

void ItemTaskGroup::nextItem()
{
	onProgressUpdate(100);

	UserCore::Item::ItemHandleI* item = getActiveItem();

	if (item)
		item->delHelper(this);

	if (m_uiActiveItem+1 >=  m_vWaitingList.size())
	{
		m_uiLastActive = m_uiActiveItem;
		m_uiActiveItem = UINT_MAX;

		if (m_bFinal)
			finish();
	}
	else
	{
		if (m_uiLastActive != UINT_MAX)
		{
			m_uiActiveItem = m_uiLastActive;
			m_uiLastActive = UINT_MAX;
		}

		m_uiActiveItem++;

		item = getActiveItem();
		item->addHelper(this);
		startAction(dynamic_cast<UserCore::Item::ItemHandle*>(item));
	}
}

void ItemTaskGroup::startAction(UserCore::Item::ItemHandle* item)
{
	if (!item)
		return;

	switch (m_Action)
	{
	case A_VERIFY:
		item->verifyOveride();
		break;

	case A_UNINSTALL:
		item->uninstallOveride();
		break;
	};
}

void ItemTaskGroup::finalize()
{
	m_bFinal = true;

	if (!getActiveItem())
		finish();
}

void ItemTaskGroup::start()
{
	m_bStarted = true;

	if (!getActiveItem())
		nextItem();
}

void ItemTaskGroup::pause()
{
	m_bPaused = true;
}

void ItemTaskGroup::unpause()
{
	m_bPaused = false;
}

void ItemTaskGroup::cancel()
{
	m_bStarted = false;
}

void ItemTaskGroup::finish()
{
	cancelAll();
}

void ItemTaskGroup::onComplete(uint32 status)
{
	nextItem();
}

void ItemTaskGroup::onComplete(gcString& string)
{
	nextItem();
}

void ItemTaskGroup::onVerifyComplete(UserCore::Misc::VerifyComplete& info)
{
	nextItem();
}

void ItemTaskGroup::onMcfProgress(MCFCore::Misc::ProgressInfo& info)
{
	onProgressUpdate(info.percent);
}

void ItemTaskGroup::onProgressUpdate(uint32 progress)
{
	MCFCore::Misc::ProgressInfo i;

	i.percent = progress;

	if (getActiveItem())
		i.totalAmmount = getActiveItem()->getItemInfo()->getId().toInt64();

	m_TaskListLock.lock();
	for (size_t x=0; x<m_vTaskList.size(); x++)
	{
		if (m_vTaskList[x]->getItemHandle() != getActiveItem())
			m_vTaskList[x]->onMcfProgressEvent(i);
	}
	m_TaskListLock.unlock();
}

void ItemTaskGroup::onError(gcException e)
{
	nextItem();
}

void ItemTaskGroup::onNeedWildCard(WCSpecialInfo& info)
{
}

void ItemTaskGroup::onDownloadProvider(UserCore::Misc::GuiDownloadProvider &provider)
{
}

uint32 ItemTaskGroup::getId()
{
	return m_uiId;
}

void ItemTaskGroup::setId(uint32 id)
{
	m_uiId = id;
}

void ItemTaskGroup::onPause(bool state)
{
}

UserCore::ItemTask::BaseItemTask* ItemTaskGroup::newTask(ItemHandle* handle)
{
	return new GroupItemTask(handle, this);
}

void ItemTaskGroup::updateEvents(UserCore::ItemTask::BaseItemTask* task)
{
	m_TaskListLock.lock();

	uint32 curItem = m_uiActiveItem;

	if (curItem == UINT_MAX && m_uiLastActive != UINT_MAX)
		curItem = m_uiLastActive+1; //need to add one as it has to do the current item (x<curItem)

	if (curItem != UINT_MAX)
	{
		MCFCore::Misc::ProgressInfo i;
		i.percent = 100;

		for (size_t x=0; x<curItem; x++)
		{
			i.totalAmmount = m_vWaitingList[x]->getItemInfo()->getId().toInt64();
			task->onMcfProgressEvent(i);
		}
	}

	m_TaskListLock.unlock();
}

void ItemTaskGroup::registerItemTask(UserCore::ItemTask::BaseItemTask* task)
{
	m_TaskListLock.lock();
	m_vTaskList.push_back(task);
	m_TaskListLock.unlock();
}

void ItemTaskGroup::deregisterItemTask(UserCore::ItemTask::BaseItemTask* task)
{
	m_TaskListLock.lock();

	for (size_t x=0; x<m_vTaskList.size(); x++)
	{
		if (m_vTaskList[x] == task)
		{
			m_vTaskList.erase(m_vTaskList.begin()+x);
			break;
		}
	}

	m_TaskListLock.unlock();
}

uint32 ItemTaskGroup::getPos(UserCore::Item::ItemHandle* item)
{
	uint32 res = 0;

	m_ListLock.lock();

	for (size_t x=0; x<m_vWaitingList.size(); x++)
	{
		if (m_vWaitingList[x] == item)
		{
			res = x+1;
			break;
		}
	}

	m_ListLock.unlock();

	if (m_uiActiveItem != UINT_MAX)
		res -= (m_uiActiveItem+1);
	else if (m_uiLastActive != UINT_MAX)
		res -= m_uiLastActive;

	return res;
}

uint32 ItemTaskGroup::getCount()
{
	uint32 res = 0;

	m_ListLock.lock();
	res = m_vWaitingList.size();
	m_ListLock.unlock();

	if (m_uiActiveItem != UINT_MAX)
		res -= (m_uiActiveItem+1);
	else if (m_uiLastActive != UINT_MAX)
		res -= m_uiLastActive;

	return res;
}

}
}
