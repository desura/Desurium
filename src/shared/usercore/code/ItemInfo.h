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

#ifndef DESURA_ITEMINFO_H
#define DESURA_ITEMINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/ItemInfoI.h"
#include "managers/WildcardManager.h"
#include "BranchInfo.h"



namespace sqlite3x
{
	class sqlite3_connection;
}

namespace UserCore
{
	class User;

namespace Item
{

class BranchInfo;
class BranchInstallInfo;

class ItemInfo : public ItemInfoI
{
public:
	//! Constuctor
	//!
	//! @param id Item internal id
	//!
	ItemInfo(UserCore::User *user, DesuraId id);

	//! Constuctor
	//!
	//! @param id Item internal id
	//! @param parid Parent internal id
	//!
	ItemInfo(UserCore::User *user, DesuraId id, DesuraId parid);
	~ItemInfo();

	//inherited methods
	virtual void updated();
	virtual void addToAccount();
	virtual void removeFromAccount();

	virtual DesuraId getParentId();
	virtual DesuraId getId();
	virtual DesuraId getInstalledModId(MCFBranch branch = MCFBranch());

	virtual uint32 getChangedFlags();
	virtual uint32 getStatus();

	virtual uint8 getPercent();
	virtual uint8 getPermissions();
	virtual uint8 getOptions();

	virtual bool isLaunchable();
	virtual bool isUpdating();
	virtual bool isInstalled();
	virtual bool isDownloadable();
	virtual bool isComplex();
	virtual bool isParentToComplex();
	virtual bool isDeleted();
	virtual bool isFirstLaunch();

	virtual bool hasAcceptedEula();
	virtual bool compare(const char* filter);

	virtual void addSFlag(uint32 status);
	virtual void addPFlag(uint8 permission);
	virtual void addOFlag(uint8 option);

	virtual void delSFlag(uint32 status);
	virtual void delPFlag(uint8 permission);
	virtual void delOFlag(uint8 option);

	virtual const char* getRating();
	virtual const char* getDev();
	virtual const char* getName();
	virtual const char* getShortName();
	virtual const char* getPath(MCFBranch branch = MCFBranch());
	virtual const char* getInsPrimary(MCFBranch branch = MCFBranch());
	virtual const char* getIcon();
	virtual const char* getLogo();
	virtual const char* getIconUrl();
	virtual const char* getLogoUrl();
	virtual const char* getDesc();
	virtual const char* getTheme();
	virtual const char* getGenre();
	virtual const char* getProfile();
	virtual const char* getDevProfile();

	virtual const char* getPublisher();
	virtual const char* getPublisherProfile();

	virtual const char* getEulaUrl();
	virtual const char* getInstallScriptPath();

	virtual Event<ItemInfoI::ItemInfo_s>* getInfoChangeEvent();


	void overrideMcfBuild(MCFBuild build, MCFBranch branch = MCFBranch());
	virtual uint64 getInstallSize(MCFBranch branch = MCFBranch());
	virtual uint64 getDownloadSize(MCFBranch branch = MCFBranch());
	virtual MCFBuild getLastInstalledBuild(MCFBranch branch = MCFBranch());
	virtual MCFBuild getInstalledBuild(MCFBranch branch = MCFBranch());
	virtual MCFBuild getNextUpdateBuild(MCFBranch branch = MCFBranch());
	virtual MCFBranch getInstalledBranch();
	virtual MCFBranch getLastInstalledBranch();
	virtual const char* getInstalledVersion(MCFBranch branch = MCFBranch());


	virtual uint32 getBranchCount();
	virtual BranchInfoI* getBranch(uint32 index);
	virtual BranchInfoI* getCurrentBranch();
	virtual BranchInfoI* getBranchById(uint32 id);

	virtual void acceptEula();


	virtual uint32 getExeCount(bool setActive, MCFBranch branch = MCFBranch());
	virtual void getExeList(std::vector<UserCore::Item::Misc::ExeInfoI*> &list, MCFBranch branch = MCFBranch());
	virtual UserCore::Item::Misc::ExeInfoI* getActiveExe(MCFBranch branch = MCFBranch());
	virtual void setActiveExe(const char* name, MCFBranch branch = MCFBranch());

	virtual bool isFavorite();
	virtual void setFavorite(bool fav);

	//! Removes this item from the db
	//!
	//! @param db Sqlite db connection
	//!
	void deleteFromDb(sqlite3x::sqlite3_connection* db);

	//! Save regular changed vars to db
	//!
	//! @param db Sqlite db connection
	//!
	void saveDb(sqlite3x::sqlite3_connection* db);

	//! Save all vars to db
	//!
	//! @param db Sqlite db connection
	//!
	void saveDbFull(sqlite3x::sqlite3_connection* db);

	//! Load vars from db
	//!
	//! @param db Sqlite db connection
	//!
	void loadDb(sqlite3x::sqlite3_connection* db);


	//! Load data for this item from xml
	//!
	//! @param xmlNode Xml to get data from
	//! @param statusOveride New status flags to add when load is complete
	//! @param pWildCard Wildcard manager to resolve wildcards from
	//!
	void loadXmlData(uint32 platform, tinyxml2::XMLNode *xmlNode, uint16 statusOveride, WildcardManager* pWildCard=NULL, bool reset = false);


	//! hash for base manager
	//!
	//! @return Item hash
	//!
	uint64 getHash();

	//! Paused the item information update event
	//!
	void pauseCallBack();

	//! Resumes the item information update event
	//!
	void resumeCallBack();

	//! Is the item information update event active
	//!
	//! @return True for active, false if not
	//!
	bool isCallBackActive();


	//! Sets the Item progress percent
	//!
	//! @param percent Item percent
	//!
	void setPercent(uint8 percent);


	//! Sets the item name
	//!
	//! @param name Name
	//!
	void setName(const char* name);

	//! Sets the item icon
	//!
	//! @param icon Icon path
	//!
	void setIcon(const char* icon);

	//! Sets the item logo
	//!
	//! @param logo Logo path
	//!
	void setLogo(const char* logo);


	//! Sets the item icon url
	//!
	//! @param icon Icon url
	//! @param hash Icon hash
	//!
	void setIconUrl(const char* icon);

	//! Sets the item logo url
	//!
	//! @param logo Logo url
	//! @param hash Logo hash
	//!
	void setLogoUrl(const char* logo);


	void processUpdateXml(tinyxml2::XMLNode *node);


	bool setInstalledMcf(MCFBranch branch, MCFBuild build);
	void overideInstalledBuild(MCFBuild build);

	void resetInstalledMcf();
	void overideFavorite(bool fav);

	//! Sets the id of the installed mod for this item. Use item manager instead to set this!!!!!
	//!
	//! @param id Mod id
	//!
	void setInstalledModId(DesuraId id, MCFBranch branch = MCFBranch());

	//! Overrides the parent id
	//!
	//! @param id Parent id
	//!
	void setParentId(DesuraId id);


	bool wasOnAccount();

	void migrateStandalone(MCFBranch branch, MCFBuild build);


	void setLinkInfo(const char* exe, const char* args);

	//! If given a global branch it will return the best branch for that global.
	//! If given an invalid branch it will select the best branch avaliable
	//! If given a valid non global branch it will return the same branch
	//!
	MCFBranch getBestBranch(MCFBranch branch);

	//! Given a list of branches it will select the best avliable
	//! If it cant work out best branch it will return 0
	//!
	MCFBranch selectBestBranch(const std::vector<BranchInfo*> &list);

protected:
	//! Event handler for item information changed. Triggers when this item information gets updated
	//!
	Event<ItemInfoI::ItemInfo_s> onInfoChangeEvent;

	//! If the info changed event is active it gets called
	//!
	void onInfoChange();

	//! Triggers the info changed event
	//!
	void triggerCallBack();

	//! Default initilzation of item vars
	//!
	void initVars();

	//! Gets the usercore handle
	//!
	//! @return UserCore
	//!
	UserCore::User* getUserCore();


	void broughtCheck();

	void processInfo(tinyxml2::XMLNode* xmlEl);
	void processSettings(uint32 platform, tinyxml2::XMLNode* setNode, WildcardManager* pWildCard, bool reset);

	void launchExeHack();

	void onBranchInfoChanged();
	bool shouldSaveDb(sqlite3x::sqlite3_connection* db);

	void loadBranchXmlData(tinyxml2::XMLElement* branch);
	
	BranchInfo* getCurrentBranchFull();
	BranchInstallInfo* getBranchOrCurrent(MCFBranch branch);


private:
	bool m_bPauseCallBack;
	bool m_bWasOnAccount;

	DesuraId m_iId;
	DesuraId m_iParentId;
	

	uint32 m_iChangedFlags;
	uint32 m_iStatus;

	uint32 m_INBranchIndex;
	MCFBranch m_INBranch;
	MCFBranch m_LastBranch;

	uint8 m_iPermissions;
	uint8 m_iOptions;
	uint8 m_iPercent;

	gcString m_szRating;	//rating
	gcString m_szDesc;		//short desc
	gcString m_szDev;		//developers name
	gcString m_szDevProfile;//developers profile
	gcString m_szPublisher;
	gcString m_szPublisherProfile;
	gcString m_szName;		//full name
	gcString m_szShortName;	//short name

	gcString m_szIcon;		//icon path or url
	gcString m_szLogo;		//logo path or url
	gcString m_szIconUrl;
	gcString m_szLogoUrl;

	gcString m_szTheme;		//items theme
	gcString m_szGenre;		//items genre
	gcString m_szProfile;	//url to profile
	gcString m_szEULAUrl;	//eula

	std::vector<BranchInfo*> m_vBranchList;
	std::map<uint32, BranchInstallInfo*> m_mBranchInstallInfo;

	UserCore::User *m_pUserCore; 
};



inline DesuraId ItemInfo::getParentId()
{
	return m_iParentId;
}

inline DesuraId ItemInfo::getId()
{
	return m_iId;
}

inline uint32 ItemInfo::getChangedFlags()
{
	return m_iChangedFlags;
}

inline uint32 ItemInfo::getStatus()
{
	return m_iStatus;
}


inline uint8 ItemInfo::getPercent()
{
	return m_iPercent;
}

inline uint8 ItemInfo::getPermissions()
{
	return m_iPermissions;
}

inline uint8 ItemInfo::getOptions()
{
	return m_iOptions;
}


inline bool ItemInfo::isLaunchable()
{
	return (m_iStatus & (UserCore::Item::ItemInfoI::STATUS_INSTALLED|UserCore::Item::ItemInfoI::STATUS_READY))?true:false;
}

inline bool ItemInfo::isInstalled()
{
	return (m_iStatus & UserCore::Item::ItemInfoI::STATUS_INSTALLED)?true:false;
}

inline bool ItemInfo::isDownloadable()
{
	return !HasAnyFlags(getStatus(), UserCore::Item::ItemInfoI::STATUS_NONDOWNLOADABLE|UserCore::Item::ItemInfoI::STATUS_LINK);
}

inline bool ItemInfo::isComplex()
{
	return HasAllFlags(getStatus(), UserCore::Item::ItemInfoI::STATUS_INSTALLCOMPLEX);
}

inline bool ItemInfo::isParentToComplex()
{
	return !getParentId().isOk() && getInstalledModId().isOk();
}

inline bool ItemInfo::isUpdating()
{
	return (isInstalled() && HasAllFlags(getStatus(), UserCore::Item::ItemInfoI::STATUS_UPDATEAVAL));
}

inline bool ItemInfo::isDeleted()
{
	return HasAllFlags(getStatus(), UserCore::Item::ItemInfoI::STATUS_DELETED);
}

inline bool ItemInfo::isFirstLaunch()
{
	return HasAllFlags(getStatus(), UserCore::Item::ItemInfoI::STATUS_LAUNCHED) == false;
}

inline const char* ItemInfo::getRating()	
{
	return m_szRating.c_str();
}

inline const char* ItemInfo::getDev()		
{
	return m_szDev.c_str();
}

inline const char* ItemInfo::getName()		
{
	return m_szName.c_str();
}

inline const char* ItemInfo::getShortName()	
{
	return m_szShortName.c_str();
}

inline const char* ItemInfo::getIcon()		
{
	return m_szIcon.c_str();
}

inline const char* ItemInfo::getLogo()		
{
	return m_szLogo.c_str();
}

inline const char* ItemInfo::getIconUrl()	
{
	return m_szIconUrl.c_str();
}

inline const char* ItemInfo::getLogoUrl()	
{
	return m_szLogoUrl.c_str();
}

inline const char* ItemInfo::getDesc()		
{
	return m_szDesc.c_str();
}

inline const char* ItemInfo::getTheme()		
{
	return m_szTheme.c_str();
}

inline const char* ItemInfo::getGenre()		
{
	return m_szGenre.c_str();
}

inline const char* ItemInfo::getProfile()	
{
	return m_szProfile.c_str();
}

inline const char* ItemInfo::getDevProfile()
{
	return m_szDevProfile.c_str();
}

inline const char* ItemInfo::getPublisher()
{
	return m_szPublisher.c_str();
}

inline const char* ItemInfo::getPublisherProfile()
{
	return m_szPublisherProfile.c_str();
}

inline const char* ItemInfo::getInstallScriptPath()
{
	if (!getCurrentBranch())
		return NULL;

	return getCurrentBranch()->getInstallScriptPath();
}

inline Event<ItemInfoI::ItemInfo_s>* ItemInfo::getInfoChangeEvent()
{
	return &onInfoChangeEvent;
}

/////////////////////////////////////////////

inline uint64 ItemInfo::getHash()
{
	return getId().toInt64();
}

inline void ItemInfo::pauseCallBack()
{
	m_bPauseCallBack = true;
}

inline void ItemInfo::resumeCallBack()
{
	m_bPauseCallBack = false;
}

inline bool ItemInfo::isCallBackActive()
{
	return m_bPauseCallBack;
}

inline void ItemInfo::setName(const char* name)		
{
	m_szName = gcString(name);
}

inline UserCore::User* ItemInfo::getUserCore()
{
	return m_pUserCore;
}

inline uint32 ItemInfo::getBranchCount()
{
	return m_vBranchList.size();
}

inline MCFBranch ItemInfo::getInstalledBranch()
{
	return m_INBranch;
}

inline MCFBranch ItemInfo::getLastInstalledBranch()
{
	return m_LastBranch;
}

inline bool ItemInfo::wasOnAccount()
{
	return m_bWasOnAccount;
}


}
}

#endif //DESURA_ItemInfo_H
