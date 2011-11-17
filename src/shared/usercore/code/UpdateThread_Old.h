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

#ifndef DESURA_UPDATETHREAD_OLD_H
#define DESURA_UPDATETHREAD_OLD_H
#ifdef _WIN32
#pragma once
#endif

#include "UpdateThread.h"

namespace UserCore
{

class UpdateThreadOld : public UpdateThreadI
{
public:
	UpdateThreadOld(EventV *onForcePollEvent, bool loadLoginItems);

protected:
	virtual void doRun();
	virtual void onStop();

	void init();
	bool pollUpdates();
	void parseXML(TiXmlDocument &doc);
	

	void updateBuildVer();
	void onForcePoll();

	virtual bool onMessageReceived(const char* resource, TiXmlNode* root);
	virtual void setInfo(UserCore::UserI* user, WebCore::WebCoreI* webcore);

#ifdef WIN32
	void checkFreeSpace();
#endif

	void loadLoginItems();

private:
	HttpHandle m_hHttpHandle;

	uint32 m_iAppId;
	uint32 m_iAppVersion;

	uint32 m_uiLastAppId;
	uint32 m_uiLastVersion;

	::Thread::WaitCondition m_WaitCond;

	EventV *m_pOnForcePollEvent;
	volatile bool m_bForcePoll;

	UserCore::UserI* m_pUser;
	WebCore::WebCoreI* m_pWebCore;

	bool m_bLastFailed;
	bool m_bLoadLoginItems;
};

}

#endif //DESURA_UPDATETHREAD_OLD_H
