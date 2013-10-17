/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Ian T. Jacobsen <iantj92@gmail.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_UPDATETHREAD_H
#define DESURA_UPDATETHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/UserThreadI.h"
#include "util_thread/BaseThread.h"
#include "BaseUserThread.h"



class UpdateThreadI
{
public:
	virtual void setInfo(UserCore::UserI* user, WebCore::WebCoreI* webcore)=0;
	virtual void doRun()=0;
	virtual void onStop()=0;
	virtual bool onMessageReceived(const char* resource, tinyxml2::XMLNode* root)=0;
	virtual ~UpdateThreadI(){}

	Event<bool> isStoppedEvent;

protected:
	bool isStopped()
	{
		bool stopped = false;
		isStoppedEvent(stopped);
		return stopped;
	}
};



namespace UserCore
{
namespace Thread
{


//! Polls for Desura updates
//!
class UpdateThread : public BaseUserThread<UserThreadI, ::Thread::BaseThread>
{
public:
	UpdateThread(EventV *onForcePollEvent, bool loadLoginItems);
	~UpdateThread();

protected:
	virtual void doRun();
	virtual void onStop();

	virtual bool onMessageReceived(const char* resource, tinyxml2::XMLNode* root);

	void isThreadStopped(bool &stopped);

private:
	UpdateThreadI *m_pBaseTask;
};

}
}

#endif //DESURA_UPDATETHREAD_H
