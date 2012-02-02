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
#include "IPCBrowser.h"
#include "IPCManager.h"

#include "../../branding/branding.h"

#ifndef DESURA_CLIENT
	#include "cef_desura_includes/ChromiumBrowserI.h"
	#include "CefEvents.h"
#else
	#include "wx_controls/gcManagers.h"
	#include "gcWCEvents.h"
	#include "MainApp.h"
	#include "ChromiumMenuInfoFromMem.h"
#endif

REG_IPC_CLASS( IPCBrowser );

IPCBrowser* g_pIPCBrowser = NULL;

IPCBrowser* GetIPCBrowser()
{
	return g_pIPCBrowser;
}


IPCBrowser::IPCBrowser(IPC::IPCManager* mang, uint32 id, DesuraId itemId) : IPC::IPCClass(mang, id, itemId)
{
	registerFunctions();
	m_pEventHandler = NULL;

#ifndef DESURA_CLIENT
	m_pBrowser = NULL;
#endif

	g_pIPCBrowser = this;
}

IPCBrowser::~IPCBrowser()
{
	g_pIPCBrowser = NULL;

#ifndef DESURA_CLIENT
	if (m_pBrowser)
		m_pBrowser->destroy();

	m_pBrowser = NULL;

	safe_delete(m_pEventHandler);
#endif
}

void IPCBrowser::registerFunctions()
{
#ifndef DESURA_CLIENT
	REG_FUNCTION_VOID( IPCBrowser, newBrowser );

	REG_FUNCTION_VOID( IPCBrowser, refresh );
	REG_FUNCTION_VOID( IPCBrowser, stop );
	REG_FUNCTION_VOID( IPCBrowser, back );
	REG_FUNCTION_VOID( IPCBrowser, forward );

	REG_FUNCTION_VOID( IPCBrowser, forceRefresh );
	REG_FUNCTION_VOID( IPCBrowser, print );
	REG_FUNCTION_VOID( IPCBrowser, viewSource );
	REG_FUNCTION_VOID( IPCBrowser, zoomIn );
	REG_FUNCTION_VOID( IPCBrowser, zoomOut );
	REG_FUNCTION_VOID( IPCBrowser, zoomNormal );

	REG_FUNCTION_VOID( IPCBrowser, onResize );
	REG_FUNCTION_VOID( IPCBrowser, onPaint );
	REG_FUNCTION_VOID( IPCBrowser, onFocus );
	REG_FUNCTION_VOID( IPCBrowser, loadUrl );

	REG_FUNCTION_VOID( IPCBrowser, setCookies );
	REG_FUNCTION_VOID( IPCBrowser, executeJScript );
#else
	REG_FUNCTION( IPCBrowser, getThemeFolder );

	REG_FUNCTION_VOID( IPCBrowser, clearCrumbs );
	REG_FUNCTION_VOID( IPCBrowser, addCrumb );
	REG_FUNCTION_VOID( IPCBrowser, setCounts );

	REG_FUNCTION( IPCBrowser, onNavigateUrl );
	REG_FUNCTION_VOID( IPCBrowser, onPageLoadStart );
	REG_FUNCTION_VOID( IPCBrowser, onPageLoadEnd );
	REG_FUNCTION( IPCBrowser, onKeyEvent );
	REG_FUNCTION_VOID( IPCBrowser, onLogConsoleMsg );
	REG_FUNCTION_VOID( IPCBrowser, launchLink );

	REG_FUNCTION( IPCBrowser, handlePopupMenu );

#endif
}

#ifdef DESURA_CLIENT

void IPCBrowser::setEventHandler(EventHandler* handler)
{
	m_pEventHandler = handler;
}

void IPCBrowser::newBrowser(uint64 hwnd, const char* url)
{
	IPC::functionCallV(this, "newBrowser", hwnd, url);
}

void IPCBrowser::refresh()
{
	IPC::functionCallAsync(this, "refresh");
}

void IPCBrowser::stop()
{
	IPC::functionCallAsync(this, "stop");
}

void IPCBrowser::back()
{
	IPC::functionCallAsync(this, "back");
}

void IPCBrowser::forward()
{
	IPC::functionCallAsync(this, "forward");
}


void IPCBrowser::forceRefresh()
{
	IPC::functionCallAsync(this, "forceRefresh");
}

void IPCBrowser::print()
{
	IPC::functionCallAsync(this, "print");
}

void IPCBrowser::viewSource()
{
	IPC::functionCallAsync(this, "viewSource");
}

void IPCBrowser::zoomIn()
{
	IPC::functionCallAsync(this, "zoomIn");
}

void IPCBrowser::zoomOut()
{
	IPC::functionCallAsync(this, "zoomOut");
}

void IPCBrowser::zoomNormal()
{
	IPC::functionCallAsync(this, "zoomNormal");
}

void IPCBrowser::undo()
{
	IPC::functionCallAsync(this, "undo");
}

void IPCBrowser::redo()
{
	IPC::functionCallAsync(this, "redo");
}

void IPCBrowser::cut()
{
	IPC::functionCallAsync(this, "cut");
}

void IPCBrowser::copy()
{
	IPC::functionCallAsync(this, "copy");
}

void IPCBrowser::paste()
{
	IPC::functionCallAsync(this, "paste");
}

void IPCBrowser::del()
{
	IPC::functionCallAsync(this, "del");
}

void IPCBrowser::selectall()
{
	IPC::functionCallAsync(this, "selectall");
}

void IPCBrowser::showInspector()
{
	IPC::functionCallAsync(this, "showInspector");
}

void IPCBrowser::onResize()
{
	IPC::functionCallAsync(this, "onResize");
}

void IPCBrowser::onPaint()
{
	IPC::functionCallAsync(this, "onPaint");
}

void IPCBrowser::onFocus()
{
	IPC::functionCallAsync(this, "onFocus");
}

void IPCBrowser::loadUrl(const char* url)
{
	IPC::functionCallAsync(this, "loadUrl", url);
}

const char* IPCBrowser::getThemeFolder()
{
	return GetGCThemeManager()->getThemeFolder();
}

void IPCBrowser::setCookies(const char* freeman, const char* masterchief)
{
	IPC::functionCallAsync(this, "setCookies", freeman, masterchief);
}

void IPCBrowser::executeJScript(const char* script)
{
	IPC::functionCallAsync(this, "executeJScript", script);
}



void IPCBrowser::clearCrumbs()
{
	if (m_pEventHandler)
		m_pEventHandler->clearCrumbs();
}

void IPCBrowser::addCrumb(const char* name, const char* url)
{
	if (m_pEventHandler)
		m_pEventHandler->addCrumb(name, url);	
}

void IPCBrowser::setCounts(int32 msg, int32 updates, int32 threads, int32 cart)
{
	if (GetUserCore())
		GetUserCore()->setCounts(msg, updates, threads, cart);
}

bool IPCBrowser::onNavigateUrl(const char* url, bool isMain)
{
	if (m_pEventHandler)
		return m_pEventHandler->onNavigateUrl(url, isMain);

	return true;
}

void IPCBrowser::onPageLoadStart()
{
	if (m_pEventHandler)
		m_pEventHandler->onPageLoadStart();
}

void IPCBrowser::onPageLoadEnd()
{
	if (m_pEventHandler)
		m_pEventHandler->onPageLoadEnd();
}

bool IPCBrowser::onKeyEvent(int type, int code, int modifiers, bool isSystemKey)
{
	if (m_pEventHandler)
		return m_pEventHandler->onKeyEvent((ChromiumDLL::KeyEventType)type, code, modifiers, isSystemKey);

	return false;
}

void IPCBrowser::onLogConsoleMsg(const char* message, const char* source, int line)
{
	if (m_pEventHandler)
		m_pEventHandler->onLogConsoleMsg(message, source, line);	
}

void IPCBrowser::launchLink(const char* url)
{
	if (m_pEventHandler)
		m_pEventHandler->launchLink(url);	
}

bool IPCBrowser::handlePopupMenu(IPC::PBlob blob)
{
	if (!m_pEventHandler)
		return false;

	m_pEventHandler->ForwardPopupMenu(new ChromiumMenuInfoFromMem(blob.getData(), blob.getSize()));
	return true;
}

#else

ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(HWND hwnd, const char* name, const char* loadUrl);

void IPCBrowser::newBrowser(uint64 hwnd, const char* url)
{
	if (m_pBrowser)
		return;

	HWND h = (HWND)hwnd;

	m_pEventHandler = new EventHandler();

	m_pBrowser = NewChromiumBrowser(h, PRODUCT_NAME, url);
	m_pBrowser->setEventCallback(m_pEventHandler);
}

void IPCBrowser::refresh()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->refresh();
}

void IPCBrowser::stop()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->stop();
}

void IPCBrowser::back()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->back();
}

void IPCBrowser::forward()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->forward();
}

void IPCBrowser::forceRefresh()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->refresh(true);
}

void IPCBrowser::print()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->print();
}

void IPCBrowser::viewSource()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->viewSource();
}

void IPCBrowser::zoomIn()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->zoomIn();
}

void IPCBrowser::zoomOut()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->zoomOut();
}

void IPCBrowser::zoomNormal()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->zoomNormal();
}

void IPCBrowser::undo()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->undo();
}

void IPCBrowser::redo()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->redo();
}

void IPCBrowser::cut()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->cut();
}

void IPCBrowser::copy()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->copy();
}

void IPCBrowser::paste()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->paste();
}

void IPCBrowser::del()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->del();
}

void IPCBrowser::selectall()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->selectall();
}

void IPCBrowser::showInspector()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->showInspector();
}

void IPCBrowser::onResize()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->onResize();
}

void IPCBrowser::onPaint()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->onPaint();
}

void IPCBrowser::onFocus()
{
	if (!m_pBrowser)
		return;

	m_pBrowser->onFocus();
}

void IPCBrowser::loadUrl(const char* url)
{
	if (!m_pBrowser)
		return;

	m_pBrowser->loadUrl(url);
}

void SetCookies(const char* freeman, const char* masterchief);

void IPCBrowser::setCookies(const char* freeman, const char* masterchief)
{
	SetCookies(freeman, masterchief);
}

void IPCBrowser::executeJScript(const char* script)
{
	if (!m_pBrowser)
		return;

	m_pBrowser->executeJScript(script);
}


const char* IPCBrowser::getThemeFolder()
{
	return IPC::functionCall<const char*>(this, "getThemeFolder");
}

void IPCBrowser::clearCrumbs()
{
	IPC::functionCallAsync(this, "clearCrumbs");
}

void IPCBrowser::addCrumb(const char* name, const char* url)
{
	IPC::functionCallAsync(this, "addCrumb", name, url);
}

void IPCBrowser::setCounts(int32 msg, int32 updates, int32 threads, int32 cart)
{
	IPC::functionCallAsync(this, "setCounts", msg, updates, threads, cart);
}


bool IPCBrowser::onNavigateUrl(const char* url, bool isMain)
{
	return IPC::functionCall<bool>(this, "onNavigateUrl", url, isMain);
}

void IPCBrowser::onPageLoadStart()
{
	IPC::functionCallAsync(this, "onPageLoadStart");
}

void IPCBrowser::onPageLoadEnd()
{
	IPC::functionCallAsync(this, "onPageLoadEnd");
}

bool IPCBrowser::onKeyEvent(int type, int code, int modifiers, bool isSystemKey)
{
	return IPC::functionCall<bool>(this, "onKeyEvent", type, code, modifiers, isSystemKey);
}

void IPCBrowser::onLogConsoleMsg(const char* message, const char* source, int line)
{
	IPC::functionCallAsync(this, "onLogConsoleMsg", message, source, line);
}

void IPCBrowser::launchLink(const char* url)
{
	IPC::functionCallAsync(this, "launchLink", url);
}

bool IPCBrowser::handlePopupMenu(IPC::PBlob blob)
{
	return IPC::functionCall<bool>(this, "handlePopupMenu", blob);
}

#endif
