#define CEF_IGNORE_FUNCTIONS 1
#include "ChromiumBrowserI.h"
#include "SharedObjectLoader.h"
#include "wx/wx.h"

#include "wx/evtloop.h"
#include "wx/scopedptr.h"

#ifndef wxUSE_THREADS
#error "wxUSE_THREADS not defined!"
#endif

typedef bool (*CEF_InitFn)(bool, const char*, const char*, const char*);
typedef void (*CEF_StopFn)();

typedef void (*CEF_DeleteCookieFn)(const char*, const char*);
typedef ChromiumDLL::CookieI* (*CEF_CreateCookieFn)();
typedef void (*CEF_SetCookieFn)(const char* ulr, ChromiumDLL::CookieI*);
typedef ChromiumDLL::ChromiumBrowserI* (*CEF_NewChromiumBrowserFn)(int*, const char *,  const char*);

typedef void (*CEF_DoWorkFn)();

bool g_bLoaded = false;
SharedObjectLoader g_CEFDll;

CEF_InitFn CEF_Init = NULL;
CEF_StopFn CEF_Stop = NULL;
CEF_DeleteCookieFn CEF_DeleteCookie = NULL;
CEF_CreateCookieFn CEF_CreateCookie = NULL;
CEF_SetCookieFn CEF_SetCookie = NULL;
CEF_NewChromiumBrowserFn CEF_NewChromiumBrowser = NULL;
CEF_DoWorkFn CEF_DoWork = NULL;

void UnloadCEFDll()
{
	CEF_Init = NULL;
	CEF_Stop = NULL;
	CEF_DeleteCookie = NULL;
	CEF_CreateCookie = NULL;
	CEF_SetCookie = NULL;
	CEF_NewChromiumBrowser = NULL;

	g_CEFDll.unload();
}

bool LoadCEFDll()
{
	if (!g_CEFDll.load("libcef_desura.so"))
	{
		printf("Failed to load so\n");
		return false;
	}

	CEF_Init = g_CEFDll.getFunction<CEF_InitFn>("CEF_Init");
	CEF_Stop = g_CEFDll.getFunction<CEF_StopFn>("CEF_Stop");
	CEF_DeleteCookie = g_CEFDll.getFunction<CEF_DeleteCookieFn>("CEF_DeleteCookie");
	CEF_CreateCookie = g_CEFDll.getFunction<CEF_CreateCookieFn>("CEF_CreateCookie");
	CEF_SetCookie = g_CEFDll.getFunction<CEF_SetCookieFn>("CEF_SetCookie");
	CEF_NewChromiumBrowser = g_CEFDll.getFunction<CEF_NewChromiumBrowserFn>("CEF_NewChromiumBrowser");

	CEF_DoWork = g_CEFDll.getFunction<CEF_DoWorkFn>("CEF_DoMsgLoop");

	if (g_CEFDll.hasFailed())
	{
		printf("Failed to load functions\n");
		UnloadCEFDll();
		return false;
	}

	return true;
}

bool InitWebControl()
{
	if (g_bLoaded)
		return true;

	if (!LoadCEFDll())
	{
		printf("Failed to load cef dll\n");
		return false;
	}

	if (!CEF_Init(false, "", "", "Chrome Test"))
	{
		printf("Failed to init cef\n");
		return false;
	}

	g_bLoaded = true;
	return true;
}

void ShutdownWebControl()
{
	g_bLoaded = false;

	if (CEF_Stop)
		CEF_Stop();

	UnloadCEFDll();
}

ChromiumDLL::ChromiumBrowserI* NewChromiumBrowser(int* gtkWidget, const char* name, const char* loadUrl)
{
	if (!g_bLoaded && !InitWebControl())
	{
		printf("Failed to load web control: %s\n", dlerror());
		return NULL;
	}

	return CEF_NewChromiumBrowser(gtkWidget, name, loadUrl);
}

class gcWebControl : public wxPanel
{
public:
	gcWebControl(wxWindow* parent, std::string defaultUrl) : wxPanel(parent, wxID_ANY)
	{
		printf(">>> gcWebControl::gcWebControl() - Entering\n");

//		gdk_threads_enter();

		GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(parent->GetConnectWidget()), vbox);
		m_pBrowser = NewChromiumBrowser((int*)vbox, "Desura", defaultUrl.c_str());

//		gdk_flush();
//		gdk_threads_leave();
		printf(">>> gcWebControl::gcWebControl() - Leaving\n");

		Bind(wxEVT_SIZE, &gcWebControl::onResize, this);
	}

	void onResize(wxSizeEvent& event)
	{
		m_pBrowser->onResize(event.GetSize().GetWidth(), event.GetSize().GetHeight());
	}

	ChromiumDLL::ChromiumBrowserI* m_pBrowser;
};

class MyFrame : public wxFrame
{
public:
    MyFrame();
};

wxDEFINE_TIED_SCOPED_PTR_TYPE(wxEventLoopBase)

gboolean on50msTimeout(gpointer data)
{
	if(CEF_DoWork)
		CEF_DoWork();

	return true;
}

class MyApp : public wxApp
{
public:
	MyApp()
	{
	}

	virtual bool OnInit()
	{
		printf(">>> MyApp::OnInit() - Entering\n");
//		g_thread_init (NULL);
//		gdk_threads_init ();

//		gdk_threads_enter();

		if ( !wxApp::OnInit() )
		    return false;

		g_timeout_add(50, on50msTimeout, NULL);

/*		gdk_flush();
		gdk_threads_leave();*/

		MyFrame *frame = new MyFrame();
		frame->Show(true);

		printf(">>> MyApp::OnInit() - Leaving\n");
		return true;
	}
/*
	virtual int MainLoop()
	{
		wxEventLoopBaseTiedPtr mainLoop(&m_mainLoop, new EventLoop());
		return m_mainLoop->Run();
	}
*/
};

IMPLEMENT_APP(MyApp)

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "CEF Test")
{
	printf(">>> MyFrame::MyFrame() - Entering\n");
	gcWebControl* web = new gcWebControl(this, "http://desura.com");
//	wxButton* web = new wxButton(this, wxID_ANY, "Test button");
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

	this->SetBackgroundColour(*wxRED);
	sizer->Add(web,1 , wxEXPAND);
	this->SetSizer(sizer);
	this->Layout();

	printf(">>> MyFrame::MyFrame() - Leaving\n");
}

