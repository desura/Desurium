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

#ifndef DESURA_IPCBROWSER_H
#define DESURA_IPCBROWSER_H
#ifdef _WIN32
#pragma once
#endif

#include "IPCClass.h"
#include "IPCParameter.h"

namespace ChromiumDLL
{
	class ChromiumBrowserI;
}

class EventHandler;

class IPCBrowser : public IPC::IPCClass
{
public:
	IPCBrowser(IPC::IPCManager* mang, uint32 id, DesuraId itemId);
	~IPCBrowser();


	/// Desura -> Host
	void refresh();
	void stop();
	void back();
	void forward();

	void forceRefresh();
	void print();
	void viewSource();
	void zoomIn();
	void zoomOut();
	void zoomNormal();

	void undo();
	void redo();
	void cut();
	void copy();
	void paste();
	void del();
	void selectall();
	void showInspector();

	void newBrowser(uint64 hwnd, const char* url);

	void onResize();
	void onPaint();
	void onFocus();

	void loadUrl(const char* url);
	void setCookies(const char* freeman, const char* masterchief);
	void executeJScript(const char* script);
	
	/// Host -> Desura
	const char* getThemeFolder();

	void clearCrumbs();
	void addCrumb(const char* name, const char* url);
	void setCounts(int32 msg, int32 updates, int32 threads, int32 cart);

	bool onNavigateUrl(const char* url, bool isMain);
	void onPageLoadStart();
	void onPageLoadEnd();
	bool onKeyEvent(int type, int code, int modifiers, bool isSystemKey);
	void onLogConsoleMsg(const char* message, const char* source, int line);
	void launchLink(const char* url);

	bool handlePopupMenu(IPC::PBlob blob);

#ifdef DESURA_CLIENT
	void setEventHandler(EventHandler* handler);
#endif

private:
	void registerFunctions();

#ifndef DESURA_CLIENT
	ChromiumDLL::ChromiumBrowserI* m_pBrowser;
#endif

	EventHandler* m_pEventHandler;
};

IPCBrowser* GetIPCBrowser();

#endif //DESURA_IPCMAIN_H
