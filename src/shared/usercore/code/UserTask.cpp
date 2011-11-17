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
#include "UserTask.h"
#include "User.h"

namespace UserCore
{
namespace Task
{


UserTask::UserTask(UserCore::User *user, DesuraId id)
{
	m_pUserCore = user;

	if (user)
		m_pWebCore = user->getWebCore();
	else
		m_pWebCore = NULL;

	m_iId = id;
	m_bStopped = false;
}

UserTask::~UserTask()
{
}

UserCore::Item::ItemInfo* UserTask::getItemInfo()
{
	if (!m_pUserCore || !m_iId.isOk())
		return NULL;

	return dynamic_cast<UserCore::Item::ItemInfo*>(m_pUserCore->getItemManager()->findItemInfo(m_iId));
}

void UserTask::onStop()
{
	m_bStopped = true;
}

volatile bool UserTask::isStopped()
{
	return m_bStopped;
}

}
}