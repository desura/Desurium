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

#ifndef DESURA_WEBCOREI_H
#define DESURA_WEBCOREI_H
#ifdef _WIN32
#pragma once
#endif

#include "ResumeUploadInfo.h"
#include "DLLVersion.h"

#define WEBCORE					"WEBCORE_INTERFACE_001"
#define WEBCORE_VER				"WEBCORE_VERSION"
#define WEBCORE_PASSREMINDER	"WEBCORE_PASSWORDREMINDER"
#define WEBCORE_USERAGENT		"WEBCORE_USERAGENT"

typedef void (*PassReminderFN)(const char*);
typedef gcString (*UserAgentFN)();
typedef const char* (*WebCoreVersionFN)();


namespace MCFCore 
{ 
	namespace Misc
	{ 
		class UserCookies; 
		class DownloadProvider;
	}
}

namespace WebCore
{

namespace Misc
{
	class DumpInfo;
	class DownloadImageInfo;
}

CEXPORT void* FactoryBuilder(const char* name);

enum WebCoreUrl
{
	Welcome,
	Cart,
	Purchase,
	Gifts,
	ActivateGame,
	Games,
	Mods,
	Community,
	Development,
	Support,
	McfUpload,
	Help,
	ThreadWatch,
	Inbox,
	Updates,
	LinuxToolHelp,
	ListKeys,
	Root,
	AppChangeLog,
	ComplexModTutorial
};

class WebCoreI
{
public:
	//! Init webcore
	//!
	//! @param appDataPath Path to save webcore cache db at
	//!
	virtual void init(const char* appDataPath)=0;

	//! Delete this instance
	//!
	virtual void destroy()=0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Getters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Gets the id cookie
	//!
	//! @return id cookie
	//!
	virtual const char* getIdCookie()=0;

	//! Gets the session cookie
	//!
	//! @return Session cookie
	//!
	virtual const char* getSessCookie()=0;
	
	//! Gets the user agent for use in browsers and http connections
	//!
	//! @return Useragent string
	//!
	virtual const char* getUserAgent()=0;

	//! Gets the url to get location of mcf files
	//!
	//! @return Mcf url
	//!
	virtual const char* getMCFDownloadUrl()=0;

	//! Gets item info from the web and saves it into the user
	//!
	//! @param internId Desura item internal id
	//! @param[out] doc Item xml
	//!
	virtual void getItemInfo(DesuraId id, tinyxml2::XMLDocument &doc, MCFBranch mcfBranch, MCFBuild mcfBuild)=0; 


	//! Gets all the items for the mod install wizard
	//!
	//! @param[out] doc Item list xml
	//!
	virtual void getInstalledItemList(tinyxml2::XMLDocument &doc)=0;

	//! Gets a cdkey for a branch
	//!
	//! @param id Item id
	//! @param branch Item branch id
	//! @return Cd key
	//!
	virtual gcString getCDKey(DesuraId id, MCFBranch branch)=0;

	//! When cookies get updated this event gets called
	//!
	//! @return CookieUpdate event
	//!
	virtual EventV* getCookieUpdateEvent()=0;


	//! Calls the update poll with the post data
	//!
	virtual void getUpdatePoll(tinyxml2::XMLDocument &doc, const std::map<std::string, std::string> &post)=0;

	//! Gets the items that where normally part of the login
	//!
	virtual void getLoginItems(tinyxml2::XMLDocument &doc)=0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Setters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Sets all cookies at once. Only works if all three cookies are valid
	//!
	//! @param name Name cookie
	//! @param pass Password cookie
	//! @param sess Session cookie
	//!
	virtual void setCookie(const char* sess)=0;

	//! Sets the three cookies for a given httphandle
	//!
	//! @param hh HttpHandle to set cookies for
	//!
	virtual void setWCCookies(HttpHandle& hh)=0;

	//! Sets cookies for MCF download
	//!
	//! @param uc UserCookie object
	//!
	virtual void setMCFCookies(MCFCore::Misc::UserCookies* uc)=0;

	//! Sets the root domain for all urls. Must be in form of desura.com not www.desura.com or http://desura.com
	//!
	//! @param domain Root domain
	//!
	virtual void setUrlDomain(const char* domain)=0;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Functions
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//! Converts a short name to an Desura item id
	//!
	//! @param name Item name
	//! @param type Item type
	//! @return Desura item id
	//!
	virtual DesuraId nameToId(const char* name, const char* type)=0;

	//! Converts a hash to an Desura item id
	//!
	//! @param itemHashId Item hash
	//! @return Desura item id
	//!
	virtual DesuraId hashToId(const char* itemHashId)=0;

	//! Starts a new upload
	//! 
	//! @param id Item internal id
	//! @param hash Upload mcf hash
	//! @param fileSize Upload mcf size
	//! @param[out] key Upload key
	//!
	virtual void newUpload(DesuraId id, const char* hash, uint64 fileSize, char **key)=0;

	//! Gets info on a existing upload
	//!
	//! @param id Item internal id
	//! @param key Upload key
	//! @param[out] info ResumeUploadInfo object to store result in
	//!
	virtual void resumeUpload(DesuraId id, const char* key, WebCore::Misc::ResumeUploadInfo &info)=0;

	//! Download an image if it can
	//!
	//! @param id Item internal id
	//! @param[out] path Where the image got saved to
	//! @param name Item name
	//! @param postfix Append short name to image
	//! @param saveFolder Folder to save it to
	//!
	virtual void downloadImage(WebCore::Misc::DownloadImageInfo* dii, volatile bool &stop)=0;

	//! Download a banner for a item download
	//! 
	//! @param dlp DownloadProvider information
	//!
	virtual void downloadBanner(MCFCore::Misc::DownloadProvider* dlp, const char* saveFolder)=0;

	//! Adds or removes items from user accounts
	//!
	//! @param internId Item interanl id
	//! @param add Add to account (if false it removes from account
	//!
	virtual void updateAccountItem(DesuraId id, bool add)=0;




	//! Checks the user login info
	//!
	//! @param user Username
	//! @param pass Password
	//! @param[out] doc Login xml
	//! @return Api Version Number
	//!
	virtual void logIn(const char* user, const char* pass, tinyxml2::XMLDocument &doc)=0;

	//! Logs the user out
	//!
	virtual void logOut()=0;

	//! Clears the name cache
	//!
	virtual void clearNameCache()=0;

	//! Enables debugging of api errors
	//!
	virtual void enableDebugging(bool state = true)=0;


	virtual gcString getUrl(WebCoreUrl id)=0;
};

}

#endif //DESURA_WEBCOREI_H
