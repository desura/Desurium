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


#ifndef DESURA_ITEMHANDLERI_H
#define DESURA_ITEMHANDLERI_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/ItemHelpersI.h"

class LanguageManagerI;

namespace UserCore
{
namespace Item
{

class ItemHandleI;

class ItemTaskGroupI
{
public:
	enum ACTION
	{
		A_VERIFY,
		A_UNINSTALL,
	};

	virtual ACTION getAction()=0;
	virtual void getItemList(std::vector<UserCore::Item::ItemHandleI*> &list)=0;
	virtual void cancelAll()=0;
};

class ItemHandleI
{
public:
	virtual void setFactory(Helper::ItemHandleFactoryI* factory)=0;

	virtual void addHelper(Helper::ItemHandleHelperI* helper)=0;
	virtual void delHelper(Helper::ItemHandleHelperI* helper)=0;

	virtual bool cleanComplexMods()=0;
	virtual bool verify(bool files, bool tools, bool hooks)=0;
	virtual bool update()=0;
	virtual bool install(Helper::ItemLaunchHelperI* helper, MCFBranch branch)=0;
	virtual bool install(MCFBranch branch, MCFBuild build, bool test = false)=0;
	virtual bool installCheck()=0;
	virtual bool launch(Helper::ItemLaunchHelperI* helper, bool offline = false, bool ignoreUpdate = false)=0;
	virtual bool switchBranch(MCFBranch branch)=0;
	virtual bool startUpCheck()=0;
	virtual bool uninstall(Helper::ItemUninstallHelperI* helper, bool complete, bool account)=0;

	virtual void setPaused(bool paused = true)=0;
	virtual void setPauseOnError(bool pause = true)=0;
	virtual bool shouldPauseOnError()=0;
	virtual bool isStopped()=0;
	virtual bool isInStage()=0;

	//! Gets the current stage
	//!
	virtual uint32 getStage()=0;

	//! Cancels the current stage
	//! 
	virtual void cancelCurrentStage()=0;

	virtual UserCore::Item::ItemInfoI* getItemInfo()=0;

	virtual Event<uint32>* getChangeStageEvent()=0;
	virtual Event<gcException>* getErrorEvent()=0;

	//! Gets the item status string
	//!
	//! @param pLangMng Language manager for translation
	//! @param buffer Buffer to save status into
	//! @param buffsize Max size of buffer
	//!
	virtual void getStatusStr(LanguageManagerI *pLangMng, char* buffer, uint32 buffsize)=0;
	virtual ItemTaskGroupI* getTaskGroup()=0;

	//! If this item is in a task group it will force start it
	//!
	virtual void force()=0;

	virtual bool createDektopShortcut()=0;

#ifdef NIX
	//! Creates the launch scripts
	virtual void installLaunchScripts()=0;
#endif


	enum STAGES
	{
		STAGE_NONE,
		STAGE_CLOSE,
		STAGE_DOWNLOAD,
		STAGE_GATHERINFO,
		STAGE_INSTALL,
		STAGE_INSTALL_CHECK,
		STAGE_INSTALL_COMPLEX,
		STAGE_UNINSTALL,
		STAGE_UNINSTALL_BRANCH,
		STAGE_UNINSTALL_COMPLEX,
		STAGE_UNINSTALL_PATCH,
		STAGE_UNINSTALL_UPDATE,
		STAGE_VERIFY,
		STAGE_LAUNCH,
		STAGE_DOWNLOADTOOL,
		STAGE_INSTALLTOOL,
		STAGE_VALIDATE,
		STAGE_WAIT,
	};
};

}
}

#endif //DESURA_ITEMHANDLERI_H
