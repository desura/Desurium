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
#include "ItemTabPage.h"
#include "ItemToolBarControl.h"
#include "Managers.h"
#include "MainApp.h"

#include "gcWebHost.h"
#include "gcWebControl.h"

#include "cef_desura_includes/ChromiumBrowserI.h"

::Thread::Mutex m_EventLock;
std::map<gcString, ChromiumDLL::JSObjHandle> g_EventMap;

bool g_bGlobalItemUpdate = false;
bool g_bMapValid = false;

ChromiumDLL::JSObjHandle findEventFunction(const gcString &name, ChromiumDLL::JSObjHandle root)
{
	::Thread::AutoLock al(&m_EventLock);

	if (!g_bMapValid)
		return NULL;

	if (g_EventMap.find(name) != g_EventMap.end())
		return g_EventMap[name];

	if (!root.get() || root->isNull())
		return NULL;

	if (g_EventMap.find("__desura__") == g_EventMap.end())
		g_EventMap["__desura__"] = root->getValue("desura");
	
	if (g_EventMap.find("__events__") == g_EventMap.end())
		g_EventMap["__events__"] = g_EventMap["__desura__"]->getValue("events");

	if (g_EventMap.find("__internal__") == g_EventMap.end())
		g_EventMap["__internal__"] = g_EventMap["__events__"]->getValue("internal");

	ChromiumDLL::JSObjHandle ret = g_EventMap["__internal__"]->getValue(name.c_str());
	g_EventMap[name] = ret;

	return ret;
}

void BrowserUICallback(ChromiumDLL::CallbackI* callback);

class JSCallback : public ChromiumDLL::CallbackI
{
public:
	JSCallback(ChromiumDLL::JavaScriptContextI* context, gcString name, const char* arg1, const char* arg2)
	{
		m_pContext = context;
		m_szName = name;
		m_uiNumArgs = 0;

		if (arg1)
		{
			m_szArg1 = arg1;
			m_uiNumArgs = 1;
		}

		if (arg2)
		{
			m_szArg2 = arg2;
			m_uiNumArgs = 2;
		}	

		if (m_szName == "onItemListUpdated")
			g_bGlobalItemUpdate = true;
	}

	virtual void destroy()
	{
		if (m_pContext)
			m_pContext->destroy();

		delete this;
	}

	virtual void run()
	{
		if (!g_bMapValid)
			return;

		if (m_szName == "onItemUpdate" && g_bGlobalItemUpdate)
			return;

		if (!m_pContext)
			return;

		m_pContext->enter();
		ChromiumDLL::JSObjHandle funct = findEventFunction(m_szName, m_pContext->getGlobalObject());

		if (funct.get())
		{
			ChromiumDLL::JSObjHandle* argv = NULL;

			if (m_uiNumArgs > 0)
				argv = new ChromiumDLL::JSObjHandle[m_uiNumArgs];

			if (m_uiNumArgs >= 1)
				argv[0] = m_pContext->getFactory()->CreateString(m_szArg1.c_str());

			if (m_uiNumArgs >= 2)
				argv[1] = m_pContext->getFactory()->CreateString(m_szArg2.c_str());

			ChromiumDLL::JavaScriptFunctionArgs args;
			args.function = NULL;
			args.context = m_pContext;
			args.argc = m_uiNumArgs;
			args.argv = argv;
			args.factory = NULL;
			args.object = NULL;

			ChromiumDLL::JSObjHandle ret = funct->executeFunction(&args);
			delete [] argv;
		}

		m_pContext->exit();

		if (m_szName == "onItemListUpdated")
			g_bGlobalItemUpdate = false;
	}

private:
	uint32 m_uiNumArgs;

	gcString m_szArg1;
	gcString m_szArg2;

	ChromiumDLL::JavaScriptContextI* m_pContext;
	gcString m_szName;
};




ItemTabPage::ItemTabPage(wxWindow* parent, gcWString homePage) : HtmlTabPage(parent, homePage, ITEMS)
{
	m_bNotifiedOfLowSpace = false;
	m_pItemControlBar = new ItemToolBarControl(parent);
	m_pItemControlBar->onSearchEvent += guiDelegate(this, &ItemTabPage::onSearch);

	m_pWebControl = NULL;

	if (m_pWebControl)
	{
		m_pWebControl->onPageStartEvent -= delegate(&m_pControlBar->onPageStartLoadingEvent);
		m_pWebControl->onPageLoadEvent -= delegate(&m_pControlBar->onPageEndLoadingEvent);

		m_pWebControl->onPageStartEvent += delegate(&m_pItemControlBar->onPageStartLoadingEvent);
		m_pWebControl->onPageLoadEvent += delegate(&m_pItemControlBar->onPageEndLoadingEvent);
	}

	killControlBar();

	m_pItemControlBar->onButtonClickedEvent += guiDelegate(this, &ItemTabPage::onButtonClicked);

	m_EventLock.lock();
	g_bMapValid = true;
	m_EventLock.unlock();
}

ItemTabPage::~ItemTabPage()
{
	m_EventLock.lock();
	g_EventMap.clear();
	g_bMapValid = false;
	m_EventLock.unlock();

	m_pItemControlBar->onButtonClickedEvent -= guiDelegate(this, &ItemTabPage::onButtonClicked);
	m_pItemControlBar->onSearchEvent -= guiDelegate(this, &ItemTabPage::onSearch);

	m_pWebControl->onPageStartEvent -= delegate(&m_pItemControlBar->onPageStartLoadingEvent);
	m_pWebControl->onPageLoadEvent -= delegate(&m_pItemControlBar->onPageEndLoadingEvent);

	if (GetUserCore())
	{
		std::vector<UserCore::Item::ItemInfoI*> aList;
		GetUserCore()->getItemManager()->getAllItems(aList);

		for (size_t x=0; x<aList.size(); x++)
			*aList[x]->getInfoChangeEvent() -= guiDelegate(this, &ItemTabPage::onItemUpdate);

		*GetUserCore()->getLowSpaceEvent() -= guiDelegate(this, &ItemTabPage::onLowDiskSpace);
		*GetUserCore()->getForcedUpdatePollEvent() -= guiDelegate(this, &ItemTabPage::onUpdatePoll);
	}

	if (GetUploadMng())
	{
		size_t count = GetUploadMng()->getCount();

		for (size_t x=0; x<count; x++)
		{
			UserCore::Misc::UploadInfoThreadI* item = GetUploadMng()->getItem(x);

			gcString key = item->getKey();
			*item->getUploadProgressEvent()  -= guiExtraDelegate(this, &ItemTabPage::onUploadProgress, key);
			*item->getActionEvent() -= guiExtraDelegate(this, &ItemTabPage::onUploadAction, key);
		}

		*GetUploadMng()->getUpdateEvent() -= guiDelegate(this, &ItemTabPage::onUploadUpdate);
	}
}

void ItemTabPage::onFind()
{
	if (m_pItemControlBar)
		m_pItemControlBar->focusSearch();
}

void ItemTabPage::postEvent(const char* name, const char* arg1, const char* arg2)
{
	gcWebControl* webCtrl = dynamic_cast<gcWebControl*>(m_pWebControl);

	if (!webCtrl)
		return;

	BrowserUICallback(new JSCallback(webCtrl->getJSContext(), name, arg1, arg2));
}

BaseToolBarControl* ItemTabPage::getToolBarControl()
{
	return m_pItemControlBar;
}

void ItemTabPage::newBrowser(const char* homeUrl)
{
	if (m_pWebControl)
		return;

	gcWebControl* host = new gcWebControl(this, homeUrl);

	m_pWebPanel = host;
	m_pWebControl = host; 
}

void ItemTabPage::constuctBrowser()
{
	HtmlTabPage::constuctBrowser();

	m_pWebControl->onPageStartEvent += delegate(&m_pItemControlBar->onPageStartLoadingEvent);
	m_pWebControl->onPageLoadEvent += delegate(&m_pItemControlBar->onPageEndLoadingEvent);
	m_pWebControl->onPageLoadEvent += delegate(this, &ItemTabPage::doneLoading);

	UserCore::ItemManagerI *im = GetUserCore()->getItemManager();

	if (im)
	{
		*im->getOnUpdateEvent() += guiDelegate(this, &ItemTabPage::onItemsUpdate);
		*im->getOnRecentUpdateEvent() += guiDelegate(this, &ItemTabPage::onRecentUpdate);
		*im->getOnFavoriteUpdateEvent() += guiDelegate(this, &ItemTabPage::onFavoriteUpdate);
		*im->getOnNewItemEvent() += guiDelegate(this, &ItemTabPage::onNewItem);

		onItemsUpdate();
	}

	*GetUserCore()->getLowSpaceEvent() += guiDelegate(this, &ItemTabPage::onLowDiskSpace);
	*GetUserCore()->getForcedUpdatePollEvent() += guiDelegate(this, &ItemTabPage::onUpdatePoll);
	*GetUploadMng()->getUpdateEvent() += guiDelegate(this, &ItemTabPage::onUploadUpdate);
	onUploadUpdate();

	*GetUserCore()->getLoginItemsLoadedEvent() += guiDelegate(this, &ItemTabPage::onLoginItemsLoaded);
}


void ItemTabPage::reset()
{
	postEvent("onTabClicked", "game");
}

void ItemTabPage::onSearchText(const wchar_t* value)
{
	postEvent("onSearch", gcString(value).c_str());
}

void ItemTabPage::onButtonClicked(int32& id)
{ 
	switch (id)
	{
	case BUTTON_EXPAND:
		postEvent("onExpand");
		break;

	case BUTTON_CONTRACT:
		postEvent("onContract");
		break;

	case BUTTON_GAME:
		postEvent("onTabClicked", "game");
		break;
		
	case BUTTON_FAV:
		postEvent("onTabClicked", "fav");
		break;

	case BUTTON_TOOL:
		postEvent("onTabClicked", "tool");
		break;

	case BUTTON_DEV:
		postEvent("onTabClicked", "dev");
		break;
	}
}

void ItemTabPage::doneLoading()
{
	if (GetUserCore()->isDelayLoading())
		postEvent("onDelayLoad");
}

void ItemTabPage::onLoginItemsLoaded()
{
	postEvent("onDelayLoadDone");
	postEvent("onItemListUpdated");
}

void ItemTabPage::onItemsUpdate()
{
	std::vector<UserCore::Item::ItemInfoI*> aList;
	GetUserCore()->getItemManager()->getAllItems(aList);

	for (size_t x=0; x<aList.size(); x++)
		*aList[x]->getInfoChangeEvent() += guiDelegate(this, &ItemTabPage::onItemUpdate);

	postEvent("onItemListUpdated");
}

void ItemTabPage::onRecentUpdate(DesuraId &id)
{
	postEvent("onRecentUpdate", id.toString().c_str());
}

void ItemTabPage::onFavoriteUpdate(DesuraId &id)
{
	postEvent("onFavoriteUpdate", id.toString().c_str());
}

void ItemTabPage::onItemUpdate(UserCore::Item::ItemInfoI::ItemInfo_s& info)
{
	postEvent("onItemUpdate", info.id.toString().c_str(), gcString("{0}", info.changeFlags).c_str());
}

void ItemTabPage::onUploadUpdate()
{
	size_t count = GetUploadMng()->getCount();

	for (size_t x=0; x<count; x++)
	{
		UserCore::Misc::UploadInfoThreadI* item = GetUploadMng()->getItem(x);

		gcString key = item->getKey();

		if (item->isDeleted())
		{
			*item->getUploadProgressEvent()  -= guiExtraDelegate(this, &ItemTabPage::onUploadProgress, key);
			*item->getActionEvent() -= guiExtraDelegate(this, &ItemTabPage::onUploadAction, key);
		}
		else
		{
			*item->getUploadProgressEvent()  += guiExtraDelegate(this, &ItemTabPage::onUploadProgress, key);
			*item->getActionEvent() += guiExtraDelegate(this, &ItemTabPage::onUploadAction, key);
		}
	}

	postEvent("onUploadUpdate", "all");
}

void ItemTabPage::onUploadProgress(gcString hash, UserCore::Misc::UploadInfo& info)
{
	uint32 prog = 0;

	m_UploadMutex.lock();
	prog = m_vUploadProgress[hash];
	m_vUploadProgress[hash] = info.percent;
	m_UploadMutex.unlock();

	if (prog != info.percent)
		postEvent("onUploadProgress", hash.c_str());
}

void ItemTabPage::onUploadAction(gcString hash)
{
	postEvent("onUploadUpdate", hash.c_str());
}

void ItemTabPage::onUpdatePoll()
{
	postEvent("onUpdatePoll", "all");
}

void ItemTabPage::onNewItem(DesuraId &id)
{
	postEvent("onNewItemAdded", id.toString().c_str());
}

void ItemTabPage::onSearch(gcString &text)
{
	postEvent("onSearch", text.c_str());
}

void ItemTabPage::onShowAlert(const gcString &text, uint32 time)
{
	postEvent("onShowAlert", text.c_str(), gcString("{0}", time).c_str());
}

void ItemTabPage::onLowDiskSpace(std::pair<bool,char> &info)
{
	if (m_bNotifiedOfLowSpace)
		return;

	m_bNotifiedOfLowSpace = true;

#ifdef WIN32
	gcString text;
	
	if (info.first)
		text = gcString(Managers::GetString("#IF_LOWSPACE_SYS"), info.second);
	else
		text = gcString(Managers::GetString("#IF_LOWSPACE_CACHE"), info.second);

	onShowAlert(text, 0);
#endif
}
