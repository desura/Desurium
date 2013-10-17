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
#include "BDManager.h"
#include "User.h"

namespace UserCore
{

BDManager::BDManager(UserCore::User* user)
{
	m_pUser = user;
}

BDManager::~BDManager()
{
	m_BannerLock.lock();

	std::map<UserCore::Task::DownloadBannerTask*, UserCore::Misc::BannerNotifierI*>::iterator it;

	for (it=m_mDownloadBannerTask.begin(); it != m_mDownloadBannerTask.end(); it++)
	{
		it->first->onDLCompleteEvent -= delegate(this, &BDManager::onBannerComplete);
	}

	m_BannerLock.unlock();
}

void BDManager::onBannerComplete(UserCore::Task::BannerCompleteInfo& bci)
{
	m_BannerLock.lock();

	std::map<UserCore::Task::DownloadBannerTask*, UserCore::Misc::BannerNotifierI*>::iterator it;

	for (it=m_mDownloadBannerTask.begin(); it != m_mDownloadBannerTask.end(); it++)
	{
		if (it->first == bci.task)
		{
			if (bci.complete)
				it->second->onBannerComplete(bci.info);

			m_mDownloadBannerTask.erase(it);

			break;
		}
	}

	m_BannerLock.unlock();
}

void BDManager::downloadBanner(UserCore::Misc::BannerNotifierI* obj, MCFCore::Misc::DownloadProvider* provider)
{
	m_BannerLock.lock();

	UserCore::Task::DownloadBannerTask *task = new UserCore::Task::DownloadBannerTask(m_pUser, provider);
	task->onDLCompleteEvent += delegate(this, &BDManager::onBannerComplete);

	m_mDownloadBannerTask[task] = obj;
	m_pUser->getThreadPool()->queueTask(task);

	m_BannerLock.unlock();
}

void BDManager::cancelDownloadBannerHooks(UserCore::Misc::BannerNotifierI* obj)
{
	m_BannerLock.lock();

	std::map<UserCore::Task::DownloadBannerTask*, UserCore::Misc::BannerNotifierI*>::iterator it=m_mDownloadBannerTask.begin();

	while (it != m_mDownloadBannerTask.end())
	{
		if (it->second == obj)
		{
			std::map<UserCore::Task::DownloadBannerTask*, UserCore::Misc::BannerNotifierI*>::iterator temp = it;
			it++;

			m_mDownloadBannerTask.erase(temp);
		}
		else
		{
			it++;
		}
	}

	m_BannerLock.unlock();
}


}
