/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>
          (C) Karol Herbst <git@karolherbst.de>
          (C) Mark Chandler <mark@moddb.com>
          (C) Wojciech Zylinski <voitek@boskee.co.uk>

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
#include "MainApp.h"

#include <wx/wx.h>
#include "wx/window.h"

#include "TaskBarIcon.h"
#include "Managers.h"
#include <branding/branding.h>
#include <branding/uicore_version.h>
#include "mcfcore/MCFMain.h"

#include "InternalLink.h"

#include "Log.h"

#include "managers/CVar.h"
#include "MainForm.h"

CVar gc_firsttime("gc_firsttime", "1", CFLAG_USER);
CVar gc_enable_news_popups("gc_enable_news_popups", "1", CFLAG_USER);

bool admin_cb(CVar* var, const char* val)
{
	return (GetUserCore() && GetUserCore()->isAdmin());
}

CVar admin_autoupdate("admin_autoupdate", "0", CFLAG_ADMIN, (CVarCallBackFn)&admin_cb);

CONCOMMAND(shownews, "shownews")
{
	g_pMainApp->showNews();
}


void MainApp::handleInternalLink(const char* link)
{
	gcString strLink(link);
	onInternalLinkStrEvent(strLink);
}

void MainApp::handleInternalLink(DesuraId id, uint8 action, std::vector<std::string> args)
{
	InternalLinkInfo ili;

	ili.action = action;
	ili.id = id;
	ili.args = args;

	onInternalLinkEvent(ili);
}

void MainApp::onInternalStrLink(gcString &link)
{
	if (isOffline())
	{
		gcMessageBox(this->getMainWindow(), Managers::GetString(L"#MF_OFFLINEWARN"), PRODUCT_NAME_CATW(L" Error"));
		return;
	}

	if (this->isLoggedIn())
		m_pInternalLink->handleInternalLink(link.c_str());
	else
		m_szDesuraCache = link;
}

void MainApp::onInternalLink(InternalLinkInfo& info)
{
	DesuraId id = info.id;
	uint8 action = info.action;

	switch (action)
	{
	case ACTION_PROFILE		: showProfile( id, info.args );			break;
	case ACTION_DEVPROFILE	: showDevProfile( id );					break;
	case ACTION_DEVPAGE		: showDevPage( id );					break;
	case ACTION_ACCOUNTSTAT	: changeAccountState(id);				break;
	case ACTION_PLAY		: showPlay();							break;
	case ACTION_SHOWCONSOLE	: showConsole();						break;

	default: 
		m_pInternalLink->handleInternalLink(id, action, info.args);
		break;
	}

	showNews();
}

void MainApp::showConsole()
{
#ifdef WIN32
	ShowLogForm(true);
#endif // LINUX TODO
}

void MainApp::closeForm(int32 wxId)
{
	m_pInternalLink->closeForm(wxId);
}

void MainApp::showPlay()
{
	if (!m_wxMainForm)
		return;

	if (m_wxMainForm->IsIconized())
		m_wxMainForm->Iconize(false);

	m_wxMainForm->showPlay();
	m_wxMainForm->Raise();
}

void MainApp::showPage(PAGE page)
{
	if (!m_wxMainForm)
		return;

	if (m_wxMainForm->IsIconized())
		m_wxMainForm->Iconize(false);

	m_wxMainForm->showPage(page);
	m_wxMainForm->Raise();
}

void MainApp::loadUrl(const char* url, PAGE page)
{
	if (!m_wxMainForm)
	{
		gcLaunchDefaultBrowser(url);
		return;
	}
	
	if (m_wxMainForm->IsIconized())
		m_wxMainForm->Iconize(false);

	m_wxMainForm->loadUrl(url, page);
	m_wxMainForm->Raise();
}



void MainApp::showProfile(DesuraId id, std::vector<std::string> args)
{
	UserCore::Item::ItemInfoI* item = GetUserCore()->getItemManager()->findItemInfo( id );

	if (item && item->getProfile())
	{
		gcString url(item->getProfile());

		if (args.size() > 0)
			url = gcString("{0}{1}", item->getProfile(), args[0]);

		PAGE page = GAMES;
			
#ifndef UI_HIDE_MODS
		if (id.getType() != DesuraId::TYPE_GAME)
			page = MODS;
#endif

		loadUrl(url.c_str(), page);
	}
	else
	{	
		Warning(gcString("Failed to locate item {0} to show profile.\n", id.toInt64()));
	}
}

void MainApp::showDevProfile(DesuraId id)
{
	UserCore::Item::ItemInfoI* item = GetUserCore()->getItemManager()->findItemInfo( id );

	if (item && item->getDevProfile())
	{
		loadUrl(item->getDevProfile(), COMMUNITY);
	}
	else
	{	
		Warning(gcString("Failed to locate item {0} to show dev profile.\n", id.toInt64()));
	}
}

void MainApp::showDevPage(DesuraId id)
{
	UserCore::Item::ItemInfoI* item = GetUserCore()->getItemManager()->findItemInfo( id );

	if (!item)
		return;

	gcString url("http://www.desura.com/{0}/publish/{1}", id.getTypeString(), item->getShortName());
	loadUrl(url.c_str(), DEVELOPMENT);
}

void MainApp::showNews()
{
	if (gc_firsttime.getBool())
	{
		gc_firsttime.setValue(false);
		return;
	}

	// std::vector<T> initialized empty
	std::vector<UserCore::Misc::NewsItem*> news_items_vec;

	m_NewsLock.lock();

	if ( gc_enable_news_popups.getBool())
	{
		// popups allowed so point to list of news items
		news_items_vec = m_vNewsItems;
	}

	m_pInternalLink->showNews(news_items_vec, m_vGiftItems);

	safe_delete(m_vNewsItems);
	m_vNewsItems.resize(0);

	safe_delete(m_vGiftItems);
	m_vGiftItems.resize(0);

	m_NewsLock.unlock();
}

void MainApp::changeAccountState(DesuraId id)
{
	UserCore::Item::ItemInfoI* item = GetUserCore()->getItemManager()->findItemInfo( id );

	if (!item)
	{
		Warning(gcString("Could not find item {0} for account status change!", id.toInt64()));
		return;
	}

	if (item->getStatus() & UserCore::Item::ItemInfoI::STATUS_ONACCOUNT)
		item->removeFromAccount();
	else
		item->addToAccount();
}









