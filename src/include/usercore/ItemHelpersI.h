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


#ifndef DESURA_ITEMHELPERSI_H
#define DESURA_ITEMHELPERSI_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/GuiDownloadProvider.h"
#include "usercore/ItemInfoI.h"
#include "mcfcore/ProgressInfo.h"
#include "managers/WildcardManager.h"
#include "usercore/VerifyComplete.h"

namespace UserCore
{
namespace Item
{
namespace Helper
{
	enum TOOL
	{
		JAVA_SUN =1,
		JAVA = 2,
		MONO = 3,
		AIR = 4,
	};	
	
	enum VALIDATION
	{
		V_OK = 0,
		V_FREESPACE = 1<<0,
		V_PARENT	= 1<<1,
		V_BADINFO	= 1<<2,
		V_NONEMPTY	= 1<<3,
		V_BADPATH	= 1<<4,
		V_FREESPACE_INS = 1<<5,
		V_FREESPACE_DL = 1<<6,
		
		V_JAVA_SUN 	= JAVA_SUN<<28,
		V_JAVA 		= JAVA<<28,
		V_MONO 		= MONO<<28,
		V_AIR		= AIR <<28,
	};

	enum ACTION
	{
		C_NONE,
		C_REMOVE,
		C_INSTALL,
		C_VERIFY,
	};
	
	class GatherInfoHandlerHelperI
	{
	public:
		virtual void gatherInfoComplete()=0;

		//! Return false to cancel install
		//!
		virtual bool selectBranch(MCFBranch& branch)=0;

		//! Show complex warning
		//!
		virtual bool showComplexPrompt()=0;

		//! Return false to continue with install
		//!
		virtual bool showError(uint8 flags)=0;

		//! Cant install due to existing files. 
		//!
		virtual ACTION showInstallPrompt(const char* path)=0;
	
		//! Destroys this class
		//!
		virtual void destroy()=0;

		//! Branch belongs to another platform. Return true to select a new branch
		//!
		virtual bool showPlatformError()=0;
		
#ifdef NIX
		//! If we are missing a tool show this to ask the user to install it or skip
		//!
		//! @return true to ignore
		//!
		virtual bool showToolPrompt(TOOL tool)=0;
#endif		

	};

	class InstallerHandleHelperI
	{
	public:
		virtual bool verifyAfterHashFail()=0;
		virtual void destroy()=0;
	};

	class ItemHandleFactoryI
	{
	public:
		virtual void getGatherInfoHelper(Helper::GatherInfoHandlerHelperI** helper)=0;
		virtual void getInstallHelper(Helper::InstallerHandleHelperI** helper)=0;
	};

	class ItemHandleHelperI
	{
	public:
		virtual void onComplete(uint32 status)=0;
		virtual void onComplete(gcString& string)=0;

		virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info)=0;
		virtual void onProgressUpdate(uint32 progress)=0;

		virtual void onError(gcException e)=0;
		virtual void onNeedWildCard(WCSpecialInfo& info)=0;

		virtual void onDownloadProvider(UserCore::Misc::GuiDownloadProvider &provider)=0;
		virtual void onVerifyComplete(UserCore::Misc::VerifyComplete& info)=0;

		virtual uint32 getId()=0;
		virtual void setId(uint32 id)=0;

		virtual void onPause(bool state)=0;
	};


	class ItemLaunchHelperI
	{
	public:
		virtual void showUpdatePrompt()=0;
		virtual void showLaunchPrompt()=0;
		virtual void showEULAPrompt()=0;
		virtual void showPreOrderPrompt()=0;
		virtual void launchError(gcException& e)=0;
#ifdef NIX
		virtual void showWinLaunchDialog()=0;
#endif
	};

	class ItemUninstallHelperI
	{
	public:
		//! Should we stop other stages to uninstall this item?
		//!
		virtual bool stopStagePrompt()=0;
	};


}
}
}

#endif //DESURA_ITEMHELPERSI_H
