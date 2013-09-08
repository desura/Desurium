/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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
#include "gcWebControl.h"

#include "wx/msw/dc.h"
#include "managers/CVar.h"
#include "wx_controls/gcCustomMenu.h"

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

const uint32 g_uiMaxKeys = 23;

const ShortCut_s g_ShortCutList[] = 
{
#ifdef WIN32
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
#else
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
#endif
};

class HookHwnd
{
public:
	HookHwnd(HWND handle)
	{
		m_Hwnd = handle;
		s_MapSet[handle] = this;
		m_OldWndProc = (WNDPROC)SetWindowLong(handle, GWL_WNDPROC, (LONG)MSWWindowProc);
	}

	~HookHwnd()
	{
		SetWindowLong(m_Hwnd, GWL_WNDPROC, (LONG)m_OldWndProc);
		s_MapSet.erase(s_MapSet.find(m_Hwnd));
	}

protected:
	static LRESULT APIENTRY MSWWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (s_MapSet.find(hwnd) != s_MapSet.end())
		{
			if (s_MapSet[hwnd]->HandleMSWWindowProc(uMsg, wParam, lParam))
				return TRUE;

			return CallWindowProc(s_MapSet[hwnd]->m_OldWndProc, hwnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	static std::map<HWND, HookHwnd*> s_MapSet;


	bool HandleMSWWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_MEASUREITEM)
		{
			return MSWOnMeasureItem((MEASUREITEMSTRUCT*)lParam);
		}
		else if (uMsg == WM_DRAWITEM)
		{
			return MSWOnDrawItem((DRAWITEMSTRUCT*)lParam);
		}

		return false;
	}

	bool MSWOnDrawItem(DRAWITEMSTRUCT *pDrawStruct)
	{
		if (pDrawStruct->CtlType == ODT_MENU)
		{
			gcMenuItem *pMenuItem = (gcMenuItem*)(pDrawStruct->itemData);

			if (!pMenuItem)
				return false;

			wxDCTemp dc((WXHDC)pDrawStruct->hDC);
			wxRect rect(pDrawStruct->rcItem.left, pDrawStruct->rcItem.top, pDrawStruct->rcItem.right - pDrawStruct->rcItem.left, pDrawStruct->rcItem.bottom - pDrawStruct->rcItem.top);

			return pMenuItem->OnDrawItem(dc,rect,(wxOwnerDrawn::wxODAction)pDrawStruct->itemAction,(wxOwnerDrawn::wxODStatus)pDrawStruct->itemState);
		}

		return false;
	}

	bool MSWOnMeasureItem(MEASUREITEMSTRUCT *pMeasureStruct)
	{
		if (pMeasureStruct->CtlType == ODT_MENU )
		{
			gcMenuItem *pMenuItem = (gcMenuItem*)(pMeasureStruct->itemData);

			if (!pMenuItem)
				return false;

			size_t w, h;
			bool rc = pMenuItem->OnMeasureItem(&w, &h);

			pMeasureStruct->itemWidth = w;
			pMeasureStruct->itemHeight = h;

			return rc;
		}

		return false;
	}

private:
	HWND m_Hwnd;
	WNDPROC m_OldWndProc;
};

std::map<HWND, HookHwnd*> HookHwnd::s_MapSet;


void DisplayContextMenu(gcWebControlI* m_pParent, ContextClientDataI* ccd, gcMenu* menu, int32 xPos, int32 yPos)
{
	HWND hwnd = m_pParent->getBrowserHWND();
	HookHwnd hook(hwnd);

	int res = TrackPopupMenu((HMENU)menu->GetHMenu(), TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD|TPM_RECURSE, xPos, yPos, 0, hwnd, NULL);
	safe_delete(menu);

	ccd->processResult(res);
	ccd->destroy();
}

void EventHandler::displayMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo, gcMenu *menu, int32 x, int32 y)
{
	int res = 0;
		
	{
		HookHwnd hook((HWND)menuInfo->getHWND());
		res = TrackPopupMenu((HMENU)menu->GetHMenu(), TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_RETURNCMD|TPM_RECURSE|TPM_NONOTIFY, x, y, 0, (HWND)menuInfo->getHWND(), NULL);
	}

	if (res == MENU_ID_VIEWPBROWSER)
	{
		gcWString url;

		if (HasAnyFlags(menuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_PAGE))
		{
			url = menuInfo->getPageUrl();
		}
		else if (HasAnyFlags(menuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_FRAME))
		{
			url = menuInfo->getFrameUrl();
		}

		gcLaunchDefaultBrowser(url);
	}
	else if (res == MENU_ID_VIEWLBROWSER)
	{
		gcLaunchDefaultBrowser(menuInfo->getLinkUrl());
	}
	else if (res == MENU_ID_VIEWIBROWSER)
	{
		gcLaunchDefaultBrowser(menuInfo->getImageUrl());
	}
	else
	{
		m_pParent->AddPendingEvent(wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, res));
	}
}
