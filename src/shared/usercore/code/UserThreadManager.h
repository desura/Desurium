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

#ifndef DESURA_USERTHREADMANAGER_H
#define DESURA_USERTHREADMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#define ENLIST_THREAD() GetThreadMang()->enlist(this);
#define DELIST_THREAD() GetThreadMang()->delist(this);

#include "usercore/UserThreadManagerI.h"
#include "usercore/MCFThreadI.h"
#include "usercore/UserThreadI.h"


namespace UserCore
{

class UserThreadManager : public UserThreadManagerI
{
public:
	UserThreadManager();
	~UserThreadManager();


	virtual void enlist(::Thread::BaseThread* pThread);
	virtual void delist(::Thread::BaseThread* pThread);

	//inherited functions
	void setUserCore(UserCore::UserI *uc);

	virtual void printThreadList();

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Application Threads
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	Thread::UserThreadI* newUpdateThread(EventV *onForcePollEvent, bool loadLoginItems);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Mcf Threads
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	Thread::MCFThreadI* newGetItemListThread();
	Thread::MCFThreadI* newInstalledWizardThread();
	Thread::MCFThreadI* newGatherInfoThread(DesuraId id, MCFBranch branch, MCFBuild build);

	Thread::MCFThreadI* newCreateMCFThread(DesuraId id, const char* path);
	Thread::MCFThreadI* newUploadPrepThread(DesuraId id, const char* file);
	Thread::MCFThreadI* newUploadResumeThread(DesuraId id, const char* key, WebCore::Misc::ResumeUploadInfo *info);

protected:
	void setUpThread(Thread::UserThreadI* thread);
	void setUpThread(Thread::MCFThreadI* thread);

private:
	std::vector< ::Thread::BaseThread*> m_vThreadList;
	UserCore::UserI* m_pUserCore;

	bool m_bDestructor;
};


inline void UserThreadManager::setUserCore(UserCore::UserI *uc)
{
	m_pUserCore = uc;
}


}


#endif //DESURA_USERTHREADMANAGER_H
