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
#include "HtmlTabPage.h"
#include "MainApp.h"

#include "gcWebHost.h"
#include "gcWebControl.h"

#include "ItemToolBarControl.h"
#include "usercore/CIPManagerI.h"
#include "gcJSBinding.h"

const char* g_szSearchArea[] =
{
	"",
	"games",
	"mods",
	"community",
	"development",
	"support",
};


DesuraJSBinding *GetJSBinding();

HtmlTabPage::HtmlTabPage(wxWindow* parent, gcString homePage, PAGE area) : baseTabPage( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxNO_BORDER )
{
	m_pWebControl = NULL;
	m_pWebPanel = NULL;

	m_szHomePage = homePage;

	SetBackgroundColour(wxColour( 0, 0, 0 ));

	m_pControlBar = new HtmlToolBarControl(parent);
	m_pControlBar->onSearchEvent += guiDelegate(this, &HtmlTabPage::onSearch);
	m_pControlBar->onFullSearchEvent += guiDelegate(this, &HtmlTabPage::onFullSearch);

	m_pBSBrowserSizer = new wxBoxSizer( wxVERTICAL );

	this->SetSizer( m_pBSBrowserSizer );
	this->Layout();

	m_SearchArea = area;
	m_pControlBar->onButtonClickedEvent += guiDelegate(this, &HtmlTabPage::onButtonClicked);

	GetUserCore()->getCIPManager()->getItemsUpdatedEvent() += guiDelegate(this, &HtmlTabPage::onCIPUpdate);
}

HtmlTabPage::~HtmlTabPage()
{
	if (m_pControlBar)
	{
		m_pControlBar->onButtonClickedEvent -= guiDelegate(this, &HtmlTabPage::onButtonClicked);
		m_pControlBar->onSearchEvent -= guiDelegate(this, &HtmlTabPage::onSearch);
	}

	if (m_pWebControl)
	{
		m_pWebControl->onNewURLEvent -= delegate(this, &HtmlTabPage::onNewUrl);
		m_pWebControl->onPageLoadEvent -= guiDelegate(this, &HtmlTabPage::onPageLoad);

		if (m_pControlBar)
		{
			m_pWebControl->onPageStartEvent -= delegate(&m_pControlBar->onPageStartLoadingEvent);
			m_pWebControl->onPageLoadEvent -= delegate(&m_pControlBar->onPageEndLoadingEvent);
		}

		m_pWebControl->onClearCrumbsEvent -= guiDelegate(this, &HtmlTabPage::clearCrumbs);
		m_pWebControl->onAddCrumbEvent -= guiDelegate(this, &HtmlTabPage::addCrumb);
		m_pWebControl->onFindEvent -= guiDelegate(this, &HtmlTabPage::onFind);
	}

	if (GetUserCore())
		GetUserCore()->getCIPManager()->getItemsUpdatedEvent() -= guiDelegate(this, &HtmlTabPage::onCIPUpdate);
}

void HtmlTabPage::onFind()
{
	if (m_pControlBar)
		m_pControlBar->focusSearch();
}

void HtmlTabPage::killControlBar()
{
	m_pControlBar->onButtonClickedEvent -= guiDelegate(this, &HtmlTabPage::onButtonClicked);
	m_pControlBar->onSearchEvent -= guiDelegate(this, &HtmlTabPage::onSearch);
	m_pControlBar->onFullSearchEvent -= guiDelegate(this, &HtmlTabPage::onFullSearch);

	m_pControlBar->Destroy();
	m_pControlBar = NULL;
}

BaseToolBarControl* HtmlTabPage::getToolBarControl()
{
	return m_pControlBar;
}

void HtmlTabPage::newBrowser(const char* homeUrl)
{
	if (m_pWebControl)
		return;

#ifdef ENABLE_OUTOFPROCESS_BROWSER
	if (m_SearchArea == GAMES)
	{
		gcWebHost* host = new gcWebHost(this, homeUrl, g_szSearchArea[m_SearchArea]);

		m_pWebPanel = host;
		m_pWebControl = host; 
	}
	else
#endif
	{
		gcWebControl* host = new gcWebControl(this, homeUrl);

		m_pWebPanel = host;
		m_pWebControl = host; 
	}
}

void HtmlTabPage::constuctBrowser()
{
	if (m_pWebControl)
		return;

	newBrowser(m_szHomePage.c_str());
	m_pWebPanel->Show(true);

	m_pWebControl->onNewURLEvent += delegate(this, &HtmlTabPage::onNewUrl);
	m_pWebControl->onPageLoadEvent += guiDelegate(this, &HtmlTabPage::onPageLoad);

	if (m_pControlBar)
	{
		m_pWebControl->onPageStartEvent += delegate(&m_pControlBar->onPageStartLoadingEvent);
		m_pWebControl->onPageLoadEvent += delegate(&m_pControlBar->onPageEndLoadingEvent);
	}

	m_pWebControl->onClearCrumbsEvent += guiDelegate(this, &HtmlTabPage::clearCrumbs);
	m_pWebControl->onAddCrumbEvent += guiDelegate(this, &HtmlTabPage::addCrumb);
	m_pWebControl->onFindEvent += guiDelegate(this, &HtmlTabPage::onFind);

	m_pBSBrowserSizer->Add( m_pWebPanel, 1, wxEXPAND, 5 );	

	Layout();
	Refresh();

	m_pWebControl->forceResize();
}

void HtmlTabPage::setSelected(bool state)
{
	if (state && !m_pWebControl)
		constuctBrowser();

	baseTabPage::setSelected(state);
}

void HtmlTabPage::setCurUrl(const char* page)
{
	if (!page)
	{
		goHome();
		return;
	}

	if (!m_pWebControl)
	{
		gcString home = m_szHomePage;
		m_szHomePage = page;
		constuctBrowser();
		m_szHomePage = home;
	}
	else
	{
		m_pWebControl->loadUrl(page);
	}
}

void HtmlTabPage::goHome()
{
	setCurUrl(m_szHomePage.c_str());
}

void HtmlTabPage::onButtonClicked(int32& id)
{ 
	switch (id)
	{
	case BUTTON_HOME:
		goHome();
		break;

	case BUTTON_STOP:
		m_pWebControl->stop();
		break;

	case BUTTON_REFRESH:
		m_pWebControl->refresh();
		break;

	case BUTTON_BACK:
		m_pWebControl->back();
		break;

	case BUTTON_FOWARD:
		m_pWebControl->forward();
		break;

	case BUTTON_LAUNCH:
		gcLaunchDefaultBrowser(m_szCurUrl.c_str());
		break;
	};

	if (m_pControlBar)
	{
		const wchar_t* url = m_pControlBar->getCrumbUrl(id);

		if (url)
			m_pWebControl->loadUrl(url);
	}
}

void HtmlTabPage::clearCrumbs()
{
	if (m_pControlBar)
		m_pControlBar->clearCrumbs();
}

void HtmlTabPage::addCrumb(Crumb& curmb)
{
	if (m_pControlBar)
		m_pControlBar->addCrumb(curmb.name.c_str(), curmb.url.c_str());
}

void HtmlTabPage::onPageLoad()
{
	if (m_szCurUrl != "" && !m_pWebPanel->IsShown())
	{
		m_pWebPanel->Show();
		Layout();
	}

	m_pWebControl->executeJScript("desura.internal.checkOldCrumbs();");
	m_pWebControl->executeJScript("desura.internal.getUpdateCounts();");
}

void HtmlTabPage::loadUrl(const char* url)
{
	setCurUrl(url);
}

void HtmlTabPage::onNewUrl(newURL_s& info)
{
	if (info.stop)
		return;

	gcString url(info.url);

	if (strncmp(url.c_str(), "desura://", 9) == 0)
	{
		info.stop = true;
		g_pMainApp->handleInternalLink(url.c_str());
	}

	if (!info.main)
		return;

	if (strncmp(url.c_str(), "javascript:", 11) == 0)
	{
		//dont do any thing
	}
	else if (strncmp(url.c_str(), "wyciwyg:", 8) == 0)
	{
		//dont do any thing
	}
	else if (isSafeUrl(url.c_str()))
	{
		m_szCurUrl = gcWString(info.url);
	}
	else
	{
		info.stop = true;
		gcLaunchDefaultBrowser(info.url);
	}
}

int HtmlTabPage::getId()
{
	return this->GetId();
}

void HtmlTabPage::onCIPUpdate()
{
	if (m_pWebControl)
		m_pWebControl->executeJScript("desura.events.internal.onCIPListUpdate()");
}

void HtmlTabPage::onSearch(gcString &text)
{
	if (m_pWebControl)
		m_pWebControl->executeJScript(gcString("if(typeof onDesuraSearch == 'function'){onDesuraSearch('{0}', '{1}');}", text, g_szSearchArea[m_SearchArea]).c_str());
}

void HtmlTabPage::onFullSearch(gcString &text)
{
	if (m_pWebControl)
		m_pWebControl->executeJScript(gcString("if(typeof onDesuraFullSearch == 'function'){onDesuraFullSearch('{0}', '{1}');}", text, g_szSearchArea[m_SearchArea]).c_str());
}
