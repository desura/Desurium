/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>
          (C) Karol Herbst <git@karolherbst.de>
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
#include <branding/branding.h>
#include "TaskBarIcon.h"
#include "MainApp.h"

#include "wx_controls/gcControls.h"

#include "TBI_GameMenu.h"
#ifndef UI_HIDE_MODS
#include "TBI_ModMenu.h"
#endif
#include "TBI_WindowMenu.h"
#include "TBI_UpdateMenu.h"

enum
{
	mcMENU_MODCORE = 1001,
	mcMENU_EXIT,
	mcMENU_LASTMENUID,
};


TaskBarIcon *g_pTaskIcon = NULL;

CONCOMMAND(testpopup, "testpopup")
{
	if (g_pTaskIcon)
		g_pTaskIcon->ShowBalloon("Game Updates Available", "\t- Golden Eye Source\n\t- Over Watch");
}



CONCOMMAND(testapppopup, "testapppopup")
{
	Msg("App update");
	UserCore::Misc::UpdateInfo a(BUILDID_PUBLIC,2);
	GetUserCore()->getAppUpdateEvent()->operator()(a);
		

	gcSleep(2000);

	Msg("App prog update");

	uint32 prog = 32;
	GetUserCore()->getAppUpdateProgEvent()->operator()(prog);

	gcSleep(2000);

	prog = 0;
	GetUserCore()->getAppUpdateProgEvent()->operator()(prog);


	Msg("UserUpdated");

	GetUserCore()->getUserUpdateEvent()->operator()();


	//*GetUserCore()->getItemManager()->getOnUpdateEvent();
	//*GetUserCore()->getItemsAddedEvent();
}

#ifdef DEBUG

CONCOMMAND(testappupdate, "testappupdate")
{
	GetUserCore()->appNeedUpdate(700, 100);
}

#endif

TaskBarIcon::TaskBarIcon(wxWindow *parent) : gcTaskBarIcon()
{
	Bind(wxEVT_TASKBAR_BALLOON_CLICK, &TaskBarIcon::onBallonClick, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &TaskBarIcon::onMenuSelect, this);
	Bind(wxEVT_TASKBAR_LEFT_DCLICK, &TaskBarIcon::onLeftButtonDClick, this);
#ifdef NIX
	Bind(wxEVT_TASKBAR_LEFT_DOWN, &TaskBarIcon::onLeftButtonDown, this);
#endif

	m_wxParent = parent;
	m_szTooltip = PRODUCT_NAME;
	
#ifdef WIN32
	m_wxIcon = wxIcon(wxICON(IDI_ICONSMALL));
	SetIcon(m_wxIcon , m_szTooltip);
#else
	gcWString path(L"{0}/desura.png", UTIL::OS::getDataPath());
	wxIcon i(path, wxBITMAP_TYPE_PNG);
	SetIcon(i, m_szTooltip);	
#endif

	g_pTaskIcon = this;

	tagItems();

	m_iLastBallon = 0;

#ifndef UI_HIDE_MODS
	m_pModMenu = new TBIModMenu();
#endif
	m_pGameMenu = new TBIGameMenu();
	m_pWindowMenu = new TBIWindowMenu();
	m_pUpdateMenu = new TBIUpdateMenu();

	m_uiLastCount = -1;
	m_uiLastProg = 0;
	
	m_imgCount = GetGCThemeManager()->getImageHandle("#count_background");
	updateIcon();	
}

TaskBarIcon::~TaskBarIcon()
{
	g_pTaskIcon = NULL;

#ifndef UI_HIDE_MODS
	safe_delete(m_pModMenu);
#endif
	safe_delete(m_pGameMenu);
	safe_delete(m_pWindowMenu);
	safe_delete(m_pUpdateMenu);
}


void TaskBarIcon::regEvents()
{
	//if we are offline user handle is null
	if (GetUserCore())
	{
		*GetUserCore()->getAppUpdateEvent() += guiDelegate(this, &TaskBarIcon::onAppUpdate);
		*GetUserCore()->getAppUpdateProgEvent() += guiDelegate(this, &TaskBarIcon::onAppUpdateProg);
		*GetUserCore()->getAppUpdateCompleteEvent() += guiDelegate(this, &TaskBarIcon::onAppUpdateComplete);
		*GetUserCore()->getItemManager()->getOnUpdateEvent() += guiDelegate(this, &TaskBarIcon::onUpdate);
		*GetUserCore()->getItemsAddedEvent() += guiDelegate(this, &TaskBarIcon::onItemsAdded);
		*GetUserCore()->getUserUpdateEvent() += guiDelegate(this, &TaskBarIcon::onUserUpdate);
	}
	updateIcon();
}

void TaskBarIcon::deregEvents()
{
	//if we are offline user handle is null
	if (GetUserCore())
	{
		*GetUserCore()->getAppUpdateEvent() -= guiDelegate(this, &TaskBarIcon::onAppUpdate);
		*GetUserCore()->getAppUpdateProgEvent() -= guiDelegate(this, &TaskBarIcon::onAppUpdateProg);
		*GetUserCore()->getAppUpdateCompleteEvent() -= guiDelegate(this, &TaskBarIcon::onAppUpdateComplete);
		*GetUserCore()->getItemManager()->getOnUpdateEvent() -= guiDelegate(this, &TaskBarIcon::onUpdate);
		*GetUserCore()->getItemsAddedEvent() -= guiDelegate(this, &TaskBarIcon::onItemsAdded);
		*GetUserCore()->getUserUpdateEvent() -= guiDelegate(this, &TaskBarIcon::onUserUpdate);
	}
}

void TaskBarIcon::onItemsAdded(uint32&)
{
	tagItems();
	updateIcon();
}

void TaskBarIcon::onUserUpdate()
{
	updateIcon();
}

wxMenu* TaskBarIcon::CreatePopupMenu()
{
	MainApp* temp = dynamic_cast<MainApp*>(m_wxParent);

	gcMenu* menu = new gcMenu();
	gcMenuItem* m_miDesura = new gcMenuItem(menu, mcMENU_MODCORE, wxT(PRODUCT_NAME));
	gcMenuItem* m_miExit = new gcMenuItem(menu, mcMENU_EXIT, Managers::GetString(L"#TB_EXIT"));

	if(!temp || !temp->isLoggedIn())
	{
		menu->Append(m_miDesura);
		menu->Append(m_miExit);
	}
	else
	{
		uint32 lastMenuId = mcMENU_LASTMENUID;

		menu->Append(m_miDesura);
		menu->AppendSeparator();
		menu->Append(-1, m_pGameMenu->getMenuName(), m_pGameMenu->createMenu(lastMenuId));
#ifndef UI_HIDE_MODS
		menu->Append(-1, m_pModMenu->getMenuName(), m_pModMenu->createMenu(lastMenuId));
#endif
		menu->AppendSeparator();
		menu->Append(-1, m_pWindowMenu->getMenuName(), m_pWindowMenu->createMenu(lastMenuId));
		menu->Append(-1, m_pUpdateMenu->getMenuName(), m_pUpdateMenu->createMenu(lastMenuId));
		menu->AppendSeparator();
		menu->Append(m_miExit);
	}

	menu->layoutMenu();

	return menu;
}


void TaskBarIcon::onMenuSelect(wxCommandEvent& event)
{
	if (event.GetId() == mcMENU_EXIT && m_wxParent)
	{
		m_wxParent->Close(true);
	}
	else if (event.GetId() == mcMENU_MODCORE)
	{
		showMainWindow();
	}
	else
	{
#ifndef UI_HIDE_MODS
		m_pModMenu->onMenuSelect(event);
#endif
		m_pGameMenu->onMenuSelect(event);
		m_pWindowMenu->onMenuSelect(event);
		m_pUpdateMenu->onMenuSelect(event);
	}
}

void TaskBarIcon::onLeftButtonDClick(wxTaskBarIconEvent&)
{
	showMainWindow();
}

#ifdef NIX
void TaskBarIcon::onLeftButtonDown(wxTaskBarIconEvent&)
{
	MainApp* temp = dynamic_cast<MainApp*>(m_wxParent);

	if(temp)
		temp->toggleCurrentForm();
}
#endif

void TaskBarIcon::showMainWindow()
{
	MainApp* temp = dynamic_cast<MainApp*>(m_wxParent);

	if(temp)
		temp->showMainWindow(true);
}

