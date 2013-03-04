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


#ifndef DESURA_ITEMINFOI_H
#define DESURA_ITEMINFOI_H
#ifdef _WIN32
#pragma once
#endif

class LanguageManagerI;

#include "usercore/BranchInfoI.h"

namespace UserCore
{
namespace Item
{

namespace Misc
{
	class ExeInfoI
	{
	public:
		virtual const char* getName()=0;

		virtual const char* getExe()=0;
		virtual const char* getExeArgs()=0;
		virtual const char* getUserArgs()=0;

		virtual void setUserArgs(const char* args)=0;
	};
}

//! Desura item interface
//!
class ItemInfoI
{
public:
	virtual ~ItemInfoI(){;}

	enum ITEM_STATUS
	{
		STATUS_UNKNOWN		= 0,		//!< unknown status
		STATUS_INSTALLED	= 1<<1,		//!< item is installed on the computer
		STATUS_ONACCOUNT	= 1<<2,		//!< item is on the users account
		STATUS_ONCOMPUTER	= 1<<3,		//!< item is local to the user
		STATUS_READY		= 1<<4,		//!< item is ready to go
		STATUS_UPDATING		= 1<<5,		//!< item is updating atm
		STATUS_DOWNLOADING	= 1<<6,		//!< item is downloading atm
		STATUS_INSTALLING	= 1<<7,		//!< item is installing atm
		STATUS_UPLOADING	= 1<<8,		//!< item is uploading atm
		STATUS_VERIFING		= 1<<9,		//!< item is verifing atm
		STATUS_DEVELOPER	= 1<<10,	//!< user is a dev for this item
		STATUS_STUB			= 1<<11,	//!< means it has info but no branches on this platform
		//STATUS_FULL			= 1<<12,	//!< means it has full info
		STATUS_UPDATEAVAL	= 1<<13,	//!< means this item has an update for it
		STATUS_DELETED		= 1<<14,	//!< means the item is deleted (safe delete)
		STATUS_NONDOWNLOADABLE = 1<<15,	//!< means this item cant be downloaded
		STATUS_NEEDCLEANUP	= 1<<16,	//!< means the item needs a clean up for its MCF files
		STATUS_PAUSED		= 1<<17,	//!< means the item is paused in its current process
		STATUS_UNAUTHED		= 1<<18,	//!< means the current install is not authed yet
		STATUS_PAUSABLE		= 1<<19,	//!< item can be paused at the current stage
		STATUS_INSTALLCOMPLEX = 1<<20,	//!< item has complex install
		STATUS_LINK			= 1<<21,	//!< item is a link and is not managed by Desura
		//STATUS_ACCEPTED_EULA= 1<<22,	//!< User agreed to eula
		STATUS_GLOBAL		= 1<<23,	//!< Global item (for tools)
		STATUS_LAUNCHED		= 1<<24,	//!< Has this item been launched
		STATUS_DLC			= 1<<25,	//!< Game has downloadable content
		STATUS_PRELOADED	= 1<<26,	//!< Item has been preordered and downloaded
	};

	enum ITEM_PERMISSION
	{
		PERMISSION_NONE		 = 0,		//!< No permissions
		PERMISSION_DEVELOPER = 1<<0,	//!< Developer permissions
		PERMISSION_BETADL	 = 1<<1,	//!< Beta download
		PERMISSION_ADMIN	 = 1<<2,	//!< Administrator
	};

	enum ITEM_OPTION
	{
		OPTION_NOUPDATE = 0,			//!< Dont update
		OPTION_PROMPTUPDATE = 1<<1,		//!< Propmt about update
		OPTION_AUTOUPDATE = 1<<2,		//!< Auto update
		OPTION_NOTREMINDUPDATE = 1<<3,	//!< Dont remind about updates
		OPTION_REMOVEFILES = 1<<4,		//!< Remove installed files
		OPTION_DONTPROMPTGAMEDISK = 1<<5,
	};

	enum ITEM_CHANGES
	{
		CHANGED_NOCHANGE = 0,			//!< No changes
		CHANGED_ICON	= 1<<1,			//!< Icon changed
		CHANGED_LOGO	= 1<<2,			//!< Logo changed
		CHANGED_STATUS	= 1<<3,			//!< Status changed
		CHANGED_PERCENT	= 1<<4,			//!< Percent changed
		CHANGED_INFO	= 1<<5,			//!< Information changed
	};

	//! Item update information stuct
	typedef struct
	{
		DesuraId id;			//!< Item internid
		uint32 changeFlags;	//!< Changed flags
	} ItemInfo_s;


	//! Changed status to updated
	//!
	virtual void updated()=0;


	//! Add item to User account
	//!
	virtual void addToAccount()=0;

	//! Remove item from user accoutn
	//!
	virtual void removeFromAccount()=0;


	//! Get parent internal id
	//!
	//! @return Parent id
	//!
	virtual DesuraId getParentId()=0;

	//! Get item internal id
	//!
	//! @return Item id
	//!
	virtual DesuraId getId()=0;

	//! Get the currently installed mod (for complex installs)
	//!
	//! @param branch Branch to get installed modid for or 0 for current
	//! @return Mod id
	//!
	virtual DesuraId getInstalledModId(MCFBranch branch = MCFBranch())=0;

	//! Get items changed flags
	//!
	//! @return Flags
	//!
	virtual uint32 getChangedFlags()=0;

	//! Get item status flags
	//!
	//! @return status
	//!
	virtual uint32 getStatus()=0;

	//! Get items current percent
	//!
	//! @return Percent
	//!
	virtual uint8 getPercent()=0;

	//! Get items current permssion set
	//!
	//! @return Permission flags
	//!
	virtual uint8 getPermissions()=0;

	//! Gets items current options
	//!
	//! @return Options flag
	//!
	virtual uint8 getOptions()=0;


	//! Can this item launch
	//!
	//! @return True if can, false if not
	//!
	virtual bool isLaunchable()=0;

	//! Is this item updating?
	//!
	//! @return True if is, false if not
	//!
	virtual bool isUpdating()=0;

	//! Is this item installed?
	//!
	//! @return True if is, false if not
	//!
	virtual bool isInstalled()=0;

	//! Is this item downloading?
	//!
	//! @return True if is, false if not
	//!
	virtual bool isDownloadable()=0;

	//! Is this a favorite for this user?
	//!
	//! @return True if favorite, false if not
	//!
	virtual bool isFavorite()=0;

	//! Is this item a complex install
	//!
	//! @return True if complex, false if not
	//!
	virtual bool isComplex()=0;

	//! Is this item a parent to a complex mod
	//!
	//! @return True if parent to a complex mod, false if not
	//!
	virtual bool isParentToComplex()=0;

	//! User has agreed to the EULA for this item
	//! 
	//! @return Agreed to EULA
	//!
	virtual bool hasAcceptedEula()=0;

	//! Compare this item to the filter
	//!
	//! @return True if matches, false if not
	//!
	virtual bool compare(const char* filter)=0;

	//! Has this item been launched before
	//!
	//! @return true if havnt been launched before, false if not
	//!
	virtual bool isFirstLaunch()=0;

	//! Add status flag
	//!
	//! @param status Status flag
	//!
	virtual void addSFlag(uint32 status)=0;

	//! Add permission flag
	//!
	//! @param permission Permission flag
	//!
	virtual void addPFlag(uint8 permission)=0;

	//! Add option flag
	//!
	//! @param option Option flag
	//!
	virtual void addOFlag(uint8 option)=0;

	//! Remove status flag
	//!
	//! @param status Status flag
	//!
	virtual void delSFlag(uint32 status)=0;

	//! Remove permissions flag
	//!
	//! @param permission Permission flag
	//!
	virtual void delPFlag(uint8 permission)=0;

	//! Remove option flag
	//!
	//! @param option Option flag
	//!
	virtual void delOFlag(uint8 option)=0;
	

	//! Get the item rating
	//!
	//! @return Rating
	//!
	virtual const char* getRating()=0;

	//! Gets the developer name
	//!
	//! @return Dev name
	//!
	virtual const char* getDev()=0;

	//! Gets the items name
	//!
	//! @return Name
	//!
	virtual const char* getName()=0;

	//! Gets the items short name
	//!
	//! @return Short name
	//!
	virtual const char* getShortName()=0;

	//! Gets the items install path
	//!
	//! @branch Branch to get path for or 0 for current
	//! @return Install path
	//!
	virtual const char* getPath(MCFBranch branch = MCFBranch())=0;

	//! Gets the items primary install path
	//!
	//! @branch Branch to get primary install path for or 0 for current
	//! @return Primary install path
	//!
	virtual const char* getInsPrimary(MCFBranch branch = MCFBranch())=0;

	//! Gets the items icon (local file path)
	//!
	//! @return Icon path
	//!
	virtual const char* getIcon()=0;

	//! Gets the items logo (local file path)
	//!
	//! @return Logo path
	//!
	virtual const char* getLogo()=0;

	//! Gets the items icon url
	//! 
	//! @return Item url
	//!
	virtual const char* getIconUrl()=0;

	//! Get the items logo url
	//!
	//! @return Logo url
	//!
	virtual const char* getLogoUrl()=0;

	//! Gets the items description
	//!
	//! @return Description
	//!
	virtual const char* getDesc()=0;

	//! Gets the items theme
	//!
	//! @return Theme
	//!
	virtual const char* getTheme()=0;

	//! Gets the items genre
	//!
	//! @return Genre
	//!
	virtual const char* getGenre()=0;

	//! Gets the item profile url
	//!
	//! @return Profile url
	//!
	virtual const char* getProfile()=0;

	//! Gets the items Developer profile url
	//!
	//! @return Dev url
	//!
	virtual const char* getDevProfile()=0;

	//! Gets the publisher name
	//!
	//! @return Publisher
	//!
	virtual const char* getPublisher()=0;

	//! Gets the items Publisher profile url
	//!
	//! @return Publisher url
	//!
	virtual const char* getPublisherProfile()=0;

	//! Gets the end user license agreement url
	//!
	//! @return Url or NULL if none
	//!
	virtual const char* getEulaUrl()=0;

	//! Get the path to the installer script file
	//!
	//! @return path or Null if none
	//!
	virtual const char* getInstallScriptPath()=0;

	//! Gets the info change event handler. Triggered when item information changes.
	//!
	//! @return Info change event
	//!
	virtual Event<ItemInfo_s>* getInfoChangeEvent()=0;
	
	//! Get the total number of branches for this item
	//!
	//! @return Number of branches
	//!
	virtual uint32 getBranchCount()=0;

	//! Get a branch at an index
	//!
	//! @param index Branch index
	//! @return Branch info or NULL if bad index
	//!
	virtual BranchInfoI* getBranch(uint32 index)=0;

	//! Get the current active branch
	//!
	//! @return Current branch
	//!
	virtual BranchInfoI* getCurrentBranch()=0;

	//! Gets a branch based on Desura branch id
	//!
	//! @param id Branch id
	//! @return Branch info or Null if invalid id
	//!
	virtual BranchInfoI* getBranchById(uint32 id)=0;

	//! Gets the install size
	//!
	//! @param branch to get or 0 for current
	//! @return Install size
	//!
	virtual uint64 getInstallSize(MCFBranch branch = MCFBranch())=0;

	//! Gets the download size
	//!
	//! @param branch to get or 0 for current
	//! @return Download size
	//!
	virtual uint64 getDownloadSize(MCFBranch branch = MCFBranch())=0;

	//! Gets the last installed build version
	//!
	//! @param branch to get or 0 for current
	//! @return Last build version
	//!
	virtual MCFBuild getLastInstalledBuild(MCFBranch branch = MCFBranch())=0;

	//! Gets the current build version
	//!
	//! @param branch to get or 0 for current
	//! @return Version string
	//!
	virtual MCFBuild getInstalledBuild(MCFBranch branch = MCFBranch())=0;

	//! Gets the next build number of the next version
	//!
	//! @param branch to get or 0 for current
	//! @return Next mcf build
	//!
	virtual MCFBuild getNextUpdateBuild(MCFBranch branch = MCFBranch())=0;

	//! Gets the currently installed branch id
	//!
	//! @return Current installed branch id
	//!
	virtual MCFBranch getInstalledBranch()=0;

	//! Gets the last installed branch id
	//!
	//! @return Last installed branch id
	//!
	virtual MCFBranch getLastInstalledBranch()=0;

	//! Gets the installed version string
	//!
	//! @param branch to get the installed version for or 0 for current
	//! @return Version string
	//!
	virtual const char* getInstalledVersion(MCFBranch branch = MCFBranch())=0;


	//! Sets the current installed branch and build. 
	//!
	//! @param branch Mcf branch
	//! @param build Mcf build If build is zero the latest build will be selected.
	//! @return true if successful 
	//!
	virtual bool setInstalledMcf(MCFBranch branch, MCFBuild build = MCFBuild())=0;

	//! Set this item as a favorite for the user
	//!
	//! @param fav Is this item a favorite
	//!
	virtual void setFavorite(bool fav = true)=0;

	//! Causes the accept eula flag to be added to the item
	//!
	virtual void acceptEula()=0;

	//! Gets the number of exe's for this item
	//!
	//! @param setActive If there is only one exe set it to be the active one
	//! @return Exe count
	//!
	virtual uint32 getExeCount(bool setActive = false, MCFBranch branch = MCFBranch())=0;

	//! Gets the list of exe info for this item
	//!
	//! @param list List to store results in
	//!
	virtual void getExeList(std::vector<UserCore::Item::Misc::ExeInfoI*> &list, MCFBranch branch = MCFBranch())=0;


	//! gets the active exe info
	//!
	//! @return active exe info
	//!
	virtual UserCore::Item::Misc::ExeInfoI* getActiveExe(MCFBranch branch = MCFBranch())=0;

	//! Sets the active exe used for launching
	//!
	//! @param name Exe name
	//!
	virtual void setActiveExe(const char* name, MCFBranch branch = MCFBranch())=0;
};


//! Contains the item update information
//!
class ItemUpdateInfo
{
public:
	//! Constuctor
	//!
	//! @param i Item id
	//! @param b Item update flags
	//! @param ii Item object
	//!
	ItemUpdateInfo(DesuraId i, MCFBuild b, ItemInfoI *ii)
	:	id(i),
		build(b),
		info(ii)
	{}

	DesuraId id;
	MCFBuild build;
	ItemInfoI* info;
};

}
}

#endif //DESURA_ITEMINFOI_H
