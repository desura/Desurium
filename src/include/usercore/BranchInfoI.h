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

#ifndef DESURA_BRANCHINFOI_H
#define DESURA_BRANCHINFOI_H
#ifdef _WIN32
#pragma once
#endif

namespace UserCore
{
namespace Item
{

class BranchInfoI
{
public:
	enum BRANCH_FLAGS
	{
		BF_DISPLAY_NAME = 1<<1,
		BF_FREE			= 1<<2,
		BF_ONACCOUNT	= 1<<3,
		BF_REGIONLOCK	= 1<<4,
		BF_MEMBERLOCK	= 1<<5,
		BF_NORELEASES	= 1<<6,
		BF_DEMO			= 1<<7,
		BF_ACCEPTED_EULA= 1<<8,
		BF_PREORDER		= 1<<9,
		BF_TEST			= 1<<10,
		BF_CDKEY		= 1<<11,

		BF_WINDOWS_32	= 1<<12,
		BF_WINDOWS_64	= 1<<13,
		BF_LINUX_32		= 1<<14,
		BF_LINUX_64		= 1<<15,
		BF_MACOSX		= 1<<16,

		BF_STEAMGAME	= 1<<17,
	};

	//! Get branch flags
	//!
	//! @return Flags
	//!
	virtual uint32 getFlags()=0;

	//! Get branch id
	//!
	//! @return Id
	//!
	virtual MCFBranch getBranchId()=0;

	//! Gets the branch global id
	//!
	//! @return Global id
	//!
	virtual MCFBranch getGlobalId()=0;
	
	//! Gets the item id that this branch belongs to
	//!
	//! @return item id
	//!
	virtual DesuraId getItemId()=0;

	//! Gets the branch name
	//!
	//! @return Name
	//!
	virtual const char* getName()=0;

	//! Gets the cost as a formated string
	//!
	//! @return cost
	//!
	virtual const char* getCost()=0;

	//! Gets the eula url
	//!
	//! @return Eula url
	//!
	virtual const char* getEulaUrl()=0;

	//! Gets the date that the pre order expires
	//!
	//! @return Pre order exp date
	//!
	virtual const char* getPreOrderExpDate()=0;

	//! Get the path to the installer script file
	//!
	//! @return path or Null if none
	//!
	virtual const char* getInstallScriptPath()=0;

	//! Gets all the ids of tools for this branch
	//!
	//! @param toolLIst Out list
	//!
	virtual void getToolList(std::vector<DesuraId> &toolList)=0;

	//! Is this branch avaliable to the user
	//!
	//! @return True if avaliable, false if not
	//!
	virtual bool isAvaliable()=0;

	//! Is this branch avaliable to download/install
	//!
	//! @return True if avaliable, false if not
	//!
	virtual bool isDownloadable()=0;

	//! Is this branch is in a pre order state
	//!
	//! @return True if preorder, false if not
	//!
	virtual bool isPreOrder()=0;

	//! Has the end user accepted the eula for this branch
	//!
	//! @return True if accepted, false if not
	//!
	virtual bool hasAcceptedEula()=0;

	//! Does this branch have a cd key assoicated with it
	//!
	//! @return True if does, false if not
	//!
	virtual bool hasCDKey()=0;

	//! Is this items cd key valid (i.e. downloaded)
	//!
	//! @return True if valid, false if not
	//!
	virtual bool isCDKeyValid()=0;


	virtual bool isWindows()=0;
	virtual bool isLinux()=0;
	virtual bool isMacOsX()=0;
	
	virtual bool is32Bit()=0;
	virtual bool is64Bit()=0;

	virtual bool isSteamGame()=0;
};

}
}
#endif //DESURA_BRANCHINFOI_H
