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

#ifndef DESURA_ITEMTASKGROUP_H
#define DESURA_ITEMTASKGROUP_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "usercore/ItemHandleI.h"
#include "usercore/ItemHelpersI.h"
#include "BaseItemTask.h"



namespace UserCore
{
class ItemManager;

namespace Item
{

class ItemHandle;

class ItemTaskGroup : public UserCore::Item::ItemTaskGroupI, public UserCore::Item::Helper::ItemHandleHelperI
{
public:
	ItemTaskGroup(UserCore::ItemManager* manager, ACTION action, uint8 activeCount = 1);
	~ItemTaskGroup();

	bool addItem(UserCore::Item::ItemHandleI* item);
	bool addItem(UserCore::Item::ItemInfoI* item);

	bool removeItem(UserCore::Item::ItemHandleI* item);

	UserCore::Item::ItemHandleI* getActiveItem();

	void start();
	void pause();
	void unpause();
	void cancel();
	void finalize();

	virtual UserCore::Item::ItemTaskGroupI::ACTION getAction();
	virtual void getItemList(std::vector<UserCore::Item::ItemHandleI*> &list);
	virtual void cancelAll();

	void startAction(UserCore::Item::ItemHandle* item);
	UserCore::ItemTask::BaseItemTask* newTask(ItemHandle* handle);

	uint32 getPos(UserCore::Item::ItemHandle* item);
	uint32 getCount();

	template <typename F>
	void sort(F f)
	{
		if (m_bStarted)
			return;

		m_ListLock.lock();
		std::sort(m_vWaitingList.begin(), m_vWaitingList.end(), f);
		m_ListLock.unlock();
	}

protected:
	virtual void onComplete(uint32 status);
	virtual void onComplete(gcString& string);

	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);
	virtual void onProgressUpdate(uint32 progress);

	virtual void onError(gcException e);
	virtual void onNeedWildCard(WCSpecialInfo& info);

	virtual void onDownloadProvider(UserCore::Misc::GuiDownloadProvider &provider);
	virtual void onVerifyComplete(UserCore::Misc::VerifyComplete& info);

	virtual uint32 getId();
	virtual void setId(uint32 id);

	virtual void onPause(bool state);

	void finish();
	void nextItem();

	void updateEvents(UserCore::ItemTask::BaseItemTask* task);
	void registerItemTask(UserCore::ItemTask::BaseItemTask* task);
	void deregisterItemTask(UserCore::ItemTask::BaseItemTask* task);

	class GroupItemTask :  public UserCore::ItemTask::BaseItemTask
	{
	public:
		GroupItemTask(ItemHandle* handle, ItemTaskGroup* group) : BaseItemTask(UserCore::Item::ItemHandleI::STAGE_WAIT, "TaskGroup", handle)
		{
			m_pGroup = group;
			m_pGroup->registerItemTask(this);
		}

		~GroupItemTask()
		{
			m_pGroup->deregisterItemTask(this);
		}

		virtual void doRun()
		{
			m_pGroup->updateEvents(this);
			m_WaitCon.wait();
		}

		virtual void onStop()
		{
			m_WaitCon.notify();
		}

		virtual void cancel()
		{
			m_WaitCon.notify();
		}

		::Thread::WaitCondition m_WaitCon;
		ItemTaskGroup* m_pGroup;
	};


private:
	bool m_bStarted;
	bool m_bPaused;
	bool m_bFinal;

	uint8 m_iActiveCount;
	uint32 m_uiId;
	
	uint32 m_uiLastActive;
	uint32 m_uiActiveItem;

	ACTION m_Action;

	::Thread::Mutex m_ListLock;
	std::vector<UserCore::Item::ItemHandle*> m_vWaitingList;

	::Thread::Mutex m_TaskListLock;
	std::vector<UserCore::ItemTask::BaseItemTask*> m_vTaskList;

	UserCore::ItemManager* m_pItemManager;
};


}
}

#endif //DESURA_ITEMTASKGROUP_H
