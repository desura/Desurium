#include <unistd.h>
#include <gtk/gtk.h>
#include "ChromiumBrowserI.h"
#include "SharedObjectLoader.h"

#define TEST_SINGLE_THREADED_MESSAGE_LOOP
#define MAX_PATH 1024
char szWorkingDir[MAX_PATH];   // The current working directory

GtkWidget* g_pWindow = NULL; 
GtkWidget* g_pEditWnd = NULL;

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    return FALSE;
}

static void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}
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


bool LoadBinaryResource(const char* name, int &size, unsigned char* buff)
{
	size = 0;
	buff = NULL;
	printf("Need to implement Load BinaryResource for item: %s\n", name);


	return false;
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

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    
	if(getcwd(szWorkingDir, MAX_PATH) == NULL)
		szWorkingDir[0] = 0;

	InitWebControl();

    g_pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(g_pWindow), "CEF Test");
	gtk_window_set_default_size(GTK_WINDOW(g_pWindow), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(g_pWindow), 10);

	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);


	gtk_container_add(GTK_CONTAINER(g_pWindow), vbox);
	gtk_widget_show_all(GTK_WIDGET(g_pWindow));

    g_signal_connect(g_pWindow, "delete-event", G_CALLBACK(delete_event), NULL);
    g_signal_connect(g_pWindow, "destroy", G_CALLBACK(destroy), NULL);

	NewChromiumBrowser((int*)vbox, "", "https://encrypted.google.com");

    gtk_widget_show(g_pWindow);

	while (gtk_main_iteration_do(false))
	{
		CEF_DoWork();
	}


	// Shut down the CEF
	CEF_Stop();
    
    return 0;
}
