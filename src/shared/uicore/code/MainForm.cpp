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
#include "MainForm.h"

#include "MainMenuButton.h"
#include "mcfcore/MCFI.h"
#include "MainApp.h"
#include "Color.h"

#include "MainFormCustomFrame.h"
#include "MainFormLeftBorder.h"

#include "HtmlTabPage.h"
#include "ItemTabPage.h"
#include "AboutForm.h"
#include "Log.h"

#include "Managers.h"
#include "DesuraControl.h"

#include "../../../branding/branding.h"


WebCore::WebCoreUrl g_uiUrlList[] =
{
	(WebCore::WebCoreUrl)-1,
	WebCore::Games,
#ifndef UI_HIDE_MODS
	WebCore::Mods,
#endif
	WebCore::Community,
	WebCore::Development,
	WebCore::Support,
	(WebCore::WebCoreUrl)-1,
};

const char* g_szButtonNames[] =
{
	"#MP_PLAY",
	"#MP_GAMES",
#ifndef UI_HIDE_MODS
	"#MP_MODS",
#endif
	"#MP_COMMUNITY",
	"#MP_DEVELOPMENT",
	"#MP_SUPPORT",
};

const bool g_bLoadDefault[] =
{
	true,
	true,
#ifdef WIN32
	true,
#endif
	false,
	false,
	false,
	true,
};

CONCOMMAND(cc_logout, "logout")
{
	g_pMainApp->logOut(true);
}

bool Changed(CVar* var, const char* val);

extern CVar gc_destroymain;
extern CVar gc_noloadtab;

#ifdef NIX
static CVar gc_allow_wm_positioning("gc_allow_wm_positioning", "true");
#endif

#ifdef NIX64
MainForm::MainForm(wxWindow* parent, bool offline) : gcFrame(parent, wxID_ANY, wxT(PRODUCT_NAME " 64"), wxDefaultPosition, wxSize(990, 690), wxDEFAULT_FRAME_STYLE, true )
#else
MainForm::MainForm(wxWindow* parent, bool offline) : gcFrame(parent, wxID_ANY, wxT(PRODUCT_NAME), wxDefaultPosition, wxSize(990, 690), wxDEFAULT_FRAME_STYLE, true )
#endif
{
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainForm::onMenuSelect, this);
	Bind(wxEVT_CLOSE_WINDOW, &MainForm::onFormClose, this);
	Bind(wxEVT_ICONIZE, &MainForm::onIconize, this);

	SetMinSize( wxSize(700,370) );

	m_iMode = MODE_UNINT;
	m_wxAboutForm = NULL;	

	m_pDesuraControl = new DesuraControl(this, offline);

	wxBoxSizer *fgSizer1 = new wxBoxSizer(wxVERTICAL);
	fgSizer1->Add( m_pDesuraControl, 1, wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	initPages(offline);
	setTitle(offline);

	loadFrame(wxDEFAULT_FRAME_STYLE);
	setupPositionSave("main", true, 990, 690);
}

MainForm::~MainForm()
{
}

void MainForm::initPages(bool offline)
{
	const char* url = GetGCThemeManager()->getWebPage("playlist");

	ItemTabPage *itemPage = new ItemTabPage(m_pDesuraControl, url);
	itemPage->constuctBrowser();

	m_vPageList.push_back(itemPage);
	m_pDesuraControl->addPage(itemPage, Managers::GetString(g_szButtonNames[ITEMS]));

	if (!offline)
	{
		for (size_t x=GAMES; x<END_PAGE; x++)
		{
			gcWString url = GetWebCore()->getUrl(g_uiUrlList[x]);
			HtmlTabPage* page = new HtmlTabPage(m_pDesuraControl, url, (PAGE)x);
			m_vPageList.push_back(page);

			if (!gc_noloadtab.getBool() && g_bLoadDefault[x])
				page->constuctBrowser();

			page->Layout();
#ifdef NIX
			page->Show(true);
#endif
			m_pDesuraControl->addPage(page, Managers::GetString(g_szButtonNames[x]));
		}

		//load default pages
		m_pDesuraControl->setActivePage(GAMES);
		m_pDesuraControl->setActivePage(ITEMS);
#ifndef UI_HIDE_MODS
		m_pDesuraControl->setActivePage(MODS);
#endif
	}

	if (offline)
		m_pDesuraControl->setActivePage(ITEMS);
	else
		m_pDesuraControl->setActivePage(GAMES);
}

void MainForm::setTitle(bool offline)
{
	if (offline)
	{
		SetTitle( Managers::GetString(L"#MP_TITLE_OFFLINE") );
	}
	else
	{

#ifdef DEBUG
	const char* title = PRODUCT_NAME " Debug";
#else
	#ifdef WIN32
		const char* title = PRODUCT_NAME " Windows";
	#else
		#ifdef NIX64
			const char* title = PRODUCT_NAME " Linux 64";
		#else
			const char* title = PRODUCT_NAME " Linux";
		#endif
	#endif
#endif

		if (GetUserCore())
			SetTitle(gcString("{0}: {1}", title, GetUserCore()->getUserName()));
		else
			SetTitle(title);
	}
}

void MainForm::loadFrame(long style)
{
	if (!getCustomFrame())
	{
		gcMainCustomFrameImpl* mcf = new gcMainCustomFrameImpl(this, this, style);
		
		m_pDesuraControl->regCustomFrame(mcf);
		init(mcf);
	}

#ifdef NIX
	Raise();
#endif
}

void MainForm::showPlay()
{
	showPage(ITEMS);
}


void MainForm::showPage(PAGE page)
{
	m_pDesuraControl->setActivePage(page, true);
}

void MainForm::loadUrl(const char* url, PAGE page)
{
	if (g_pMainApp && g_pMainApp->isOffline())
	{
		gcLaunchDefaultBrowser(url);
		return;
	}

	if (page < 0 || page >= (int32)m_vPageList.size())
	{
		Warning(gcString("Failed to load url [{0}] for page {1}. Cant find page.\n", url, page));
	}
	else
	{
		HtmlTabPage *ph = dynamic_cast<HtmlTabPage*>(m_vPageList[page]);

		if (ph)
			ph->loadUrl(url);

		m_pDesuraControl->setActivePage(page);
	}
}

void MainForm::Exit()
{
	Show(false);
	GetParent()->Close(true);
}

void MainForm::Maximize(bool state)
{
	gcFrame::Maximize(state);
	m_pDesuraControl->showLeftBorder(!state);
}

void MainForm::setMode(uint8 mode)
{
	if (mode == m_iMode)
		return;

	if (mode == MODE_ONLINE || mode == MODE_OFFLINE)
	{
#ifdef NIX
		if (!gc_allow_wm_positioning.getBool())
		{
#endif
			enablePositionSave();
			loadSavedWindowPos();
#ifdef NIX
		}
#endif
		m_pDesuraControl->refreshSearch();
	}

	//need to set mode after other wise cvar gets the wrong value
	m_iMode = mode;
}

void MainForm::onFormClose(wxCloseEvent& event)
{
	if (m_wxAboutForm)
		m_wxAboutForm->EndModal(0);

	if (gc_destroymain.getBool())
	{
		MainApp* par = dynamic_cast<MainApp*>(GetParent());

		if (par)
			par->closeMainForm();	
	}
	else if (event.CanVeto())
	{
		Show(false);
		event.Veto();
	}
	else
	{
		event.Skip();
	}
}

void MainForm::onMenuSelect(wxCommandEvent& event )
{
	switch (event.GetId())
	{
	case DESURA_wxLog:
		ShowLogForm(true);
		break;

	case DESURA_wxSettings:
		g_pMainApp->handleInternalLink(0, ACTION_SHOWSETTINGS);
		break;

	case DESURA_wxAbout:
		showAbout();
		break;

	case DESURA_wxHelp:
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Help).c_str(), SUPPORT);
		break;

	case DESURA_wxExit:
		Exit();
		break;

	case DESURA_wxLogOut:
		logOut();
		break;

	case DESURA_wxOffline:
		if (m_iMode == MODE_OFFLINE)
			logOut();
		else
			goOffline();
		break;

	case DESURA_wxModWizard:
		g_pMainApp->handleInternalLink(0, ACTION_INSTALLEDW);
		break;

	case DESURA_wxChangeLog:
		showChangeLog();
		break;

	case DESURA_wxCustomInstallPath:
		g_pMainApp->handleInternalLink(0, ACTION_SHOWSETTINGS, FormatArgs("tab=cip"));
		break;

	case DESURA_wxListKeys:
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::ListKeys).c_str(), SUPPORT);
		break;

	case DESURA_wxCart:
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Cart).c_str(), COMMUNITY);
		break;

	case DESURA_wxPurchase:
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Purchase).c_str(), COMMUNITY);
		break;

	case DESURA_wxGifts:
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Gifts).c_str(), COMMUNITY);
		break;

	case DESURA_wxActivateGame:
		g_pMainApp->handleInternalLink(0, ACTION_SHOWSETTINGS, FormatArgs("tab=games"));
		break;
	}
}

void MainForm::showChangeLog()
{
	uint32 version = BUILDID_PUBLIC;

	std::string szAppid = UTIL::OS::getConfigValue(APPID);

	if (szAppid.size() > 0)
		version = atoi(szAppid.c_str());

	g_pMainApp->handleInternalLink(DesuraId(version,0), ACTION_APPUPDATELOG);
}

void MainForm::logOut()
{
	g_pMainApp->logOut(true, (m_iMode == MODE_OFFLINE) );
}

void MainForm::showAbout()
{
	if (!m_wxAboutForm)
		m_wxAboutForm = new AboutForm(this);

	m_wxAboutForm->Show(true);
	m_wxAboutForm->Raise();
}

void MainForm::goOffline()
{
	MainApp* par = dynamic_cast<MainApp*>(GetParent());

	if (par)
		par->goOffline();
}

void MainForm::onSubFormClose(uint32 formId)
{
	wxWindow* win = NULL;

	if (m_wxAboutForm && m_wxAboutForm->GetId() == (int)formId)
	{
		win = m_wxAboutForm;
		m_wxAboutForm = NULL;
	}

	if (!win)
		return;

	win->Show(false);
	win->Destroy();
	win = NULL;
}

void MainForm::onIconize(wxIconizeEvent& event )
{
	event.StopPropagation();
}

void MainForm::forceRaise()
{
#ifdef WIN32
	DWORD nForeThread = GetWindowThreadProcessId(GetForegroundWindow(), 0);
	DWORD nAppThread = GetCurrentThreadId();

	HWND hwnd = (HWND)GetHWND();

	if (nForeThread != nAppThread)
	{
		AttachThreadInput(nForeThread, nAppThread, true);
		BringWindowToTop(hwnd);
		AttachThreadInput(nForeThread, nAppThread, false);
	}
	else
	{
		BringWindowToTop(hwnd);
	}
#else
	Raise();
#endif

	Show(true);
}
