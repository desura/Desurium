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

#ifndef DESURA_CIPMANAGER_H
#define DESURA_CIPMANAGER_H
#ifdef _WIN32
#pragma once
#endif


#include "usercore/CIPManagerI.h"

namespace UserCore
{

class User;

namespace Thread
{
	class MCFThreadI;
}

class CIPManager : public UserCore::CIPManagerI
{
public:
	CIPManager(UserCore::User* user);
	~CIPManager();

	virtual void getCIPList(std::vector<UserCore::Misc::CIPItem> &list);
	virtual void getItemList(std::vector<UserCore::Misc::CIPItem> &list);

	virtual void updateItem(DesuraId id, gcString path);
	virtual void deleteItem(DesuraId id);

	virtual void refreshList();
	virtual bool getCIP(UserCore::Misc::CIPItem& info);

	virtual EventV& getItemsUpdatedEvent();

protected:
	void onRefreshComplete(uint32&);
	void onRefreshError(gcException& e);

	EventV onItemsUpdatedEvent;

private:
	gcString m_szDBName;
	User* m_pUser;

	bool m_bRefreshComplete;
	UserCore::Thread::MCFThreadI* m_pThread;
};

}

#endif //DESURA_CIPMANAGER_H
