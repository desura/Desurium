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

#ifndef DESURA_CDKEYMANAGER_H
#define DESURA_CDKEYMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "usercore/CDKeyManagerI.h"
#include "UserTasks.h"

typedef std::vector<std::pair<DesuraId, UserCore::Misc::CDKeyCallBackI*> > CDKeyList;
typedef std::vector<UserCore::Task::CDKeyTask*> CDKeyTaskList;

namespace UserCore
{
class User;

class CDKeyManager : public CDKeyManagerI
{
public:
	CDKeyManager(UserCore::User* user);
	~CDKeyManager();

	virtual void getCDKeyForCurrentBranch(DesuraId id, UserCore::Misc::CDKeyCallBackI* callback);
	virtual void cancelRequest(DesuraId id, UserCore::Misc::CDKeyCallBackI* callback);
	virtual bool hasCDKeyForCurrentBranch(DesuraId id);

protected:
	void onCDKeyComplete(UserCore::Task::CDKeyEventInfo<gcString> &info);
	void onCDKeyError(UserCore::Task::CDKeyEventInfo<gcException> &info);

	void removeTask(UserCore::Task::CDKeyTask* task);

private:
	::Thread::Mutex m_MapLock;
	CDKeyList m_mCDKeyCallbackList;

	::Thread::Mutex m_TaskListLock;
	CDKeyTaskList m_vCDKeyTaskList;

	UserCore::User* m_pUser;
};

}

#endif //DESURA_CDKEYMANAGER_H
