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

#ifndef DESURA_GCWCEVENTS_H
#define DESURA_GCWCEVENTS_H
#ifdef _WIN32
#pragma once
#endif

#include "cef_desura_includes/ChromiumBrowserI.h"

class gcWebControlI;
class gcCustomMenu;

enum Action
{
	IDC_BACK,
	IDC_FORWARD,
	IDC_DEV_TOOLS,
	IDC_DEV_TOOLS_CONSOLE,
	IDC_PRINT,
	IDC_RELOAD,
	IDC_RELOAD_IGNORING_CACHE,
	IDC_HOME,
	IDC_STOP,
	IDC_VIEW_SOURCE,
	IDC_ZOOM_MINUS,
	IDC_ZOOM_NORMAL,
	IDC_ZOOM_PLUS,
	IDC_FIND,
};

enum MenuEvents
{
	MENU_ID_UNDO = 10,
	MENU_ID_REDO,
	MENU_ID_CUT,
	MENU_ID_COPY,
	MENU_ID_PASTE,
	MENU_ID_DELETE,
	MENU_ID_SELECTALL,
	MENU_ID_BACK,
	MENU_ID_FORWARD,
	MENU_ID_RELOAD,
	MENU_ID_PRINT,
	MENU_ID_VIEWSOURCE,
	MENU_ID_VIEWPBROWSER,
	MENU_ID_VIEWLBROWSER,
	MENU_ID_VIEWIBROWSER,
	MENU_ID_ZOOM_MINUS,
	MENU_ID_ZOOM_NORMAL,
	MENU_ID_ZOOM_PLUS,
	MENU_ID_INSPECTELEMENT,

	MENU_ID_CUSTOMACTION, //must be last
};

class ContextClientDataI
{
public:
	virtual bool processResult(uint32 res)=0;
	virtual void destroy()=0;
};

class gcMenu;
class ChromiumMenuInfoFromMem;


class EventHandler : public ChromiumDLL::ChromiumBrowserEventI
{
public:
	EventHandler(gcWebControlI* parent);
	~EventHandler();

	virtual bool onNavigateUrl(const char* url, bool isMain);
	virtual void onPageLoadStart();
	virtual void onPageLoadEnd();

	virtual bool onJScriptAlert(const char* msg);
	virtual bool onJScriptConfirm(const char* msg, bool* result);
	virtual bool onJScriptPrompt(const char* msg, const char* defualtVal, bool* handled, char result[255]);

	virtual bool onKeyEvent(ChromiumDLL::KeyEventType type, int code, int modifiers, bool isSystemKey);

	virtual void onLogConsoleMsg(const char* message, const char* source, int line);

	virtual void launchLink(const char* url);
	virtual bool onLoadError(const char* errorMsg, const char* url, char* buff, size_t size);

	virtual void HandleWndProc(int message, int wparam, int lparam);
	virtual bool HandlePopupMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo);

	virtual void HandleJSBinding(ChromiumDLL::JavaScriptObjectI* jsObject, ChromiumDLL::JavaScriptFactoryI* factory);

	uint32 getLastX()
	{
		return m_uiLastContextMenuX;
	}

	uint32 getLastY()
	{
		return m_uiLastContextMenuY;
	}

	void clearCrumbs();
	void addCrumb(const char* name, const char* url);

	void ForwardPopupMenu(ChromiumMenuInfoFromMem* menu);
	gcMenu* createMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo);

protected:
	void displayMenu(ChromiumDLL::ChromiumMenuInfoI* menuInfo, gcMenu *menu, int32 x, int32 y);

private:
	uint32 m_uiLastContextMenuX;
	uint32 m_uiLastContextMenuY;

	gcWebControlI* m_pParent;
};



#endif //DESURA_GCWCEVENTS_H
