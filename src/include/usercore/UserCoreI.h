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

#ifndef DESURA_USERI_H
#define DESURA_USERI_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/NewsItem.h"
#include "usercore/ItemInfoI.h"
#include "usercore/ItemManagerI.h"


class WCSpecialInfo;
class IPCServiceMain;


#define USERCORE					"USERCORE_INTERFACE_001"
#define USERCORE_VER				"USERCORE_VERSION"
#define USERCORE_GETLOGIN			"USERCORE_GETLOGIN"

typedef const char* (*UserCoreVersionFN)();
typedef void *UserCoreGetLoginFN(char**, char**);

namespace Thread
{
	class ThreadPool;
}

namespace WebCore
{
	class WebCoreI;
}

namespace UserCore
{

extern "C"
{
	CEXPORT void* FactoryBuilderUC(const char* name);
}

class UserThreadManagerI;
class UploadManagerI;
class ToolManagerI;
class GameExplorerManager;
class CDKeyManagerI;
class CIPManagerI;

namespace Misc
{
	//! Get Cvar information stuct
	typedef struct
	{
		const char* name;
		const char* value;
	} CVar_s;

	class UpdateInfo
	{
	public:
		UpdateInfo()
		{
			branch = 0;
			build = 0;
		}

		UpdateInfo(uint32 branch, uint32 build)
		{
			this->branch = branch;
			this->build = build;
		}

		uint32 branch;
		uint32 build;
	};
}



class UserI
{
public:
	virtual ~UserI(){};

	//! Set up default user
	//!
	//! @param path New profile path
	//!
	virtual void init(const char* appDataPath)=0;

	//! Gets the current app data path
	//!
	virtual const char* getAppDataPath()=0;

	//! Gets the cache path mcf's are saved to
	//!
	virtual const char* getMcfCachePath()=0;

	//! Locks the user and stops it being deleted till unlock is called.
	//!
	virtual void lockDelete()=0;

	//! Unlock the user allowing it to be deleted
	//!
	virtual void unlockDelete()=0;

	///////////////////////////////////////////////////////////////////////////////
	// Login
	///////////////////////////////////////////////////////////////////////////////

	//! Log in as a Desura User
	//!
	//! @param user Username
	//! @param password User password
	//!
	virtual void logIn(const char* user, const char* password)=0;

	//! Logout
	//!
	virtual void logOut(bool delAutoLogin = false, bool reInit = true)=0;

	//! Saves the username and password hash to a file
	//!
	virtual void saveLoginInfo()=0;

	//! Login for the cleanup utility so it doesnt need a valid desura login
	//!
	virtual void logInCleanUp()=0;

	///////////////////////////////////////////////////////////////////////////////
	// Misc
	///////////////////////////////////////////////////////////////////////////////

	//! Is user an admin
	//!
	//! @return True if logged in user is an admin
	//!
	virtual bool isAdmin()=0;

	//! Is the login items delay loading
	//!
	virtual bool isDelayLoading()=0;

	//! Call this if a update needs to be downloaded
	//!
	//! @param appid Application branch (100, 300, 500)
	//! @param appver Application build
	//!
	virtual void appNeedUpdate(uint32 appid = 0, uint32 appver = 0)=0;

	//! Used to restart connection to desura serivce
	//!
	virtual void restartPipe()=0;

	//! Forces the update poll to run
	//!
	virtual void forceUpdatePoll()=0;

	///////////////////////////////////////////////////////////////////////////////
	// Getters
	///////////////////////////////////////////////////////////////////////////////

	//! Get user id
	//!
	//! @return User id
	//!
	virtual uint32 getUserId()=0;

	//! Get user avatar (as web url or local file path)
	//!
	//! @return User avatar
	//!
	virtual const char* getAvatar()=0;

	//! Get user profile url
	//!
	//! @return User profile url
	//!
	virtual const char* getProfileUrl()=0;

	//! Get user profile url in edit mode
	//!
	//! @return User edit url
	//!
	virtual const char* getProfileEditUrl()=0;

	//! Get user name id
	//!
	//! @return User name id
	//!
	virtual const char* getUserNameId()=0;
	
	//! Get user login name
	//!
	//! @return User login name
	//!
	virtual const char* getUserName()=0;

	//! Gets the personal message count
	//!
	//! @return PM count
	//!
	virtual uint32 getPmCount()=0;

	//! Gets the update count
	//!
	//! @return Update count
	//!
	virtual uint32 getUpCount()=0;

	//! Gets the number of items in a users cart
	//!
	//! @return Cart count
	//!
	virtual uint32 getCartCount()=0;

	//! Gets the number of new threads
	//!
	//! @return Thread count
	//!
	virtual uint32 getThreadCount()=0;

	//! Get a cvar value from ui core
	//!
	//! @param cvarName The name of the cvar to find the value of
	//! @return CVar value or null if not found
	//!
	virtual const char* getCVarValue(const char* cvarName)=0;

	//! Gets the thread pool for this user
	//!
	//! @return Threadpool
	//!
	virtual ::Thread::ThreadPool* getThreadPool()=0;

	//! Gets the service main for talking to Desura service
	//!
	//! @return ServiceMain
	//!
	virtual IPCServiceMain* getServiceMain()=0;

	//! Gets the webcore handle
	//!
	//! @return WebCore
	//!
	virtual WebCore::WebCoreI* getWebCore()=0;

	//! Gets the Thread manager handle
	//!
	//! @return Thread manager
	//!
	virtual UserCore::UserThreadManagerI* getThreadManager()=0;

	//! Gets the Upload manager handle
	//!
	//! @return Upload manager
	//!
	virtual UserCore::UploadManagerI* getUploadManager()=0;

	//! Gets the Item manager handle
	//!
	//! @return Item manager
	//!
	virtual UserCore::ItemManagerI* getItemManager()=0;

	//! Gets the Tool manager handle
	//!
	//! @return Tool manager
	//!
	virtual UserCore::ToolManagerI* getToolManager()=0;

	//! Gets the Game Explorer manager handle
	//!
	//! @return Game Explorer manager
	//!
	virtual UserCore::GameExplorerManager* getGameExplorerManager()=0;

	//! Gets the cd key manager
	//!
	//! @return CDKey Manager
	//!
	virtual UserCore::CDKeyManagerI* getCDKeyManager()=0;

	//! Gets the custom install path manager
	//!
	//! @return custom install path manager
	//!
	virtual UserCore::CIPManagerI* getCIPManager()=0;


	///////////////////////////////////////////////////////////////////////////////
	// Events
	///////////////////////////////////////////////////////////////////////////////

	//! Gets the item added event handler. Triggered when an item is added to the list
	//!
	//! @return Item added event
	//!
	virtual Event<uint32>* getItemsAddedEvent()=0;

	//! Gets the application update event handler. Triggered when an update is about to downloaded
	//!
	//! @return Application update event
	//!
	virtual Event<UserCore::Misc::UpdateInfo>* getAppUpdateEvent()=0;

	//! Gets the application update event handler. Triggered when an update is fully downloaded
	//!
	//! @return Application update complete event
	//!
	virtual Event<UserCore::Misc::UpdateInfo>* getAppUpdateCompleteEvent()=0;

	//! Gets the application update progress event handler. Triggered when progress is made on update download
	//! uint32 = progress
	//!
	//! @return Application update progress event
	//!
	virtual Event<uint32>* getAppUpdateProgEvent()=0;

	//! Gets the need cvar event handler. Triggered when needing resolving of a cvar in uicore
	//!
	//! @return Cvar event
	//!
	virtual Event<UserCore::Misc::CVar_s>* getNeedCvarEvent()=0;

	//! Gets the new avatar event handler. Triggered when user avatar is resolved
	//!
	//! @return Avatar event
	//!
	virtual Event<gcString>* getNewAvatarEvent()=0;

	//! Gets the wildcard event handler. Triggered when needing a wildcard resolved by ui core
	//!
	//! @return Wildcard event
	//!
	virtual Event<WCSpecialInfo>* getNeedWildCardEvent()=0;

	//! Gets the news event handler. Triggered when new news
	//!
	//! @return News event
	//!
	virtual Event<std::vector<UserCore::Misc::NewsItem*> >* getNewsUpdateEvent()=0;

	//! Gets the gift event handler. Triggered when new gifts
	//!
	//! @return Gift event
	//!
	virtual Event<std::vector<UserCore::Misc::NewsItem*> >* getGiftUpdateEvent()=0;

	//! Gets the item update event handler. Triggered when an item information has been updated
	//!
	//! @return Item update event
	//!
	virtual Event<std::vector<UserCore::Item::ItemUpdateInfo*> >* getItemUpdateEvent()=0;

	//! Gets the user update event handler. Triggered when an user information has been updated
	//!
	//! @return User update event
	//!
	virtual EventV* getUserUpdateEvent()=0;

	//! Gets the event for pipe disconnect. Triggered when desura service dies.
	//!
	//! @return Pipe disconnect event
	//!
	virtual EventV* getPipeDisconnectEvent()=0;

	//! Gets the event for when a forced update poll is triggered.
	//!
	//! @return Force update poll event
	//!
	virtual EventV* getForcedUpdatePollEvent()=0;

	//! Gets the event for when the login items are loaded
	//!
	virtual EventV* getLoginItemsLoadedEvent()=0;

	//! Gets the event for when low hdd space for mcf saves is detected
	//! First Event paramater is true if the low space is on the system disk, or false on another disk
	//! Second Event paramater is the hdd leter
	//!
	virtual Event<std::pair<bool, char> >* getLowSpaceEvent()=0;

	//! Updates the user counts. Used by javascript api as a cheap update method
	//!
	//! @param msgs Msg count
	//! @param updates Update count
	//! @param threads Thread count
	//! @param cart Cart count
	//!
	virtual void setCounts(uint32 msgs, uint32 updates, uint32 threads, uint32 cart)=0;

	//! Sets the main window handle to use with ShellExecute
	//!
	//! @pram handle Main window handle
	//!
#ifdef WIN32
	virtual void setMainWindowHandle(HWND handle)=0;
#endif

	//! Updates the uninstall registry list
	//!
	virtual void updateUninstallInfo()=0;

	//! Updates the uninstall registry for one item
	//!
	//! @param id Item id
	//! @param installSize Install size
	//!
	virtual void updateUninstallInfo(DesuraId id, uint64 installSize)=0;

	//! Removes the uninstall registry for one item
	//!
	//! @param id Item id
	//!
	virtual void removeUninstallInfo(DesuraId id)=0;

	//! Updates a registry key using desura install service
	//!
	//! @param key Key to update
	//! @param value Value to set it to
	//!
	virtual void updateRegKey(const char* key, const char* value)=0;
	virtual void updateBinaryRegKey(const char* key, const char* value, size_t size)=0;

	virtual void runInstallScript(const char* file, const char* installPath, const char* function)=0;

	//! Logs in with out starting all the extra stuff
	virtual void logInTool(const char* user, const char* pass)=0;
	
	virtual void init(const char* appDataPath, const char* szProviderUrl)=0;
};


}

#endif //DESURA_UserI_H
