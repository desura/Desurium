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

#ifndef DESURA_BDMANAGER_H
#define DESURA_BDMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "mcfcore/DownloadProvider.h"
#include "util_thread/BaseThread.h"
#include "UserTasks.h"



namespace UserCore
{
class User;

namespace Misc
{
	class BannerNotifierI
	{
	public:
		virtual void onBannerComplete(MCFCore::Misc::DownloadProvider &info)=0;
	};
}

class BDManager
{
public:
	BDManager(UserCore::User* user);
	~BDManager();

	void downloadBanner(UserCore::Misc::BannerNotifierI* obj, MCFCore::Misc::DownloadProvider* provider);
	void cancelDownloadBannerHooks(UserCore::Misc::BannerNotifierI* obj);

protected:
	void onBannerComplete(UserCore::Task::BannerCompleteInfo& bci);

private:
	::Thread::Mutex m_BannerLock;
	std::map<UserCore::Task::DownloadBannerTask*, UserCore::Misc::BannerNotifierI*> m_mDownloadBannerTask;

	UserCore::User* m_pUser;
};

}

#endif //DESURA_BDMANAGER_H
