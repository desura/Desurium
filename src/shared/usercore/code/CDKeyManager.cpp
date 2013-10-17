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
#include "CDKeyManager.h"
#include "User.h"


namespace UserCore
{

CDKeyManager::CDKeyManager(UserCore::User* user)
{
	m_pUser = user;
}

CDKeyManager::~CDKeyManager()
{
	m_TaskListLock.lock();

	for (size_t x=0; x<m_vCDKeyTaskList.size(); x++)
	{
		m_vCDKeyTaskList[x]->onCompleteEvent -= delegate(this, &CDKeyManager::onCDKeyComplete);
		m_vCDKeyTaskList[x]->onErrorEvent -= delegate(this, &CDKeyManager::onCDKeyError);
	}

	m_TaskListLock.unlock();
}

void CDKeyManager::getCDKeyForCurrentBranch(DesuraId id, UserCore::Misc::CDKeyCallBackI* callback)
{
	if (!callback)
		return;

	UserCore::Item::ItemInfoI* info = NULL;
	UserCore::Item::BranchInfo* binfo = NULL;

	try
	{
		info = m_pUser->getItemManager()->findItemInfo(id);
	
		if (!info)
			throw gcException(ERR_CDKEY, 110, "Item can not be found");
	
		binfo = dynamic_cast<UserCore::Item::BranchInfo*>(info->getCurrentBranch());

		if (!binfo)
			throw gcException(ERR_CDKEY, 110, "Item has no currently installed branches");
	
		if (!hasCDKeyForCurrentBranch(id))
			throw gcException(ERR_CDKEY, 117, "No CDKey required for item");
	}
	catch (gcException &e)
	{
		callback->onCDKeyError(id, e);
		return;
	}

	gcString key(binfo->getCDKey());

	if (key.size() > 0)
	{
		callback->onCDKeyComplete(id, key);
		return;
	}

	m_MapLock.lock();

	UserCore::Task::CDKeyTask* task = new UserCore::Task::CDKeyTask(m_pUser, id);
	task->onCompleteEvent += delegate(this, &CDKeyManager::onCDKeyComplete);
	task->onErrorEvent += delegate(this, &CDKeyManager::onCDKeyError);

	m_mCDKeyCallbackList.push_back(std::pair<DesuraId, UserCore::Misc::CDKeyCallBackI*>(id, callback));
	m_pUser->getThreadPool()->forceTask(task);

	m_MapLock.unlock();
}

void CDKeyManager::cancelRequest(DesuraId id, UserCore::Misc::CDKeyCallBackI* callback)
{
	m_MapLock.lock();

	CDKeyList::iterator it=m_mCDKeyCallbackList.begin();

	while (it != m_mCDKeyCallbackList.end())
	{
		if (it->second == callback && it->first == id)
		{
			CDKeyList::iterator temp = it;
			++it;

			m_mCDKeyCallbackList.erase(temp);
		}
		else
		{
			it++;
		}
	}

	m_MapLock.unlock();
}

bool CDKeyManager::hasCDKeyForCurrentBranch(DesuraId id)
{
	UserCore::Item::ItemInfoI* info = m_pUser->getItemManager()->findItemInfo(id);
	
	if (!info)
		return false;

	UserCore::Item::BranchInfoI* binfo = info->getCurrentBranch();

	if (!binfo)
		return false;

	return binfo->hasCDKey();
}

void CDKeyManager::onCDKeyComplete(UserCore::Task::CDKeyEventInfo<gcString> &info)
{
	UserCore::Item::ItemInfoI* iinfo = NULL;
	UserCore::Item::BranchInfo* binfo = NULL;

	iinfo = m_pUser->getItemManager()->findItemInfo(info.id);
	
	if (iinfo)
	{
		binfo = dynamic_cast<UserCore::Item::BranchInfo*>(iinfo->getCurrentBranch());

		if (binfo)
			binfo->setCDKey(info.t);
	}
	

	m_MapLock.lock();

	std::vector<size_t> delList;

	for (size_t x=0; x<m_mCDKeyCallbackList.size(); x++)
	{
		if (m_mCDKeyCallbackList[x].first == info.id)
		{
			if (m_mCDKeyCallbackList[x].second)
				m_mCDKeyCallbackList[x].second->onCDKeyComplete(info.id, info.t);

			delList.push_back(x);
		}
	}

	std::for_each(delList.rbegin(), delList.rend(), [this](size_t x){
		m_mCDKeyCallbackList.erase(m_mCDKeyCallbackList.begin()+x);
	});

	m_MapLock.unlock();

	removeTask(info.task);
}

void CDKeyManager::onCDKeyError(UserCore::Task::CDKeyEventInfo<gcException> &info)
{
	m_MapLock.lock();

	std::vector<size_t> delList;

	for (size_t x=0; x<m_mCDKeyCallbackList.size(); x++)
	{
		if (m_mCDKeyCallbackList[x].first == info.id)
		{
			if (m_mCDKeyCallbackList[x].second)
				m_mCDKeyCallbackList[x].second->onCDKeyError(info.id, info.t);

			delList.push_back(x);
		}
	}

	std::for_each(delList.rbegin(), delList.rend(), [this](size_t x){
		m_mCDKeyCallbackList.erase(m_mCDKeyCallbackList.begin()+x);
	});

	m_MapLock.unlock();
	removeTask(info.task);
}

void CDKeyManager::removeTask(UserCore::Task::CDKeyTask* task)
{
	m_TaskListLock.lock();

	for (size_t x=0; x<m_vCDKeyTaskList.size(); x++)
	{
		if (m_vCDKeyTaskList[x] == task)
		{
			m_vCDKeyTaskList.erase(m_vCDKeyTaskList.begin()+x);
			break;
		}
	}

	m_TaskListLock.unlock();
}

}
