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

#ifndef DESURA_WEBCORE_H
#define DESURA_WEBCORE_H

#include "webcore/WebCoreI.h"
#include "util_thread/BaseThread.h"
#include "ImageCache.h"

namespace sqlite3x
{
	class sqlite3_connection;
}



namespace WebCore
{

namespace Misc
{
	class DumpInfo;
}


class PostString : public std::string
{
public:
	std::string& operator=(const uint32 &val)
	{
		std::string::operator=(gcString("{0}", val));
		return *this;
	}

	std::string& operator=(const std::string &val)
	{
		std::string::operator=(val);
		return *this;
	}

	std::string& operator=(const char* val)
	{
		std::string::operator=(val);
		return *this;
	}
};

typedef std::map<std::string, PostString> PostMap;


class WebCoreClass : public WebCoreI
{
public:
	WebCoreClass();
	~WebCoreClass();

	virtual void init(const char* appDataPath);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Getters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual const char* getIdCookie();
	virtual const char* getSessCookie();

	virtual const char* getUserAgent();
	virtual void getItemInfo(DesuraId id, tinyxml2::XMLDocument &doc, MCFBranch mcfBranch, MCFBuild mcfBuild); 
	virtual void getInstalledItemList(tinyxml2::XMLDocument &doc);
	virtual EventV* getCookieUpdateEvent();
	virtual gcString getCDKey(DesuraId id, MCFBranch branch);
	virtual void getUpdatePoll(tinyxml2::XMLDocument &doc, const std::map<std::string, std::string> &post);
	virtual void getLoginItems(tinyxml2::XMLDocument &doc);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Setters
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual void setCookie(const char* sess);
	virtual void setWCCookies(HttpHandle& hh);
	virtual void setMCFCookies(MCFCore::Misc::UserCookies* uc);
	virtual void setUrlDomain(const char* domain);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Functions
	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual DesuraId nameToId(const char* name, const char* type);
	virtual DesuraId hashToId(const char* itemHashId);
	virtual void newUpload(DesuraId id, const char* hash, uint64 fileSize, char **key);
	virtual void resumeUpload(DesuraId id, const char* key, WebCore::Misc::ResumeUploadInfo &info);
	virtual void downloadImage(WebCore::Misc::DownloadImageInfo *dii, volatile bool &stop);
	virtual void downloadBanner(MCFCore::Misc::DownloadProvider* dlp, const char* saveFolder);
	virtual void updateAccountItem(DesuraId id, bool add);
	virtual void clearNameCache();


	virtual void enableDebugging(bool state = true);

	virtual void destroy()
	{
		delete this;
	}

	virtual gcString getUrl(WebCoreUrl id);

	//! Sends a password reminder to the dest email
	//!
	//! @param email Email to send reminder to
	//!
	static void sendPassReminder(const char* email);

	const char* getMCFDownloadUrl();

	static gcString getPassWordReminderUrl();
	static gcString getLoginUrl();
	static gcString getRefreshUrl();
	static gcString getUpdatePollUrl();
	static gcString getNameLookUpUrl();
	static gcString getMcfUploadUrl();
	static gcString getItemInfoUrl();
	static gcString getInstalledWizardUrl();
	static gcString getUpdateAccountUrl();
	static gcString getCDKeyUrl();
	static gcString getMemberDataUrl();

protected:
	virtual void logIn(const char* user, const char* pass, tinyxml2::XMLDocument &doc);
	virtual void logOut();

	void startRefreshCycle();
	void stopRefreshCycle();

	tinyxml2::XMLNode* postToServer(std::string url, std::string resource, PostMap &postData, tinyxml2::XMLDocument &doc, bool useHTTPS = false);
	tinyxml2::XMLNode* loginToServer(std::string url, std::string resource, PostMap &postData, tinyxml2::XMLDocument &doc);

	EventV onCookieUpdateEvent;

	void onHttpProg(volatile bool& stop, Prog_s& prog);

private:
	bool m_bUserAuth;
	uint32 m_uiUserId;

	gcString m_szUserAgent;
	gcString m_szIdCookie;
	gcString m_szSessCookie;
	gcString m_szMCFDownloadUrl;
	gcString m_szAppDataPath;

	bool m_bDebuggingOut;

	ImageCache m_ImageCache;
};

inline const char* WebCoreClass::getUserAgent()
{
	return m_szUserAgent.c_str();
}

inline const char* WebCoreClass::getIdCookie()
{
	return m_szIdCookie.c_str();
}

inline const char* WebCoreClass::getSessCookie()
{
	return m_szSessCookie.c_str();
}

inline EventV* WebCoreClass::getCookieUpdateEvent()
{
	return &onCookieUpdateEvent;
}

}

#endif
