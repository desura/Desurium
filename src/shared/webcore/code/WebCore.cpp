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

#include "Common.h"
#include "WebCore.h"
#include "XMLMacros.h"

#ifdef WIN32
	#include <Wininet.h>
#endif

#include "mcfcore/MCFI.h"
#include "mcfcore/MCFMain.h"
#include "mcfcore/UserCookies.h"

#include "sqlite3x.hpp"
#include "sql/WebCoreSql.h"



static gcString g_szRootDomain = "desura.com";



namespace WebCore
{

extern gcString genUserAgent();

WebCoreClass::WebCoreClass()
	: m_bValidateCert(true)
{
	m_bUserAuth = false;
	m_uiUserId = 0;
	m_szUserAgent = genUserAgent();

#ifdef DEBUG
	setUrlDomain("desura.com");
#else
	setUrlDomain("desura.com");
#endif

#ifdef DEBUG
	m_bDebuggingOut = true;
#else
	m_bDebuggingOut = false;
#endif
}

WebCoreClass::~WebCoreClass()
{
	m_ImageCache.saveToDb();
}

void WebCoreClass::enableDebugging(bool state)
{
	m_bDebuggingOut = state;
}

void WebCoreClass::init(const char* appDataPath)
{
	m_szAppDataPath = appDataPath;
	createWebCoreDbTables(appDataPath);

	m_ImageCache.init(appDataPath);
	m_ImageCache.loadFromDb();
}

void WebCoreClass::setUrlDomain(const char* domain)
{
	g_szRootDomain = domain;
	m_szMCFDownloadUrl = gcString("http://api.") + g_szRootDomain + "/api/itemdownloadurl";

	m_bValidateCert = g_szRootDomain == "desura.com";
}

const char* WebCoreClass::getMCFDownloadUrl()
{
	return m_szMCFDownloadUrl.c_str();
}

gcString WebCoreClass::getPassWordReminderUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/api/memberpasswordreminder";
}

gcString WebCoreClass::getLoginUrl()
{
	return gcString("https://secure.") + g_szRootDomain + "/3/memberlogin";
}

gcString WebCoreClass::getRefreshUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/api/refresh";
}

gcString WebCoreClass::getUpdatePollUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/2/updatepoll";
}

gcString WebCoreClass::getNameLookUpUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/api/itemnamelookup";
}

gcString WebCoreClass::getMcfUploadUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/api/itemupload";
}

gcString WebCoreClass::getItemInfoUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/2/item";
}

gcString WebCoreClass::getInstalledWizardUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/2/itemwizard";
}

gcString WebCoreClass::getUpdateAccountUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/api/iteminstall";
}

gcString WebCoreClass::getCDKeyUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/api/cdkey";
}

gcString WebCoreClass::getMemberDataUrl()
{
	return gcString("http://api.") + g_szRootDomain + "/1/memberdata";
}

void WebCoreClass::setCookie(const char* sess)
{
	if (!sess)
		return;

	if (m_szSessCookie == sess)
		return;

	m_szSessCookie = gcString(sess);

	onCookieUpdateEvent();
}

void WebCoreClass::setWCCookies(HttpHandle& hh)
{
	hh->addCookie("freeman", m_szIdCookie.c_str());
	hh->addCookie("masterchief", m_szSessCookie.c_str());
	hh->setUserAgent(getUserAgent());
}

void WebCoreClass::setMCFCookies(MCFCore::Misc::UserCookies* uc)
{
	uc->setUserId(m_uiUserId);
	uc->setId(m_szIdCookie.c_str());
	uc->setSess(m_szSessCookie.c_str());
	uc->setUAgent(getUserAgent());
}

void WebCoreClass::clearNameCache()
{
	try
	{
		sqlite3x::sqlite3_connection db(getWebCoreDb(m_szAppDataPath.c_str()).c_str());
		db.executenonquery("DELETE FROM namecache");
	}
	catch (std::exception &ex)
	{
		Warning(gcString("Failed to clear namecache table: {0}\n", ex.what()));
	}
}

gcString WebCoreClass::getUrl(WebCoreUrl id)
{
	gcString url("http://www.");
	url += g_szRootDomain;

	switch (id)
	{
	case Welcome:
		url += "?firsttime=t";
		break;

	case Cart:
		url = "https://secure." + g_szRootDomain + "/cart";
		break;

	case Purchase:
		url += "/cart/history";
		break;

	case Gifts:
		url += "/cart/gifts";
		break;

	case ActivateGame:
		url += "/cart/gifts/activate";
		break;

	case Games:
		url += "/games";
		break;

	case Mods:
		url += "/mods";
		break;

	case Community:
		url += "/community";
		break;

	case Development:
		url += "/development";
		break;

	case Support:
		url += "/support";
		break;

	case Help:
		url += "/groups/desura/forum";
		break;

	case McfUpload:
		url = gcString("http://api.{0}/1/itemupload", g_szRootDomain);
		break;

	case ThreadWatch:
		url += "/forum/board/thread-watch";
		break;

	case Inbox:
		url += "/messages/inbox";
		break;

	case Updates:
		url += "/messages/updates";
		break;
		
	case LinuxToolHelp:
		url += "/tutorials/linux-tool-help";
		break;
	
	case ListKeys:
		url = "https://secure." + g_szRootDomain + "/cart/cdkeys";
		break;

	case AppChangeLog:
		url += "/app/changelog";
		break;

	case ComplexModTutorial:
		url += "/groups/desura/tutorials/complex-mod-installing";
		break;
	};

	return  url;
}


}


