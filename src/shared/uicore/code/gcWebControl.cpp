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
#include "gcWebControl.h"

#include "gcWCEvents.h"
#include "cef_desura_includes/ChromiumBrowserI.h"

#include "gcWebFakeBrowser.h"

#include <branding/branding.h>

#ifdef WIN32
ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(HWND hwnd, const char* name, const char* loadUrl);
#else
ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(int* gtkWidget, const char* name, const char* loadUrl);
#endif


gcWebControl::gcWebControl(wxWindow* parent, const char* defaultUrl, const char* unused) : gcPanel(parent, wxID_ANY)
{
	Bind(wxEVT_MOUSEWHEEL, &gcWebControl::onMouseScroll, this);
	Bind(wxEVT_SIZE, &gcWebControl::onResize, this);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &gcWebControl::onMenuClicked, this);
	
	Bind(wxEVT_ERASE_BACKGROUND, &gcWebControl::onPaintBg, this);
	Bind(wxEVT_PAINT, &gcWebControl::onPaint, this);
	Bind(wxEVT_SET_FOCUS, &gcWebControl::onFocus, this);	

	m_bStartedLoading = false;
	gcString loadingurl = gcString(GetGCThemeManager()->getWebPage("loading"));
	loadingurl += gcString("?url={0}", UTIL::STRING::urlEncode(gcString(defaultUrl)));

	m_pEventHandler = new EventHandler(this);

#ifdef WIN32
	m_pChromeBrowser = NewChromiumBrowser((HWND)GetHWND(), PRODUCT_NAME, loadingurl.c_str());
#else
	GtkWidget* gtkParent = this->GetConnectWidget();
	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
	
	if (!gtkParent)
		return;
	
	GdkColor blackCol;
	gdk_color_parse("black", &blackCol);
	
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_NORMAL, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_ACTIVE, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_PRELIGHT, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_SELECTED, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_INSENSITIVE, &blackCol);
	
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_NORMAL, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_ACTIVE, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_PRELIGHT, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_SELECTED, &blackCol);
	gtk_widget_modify_bg(GTK_WIDGET(vbox), GTK_STATE_INSENSITIVE, &blackCol);
	
	gtk_container_add(GTK_CONTAINER(gtkParent), vbox);
	
	m_pChromeBrowser = NewChromiumBrowser((int*)vbox, PRODUCT_NAME, loadingurl.c_str());
#endif

	if (!m_pChromeBrowser)
		m_pChromeBrowser = new gcWebFakeBrowser(this);

	m_pChromeBrowser->setEventCallback(m_pEventHandler);

	onPageStartEvent += guiDelegate(this, &gcWebControl::onStartLoad);
	onAnyPageLoadEvent += guiDelegate(this, &gcWebControl::onPageLoad);

	m_szHomeUrl = defaultUrl;
	m_bContentLoaded = false;
}

gcWebControl::~gcWebControl()
{
	m_pChromeBrowser->setEventCallback(NULL);
	m_pChromeBrowser->destroy();

	safe_delete(m_pEventHandler);
}

void gcWebControl::onFocus(wxFocusEvent& event)
{
	m_pChromeBrowser->onFocus();
}

#ifdef WIN32
HWND gcWebControl::getBrowserHWND()
{
	return (HWND)m_pChromeBrowser->getBrowserHandle();
}
#endif

void gcWebControl::onMouseScroll(wxMouseEvent& event)
{
#ifdef WIN32
	wxPoint pos = ClientToScreen(event.GetPosition());

	uint32 delta = event.m_wheelRotation;
	uint32 flags = 0;

	if (event.m_shiftDown)
		flags |= MK_SHIFT;

	if (event.m_controlDown)
		flags |= MK_CONTROL;

	if (event.m_leftDown)
		flags |= MK_LBUTTON;

	if (event.m_middleDown)
		flags |= MK_MBUTTON;

	if (event.m_rightDown)
		flags |= MK_RBUTTON;

	if (event.m_aux1Down)
		flags |= MK_XBUTTON1;

	if (event.m_aux2Down)
		flags |= MK_XBUTTON2;

	m_pChromeBrowser->scroll(pos.x, pos.y, delta, flags);
#endif
}

void gcWebControl::onMenuClicked(wxCommandEvent& event)
{
	wxMenu* menu = dynamic_cast<wxMenu*>(event.GetEventObject());
	
	if (menu)
	{
		ContextClientDataI* ccd = dynamic_cast<ContextClientDataI*>(menu->GetClientObject());

		if (ccd && ccd->processResult(event.GetId()))
			return;
	}

	switch (event.GetId())
	{
		case MENU_ID_UNDO:			m_pChromeBrowser->undo(); break;
		case MENU_ID_REDO:			m_pChromeBrowser->redo(); break;
		case MENU_ID_CUT:			m_pChromeBrowser->cut(); break;
		case MENU_ID_COPY:			m_pChromeBrowser->copy(); break;
		case MENU_ID_PASTE:			m_pChromeBrowser->paste(); break;
		case MENU_ID_DELETE:		m_pChromeBrowser->del(); break;
		case MENU_ID_SELECTALL:		m_pChromeBrowser->selectall(); break;
		case MENU_ID_BACK:			m_pChromeBrowser->back(); break;
		case MENU_ID_FORWARD:		m_pChromeBrowser->forward(); break;
		case MENU_ID_RELOAD:		m_pChromeBrowser->refresh(); break;
		case MENU_ID_PRINT:			m_pChromeBrowser->print(); break;
		case MENU_ID_VIEWSOURCE:	m_pChromeBrowser->viewSource(); break;
		case MENU_ID_ZOOM_MINUS:	m_pChromeBrowser->zoomOut(); break;
		case MENU_ID_ZOOM_NORMAL:	m_pChromeBrowser->zoomNormal(); break;
		case MENU_ID_ZOOM_PLUS:		m_pChromeBrowser->zoomIn(); break;
		case MENU_ID_INSPECTELEMENT: m_pChromeBrowser->showInspector(); break;
	}
}

void gcWebControl::handleKeyEvent(int eventCode)
{
	switch (eventCode)
	{
		case IDC_BACK:						m_pChromeBrowser->back(); break;
		case IDC_FORWARD:					m_pChromeBrowser->forward(); break;
		case IDC_DEV_TOOLS:					break;
		case IDC_DEV_TOOLS_CONSOLE:			break;
		case IDC_PRINT:						m_pChromeBrowser->print(); break;
		case IDC_RELOAD:					m_pChromeBrowser->refresh(); break;
		case IDC_RELOAD_IGNORING_CACHE:		m_pChromeBrowser->refresh(true); break;
		case IDC_HOME:						home(); break;
		case IDC_STOP:						m_pChromeBrowser->stop(); break;
		case IDC_VIEW_SOURCE:				m_pChromeBrowser->viewSource(); break;
		case IDC_ZOOM_MINUS:				m_pChromeBrowser->zoomOut(); break;
		case IDC_ZOOM_NORMAL:				m_pChromeBrowser->zoomNormal(); break;
		case IDC_ZOOM_PLUS:					m_pChromeBrowser->zoomIn(); break;

		case IDC_FIND: 
			onFindEvent();
			break;
	};
}

void gcWebControl::onStartLoad()
{
	m_bStartedLoading = true;
	onPageStartEvent -= guiDelegate(this, &gcWebControl::onStartLoad);
	forceResize();
}

void gcWebControl::onPageLoad()
{
	if (!m_bContentLoaded)
		m_bContentLoaded = true;
	else
		onPageLoadEvent();
}

void gcWebControl::forceResize()
{
#ifdef WIN32
	m_pChromeBrowser->onResize();
#else
	m_pChromeBrowser->onResize(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
#endif
}

void gcWebControl::onResize( wxSizeEvent& event )
{
#ifdef WIN32
	m_pChromeBrowser->onResize();
#else
	wxSize s=event.GetSize();
	m_pChromeBrowser->onResize(0, 0, s.GetWidth(), s.GetHeight());
#endif
}

void gcWebControl::onPaintBg( wxEraseEvent& event )
{
#ifdef WIN32
	m_pChromeBrowser->onPaintBg();
#endif
	m_pChromeBrowser->onResize(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
}

void gcWebControl::onPaint( wxPaintEvent& event )
{
#ifdef WIN32
	m_pChromeBrowser->onPaint();
#endif
	m_pChromeBrowser->onResize(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
}

void gcWebControl::loadUrl(const gcString& url)
{
	if (m_szHomeUrl == L"about:blank")
		m_szHomeUrl = url;

	m_pChromeBrowser->loadUrl(url.c_str());
}

void gcWebControl::loadString(const gcString& string)
{
	m_pChromeBrowser->loadString(string.c_str());
}

void gcWebControl::executeJScript(const gcString& code)
{
	m_pChromeBrowser->executeJScript(code.c_str());
}

void gcWebControl::home()
{
	loadUrl(m_szHomeUrl.c_str());
}

bool gcWebControl::refresh()
{
	m_pChromeBrowser->refresh();
	return true;
}

bool gcWebControl::stop()
{
	m_pChromeBrowser->stop();
	return true;
}


bool gcWebControl::back()
{
	m_pChromeBrowser->back();
	return true;
}

bool gcWebControl::forward()
{
	m_pChromeBrowser->forward();
	return true;
}

ChromiumDLL::JavaScriptContextI* gcWebControl::getJSContext()
{
	return m_pChromeBrowser->getJSContext();
}
