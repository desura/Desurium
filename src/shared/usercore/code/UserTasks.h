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

#ifndef DESURA_USERTASKS_H
#define DESURA_USERTASKS_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "util_thread/ThreadPool.h"
#include "UserTask.h"

#include "mcfcore/DownloadProvider.h"

namespace sqlite3x
{
	class sqlite3_connection;
}

namespace MCFCore
{
	namespace MISC
	{
		class DownloadProvider;
	}
}

namespace UserCore
{
namespace Task
{

class DeleteThread : public UserTask
{
public:
	DeleteThread(UserCore::User* user, ::Thread::BaseThread *thread);
	~DeleteThread();

	void doTask();
	const char* getName(){return "DeleteThread";}

private:
	::Thread::BaseThread *m_pThread;
};



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


class DownloadImgTask : public UserTask
{
public:
	enum IMAGE_IDENT
	{
		ICON = 0,
		LOGO,
	};

	//need iteminfo in this case as we might try this before the item is added to the list
	DownloadImgTask(UserCore::User* user, UserCore::Item::ItemInfo* itemInfo, uint8 image);
	void doTask();

	const char* getName(){return "DownloadImgTask";}

protected:
	void onComplete();

private:
	UserCore::Item::ItemInfo* m_pItem;
	uint8 m_Image;
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////




class ChangeAccountTask : public UserTask
{
public:
	enum ACTION
	{
		ACCOUNT_ADD = 0,
		ACCOUNT_REMOVE,
	};

	ChangeAccountTask(UserCore::User* user, DesuraId id, uint8 action);
	void doTask();

	const char* getName(){return "ChangeAccountTask";}

private:
	uint8 m_Action;
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class DownloadBannerTask;

class BannerCompleteInfo
{
public:
	bool complete;
	DownloadBannerTask* task;
	MCFCore::Misc::DownloadProvider info;
};


class DownloadBannerTask : public UserTask
{
public:
	DownloadBannerTask(UserCore::User* user, MCFCore::Misc::DownloadProvider *dp);

	void doTask();
	Event<BannerCompleteInfo> onDLCompleteEvent;

	const char* getName(){return "DownloadBannerTask";}

private:
	MCFCore::Misc::DownloadProvider m_DPInfo;
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


class DownloadAvatarTask : public UserTask
{
public:
	DownloadAvatarTask(UserCore::User* user, const char* url, uint32 userId);
	void doTask();

	Event<gcException> onErrorEvent;

	const char* getName(){return "DownloadAvatarTask";}

private:
	gcString m_szUrl;
	uint32 m_uiUserId;
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


class GatherInfoTask : public UserTask
{
public:
	GatherInfoTask(UserCore::User* user, DesuraId id, bool addToAccount = false);
	void doTask();

	const char* getName(){return "GatherInfoTask";}

private:
	bool m_bAddToAccount;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class CDKeyTask;

template <typename T>
class CDKeyEventInfo
{
public:
	T t;
	DesuraId id;
	CDKeyTask* task;
};


class CDKeyTask : public UserTask
{
public:
	CDKeyTask(UserCore::User* user, DesuraId id);
	void doTask();

	Event<CDKeyEventInfo<gcString> > onCompleteEvent;
	Event<CDKeyEventInfo<gcException> > onErrorEvent;

	const char* getName(){return "CDKeyTask";}
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class MigrateStandaloneTask : public UserTask
{
public:
	MigrateStandaloneTask(UserCore::User* user, const std::vector<UTIL::FS::Path> &fileList);
	void doTask();
	const char* getName(){return "MigrateStandaloneTask";}

private:
	std::vector<UTIL::FS::Path> m_vFileList;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

class RegenLaunchScriptsTask : public UserTask
{
public:
	RegenLaunchScriptsTask(UserCore::User* user);
	void doTask();
	const char* getName(){return "RegenLaunchScriptsTask";}
};


}
}

#endif //DESURA_USERTASKS_H
