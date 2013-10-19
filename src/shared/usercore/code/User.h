////////////// Copyright 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : UserCore::Usercore
//   File        : UserCore::User.h
//   Description :
//      [Write the purpose of UserCore::User.h.]
//
//   Created On: 9/23/2008 5:00:03 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_USER_H
#define DESURA_USER_H
#ifdef _WIN32
#pragma once
#endif

#include "Common.h"

#include "usercore/UserCoreI.h"

#include "ItemManager.h"
#include "ToolManager.h"
#include "CDKeyManager.h"
#include "CIPManager.h"

#include "BaseManager.h"
#include "webcore/WebCoreI.h"
#include "managers/WildcardManager.h"
#include "util_thread/BaseThread.h"
#include "util_thread/ThreadPool.h"

#include "Event.h"

#include "ItemInfo.h"
#include "usercore/NewsItem.h"
#include "UserThreadManager.h"
#include "UploadManager.h"
#include "UserTasks.h"
#include "UserIPCPipeClient.h"

CEXPORT const char* GetUserCoreVersion();

class UpdateThread;
class ThreadPool;



namespace Thread
{
	class WaitCondition;
}

namespace UserCore
{

class ItemManager;
class ToolManager;
class GameExplorerManager;
class BDManager;


namespace Thread
{
	class UserThreadI;
}

namespace Misc
{
	//! Update avalible stuct
	typedef struct
	{
		bool alert;
		uint32 build;
	} update_s;
}


enum PlatformType
{
	PT_Item,
	PT_Tool,
};

class User : public UserI
{
public:
	User();
	~User();

	virtual void init(const char* appDataPath, const char* szProviderUrl) override;
	virtual void init(const char* appDataPath);
	virtual const char* getAppDataPath();
	virtual const char* getMcfCachePath();

	virtual void lockDelete();
	virtual void unlockDelete();

	///////////////////////////////////////////////////////////////////////////////
	// Login
	///////////////////////////////////////////////////////////////////////////////

	virtual void logIn(const char* user, const char* password);
	virtual void logOut(bool delAutoLogin = false, bool reInit = true);
	virtual void saveLoginInfo();
	virtual void logInCleanUp();

	///////////////////////////////////////////////////////////////////////////////
	// Misc
	///////////////////////////////////////////////////////////////////////////////

	virtual bool isAdmin();
	virtual bool isDelayLoading();

	virtual void appNeedUpdate(uint32 appid = 0, uint32 appver = 0);
	virtual void restartPipe();
	virtual void forceUpdatePoll();
	///////////////////////////////////////////////////////////////////////////////
	// Getters
	///////////////////////////////////////////////////////////////////////////////

	virtual uint32 getUserId();
	virtual const char* getAvatar();
	virtual const char* getProfileUrl();
	virtual const char* getProfileEditUrl();
	virtual const char* getUserNameId();
	virtual const char* getUserName();

	virtual uint32 getPmCount();
	virtual uint32 getUpCount();
	virtual uint32 getCartCount();
	virtual uint32 getThreadCount();

	virtual const char* getCVarValue(const char* cvarName);
	virtual ::Thread::ThreadPool* getThreadPool();
	virtual IPCServiceMain* getServiceMain();
	virtual WebCore::WebCoreI* getWebCore();
	virtual UserCore::UserThreadManagerI* getThreadManager();
	virtual UserCore::UploadManagerI* getUploadManager();
	virtual UserCore::ItemManagerI* getItemManager();
	virtual UserCore::ToolManagerI* getToolManager();
	virtual UserCore::GameExplorerManager* getGameExplorerManager();
	virtual UserCore::CDKeyManagerI* getCDKeyManager();
	virtual UserCore::CIPManagerI* getCIPManager();

	///////////////////////////////////////////////////////////////////////////////
	// Events
	///////////////////////////////////////////////////////////////////////////////

	virtual Event<uint32>* getItemsAddedEvent();
	virtual Event<UserCore::Misc::UpdateInfo>* getAppUpdateEvent();
	virtual Event<UserCore::Misc::UpdateInfo>* getAppUpdateCompleteEvent();
	virtual Event<uint32>* getAppUpdateProgEvent();
	virtual Event<UserCore::Misc::CVar_s>* getNeedCvarEvent();
	virtual Event<gcString>* getNewAvatarEvent();
	virtual Event<WCSpecialInfo>* getNeedWildCardEvent();
	virtual Event<std::vector<UserCore::Misc::NewsItem*> >* getNewsUpdateEvent();
	virtual Event<std::vector<UserCore::Misc::NewsItem*> >* getGiftUpdateEvent();
	virtual Event<std::vector<UserCore::Item::ItemUpdateInfo*> >* getItemUpdateEvent();
	virtual EventV* getUserUpdateEvent();
	virtual EventV* getPipeDisconnectEvent();
	virtual EventV* getForcedUpdatePollEvent();
	virtual EventV* getLoginItemsLoadedEvent();
	virtual Event<std::pair<bool, char> >* getLowSpaceEvent();

	virtual void setCounts(uint32 msgs, uint32 updates, uint32 threads, uint32 cart);
#ifdef WIN32
	virtual void setMainWindowHandle(HWND handle);
#endif

	virtual void updateUninstallInfo();
	virtual void updateUninstallInfo(DesuraId id, uint64 installSize);
	virtual void removeUninstallInfo(DesuraId id);
	virtual void updateRegKey(const char* key, const char* value);
	virtual void updateBinaryRegKey(const char* key, const char* value, size_t size);

	virtual void runInstallScript(const char* file, const char* installPath, const char* function);

	bool isAltProvider();

	//! Parses news xml
	//!
	//! @param newsNode News xml
	//!
	void parseNews(tinyxml2::XMLNode* newsNode);

	//! Parses gifts xml
	//!
	//! @param giftNode Gifts xml
	//!
	void parseGifts(tinyxml2::XMLNode* giftNode);

	//! Set user avatar
	//!
	//! @param path Avatar path
	//!
	void setAvatarPath(const char* path);

	
	//! Downloads an image for an item
	//!
	//! @param itemInfo Item for which the image belongs
	//! @param image Which image to download
	//!
	void downloadImage(UserCore::Item::ItemInfo* itemInfo, uint8 image);

	//! Removes or adds an item to a desura account
	//!
	//! @param item Item for which to act appon
	//! @param action Add or remove it
	//!
	void changeAccount(DesuraId id, uint8 action);

	//! Checks for saved login info and returns it if found
	//!
	//! @param internId Item id
	//! @param pWildCard Wildcard manager
	//!
	static void getLoginInfo(char** userhash, char** passhash);

#ifdef WIN32
	HWND getMainWindowHandle();
#endif

	//! Start the pipe to the desura service
	//!
	void initPipe();


	//! Should this platform be filtered out.
	//! 
	//! @param platform Platform node form xml
	//! @param type Type of item applying the filter for
	//! @return true to filter, false to inclue
	//!
	bool platformFilter(tinyxml2::XMLElement* platform, PlatformType type);

	BDManager* getBDManager();

protected:
	Event<uint32> onItemsAddedEvent;
	Event<UserCore::Misc::UpdateInfo> onAppUpdateEvent;
	Event<UserCore::Misc::UpdateInfo> onAppUpdateCompleteEvent;
	Event<uint32> onAppUpdateProgEvent;
	EventV onUserUpdateEvent;
	Event<UserCore::Misc::CVar_s> onNeedCvarEvent;
	Event<gcString> onNewAvatarEvent;
	Event<WCSpecialInfo> onNeedWildCardEvent;
	Event<std::vector<UserCore::Misc::NewsItem*> > onNewsUpdateEvent;
	Event<std::vector<UserCore::Misc::NewsItem*> > onGiftUpdateEvent;
	Event<std::vector<UserCore::Item::ItemUpdateInfo*> > onItemUpdateEvent;
	EventV onPipeDisconnect;
	EventV onForcePollEvent;
	EventV onLoginItemsLoadedEvent;
	Event<std::pair<bool, char> > onLowSpaceEvent;

	//! Update has finished downloading
	//! 
	//! @param info Update info
	//!
	void onUpdateComplete(UserCore::Misc::update_s& info);

	//! Update has started downloading
	//! 
	//! @param info Update info
	//!
	void onUpdateStart(UserCore::Misc::update_s& info);

	//! Resolve Wildcards
	//!
	//! @param info Wildcard Info
	//!
	void onNeedWildCardCB(WCSpecialInfo& info);

	void parseNewsAndGifts(tinyxml2::XMLNode* xmlNode, Event<std::vector<UserCore::Misc::NewsItem*> > &onEvent);

	void testMcfCache();

private:
	void init();
	void cleanUp();
	void onLoginItemsLoaded();
	
	gcString m_szMcfCachePath;
	gcString m_szAppDataPath;

	gcString m_szUserName;
	gcString m_szUserNameId;
	gcString m_szAvatar;

	gcString m_szProfileUrl;
	gcString m_szProfileEditUrl;

	bool m_bDelayLoading;
	bool m_bAdmin;
	bool m_bDownloadingUpdate;

	uint32 m_iCartItems;
	uint32 m_iUserId;
	uint32 m_iUpdates;
	uint32 m_iPms;
	uint32 m_iThreads;

	uint32 m_iSelectedIndex;
	uint32 m_uiLastUpdateBuild;
	uint32 m_uiLastUpdateVer;

	UserIPCPipeClient* m_pPipeClient;

	::Thread::ThreadPool* m_pThreadPool;

	WebCore::WebCoreI* m_pWebCore;
	UserCore::UserThreadManager* m_pThreadManager;
	UserCore::UploadManager* m_pUploadManager;
	UserCore::Thread::UserThreadI* m_pUThread;
	UserCore::ItemManager* m_pItemManager;
	UserCore::ToolManager* m_pToolManager;
	UserCore::GameExplorerManager* m_pGameExplorerManager;

	UserCore::CDKeyManager* m_pCDKeyManager;
	UserCore::BDManager* m_pBannerDownloadManager;
	UserCore::CIPManager* m_pCIPManager;

	volatile bool m_bLocked;
	::Thread::WaitCondition *m_pWaitCond;

#ifdef WIN32
	HWND m_WinHandle;
#endif

	bool m_bAltProvider;

	friend class ItemManager;
};


inline bool User::isAdmin()
{
	return m_bAdmin;
}

inline bool User::isDelayLoading()
{
	return m_bDelayLoading;
}

inline uint32 User::getUserId()
{
	return m_iUserId;
}


inline const char* User::getAvatar()
{
	return m_szAvatar.c_str();
}

inline const char* User::getProfileUrl()
{
	return m_szProfileUrl.c_str();
}

inline const char* User::getProfileEditUrl()
{
	return m_szProfileEditUrl.c_str();
}

inline const char* User::getUserNameId()
{
	return m_szUserNameId.c_str();
}

inline const char* User::getUserName()
{
	return m_szUserName.c_str();
}

inline uint32 User::getPmCount()
{
	return m_iPms;
}

inline uint32 User::getUpCount()
{
	return m_iUpdates;
}

inline uint32 User::getCartCount()
{
	return m_iCartItems;
}

inline uint32 User::getThreadCount()
{
	return m_iThreads;
}

inline ::Thread::ThreadPool* User::getThreadPool()
{
	return m_pThreadPool;
}

inline IPCServiceMain* User::getServiceMain()
{
	if (!m_pPipeClient)
		return NULL;

	return m_pPipeClient->getServiceMain();
}

inline WebCore::WebCoreI* User::getWebCore()
{
	return m_pWebCore;
}

inline UserCore::UserThreadManagerI* User::getThreadManager()
{
	return m_pThreadManager;
}

inline UserCore::UploadManagerI* User::getUploadManager()
{
	return m_pUploadManager;
}

inline UserCore::ItemManagerI* User::getItemManager()
{
	return m_pItemManager;
}

inline UserCore::ToolManagerI* User::getToolManager()
{
	return m_pToolManager;
}

inline UserCore::GameExplorerManager* User::getGameExplorerManager()
{
	return m_pGameExplorerManager;
}

inline UserCore::CDKeyManagerI* User::getCDKeyManager()
{
	return m_pCDKeyManager;
}

inline UserCore::CIPManagerI* User::getCIPManager()
{
	return m_pCIPManager;
}

inline Event<uint32>* User::getItemsAddedEvent()
{
	return &onItemsAddedEvent;
}

inline Event<UserCore::Misc::UpdateInfo>* User::getAppUpdateEvent()
{
	return &onAppUpdateEvent;
}

inline Event<UserCore::Misc::UpdateInfo>* User::getAppUpdateCompleteEvent()
{
	return &onAppUpdateCompleteEvent;
}

inline Event<uint32>* User::getAppUpdateProgEvent()
{
	return &onAppUpdateProgEvent;
}

inline Event<UserCore::Misc::CVar_s>* User::getNeedCvarEvent()
{
	return &onNeedCvarEvent;
}

inline Event<gcString>* User::getNewAvatarEvent()
{
	return &onNewAvatarEvent;
}

inline Event<WCSpecialInfo>* User::getNeedWildCardEvent()
{
	return &onNeedWildCardEvent;
}

inline Event<std::vector<UserCore::Misc::NewsItem*> >* User::getNewsUpdateEvent()
{
	return &onNewsUpdateEvent;
}

inline Event<std::vector<UserCore::Misc::NewsItem*> >* User::getGiftUpdateEvent()
{
	return &onGiftUpdateEvent;
}

inline Event<std::vector<UserCore::Item::ItemUpdateInfo*> >* User::getItemUpdateEvent()
{
	return &onItemUpdateEvent;
}

inline EventV* User::getUserUpdateEvent()
{
	return &onUserUpdateEvent;
}

inline EventV* User::getPipeDisconnectEvent()
{
	return &onPipeDisconnect;
}

inline EventV* User::getForcedUpdatePollEvent()
{
	return &onForcePollEvent;
}

inline EventV* User::getLoginItemsLoadedEvent()
{
	return &onLoginItemsLoadedEvent;
}

inline Event<std::pair<bool, char> >* User::getLowSpaceEvent()
{
	return &onLowSpaceEvent;
}

//other

#ifdef WIN32
inline void User::setMainWindowHandle(HWND handle)
{
	m_WinHandle = handle;
}

inline HWND User::getMainWindowHandle()
{
	return m_WinHandle;
}
#endif

inline BDManager* User::getBDManager()
{
	return m_pBannerDownloadManager;
}

inline bool User::isAltProvider()
{
	return m_bAltProvider;
}

}

#endif //DESURA_UserCore::User_H
