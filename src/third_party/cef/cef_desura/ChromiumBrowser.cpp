///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : minimal
//   File        : ChromiumBrowser.cpp
//   Description :
//      [TODO: Write the purpose of ChromiumBrowser.cpp.]
//
//   Created On: 5/25/2010 5:40:26 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32)
#include "windows.h"
#endif

#include "ChromiumBrowser.h"
#include "include/cef.h"
#include "JavaScriptExtender.h"
#include "SchemeExtender.h"

#include "JavaScriptContext.h"
#include "ChromiumBrowserEvents.h"

#ifdef OS_LINUX
#include <gtk/gtk.h>
#endif

#if defined __x86_64 || defined __amd64 || defined __x86_64__
	#define NIX64 1
#endif

ChromiumDLL::LogMessageHandlerFn g_pLogHandler = NULL;

bool logHandler(int level, const std::string& msg)
{
	if (g_pLogHandler)
		return g_pLogHandler(level, msg.c_str());

	return false;
}

#ifdef OS_LINUX
static void gtkFocus(GtkWidget *widget, GdkEvent *event, ChromiumBrowser *data)
{
	if (data)
		data->onFocus();
}
#endif

extern "C"
{
	DLLINTERFACE void CEF_DoMsgLoop()
	{
		CefDoMessageLoopWork();
	}

	DLLINTERFACE bool CEF_Init(bool threaded, const char* cachePath, const char* logPath, const char* userAgent)
	{
		CefSettings settings;


		cef_string_copy(cachePath, strlen(cachePath), &settings.cache_path);
		cef_string_copy(userAgent, strlen(userAgent), &settings.user_agent);

		settings.multi_threaded_message_loop = threaded;


		if (!CefInitialize(settings))
			return false;

#if defined(_WIN32)
		CefRegisterFlashPlugin("gcswf32.dll");
#else
	#ifdef NIX64
		CefRegisterFlashPlugin("libdesura_flashwrapper_64.so");
	#else
		CefRegisterFlashPlugin("libdesura_flashwrapper_32.so");
	#endif
#endif

		return true;
	}

	DLLINTERFACE void CEF_Stop()
	{
		CefShutdown();
	}

	DLLINTERFACE bool CEF_RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* extender)
	{
		return JavaScriptExtender::Register(extender);
	}

	DLLINTERFACE bool CEF_RegisterSchemeExtender(ChromiumDLL::SchemeExtenderI* extender)
	{
		return SchemeExtender::Register(extender);
	}

	DLLINTERFACE ChromiumDLL::ChromiumBrowserI* CEF_NewChromiumBrowser(int* formHandle, const char* name, const char* defaultUrl)
	{
		return new ChromiumBrowser((WIN_HANDLE)formHandle, defaultUrl);
	}

	DLLINTERFACE void CEF_SetLogHandler(ChromiumDLL::LogMessageHandlerFn logFn)
	{
		g_pLogHandler = logFn;
	}

	DLLINTERFACE void CEF_PostCallback(ChromiumDLL::CallbackI* callback)
	{
		CefPostTask(TID_UI, CefRefPtr<CefTask>(new TaskWrapper(callback)));
	}
}

enum ACTION
{
	A_STOPLOAD,
	A_REFRESH,
	A_BACK,
	A_FORWARD,
	A_ZOOMIN,
	A_ZOOMOUT,
	A_ZOOMNORMAL,
	A_PRINT,
	A_VIEWSOURCE,
	A_UNDO,
	A_REDO,
	A_CUT,
	A_COPY,
	A_PASTE,
	A_DEL,
	A_SELECTALL,
};

class BrowserTask : public CefTask
{
public:
	BrowserTask(CefBrowser* browser, ACTION action)
	{
		m_pBrowser = browser;
		m_iRef = 1;
		m_Action = action;
	}

	virtual void Execute(CefThreadId threadId)
	{
		if (!m_pBrowser)
			return;

		bool handled = true;

		switch (m_Action)
		{
			case A_STOPLOAD:	m_pBrowser->StopLoad();						break;
			case A_REFRESH:		m_pBrowser->ReloadIgnoreCache();			break;
			case A_BACK:		m_pBrowser->GoBack();						break;
			case A_FORWARD:		m_pBrowser->GoForward();					break;
			default:
				handled = false;
				break;
		};

		if (handled)
			return;

		CefRefPtr<CefFrame> frame = m_pBrowser->GetFocusedFrame();

		if (!frame.get())
			return;

		switch (m_Action)
		{
			case A_ZOOMIN:		frame->ZoomIn();	break;
			case A_ZOOMOUT:		frame->ZoomOut();	break;
			case A_ZOOMNORMAL:	frame->ZoomNormal(); break;
			case A_PRINT:		frame->Print();		break;
			case A_VIEWSOURCE:	frame->ViewSource(); break;
			case A_UNDO:		frame->Undo();		break;
			case A_REDO:		frame->Redo();		break;
			case A_CUT:			frame->Cut();		break;
			case A_COPY:		frame->Copy();		break;
			case A_PASTE:		frame->Paste();		break;
			case A_DEL:			frame->Delete();	break;
			case A_SELECTALL:	frame->SelectAll(); break;
			default:
				handled = false;
				break;
		};
	}

	virtual int AddRef()
	{
		m_iRef++;
		return m_iRef;
	}

	virtual int Release()
	{
		m_iRef--;

		if (m_iRef == 0)
			delete this;

		return m_iRef;
	}

	virtual int GetRefCt()
	{
		return m_iRef;
	}

	int m_iRef;
	ACTION m_Action;
	CefBrowser* m_pBrowser;
};

ChromiumBrowser::ChromiumBrowser(WIN_HANDLE handle, const char* defaultUrl)
{
	m_iLastTask = 0;
	m_hFormHandle = handle;
	m_pBrowser = NULL;

	m_rEventHandler = (CefClient*)new ChromiumBrowserEvents(this);
	init(defaultUrl);
}

ChromiumBrowser::~ChromiumBrowser()
{
	ChromiumBrowserEvents* cbe = (ChromiumBrowserEvents*)m_rEventHandler.get();

	if (cbe)
	{
		cbe->setParent(NULL);
		cbe->setCallBack(NULL);
	}
}

CefBrowserSettings ChromiumBrowser::getBrowserDefaults()
{
	CefBrowserSettings browserDefaults;

	browserDefaults.developer_tools_disabled = false;
	browserDefaults.webgl_disabled = true;
	browserDefaults.universal_access_from_file_urls_allowed = true;
	browserDefaults.file_access_from_file_urls_allowed = true;
	browserDefaults.java_disabled = true;
	browserDefaults.javascript_close_windows_disallowed = true;
	browserDefaults.javascript_open_windows_disallowed = true;
	browserDefaults.drag_drop_disabled = true;

	return browserDefaults;
}

#ifdef OS_WIN
void ChromiumBrowser::init(const char *defaultUrl)
{
	CefWindowInfo winInfo;

	winInfo.m_dwStyle =  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP;
	winInfo.m_nHeight = 500;
	winInfo.m_nWidth = 500;
	winInfo.m_hWndParent = m_hFormHandle;

	const char* name = "DesuraCEFBrowser";
	cef_string_copy(name, strlen(name), &winInfo.m_windowName);

	CefBrowser::CreateBrowser(winInfo, m_rEventHandler, defaultUrl, getBrowserDefaults());
}

#else

class CreateTask : public CefTask
{
public:
	CreateTask(ChromiumBrowser* browser, const std::string& defaultUrl)
	{
		m_pBrowser = browser;
		m_szDefaultUrl = defaultUrl;
	}

	void Execute(CefThreadId threadId)
	{
		m_pBrowser->initCallback(m_szDefaultUrl);
	}

	virtual int AddRef(){return 1;}
	virtual int Release(){return 1;}
	virtual int GetRefCt(){return 1;}

	ChromiumBrowser *m_pBrowser;
	std::string m_szDefaultUrl;
};

void ChromiumBrowser::init(const char *defaultUrl)
{
	CefPostTask(TID_UI, new CreateTask(this, defaultUrl));
}

void ChromiumBrowser::initCallback(const std::string& defaultUrl)
{
	CefWindowInfo winInfo;
	winInfo.SetAsChild(GTK_WIDGET(m_hFormHandle));

	m_pBrowser = CefBrowser::CreateBrowserSync(winInfo, m_rEventHandler, defaultUrl.c_str(), getBrowserDefaults());
	g_signal_connect(GTK_WIDGET(m_hFormHandle), "button-press-event", G_CALLBACK(gtkFocus), this);
	gtk_widget_show_all(GTK_WIDGET(m_hFormHandle));
}
#endif






void ChromiumBrowser::loadString(const char* string)
{
	if (m_pBrowser && m_pBrowser->GetMainFrame())
	{
		m_pBrowser->GetMainFrame()->LoadString(string, "http://local");
	}
	else
	{
		if (string)
			m_szBuffer = string;
		m_iLastTask = 1;
	}
}

void ChromiumBrowser::loadUrl(const char* url)
{
	if (m_pBrowser)
	{
		m_pBrowser->GetMainFrame()->LoadURL(url);
	}
	else
	{
		m_szBuffer = url;
		m_iLastTask = 2;
	}
}


void ChromiumBrowser::stop()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_STOPLOAD));
}

void ChromiumBrowser::refresh(bool ignoreCache)
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_REFRESH));
}

void ChromiumBrowser::back()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_BACK));
}

void ChromiumBrowser::forward()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_FORWARD));
}

void ChromiumBrowser::zoomIn()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_ZOOMIN));
}

void ChromiumBrowser::zoomOut()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_ZOOMOUT));	
}

void ChromiumBrowser::zoomNormal()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_ZOOMNORMAL));
}

void ChromiumBrowser::print()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_PRINT));
}

void ChromiumBrowser::viewSource()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_VIEWSOURCE));	
}

void ChromiumBrowser::undo()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_UNDO));
}

void ChromiumBrowser::redo()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_REDO));
}

void ChromiumBrowser::cut()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_CUT));
}


void ChromiumBrowser::copy()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_COPY));	
}

void ChromiumBrowser::paste()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_PASTE));	
}

void ChromiumBrowser::del()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_DEL));
}

void ChromiumBrowser::selectall()
{
	CefPostTask(TID_UI, new BrowserTask(m_pBrowser, A_SELECTALL));
}



void ChromiumBrowser::setEventCallback(ChromiumDLL::ChromiumBrowserEventI* cbeI)
{
	ChromiumBrowserEvents* cbe = (ChromiumBrowserEvents*)m_rEventHandler.get();

	if (cbe)
		cbe->setCallBack(cbeI);
}

void ChromiumBrowser::executeJScript(const char* code, const char* scripturl, int startline)
{
	if (!m_pBrowser || !m_pBrowser->GetMainFrame() || !code)
		return;

	m_pBrowser->GetMainFrame()->ExecuteJavaScript(code, scripturl?scripturl:"", startline);
}

void ChromiumBrowser::onFocus()
{
	if (m_pBrowser)
		m_pBrowser->SetFocus(true);
}

#if defined(_WIN32)
void ChromiumBrowser::onPaintBg()
{
	// Dont erase the background if the browser window has been loaded
	// (this avoids flashing)
}


void ChromiumBrowser::onPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hFormHandle, &ps);
	EndPaint(m_hFormHandle, &ps);
}

void ChromiumBrowser::onResize()
{
	HWND hWnd = m_hFormHandle;

	if(m_pBrowser && m_pBrowser->GetWindowHandle())
	{
		// Resize the browser window and address bar to match the new frame
		// window size
		RECT rect;
		::GetClientRect(hWnd, &rect);

		HDWP hdwp = BeginDeferWindowPos(1);
		hdwp = DeferWindowPos(hdwp, m_pBrowser->GetWindowHandle(), NULL,rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,SWP_NOZORDER);
		EndDeferWindowPos(hdwp);
	}
}

#else
void ChromiumBrowser::onResize(int x, int y, int width, int height)
{
	GtkAllocation a;
	a.x = x;
	a.y = y;
	a.width = width;
	a.height = height;

	gtk_widget_size_allocate(GTK_WIDGET(m_hFormHandle), &a);
	gtk_widget_set_size_request(GTK_WIDGET(m_hFormHandle), width, height);
}
#endif

void ChromiumBrowser::setBrowser(CefBrowser* browser)
{
	m_pBrowser = browser;

	if (m_iLastTask == 1)
	{
		loadString(m_szBuffer.c_str());
	}
	else if (m_iLastTask == 2)
	{
		loadUrl(m_szBuffer.c_str());
	}

	m_szBuffer = "";

#ifdef WIN32
	onResize();
#endif
}

void ChromiumBrowser::showInspector()
{
	if (m_pBrowser)
		m_pBrowser->ShowDevTools();
}

void ChromiumBrowser::hideInspector()
{
	if (m_pBrowser)
		m_pBrowser->CloseDevTools();
}

void ChromiumBrowser::inspectElement(int x, int y)
{
	if (m_pBrowser)
		m_pBrowser->InspectElement(x, y);
}

void ChromiumBrowser::scroll(int x, int y, int delta, unsigned int flags)
{
	if (m_pBrowser)
		m_pBrowser->MouseWheelEvent(x, y, delta, flags);
}

int* ChromiumBrowser::getBrowserHandle()
{
	if (m_pBrowser)
		return (int*)m_pBrowser->GetWindowHandle();

	return 0;
}

ChromiumDLL::JavaScriptContextI* ChromiumBrowser::getJSContext()
{
	if (m_pBrowser)
		return new JavaScriptContext(m_rContext);

	return NULL;
}

void ChromiumBrowser::setContext(CefRefPtr<CefV8Context> context)
{
	m_rContext = context;
}
