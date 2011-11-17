///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : ChromiumBrowserDll
//   File        : ChromiumBrowser.h
//   Description :
//      [TODO: Write the purpose of ChromiumBrowser.h.]
//
//   Created On: 5/27/2010 1:52:56 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_CHROMIUMBROWSER_H
#define DESURA_CHROMIUMBROWSER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef.h"

class ChromiumBrowserEvents;


#ifdef OS_WIN
	typedef HWND WIN_HANDLE;
#else
	typedef void* WIN_HANDLE;
#endif

class ChromiumBrowser : public ChromiumDLL::ChromiumBrowserI
{
public:
	ChromiumBrowser(WIN_HANDLE handle, const char* defaultUrl);
	~ChromiumBrowser();

	void init(const char *defaultUrl);
	virtual void onFocus();

#ifdef OS_WIN
	virtual void onPaintBg();
	virtual void onPaint();
	virtual void onResize();
#else
	void initCallback(const std::string& defaultUrl);
	virtual void onResize(int x, int y, int width, int height);
#endif


	virtual void loadUrl(const char* url);
	virtual void loadString(const char* string);

	virtual void stop();
	virtual void refresh(bool ignoreCache = false);
	virtual void back();
	virtual void forward();

	virtual void zoomIn();
	virtual void zoomOut();
	virtual void zoomNormal();

	virtual void print();
	virtual void viewSource();

	virtual void undo();
	virtual void redo();
	virtual void cut();
	virtual void copy();
	virtual void paste();
	virtual void del();
	virtual void selectall();

	virtual void setEventCallback(ChromiumDLL::ChromiumBrowserEventI* cbe);
	virtual void executeJScript(const char* code, const char* scripturl = 0, int startline = 0);

	virtual void showInspector();
	virtual void hideInspector();
	virtual void inspectElement(int x, int y);

	virtual void scroll(int x, int y, int delta, unsigned int flags);

	virtual int* getBrowserHandle();

	virtual void destroy()
	{
		delete this;
	}

	virtual ChromiumDLL::JavaScriptContextI* getJSContext();

	void setBrowser(CefBrowser* browser);
	
	void setContext(CefRefPtr<CefV8Context> context);

protected:
	CefBrowserSettings getBrowserDefaults();

private:
	CefRefPtr<CefV8Context> m_rContext;
	CefRefPtr<CefClient> m_rEventHandler;
	CefBrowser* m_pBrowser;


	WIN_HANDLE m_hFormHandle;
	std::string m_szBuffer;
	int m_iLastTask;
};

class TaskWrapper : public CefRefCountWrapper<CefTask>
{
public:
	TaskWrapper(ChromiumDLL::CallbackI* callback)
	{
		m_pCallback = callback;
	}

	~TaskWrapper()
	{
		if (m_pCallback)
			m_pCallback->destroy();
	}

	virtual void Execute(CefThreadId threadId)
	{
		if (m_pCallback)
			m_pCallback->run();
	}

private:
	ChromiumDLL::CallbackI* m_pCallback;
};




#endif //DESURA_CHROMIUMBROWSER_H





