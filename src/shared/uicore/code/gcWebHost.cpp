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
#include "gcWebHost.h"

#include "CefIPCPipeClient.h"
#include "gcWCEvents.h"
#include "ChromiumMenuInfoFromMem.h"

#include "mcfcore/UserCookies.h"
#include "MainApp.h"

#define IPC_TRY_CATCH( b, x ) try{ if (b){x;} } catch (...){}

gcWebHost::gcWebHost(wxWindow* parent, const char* defaultUrl, const char* hostName) : gcPanel(parent, wxID_ANY)
{
	m_pBrowser = NULL;
	m_pPipeClient = NULL;
	m_MenuInfo = NULL;

	m_szHostName = hostName;
	m_szDefaultUrl = defaultUrl;

	m_pEventHandler = new EventHandler(this);

	start(defaultUrl);

	Bind(wxEVT_ERASE_BACKGROUND, &gcWebHost::onPaintBg, this);
	Bind(wxEVT_PAINT, &gcWebHost::onPaint, this);
	Bind(wxEVT_SIZE, &gcWebHost::onResize, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &gcWebHost::onMenuClicked, this);

	Bind(wxEVT_LEFT_DOWN, &gcWebHost::onMouseDown, this);

	//Bind(wxEVT_SET_FOCUS, &gcWebHost::onFocus, this);

	onMenuEvent += guiDelegate(this, &gcWebHost::onMenuShow);
	onAnyPageLoadEvent += delegate(&onPageLoadEvent);

	if (GetWebCore())
	{
		*GetWebCore()->getCookieUpdateEvent() += guiDelegate(this, &gcWebHost::onCookieUpdate);
		onCookieUpdate();
	}

	m_szDieString = Managers::GetString(L"#BH_CRASH");
}

gcWebHost::~gcWebHost()
{
	if (GetWebCore())
		*GetWebCore()->getCookieUpdateEvent() -= guiDelegate(this, &gcWebHost::onCookieUpdate);

	onBrowserDisconnect();
}

void gcWebHost::onCookieUpdate()
{
	if (!m_pBrowser)
		return;

	MCFCore::Misc::UserCookies uc;
	GetWebCore()->setMCFCookies(&uc);

	gcString fD(uc.getIdCookie()); //= UTIL::STRING::urlDecode
	gcString mD(uc.getSessCookie()); //= UTIL::STRING::urlDecode

	m_pBrowser->setCookies(fD.c_str(), mD.c_str());
}

void gcWebHost::HandlePopupMenu(ChromiumMenuInfoFromMem* menu)
{
	m_MenuInfo = menu;
	onMenuEvent();
}

void gcWebHost::onMenuShow()
{
	if (m_pEventHandler)
	{
		gcMenu* menu = m_pEventHandler->createMenu(m_MenuInfo);
		PopupMenu((wxMenu*)menu);
	}

	safe_delete(m_MenuInfo);
}

void gcWebHost::start(const char* url)
{
	if (m_pBrowser)
		return;

	if (m_pPipeClient)
		safe_delete(m_pPipeClient);

	try
	{
		gcString id("DesuraRenderPipe-{0}", (uint64)this);

		m_pPipeClient = new CefIPCPipeClient(id.c_str(), m_szHostName.c_str());
		m_pPipeClient->onDisconnectEvent += guiDelegate(this, &gcWebHost::onBrowserDisconnect);

		m_pPipeClient->start();
	}
	catch (gcException)
	{
		return;
	}

	m_pBrowser = m_pPipeClient->createBrowser((HWND)GetHWND(), url);

	if (m_pBrowser)
		m_pBrowser->setEventHandler(m_pEventHandler);
}

void gcWebHost::onBrowserDisconnect()
{
	if (m_pBrowser)
		m_pBrowser->destroy();

	m_pBrowser = NULL;
	safe_delete(m_pPipeClient);
}

void gcWebHost::loadUrl(const gcString& url)
{
	if (m_szDefaultUrl == "about:blank")
		m_szDefaultUrl = url;

	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->loadUrl(url.c_str()));
}

void gcWebHost::loadString(const gcString& string)
{
}

void gcWebHost::executeJScript(const gcString& code)
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->executeJScript(code.c_str()));
}

bool gcWebHost::refresh()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->refresh());
	return true;
}

bool gcWebHost::stop()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->stop());
	return true;
}

bool gcWebHost::back()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->back());
	return true;
}

bool gcWebHost::forward()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->forward());
	return true;
}

void gcWebHost::forceResize()
{
}

void gcWebHost::home()
{
	loadUrl(m_szDefaultUrl.c_str());
}

void gcWebHost::forceRefresh()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->forceRefresh());
}

void gcWebHost::print()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->print());
}

void gcWebHost::viewSource()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->viewSource());
}

void gcWebHost::zoomIn()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->zoomIn());
}

void gcWebHost::zoomOut()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->zoomOut());
}

void gcWebHost::zoomNormal()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->zoomNormal());
}

void gcWebHost::undo()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->undo());
}

void gcWebHost::redo()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->redo());
}

void gcWebHost::cut()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->cut());
}

void gcWebHost::copy()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->copy());
}

void gcWebHost::paste()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->paste());
}

void gcWebHost::del()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->del());
}

void gcWebHost::selectall()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->selectall());
}

void gcWebHost::showInspector()
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->showInspector());
}


void gcWebHost::onMenuClicked(wxCommandEvent& event)
{
	wxMenu* menu = dynamic_cast<wxMenu*>(event.GetEventObject());
	
	if (menu)
	{
		ContextClientDataI* ccd = dynamic_cast<ContextClientDataI*>(menu->GetClientObject());

		if (ccd && ccd->processResult(event.GetId()))
			return;
	}

	int res = event.GetId();

	if (m_MenuInfo)
	{
		if (res == MENU_ID_VIEWPBROWSER)
		{
			gcWString url;

			if (HasAnyFlags(m_MenuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_PAGE))
			{
				url = m_MenuInfo->getPageUrl();
			}
			else if (HasAnyFlags(m_MenuInfo->getTypeFlags(), ChromiumDLL::ChromiumMenuInfoI::MENUTYPE_FRAME))
			{
				url = m_MenuInfo->getFrameUrl();
			}

			gcLaunchDefaultBrowser(url);
		}
		else if (res == MENU_ID_VIEWLBROWSER)
		{
			gcLaunchDefaultBrowser(m_MenuInfo->getLinkUrl());
		}
		else if (res == MENU_ID_VIEWIBROWSER)
		{
			gcLaunchDefaultBrowser(m_MenuInfo->getImageUrl());
		}
	}


	switch (event.GetId())
	{
		case MENU_ID_UNDO:			undo(); break;
		case MENU_ID_REDO:			redo(); break;
		case MENU_ID_CUT:			cut(); break;
		case MENU_ID_COPY:			copy(); break;
		case MENU_ID_PASTE:			paste(); break;
		case MENU_ID_DELETE:		del(); break;
		case MENU_ID_SELECTALL:		selectall(); break;
		case MENU_ID_BACK:			back(); break;
		case MENU_ID_FORWARD:		forward(); break;
		case MENU_ID_RELOAD:		refresh(); break;
		case MENU_ID_PRINT:			print(); break;
		case MENU_ID_VIEWSOURCE:	viewSource(); break;
		case MENU_ID_ZOOM_MINUS:	zoomOut(); break;
		case MENU_ID_ZOOM_NORMAL:	zoomNormal(); break;
		case MENU_ID_ZOOM_PLUS:		zoomIn(); break;
		case MENU_ID_INSPECTELEMENT: showInspector(); break;
	}
}

void gcWebHost::handleKeyEvent(int eventCode)
{
	//TODO
	switch (eventCode)
	{
		case IDC_BACK:						back(); break;
		case IDC_FORWARD:					forward(); break;
		case IDC_DEV_TOOLS:					break;
		case IDC_DEV_TOOLS_CONSOLE:			break;
		case IDC_PRINT:						print(); break;
		case IDC_RELOAD:					refresh(); break;
		case IDC_RELOAD_IGNORING_CACHE:		forceRefresh(); break;
		case IDC_HOME:						home(); break;
		case IDC_STOP:						stop(); break;
		case IDC_VIEW_SOURCE:				viewSource(); break;
		case IDC_ZOOM_MINUS:				zoomOut(); break;
		case IDC_ZOOM_NORMAL:				zoomNormal(); break;
		case IDC_ZOOM_PLUS:					zoomIn(); break;

		case IDC_FIND: 
			onFindEvent();
			break;
	};
}

void gcWebHost::onResize(wxSizeEvent& event)
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->onResize());
	event.Skip();
}

void gcWebHost::onPaint(wxPaintEvent& event)
{
	try
	{
		if (m_pBrowser)
			m_pBrowser->onPaint();
		else
			paintNoBrowser();
	}
	catch (...)
	{
	}
}

void gcWebHost::onFocus(wxFocusEvent& event)
{
	IPC_TRY_CATCH(m_pBrowser, m_pBrowser->onFocus());
	event.Skip();
}

void gcWebHost::onPaintBg(wxEraseEvent& event)
{
}

void gcWebHost::onMouseDown(wxMouseEvent& event)
{
	if (!m_pBrowser)
		start(m_szDefaultUrl.c_str());
}

void gcWebHost::paintNoBrowser()
{
	wxPaintDC dc(this);

	wxColor bg = GetParent()->GetBackgroundColour();
	wxColor fg = GetForegroundColour();

	dc.SetFont(GetFont());
	dc.SetTextForeground(*wxWHITE);
	dc.SetTextBackground(*wxBLACK);

	wxSize s = GetSize();
	
	dc.SetBackground(wxBrush(*wxBLACK));
	dc.Clear();

	wxSize te = dc.GetTextExtent(m_szDieString);

	int x = (s.GetWidth()-te.GetWidth())/2;
	int y = (s.GetHeight()-te.GetHeight())/2;

	dc.SetBrush(wxBrush(*wxBLACK));
	dc.DrawText(m_szDieString, x, y);
}