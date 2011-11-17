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

#ifndef DESURA_ITEMHANDLE_H
#define DESURA_ITEMHANDLE_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/ItemInfoI.h"
#include "usercore/ItemHandleI.h"
#include "ItemInfo.h"
#include "BaseItemTask.h"
#include "util_thread/BaseThread.h"

class BaseHandler;

namespace UserCore
{

class User;

namespace Thread
{
	class MCFThreadI;
	class UserServiceI;
}

namespace ItemTask
{
	enum GI_FLAGS
	{
		GI_FLAG_NONE = 0,
		GI_FLAG_EXISTING = 1<<1,
		GI_FLAG_UPDATE = 1<<2,
		GI_FLAG_TEST = 1<<3,
	};
}

namespace Item
{

class ItemThread;
class ItemHandleEvents;
class ItemTaskGroup;

class ItemHandle : public ItemHandleI
{
public:
	ItemHandle(ItemInfo* itemInfo, UserCore::User* user);
	~ItemHandle();

	virtual void setFactory(Helper::ItemHandleFactoryI* factory);

	virtual void addHelper(Helper::ItemHandleHelperI* helper);
	virtual void delHelper(Helper::ItemHandleHelperI* helper);

	virtual bool cleanComplexMods();
	virtual bool verify(bool files, bool tools, bool hooks);
	virtual bool update();
	virtual bool install(Helper::ItemLaunchHelperI* helper, MCFBranch branch);
	virtual bool install(MCFBranch branch, MCFBuild build, bool test = false);
	virtual bool installCheck();
	virtual bool launch(Helper::ItemLaunchHelperI* helper, bool offline = false, bool ignoreUpdate = false);
	virtual bool switchBranch(MCFBranch branch);
	virtual bool startUpCheck();
	virtual bool uninstall(Helper::ItemUninstallHelperI* helper, bool complete, bool account);
	virtual bool isInStage();

	virtual uint32 getStage();
	virtual void cancelCurrentStage();
	virtual UserCore::Item::ItemInfoI* getItemInfo();

	virtual void stop(bool block = true);
	virtual void setPauseOnError(bool pause = true);
	virtual bool shouldPauseOnError();
	virtual bool isStopped();

	virtual Event<uint32>* getChangeStageEvent();
	virtual Event<gcException>* getErrorEvent();

	virtual void getStatusStr(LanguageManagerI *pLangMng, char* buffer, uint32 buffsize);
	virtual ItemTaskGroupI* getTaskGroup();
	virtual void force();

	virtual bool createDektopShortcut();
	
#ifdef NIX
	virtual void installLaunchScripts();
#endif

	void setPausable(bool state = true);
	void setPaused(bool state, bool force);
	void setPaused(bool state);

	void completeStage(bool close = false);
	void resetStage(bool close = false);
	void goToStageGatherInfo(MCFBranch branch, MCFBuild build, UserCore::ItemTask::GI_FLAGS flags);
	void goToStageDownload(MCFBranch branch, MCFBuild build, bool test = false);
	void goToStageDownload(const char* path);
	void goToStageInstallComplex(MCFBranch branch, MCFBuild build, bool launch=false);
	void goToStageInstall(const char* path, MCFBranch branch);
	void goToStageVerify(MCFBranch branch, MCFBuild build, bool files, bool tools, bool hooks);
	void goToStageInstallCheck();
	void goToStageUninstall(bool complete, bool account);
	void goToStageUninstallBranch(MCFBranch branch, MCFBuild build, bool test = false);
	void goToStageUninstallComplexBranch(MCFBranch branch, MCFBuild build, bool complexLaunch = false);
	void goToStageUninstallPatch(MCFBranch branch, MCFBuild build);
	void goToStageUninstallUpdate(const char* path, MCFBuild lastBuild);
	void goToStageLaunch();

	void goToStageDownloadTools(uint32 ttid, const char* downloadPath, MCFBranch branch, MCFBuild build);
	void goToStageDownloadTools(bool launch);
	void goToStageInstallTools(bool launch);

	bool install(MCFBranch branch, MCFBuild build, UserCore::ItemTask::GI_FLAGS flags);

	uint64 getHash(){return getItemInfo()->getId().toInt64();}
	const char* getName(){return getItemInfo()->getName();}	

	UserCore::Item::ItemInfo* getItemInfoNorm();

	UserCore::User* getUserCore();

	bool getLock(void* obj);
	bool isLocked();
	bool hasLock(void* obj);
	void releaseLock(void* obj);

	void verifyOveride();
	void uninstallOveride();

	ItemHandleEvents* getEventHandler();
	bool setTaskGroup(ItemTaskGroup* group, bool force = false);

	//used to get around the is in stage check
	bool installPrivate(MCFBranch branch, MCFBuild build, UserCore::ItemTask::GI_FLAGS flags);

protected:
	Event<uint32> onChangeStageEvent;
	Event<gcException> onErrorEvent;

	void startGatherInfo();

	void setStage(uint32 stage);
	void registerTask(UserCore::ItemTask::BaseItemTask* task);

	void stopThread();

	void preLaunchCheck();
	
	void doLaunch(Helper::ItemLaunchHelperI* helper);
#ifdef NIX
	void doLaunch(bool useXdgOpen, const char* globalExe, const char* globalArgs);
#endif

	bool launchForReal(Helper::ItemLaunchHelperI* helper, bool offline = false);

	void onTaskStart(uint32 &stage);
	void onTaskComplete(uint32 &stage);

	bool getComplexLock();
	void releaseComplexLock();

	bool preDownloadCheck(MCFBranch branch, bool test);
	bool checkPaused();

private:
	bool m_bPauseOnError;
	bool m_bStopped;

	bool m_bLock;
	void* m_pLockObject;

	std::vector<Helper::ItemHandleHelperI*> m_vHelperList;

	uint32 m_uiHelperId;
	uint32 m_uiStage;

	::Thread::ReadWriteMutex m_ThreadMutex;
	UserCore::Item::ItemThread *m_pThread;
	UserCore::Item::ItemInfo* m_pItemInfo;
	UserCore::User* m_pUserCore;

	Helper::ItemHandleFactoryI* m_pFactory;
	ItemHandleEvents* m_pEventHandler;
	ItemTaskGroup* m_pGroup;
};


}
}










#endif //DESURA_ITEMHANDLE_H
