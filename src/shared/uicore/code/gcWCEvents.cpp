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
#include "gcWCEvents.h"
#include "gcWebControlI.h"

#ifdef WIN32
	#include "wx/msw/dc.h"
#endif

#ifdef MACOS
	#include "wx/osx/dc.h"
#endif

#ifdef NIX
	#include <linux/input.h>
#endif

#include "wx_controls/gcCustomMenu.h"
#include "managers/CVar.h"

#include "wx_controls/gcManagers.h"
#include "wx_controls/gcControls.h"

#ifdef DEBUG
	CVar gc_enablebrowserconsole("gc_enablebrowserconsole", "1", CFLAG_USER);
	CVar gc_browserdebug("gc_browserdebug", "1", CFLAG_USER);
#else
	CVar gc_enablebrowserconsole("gc_enablebrowserconsole", "0", CFLAG_USER);
	CVar gc_browserdebug("gc_browserdebug", "0", CFLAG_USER);
#endif

enum Modifiers 
{
	// modifiers for all events:
	ShiftKey         = 1 << 0,
	ControlKey       = 1 << 1,
	AltKey           = 1 << 2,
	MetaKey          = 1 << 3,

	// modifiers for keyboard events:
	IsKeyPad         = 1 << 4,
	IsAutoRepeat     = 1 << 5,

	// modifiers for mouse events:
	LeftButtonDown   = 1 << 6,
	MiddleButtonDown = 1 << 7,
	RightButtonDown  = 1 << 8,
};

typedef struct
{
	uint32 key;
	uint32 action;
	uint32 modifiers;
} ShortCut_s;

#ifdef WIN32

const uint32 g_uiMaxKeys = 24;

const ShortCut_s g_ShortCutList[] = 
{
	{VK_LEFT,		IDC_BACK,						AltKey},
	//{VK_BACK,		IDC_BACK,						0},			//re enable when i find out how to find what thing has focus
	{VK_RIGHT,		IDC_FORWARD,					AltKey},
	//{VK_BACK,		IDC_FORWARD,					ShiftKey},
	{VK_BROWSER_BACK,		IDC_BACK,				0},
	{VK_BROWSER_FORWARD,	IDC_FORWARD,			0},
	{'I',			IDC_DEV_TOOLS,					ShiftKey|ControlKey},
	{'J',			IDC_DEV_TOOLS_CONSOLE,			ShiftKey|ControlKey},
	{'P',			IDC_PRINT,						ControlKey},
	{'R',			IDC_RELOAD,						ControlKey},
	{'R',			IDC_RELOAD_IGNORING_CACHE,		ShiftKey|ControlKey},
	{VK_F5,			IDC_RELOAD,						0},
	{VK_F5,			IDC_RELOAD_IGNORING_CACHE,		ControlKey},
	{VK_F5,			IDC_RELOAD_IGNORING_CACHE,		ShiftKey},
	{VK_HOME,		IDC_HOME,						AltKey},
	{VK_ESCAPE,		IDC_STOP,						0},
	{'U',			IDC_VIEW_SOURCE,				ControlKey},
	{VK_OEM_MINUS,	IDC_ZOOM_MINUS,					ControlKey},
	{VK_OEM_MINUS,	IDC_ZOOM_MINUS,					ShiftKey|ControlKey},
	{VK_SUBTRACT,	IDC_ZOOM_MINUS,					ControlKey},
	{'0',			IDC_ZOOM_NORMAL,				ControlKey},
	{VK_NUMPAD0,	IDC_ZOOM_NORMAL,				ControlKey},
	{VK_OEM_PLUS,	IDC_ZOOM_PLUS,					ControlKey},
	{VK_OEM_PLUS,	IDC_ZOOM_PLUS,					ShiftKey|ControlKey},
	{VK_ADD,		IDC_ZOOM_PLUS,					ControlKey},
	{'F',			IDC_FIND,						ControlKey},
};
#elif defined MACOS
const uint32 g_uiMaxKeys = 0;

const ShortCut_s g_ShortCutList[] = 
{

};
#else
const uint32 g_uiMaxKeys = 23;

const ShortCut_s g_ShortCutList[] = 
{
	{KEY_LEFT,		IDC_BACK,						AltKey},
	//{KEY_BACK,	IDC_BACK,						0},			//re enable when i find out how to find what thing has focus
	{KEY_RIGHT,		IDC_FORWARD,					AltKey},
	//{KEY_BACK,	IDC_FORWARD,					ShiftKey},
	{KEY_BACK,		IDC_BACK,						0},
	{KEY_FORWARD,	IDC_FORWARD,					0},
	{'I',			IDC_DEV_TOOLS,					ShiftKey|ControlKey},
	{'J',			IDC_DEV_TOOLS_CONSOLE,			ShiftKey|ControlKey},
//	{'P',			IDC_PRINT,						ControlKey}, // printing disabled on Linux
	{'R',			IDC_RELOAD,						ControlKey},
	{'R',			IDC_RELOAD_IGNORING_CACHE,		ShiftKey|ControlKey},
	{KEY_F5,		IDC_RELOAD,						0},
	{KEY_F5,		IDC_RELOAD_IGNORING_CACHE,		ControlKey},
	{KEY_F5,		IDC_RELOAD_IGNORING_CACHE,		ShiftKey},
	{KEY_HOME,		IDC_HOME,						AltKey},
	{KEY_ESC,		IDC_STOP,						0},
	{'U',			IDC_VIEW_SOURCE,				ControlKey},
	{KEY_MINUS,		IDC_ZOOM_MINUS,					ControlKey},
	{KEY_ZOOMOUT,	IDC_ZOOM_MINUS,					0},
	{KEY_KPMINUS,	IDC_ZOOM_MINUS,					ControlKey},
	{'0',			IDC_ZOOM_NORMAL,				ControlKey},
	{KEY_KP0,		IDC_ZOOM_NORMAL,				ControlKey},
	{KEY_EQUAL,		IDC_ZOOM_MINUS,					ControlKey},
	{KEY_ZOOMIN,	IDC_ZOOM_MINUS,					0},
	{KEY_KPPLUS,	IDC_ZOOM_MINUS,					ControlKey},
	{'F',			IDC_FIND,						ControlKey},
};
#endif

EventHandler::EventHandler(gcWebControlI* parent)
{
	m_pParent = parent;
}

EventHandler::~EventHandler()
{
}

bool EventHandler::onNavigateUrl(const char* url, bool isMain)
{
	newURL_s nu;

	nu.stop = false;
	nu.url = url;
	nu.main = isMain;

	m_pParent->onNewURLEvent(nu);

	if (!nu.stop)
		m_pParent->onPageStartEvent();

	return !nu.stop;
}

void EventHandler::onPageLoadStart()
{
	m_pParent->onPageStartEvent();
}

void EventHandler::onPageLoadEnd()
{
	m_pParent->onAnyPageLoadEvent();
}

bool EventHandler::onJScriptAlert(const char* msg)
{

	return false;
}

bool EventHandler::onJScriptConfirm(const char* msg, bool* result)
{
	return false;
}

bool EventHandler::onJScriptPrompt(const char* msg, const char* defualtVal, bool* handled, char result[255])
{
	return false;
}

bool EventHandler::onKeyEvent(ChromiumDLL::KeyEventType type, int code, int modifiers, bool isSystemKey)
{
	if (type == ChromiumDLL::KEYEVENT_RAWKEYDOWN)
	{
		for (size_t x=0; x<g_uiMaxKeys; x++)
		{
			if (g_ShortCutList[x].key == (uint32)code && HasAllFlags(modifiers, g_ShortCutList[x].modifiers))
			{
				m_pParent->handleKeyEvent(g_ShortCutList[x].action);
				return true;
			}
		}
	}

	return false;
}

void EventHandler::onLogConsoleMsg(const char* message, const char* source, int line)
{
	if (gc_enablebrowserconsole.getBool())
		Warning(gcString("Webkit: {0} [{1} : {2}]\n", message, source, line));
}

void EventHandler::launchLink(const char* url)
{
	if (url)
		gcLaunchDefaultBrowser(url);
}

bool EventHandler::onLoadError(const char* errorMsg, const char* url, char* buff, size_t size)
{
	gcString errorpage = GetGCThemeManager()->getWebPage("error");

	size_t fileStart = errorpage.find("file:///");

	if (fileStart != gcString::npos)
		errorpage.replace(fileStart, 8, "");

	if (!UTIL::FS::isValidFile(UTIL::FS::PathWithFile(errorpage)))
		return false;

	char* fileBuff = NULL;

	try
	{
		UTIL::FS::readWholeFile(UTIL::FS::PathWithFile(errorpage), &fileBuff);
	}
	catch (gcException &)
	{
		safe_delete(fileBuff);
		return false;
	}

	gcString outFile = fileBuff;
	safe_delete(fileBuff);


	size_t errorMsgPos = outFile.find("[ERROR_MSG]");
	while (errorMsgPos != gcString::npos)
	{
		outFile.replace(errorMsgPos, 11, errorMsg);
		errorMsgPos = outFile.find("[ERROR_MSG]", errorMsgPos+1);
	}

	size_t errorUrlPos = outFile.find("[ERROR_URL]");
	while (errorUrlPos != gcWString::npos)
	{
		outFile.replace(errorUrlPos, 11, url);
		errorUrlPos = outFile.find("[ERROR_URL]", errorUrlPos+1);
	}

	size_t count = outFile.size();

	if (count > size)
		count = size-1;

	Safe::strcpy(buff, size, outFile.c_str());
	buff[count] = 0;
	return true;
}


void EventHandler::HandleWndProc(int message, int wparam, int lparam)
{
}

bool EventHandler::HandlePopupMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo)
{
	int x,y;
	menuInfo->getMousePos(&x, &y);

	gcMenu *menu = createMenu(menuInfo);

	if (menu->GetMenuItemCount() > 0)
	{
		menu->layoutMenu();
		displayMenu(menuInfo, menu, x, y);
	}

	safe_delete(menu);
	return true;
}

gcMenu* EventHandler::createMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo)
{
	gcMenu *menu = new gcMenu();
	gcMenu *custom = NULL;

	if (menuInfo->getCustomCount() > 0)
	{
		for (int x=0; x<menuInfo->getCustomCount(); x++)
		{
			ChromiumDLL::ChromiumMenuItemI* item = menuInfo->getCustomItem(x);

			if (!item || !item->getLabel())
				continue;

			if (!custom)
				custom = new gcMenu();

			gcMenuItem *menuItem = NULL;

			switch (item->getType())
			{
			case ChromiumDLL::ChromiumMenuItemI::MENUITEM_OPTION:
				menuItem = new gcMenuItem(custom, (item->getAction() + MENU_ID_CUSTOMACTION), item->getLabel());
				break;

			case ChromiumDLL::ChromiumMenuItemI::MENUITEM_CHECKABLEOPTION:
				menuItem = new gcMenuItem(custom, (item->getAction() + MENU_ID_CUSTOMACTION), item->getLabel(), L"", wxITEM_CHECK);
				break;

			case ChromiumDLL::ChromiumMenuItemI::MENUITEM_SEPERATOR:
				custom->AppendSeparator();
				break;

			case ChromiumDLL::ChromiumMenuItemI::MENUITEM_GROUP:
				break;
			}

			if (menuItem)
			{
				custom->Append(menuItem);

				menuItem->Enable(item->isEnabled());

				if (menuItem->IsCheckable())
					menuItem->Check(item->isChecked());
			}
		}
	}

	if (HasAnyFlags(menuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_EDITABLE))
	{
		gcMenuItem *undo	= new gcMenuItem(menu, MENU_ID_UNDO, Managers::GetString(L"#CHROME_UNDO"));
		gcMenuItem *redo	= new gcMenuItem(menu, MENU_ID_REDO, Managers::GetString(L"#CHROME_REDO"));
		gcMenuItem *cut		= new gcMenuItem(menu, MENU_ID_CUT, Managers::GetString(L"#CHROME_CUT"));
		gcMenuItem *copy	= new gcMenuItem(menu, MENU_ID_COPY, Managers::GetString(L"#CHROME_COPY"));
		gcMenuItem *paste	= new gcMenuItem(menu, MENU_ID_PASTE, Managers::GetString(L"#CHROME_PASTE"));
		gcMenuItem *del		= new gcMenuItem(menu, MENU_ID_DELETE, Managers::GetString(L"#CHROME_DELETE"));
		gcMenuItem *sall	= new gcMenuItem(menu, MENU_ID_SELECTALL, Managers::GetString(L"#CHROME_SELECTALL"));

		menu->Append(undo);
		menu->Append(redo);
		menu->AppendSeparator();
		menu->Append(cut);
		menu->Append(copy);
		menu->Append(paste);
		menu->Append(del);
		menu->AppendSeparator();
		menu->Append(sall);

		undo->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_UNDO));
		redo->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_REDO));
		cut->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_CUT));
		copy->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_COPY));
		paste->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_PASTE));
		del->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_DELETE));
		sall->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_SELECT_ALL));
	}
	else
	{
		if (HasAnyFlags(menuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_SELECTION))
		{
			gcMenuItem *copy	= new gcMenuItem(menu, MENU_ID_COPY, Managers::GetString(L"#CHROME_COPY"));

			menu->Append(copy);

			copy->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_COPY));
		}
		else if (HasAnyFlags(menuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_PAGE|ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_FRAME))
		{
			gcMenuItem *back	= new gcMenuItem(menu, MENU_ID_BACK, Managers::GetString(L"#CHROME_BACK"));
			gcMenuItem *forward	= new gcMenuItem(menu, MENU_ID_FORWARD, Managers::GetString(L"#CHROME_FORWARD"));

			menu->Append(back);
			menu->Append(forward);
			menu->Append(MENU_ID_RELOAD, Managers::GetString(L"#CHROME_RELOAD"));
			menu->AppendSeparator();

			menu->Append(MENU_ID_ZOOM_PLUS, Managers::GetString(L"#CHROME_ZOOMIN"));
			menu->Append(MENU_ID_ZOOM_MINUS, Managers::GetString(L"#CHROME_ZOOMOUT"));
			menu->Append(MENU_ID_ZOOM_NORMAL, Managers::GetString(L"#CHROME_ZOOMNORMAL"));
			menu->AppendSeparator();

#ifndef NIX
			menu->Append(MENU_ID_PRINT, Managers::GetString(L"#CHROME_PRINT"));
#endif // LINUX TODO
			menu->Append(MENU_ID_VIEWSOURCE, Managers::GetString(L"#CHROME_VIEWSOURCE"));

#ifndef NIX
			menu->AppendSeparator();
			menu->Append(MENU_ID_VIEWPBROWSER, Managers::GetString(L"#CHROME_VIEWPAGE"));
#endif // LINUX TODO

			back->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_GO_BACK));
			forward->Enable(HasAnyFlags(menuInfo->getEditFlags(), ChromiumDLL::ChromiumMenuInfoI::MENU_CAN_GO_FORWARD));
		}

#ifndef NIX	
		if (HasAnyFlags(menuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_LINK))
		{
			menu->Append(MENU_ID_VIEWLBROWSER, Managers::GetString(L"#CHROME_VIEWLINK"));
		}

		if (HasAnyFlags(menuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_IMAGE))
		{
			menu->Append(MENU_ID_VIEWIBROWSER, Managers::GetString(L"#CHROME_VIEWIMAGE"));
		}
#endif // LINUX TODO

		if (gc_browserdebug.getBool())
		{
			menu->AppendSeparator();
			menu->Append(MENU_ID_INSPECTELEMENT, Managers::GetString(L"#CHROME_INSPECTELEMENT"));
		}
	}

	if (custom)
	{
		if (custom->GetMenuItemCount() > 0)
		{
			if (menu->GetMenuItemCount() > 0)
			{
				custom->AppendSeparator();
				custom->AppendSubMenu(menu, Managers::GetString(L"#CHROME_NORMALMENU"));
			}

			menu = custom;
			custom = NULL;
		}
		else
		{
			safe_delete(custom);
		}
	}

	return menu;
}

#if defined NIX || defined MACOS
void EventHandler::displayMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo, gcMenu *menu, int32 x, int32 y)
{
	m_pParent->PopupMenu((wxMenu*)menu);
}
#endif

void EventHandler::clearCrumbs()
{
	m_pParent->onClearCrumbsEvent();
}

void EventHandler::addCrumb(const char* name, const char* url)
{
	Crumb c;

	c.url = url;
	c.name = name;

	m_pParent->onAddCrumbEvent(c);
}

void EventHandler::ForwardPopupMenu(ChromiumMenuInfoFromMem* menu)
{
	m_pParent->HandlePopupMenu(menu);
}