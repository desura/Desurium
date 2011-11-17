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

#ifndef DESURA_USERTASK_H
#define DESURA_USERTASK_H
#ifdef _WIN32
#pragma once
#endif

#include "webcore/WebCoreI.h"
#include "util_thread/ThreadPool.h"

namespace UserCore
{
	class User;

namespace Item
{
	class ItemInfo;
}

namespace Task
{

class UserTask : public ::Thread::BaseTask
{
public:
	//! Constuctor
	//!
	//! @param user Usercore handle
	//! @param itemId Item id
	//!
	UserTask(UserCore::User *user, DesuraId itemId = DesuraId());
	virtual ~UserTask();

	UserCore::User* getUserCore();
	WebCore::WebCoreI* getWebCore();

protected:
	DesuraId getItemId();
	UserCore::Item::ItemInfo* getItemInfo();

	virtual void onStop();
	volatile bool isStopped();

	volatile bool m_bStopped;
private:
	DesuraId m_iId;

	WebCore::WebCoreI* m_pWebCore;
	UserCore::User* m_pUserCore;
};


inline DesuraId UserTask::getItemId()
{
	return m_iId;
}

inline WebCore::WebCoreI* UserTask::getWebCore()
{
	return m_pWebCore;
}

inline UserCore::User* UserTask::getUserCore()
{
	return m_pUserCore;
}

}
}

#endif //DESURA_USERTASK_H
