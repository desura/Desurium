/*
 *  npw-wrapper.c - Host Mozilla plugin (loads the actual viewer)
 *
 *  nspluginwrapper (C) 2005-2009 Gwenole Beauchesne
 *                  (C) 2011 David Benjamin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define _GNU_SOURCE 1 /* RTLD_DEFAULT */
#include "sysdeps.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/wait.h>

#include <glib.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>

#include "utils.h"
#include "npw-common.h"
#include "npw-malloc.h"

#define DEBUG 1
#include "debug.h"


// Define to enable direct execution of native plugins
// (i.e. not running through npviewer.bin)
#define ALLOW_DIRECT_EXECUTION 1


// Globally exported plugin ident, used by the "npconfig" tool
#ifdef NIX32
const NPW_PluginInfo NPW_PluginDesura = {
  NPW_PLUGIN_IDENT,
  "libflashplayer.so",
  0,
  "",
  "",
  NPW_PLUGIN_INFO_VERSION,
  "desura_flashhost"
};
#else
const NPW_PluginInfo NPW_PluginDesura = {
  NPW_PLUGIN_IDENT,
  "libflashplayer.so",
  0,
  "",
  "",
  NPW_PLUGIN_INFO_VERSION,
  "desura_flashhost"
};
#endif

// Path to plugin to use
static const char *plugin_path = NPW_PluginDesura.path;

// Path to associated plugin viewer
static const char *plugin_viewer_path = NPW_PluginDesura.viewer_path;

// Netscape exported functions
static NPNetscapeFuncs mozilla_funcs;

// NPAPI version nspluginwrapper supports
static int npapi_version = 0;

// Wrapper plugin data
typedef struct {
  int initialized;
  int viewer_pid;
  int is_wrapper;
  char *name;
  char *description;
  char *formats;
} Plugin;

static Plugin g_plugin = { 0, -1, 0, NULL, NULL, NULL };

// Instance state information about the plugin
typedef struct _PluginInstance {
  NPW_DECL_PLUGIN_INSTANCE;
  rpc_connection_t *connection;
  NPP native_instance;
} PluginInstance;

#define PLUGIN_INSTANCE(instance) \
  ((PluginInstance *)NPW_PLUGIN_INSTANCE(instance))

#define PLUGIN_INSTANCE_NPP(plugin) \
  NPW_PLUGIN_INSTANCE_NPP((NPW_PluginInstance *)(plugin))

// Plugin side data for an NPStream instance
typedef struct _StreamInstance {
  NPW_DECL_STREAM_INSTANCE;
} StreamInstance;

// Prototypes
static void plugin_init(int is_NP_Initialize);
static void plugin_exit(void);

static void plugin_kill_cb(rpc_connection_t *connection, void *user_data);
static NPError plugin_start(void);
static NPError plugin_start_if_needed(void);
static int plugin_killed = 0;

/*
 *  Notes concerning NSPluginWrapper recovery model.
 *
 *  NSPluginWrapper will restart the Viewer if it detected to be
 *  dead. It will not attempt to "replay" the plugin. This means that
 *  if a plugin crashed, its window will remain grayed: only new
 *  instances will start a new viewer.
 *
 *  Each PlugInstance holds a reference to the RPC connection it was
 *  created with. g_rpc_connection can be seen as the "master"
 *  connection (used to initialize and shutdown things). The RPC
 *  connections are reference counted so that when the master
 *  connection is set to a new one, previous connections are still
 *  live. That way, old NPP instances are not passed down with the new
 *  connection and thus can fail early/gracefully in subsequent calls
 *  to NPP_*() functions.
 *
 *  All active NPRuntime objects are marked as inactive and 
 *  are no longer processed.
 */

// Minimal time between two plugin restarts in sec
#define MIN_RESTART_INTERVAL 1

// Consume as many bytes as possible when we are not NPP_WriteReady()
// XXX: move to a common place to Wrapper and Viewer
#define NPERR_STREAM_BUFSIZ 65536


/* ====================================================================== */
/* === Helpers                                                        === */
/* ====================================================================== */

// Flush the X output buffer
static void toolkit_flush(NPP instance)
{
  // Always prefer gdk_flush() if the master binary is linked against Gtk
  static void (*INVALID)(void) = (void (*)(void))(intptr_t)-1;
  static void (*lib_gdk_flush)(void) = NULL;
  if (lib_gdk_flush == NULL) {
	if ((lib_gdk_flush = dlsym(RTLD_DEFAULT, "gdk_flush")) == NULL)
	  lib_gdk_flush = INVALID;
  }
  if (lib_gdk_flush != INVALID) {
	lib_gdk_flush();
	return;
  }

  // Try raw X11
  Display *x_display = NULL;
  int error = mozilla_funcs.getvalue(instance, NPNVxDisplay, (void *)&x_display);
  if (error == NPERR_NO_ERROR && x_display) {
	XSync(x_display, False);
	return;
  }
}

static void pointer_ungrab(NPP instance, Time time)
{
  // Always prefer gdk_pointer_ungrab() if the master binary is linked against Gtk
  static void (*INVALID)(uint32_t) = (void (*)(uint32_t))(intptr_t)-1;
  static void (*lib_gdk_pointer_ungrab)(uint32_t) = NULL;
  if (lib_gdk_pointer_ungrab == NULL) {
	if ((lib_gdk_pointer_ungrab = dlsym(RTLD_DEFAULT, "gdk_pointer_ungrab")) == NULL)
	  lib_gdk_pointer_ungrab = INVALID;
  }
  if (lib_gdk_pointer_ungrab != INVALID) {
	lib_gdk_pointer_ungrab(time);
	return;
  }

  // Try raw X11
  Display *x_display = NULL;
  int error = mozilla_funcs.getvalue(instance, NPNVxDisplay, (void *)&x_display);
  if (error == NPERR_NO_ERROR && x_display) {
	XUngrabPointer(x_display, time);
	return;
  }
}

// PluginInstance vfuncs
static void *plugin_instance_allocate(void);
static void plugin_instance_deallocate(PluginInstance *plugin);
static void plugin_instance_finalize(PluginInstance *plugin);
static void plugin_instance_invalidate(PluginInstance *plugin);

static NPW_PluginInstanceClass PluginInstanceClass = {
  (NPW_PluginInstanceAllocateFunctionPtr)plugin_instance_allocate,
  (NPW_PluginInstanceDeallocateFunctionPtr)plugin_instance_deallocate,
  (NPW_PluginInstanceFinalizeFunctionPtr)plugin_instance_finalize,
  (NPW_PluginInstanceInvalidateFunctionPtr)plugin_instance_invalidate
};

static void *plugin_instance_allocate(void)
{
  return NPW_MemNew0(PluginInstance, 1);
}

static void plugin_instance_deallocate(PluginInstance *plugin)
{
  NPW_MemFree(plugin);
}

static void plugin_instance_finalize(PluginInstance *plugin)
{
  id_remove(plugin->instance_id);
  rpc_connection_unref(plugin->connection);
}

static void plugin_instance_invalidate(PluginInstance *plugin)
{
  /* Browser's NPP instance is no longer valid beyond this point. So,
	 let's just break the link to nspluginwrapper's PluginInstance now.  */
  if (plugin->instance) {
	plugin->instance->pdata = NULL;
	plugin->instance = NULL;
  }
  /* We don't reset instance_id here because we still need the NPP ->
	 PluginInstance mapping for incoming RPC. However, the important
	 thing is plugin->instance to be NULL.  */
}


/* ====================================================================== */
/* === Plug-in side data                                              === */
/* ====================================================================== */

// Functions supplied by the plug-in
static NPPluginFuncs plugin_funcs;

// Allows the browser to query the plug-in supported formats
static NP_GetMIMEDescriptionFunc g_plugin_NP_GetMIMEDescription = NULL;

// Allows the browser to query the plug-in for information
static NP_GetValueFunc g_plugin_NP_GetValue = NULL;

// Provides global initialization for a plug-in
static NP_InitializeFunc g_plugin_NP_Initialize = NULL;

// Provides global deinitialization for a plug-in
static NP_ShutdownFunc g_plugin_NP_Shutdown = NULL;

// Plugin native library handle
static void *plugin_handle = NULL;

static bool plugin_load_native(void)
{
  void *handle;
  const char *error;
  if ((handle = dlopen(plugin_path, RTLD_LOCAL|RTLD_LAZY)) == NULL) {
	npw_printf("ERROR: %s\n", dlerror());
	return false;
  }
  dlerror();
  g_plugin_NP_GetMIMEDescription = (NP_GetMIMEDescriptionFunc)dlsym(handle, "NP_GetMIMEDescription");
  if ((error = dlerror()) != NULL) {
	npw_printf("ERROR: %s\n", error);
	dlclose(handle);
	return false;
  }
  g_plugin_NP_Initialize = (NP_InitializeFunc)dlsym(handle, "NP_Initialize");
  if ((error = dlerror()) != NULL) {
	npw_printf("ERROR: %s\n", error);
	dlclose(handle);
	return false;
  }
  g_plugin_NP_Shutdown = (NP_ShutdownFunc)dlsym(handle, "NP_Shutdown");
  if ((error = dlerror()) != NULL) {
	npw_printf("ERROR: %s\n", error);
	dlclose(handle);
	return false;
  }
  g_plugin_NP_GetValue = (NP_GetValueFunc)dlsym(handle, "NP_GetValue");
  plugin_handle = handle;
  return true;
}

// Check for direct execution of the plugin
static inline bool plugin_has_direct_exec_env(void)
{
  if (getenv("NPW_DIRECT_EXEC"))
	return true;
  if (getenv("NPW_DIRECT_EXECUTION"))
	return true;
  return false;
}

static bool plugin_can_direct_exec(void)
{
  if (!plugin_has_direct_exec_env())
	return false;
  if (!plugin_load_native())
	return false;
  // XXX: really check for same OS/ARCH
  D(bug("Run plugin natively\n"));
  return true;
}

static inline bool plugin_direct_exec(void)
{
#if ALLOW_DIRECT_EXECUTION
  static int g_plugin_direct_exec = -1;
  if (G_UNLIKELY(g_plugin_direct_exec < 0))
	g_plugin_direct_exec = plugin_can_direct_exec();
  return g_plugin_direct_exec;
#else
  return false;
#endif
}

#define PLUGIN_DIRECT_EXEC plugin_direct_exec()


/* ====================================================================== */
/* === RPC communication                                              === */
/* ====================================================================== */

static GSource *g_rpc_source;
static GSource *g_rpc_sync_source;
static XtInputId xt_rpc_source_id;
static XtBlockHookId xt_rpc_sync_id;
rpc_connection_t *g_rpc_connection attribute_hidden = NULL;


/* ====================================================================== */
/* === Browser side plug-in API                                       === */
/* ====================================================================== */

// Does browser have specified feature?
#define NPN_HAS_FEATURE(FEATURE) ((mozilla_funcs.version & 0xff) >= NPVERS_HAS_##FEATURE)

// NPN_MemAlloc
static inline void *g_NPN_MemAlloc(uint32_t size)
{
  return mozilla_funcs.memalloc(size);
}

// NPN_MemFree
static inline void g_NPN_MemFree(void* ptr)
{
  mozilla_funcs.memfree(ptr);
}

// NPN_MemFlush
static inline uint32_t g_NPN_MemFlush(uint32_t size)
{
  return mozilla_funcs.memflush(size);
}

// NPN_ReloadPlugins
static void
g_NPN_ReloadPlugins(NPBool reloadPages)
{
  D(bug("NPN_ReloadPlugins reloadPages=%d\n", reloadPages));

  NPW_UNIMPLEMENTED();
}

// NPN_GetJavaEnv
static void *
g_NPN_GetJavaEnv(void)
{
  D(bug("NPN_GetJavaEnv\n"));

  return NULL;
}

// NPN_GetJavaPeer
static void *
g_NPN_GetJavaPeer(NPP instance)
{
  D(bug("NPN_GetJavaPeer instance=%p\n", instance));

  return NULL;
}

// NPN_UserAgent
static const char *g_NPN_UserAgent(NPP instance)
{
  if (mozilla_funcs.uagent == NULL)
	return NULL;

  D(bugiI("NPN_UserAgent instance=%p\n", instance));
  const char *user_agent = mozilla_funcs.uagent(instance);
  D(bugiD("NPN_UserAgent return: '%s'\n", user_agent));
  return user_agent;
}

static int handle_NPN_UserAgent(rpc_connection_t *connection)
{
  D(bug("handle_NPN_UserAgent\n"));

  int error = rpc_method_get_args(connection, RPC_TYPE_INVALID);
  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_UserAgent() get args", error);
	return error;
  }

  const char *user_agent = g_NPN_UserAgent(NULL);
  return rpc_method_send_reply(connection, RPC_TYPE_STRING, user_agent, RPC_TYPE_INVALID);
}

// NPN_Status
static void
g_NPN_Status(NPP instance, const char *message)
{
  if (mozilla_funcs.status == NULL)
	return;

  D(bugiI("NPN_Status instance=%p, message='%s'\n", instance, message));
  mozilla_funcs.status(instance, message);
  D(bugiD("NPN_Status done\n"));
}

static int handle_NPN_Status(rpc_connection_t *connection)
{
  D(bug("handle_NPN_Status\n"));

  PluginInstance *plugin;
  char *message;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_STRING, &message,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_Status() get args", error);
	return error;
  }

  g_NPN_Status(PLUGIN_INSTANCE_NPP(plugin), message);

  if (message)
	free(message);
  return rpc_method_send_reply (connection, RPC_TYPE_INVALID);
}

// NPN_GetValue
static NPError
g_NPN_GetValue(NPP instance, NPNVariable variable, void *value)
{
  if (mozilla_funcs.getvalue == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_GetValue instance=%p, variable=%d [%s]\n", instance, variable, string_of_NPNVariable(variable)));
  NPError ret = mozilla_funcs.getvalue(instance, variable, value);
  D(bugiD("NPN_GetValue return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_GetValue(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetValue\n"));

  PluginInstance *plugin;
  uint32_t variable;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_UINT32, &variable,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetValue() get args", error);
	return error;
  }

  // Work around a Chromium sort-of bug and check for NULL NPPs. It is
  // only sort of a bug in that we are doing something our plugin had
  // never requested. Unfortunately, the existing RPC system makes
  // detecting this very difficult. So, hack in the checks Chromium
  // was missing.
  //
  // TODO: Either fix bug #13 or remove this when the fix in Chromium
  // has trickled down to the stable channel.
  bool valid_instance = true;
  if (PLUGIN_INSTANCE_NPP(plugin) == NULL) {
	switch (variable) {
	case NPNVWindowNPObject:
	case NPNVPluginElementNPObject:
	case NPNVprivateModeBool:
	case NPNVnetscapeWindow:
	  D(bug("Skipping NPN_GetValue on NULL instance to avoid possible crash.\n"));
	  valid_instance = false;
	}
  }

  NPError ret = NPERR_GENERIC_ERROR;
  switch (rpc_type_of_NPNVariable(variable)) {
  case RPC_TYPE_UINT32:
	{
	  uint32_t n = 0;
	  if (valid_instance)
		ret = g_NPN_GetValue(PLUGIN_INSTANCE_NPP(plugin), variable, (void *)&n);
	  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_UINT32, n, RPC_TYPE_INVALID);
	}
  case RPC_TYPE_BOOLEAN:
	{
	  NPBool b = FALSE;
	  if (valid_instance)
		ret = g_NPN_GetValue(PLUGIN_INSTANCE_NPP(plugin), variable, (void *)&b);
	  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_BOOLEAN, b, RPC_TYPE_INVALID);
	}
  case RPC_TYPE_NP_OBJECT:
	{
	  NPObject *npobj = NULL;
	  if (valid_instance)
		ret = g_NPN_GetValue(PLUGIN_INSTANCE_NPP(plugin), variable, (void *)&npobj);
	  return rpc_method_send_reply(connection,
								   RPC_TYPE_INT32, ret,
								   RPC_TYPE_NP_OBJECT_PASS_REF, npobj,
								   RPC_TYPE_INVALID);
	}
  }

  abort();
}

// NPN_SetValue
static NPError
g_NPN_SetValue(NPP instance, NPPVariable variable, void *value)
{
  if (mozilla_funcs.setvalue == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_SetValue instance=%p, variable=%d [%s]\n", instance, variable, string_of_NPPVariable(variable)));
  NPError ret = mozilla_funcs.setvalue(instance, variable, value);
  D(bugiD("NPN_SetValue return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_SetValue(rpc_connection_t *connection)
{
  D(bug("handle_NPN_SetValue\n"));

  PluginInstance *plugin;
  uint32_t variable, value;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_UINT32, &variable,
								  RPC_TYPE_BOOLEAN, &value,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_SetValue() get args", error);
	return error;
  }

  NPError ret = NPERR_GENERIC_ERROR;

  // Work around a Chromium sort-of bug and check for NULL NPPs. It is
  // only sort of a bug in that we are doing something our plugin had
  // never requested. Unfortunately, the existing RPC system makes
  // detecting this very difficult. So, hack in the checks Chromium
  // was missing.
  //
  // TODO: Either fix bug #13 or remove this when the fix in Chromium
  // has trickled down to the stable channel.
  if (PLUGIN_INSTANCE_NPP(plugin) == NULL) {
	D(bug("Skipping NPN_SetValue on NULL instance to avoid possible crash.\n"));
	ret = NPERR_INVALID_INSTANCE_ERROR;
  } else {
	ret = g_NPN_SetValue(PLUGIN_INSTANCE_NPP(plugin), variable, (void *)(uintptr_t)value);
  }

  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_InvalidateRect
static void g_NPN_InvalidateRect(NPP instance, NPRect *invalidRect)
{
  if (mozilla_funcs.invalidaterect == NULL)
	return;

  D(bugiI("NPN_InvalidateRect instance=%p "
		  "rect.top=%d rect.left=%d rect.bottom=%d rect.right=%d\n",
		  instance,
		  invalidRect->top, invalidRect->left,
		  invalidRect->bottom, invalidRect->right));
  mozilla_funcs.invalidaterect(instance, invalidRect);
  D(bugiD("NPN_InvalidateRect done\n"));
}

static int handle_NPN_InvalidateRect(rpc_connection_t *connection)
{
  D(bug("handle_NPN_InvalidateRect\n"));

  PluginInstance *plugin;
  NPRect invalidRect;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_RECT, &invalidRect,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_InvalidateRect() get args", error);
	return error;
  }

  g_NPN_InvalidateRect(PLUGIN_INSTANCE_NPP(plugin), &invalidRect);

  return rpc_method_send_reply (connection, RPC_TYPE_INVALID);
}

// NPN_InvalidateRegion
static void
g_NPN_InvalidateRegion(NPP instance, NPRegion invalidRegion)
{
  D(bug("NPN_InvalidateRegion instance=%p\n", instance));

  NPW_UNIMPLEMENTED();
}

// NPN_ForceRedraw
static void
g_NPN_ForceRedraw(NPP instance)
{
  D(bug("NPN_ForceRedraw instance=%p\n", instance));

  NPW_UNIMPLEMENTED();
}

// NPN_GetURL
static NPError g_NPN_GetURL(NPP instance, const char *url, const char *target)
{
  if (mozilla_funcs.geturl == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_GetURL instance=%p, url='%s', target='%s'\n", instance, url, target));
  NPError ret = mozilla_funcs.geturl(instance, url, target);
  D(bugiD("NPN_GetURL return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_GetURL(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetURL\n"));

  PluginInstance *plugin;
  char *url, *target;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_STRING, &url,
								  RPC_TYPE_STRING, &target,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetURL() get args", error);
	return error;
  }

  NPError ret = g_NPN_GetURL(PLUGIN_INSTANCE_NPP(plugin), url, target);

  if (url)
	free(url);
  if (target)
	free(target);

  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_GetURLNotify
static NPError g_NPN_GetURLNotify(NPP instance, const char *url, const char *target, void *notifyData)
{
  if (mozilla_funcs.geturlnotify == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_GetURLNotify instance=%p, url='%s', target='%s', notifyData=%p\n", instance, url, target, notifyData));
  NPError ret = mozilla_funcs.geturlnotify(instance, url, target, notifyData);
  D(bugiD("NPN_GetURLNotify return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_GetURLNotify(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetURLNotify\n"));

  PluginInstance *plugin;
  char *url, *target;
  void *notifyData;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_STRING, &url,
								  RPC_TYPE_STRING, &target,
								  RPC_TYPE_NP_NOTIFY_DATA, &notifyData,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetURLNotify() get args", error);
	return error;
  }

  NPError ret = g_NPN_GetURLNotify(PLUGIN_INSTANCE_NPP(plugin), url, target, notifyData);

  if (url)
	free(url);
  if (target)
	free(target);

  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_PostURL
static NPError g_NPN_PostURL(NPP instance, const char *url, const char *target, uint32_t len, const char *buf, NPBool file)
{
  if (mozilla_funcs.posturl == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_PostURL instance=%p, url='%s', target='%s', file='%s'\n", instance, url, target, file ? buf : "<raw-data>"));
  NPError ret = mozilla_funcs.posturl(instance, url, target, len, buf, file);
  D(bugiD("NPN_PostURL return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_PostURL(rpc_connection_t *connection)
{
  D(bug("handle_NPN_PostURL\n"));

  PluginInstance *plugin;
  char *url, *target;
  uint32_t len;
  char *buf;
  uint32_t file;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_STRING, &url,
								  RPC_TYPE_STRING, &target,
								  RPC_TYPE_ARRAY, RPC_TYPE_CHAR, &len, &buf,
								  RPC_TYPE_BOOLEAN, &file,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_PostURL() get args", error);
	return error;
  }

  NPError ret = g_NPN_PostURL(PLUGIN_INSTANCE_NPP(plugin), url, target, len, buf, file);

  if (url)
	free(url);
  if (target)
	free(target);
  if (buf)
	free(buf);

  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_PostURLNotify
static NPError g_NPN_PostURLNotify(NPP instance, const char *url, const char *target, uint32_t len, const char *buf, NPBool file, void *notifyData)
{
  if (mozilla_funcs.posturlnotify == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_PostURLNotify instance=%p, url='%s', target='%s', file='%s', notifyData=%p\n", instance, url, target, file ? buf : "<raw-data>", notifyData));
  NPError ret = mozilla_funcs.posturlnotify(instance, url, target, len, buf, file, notifyData);
  D(bugiD("NPN_PostURLNotify return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_PostURLNotify(rpc_connection_t *connection)
{
  D(bug("handle_NPN_PostURLNotify\n"));

  PluginInstance *plugin;
  char *url, *target;
  int32_t len;
  char *buf;
  uint32_t file;
  void *notifyData;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_STRING, &url,
								  RPC_TYPE_STRING, &target,
								  RPC_TYPE_ARRAY, RPC_TYPE_CHAR, &len, &buf,
								  RPC_TYPE_BOOLEAN, &file,
								  RPC_TYPE_NP_NOTIFY_DATA, &notifyData,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_PostURLNotify() get args", error);
	return error;
  }

  NPError ret = g_NPN_PostURLNotify(PLUGIN_INSTANCE_NPP(plugin), url, target, len, buf, file, notifyData);

  if (url)
	free(url);
  if (target)
	free(target);
  if (buf)
	free(buf);

  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_PrintData
static int handle_NPN_PrintData(rpc_connection_t *connection)
{
  D(bug("handle_NPN_PrintData\n"));

  uint32_t platform_print_id;
  NPPrintData printData;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_UINT32, &platform_print_id,
								  RPC_TYPE_NP_PRINT_DATA, &printData,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_PrintData() get args", error);
	return error;
  }

  NPPrintCallbackStruct *platformPrint = id_lookup(platform_print_id);
  if (platformPrint == NULL)
	return RPC_ERROR_GENERIC;
  D(bug(" platformPrint=%p, printData.size=%d\n", platformPrint, printData.size));
  if (fwrite(printData.data, printData.size, 1, platformPrint->fp) != 1)
	return RPC_ERROR_ERRNO_SET;

  return rpc_method_send_reply (connection, RPC_TYPE_INVALID);
}

// NPN_RequestRead
static NPError g_NPN_RequestRead(NPStream *stream, NPByteRange *rangeList)
{
  if (mozilla_funcs.requestread == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_RequestRead stream=%p, rangeList=%p\n", stream, rangeList));
  NPError ret = mozilla_funcs.requestread(stream, rangeList);
  D(bugiD("NPN_RequestRead return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_RequestRead(rpc_connection_t *connection)
{
  D(bug("handle_NPN_RequestRead\n"));

  NPStream *stream;
  NPByteRange *rangeList;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_STREAM, &stream,
								  RPC_TYPE_NP_BYTE_RANGE, &rangeList,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_RequestRead() get args", error);
	return error;
  }

  NPError ret = g_NPN_RequestRead(stream, rangeList);

  while (rangeList) {
	NPByteRange *p = rangeList;
	rangeList = rangeList->next;
	free(p);
  }

  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_NewStream
static NPError g_NPN_NewStream(NPP instance, NPMIMEType type, const char *target, NPStream **stream)
{
  if (mozilla_funcs.newstream == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  if (stream == NULL)
	return NPERR_INVALID_PARAM;

  D(bugiI("NPN_NewStream instance=%p, type='%s', target='%s'\n", instance, type, target));
  NPError ret = mozilla_funcs.newstream(instance, type, target, stream);
  D(bugiD("NPN_NewStream return: %d [%s]\n", ret, string_of_NPError(ret)));

  if (PLUGIN_DIRECT_EXEC)
	return ret;

  if (ret == NPERR_NO_ERROR) {
	StreamInstance *stream_pdata = malloc(sizeof(*stream_pdata));
	if (stream_pdata == NULL)
	  return NPERR_OUT_OF_MEMORY_ERROR;
	memset(stream_pdata, 0, sizeof(*stream_pdata));
	stream_pdata->stream = *stream;
	stream_pdata->stream_id = id_create(stream_pdata);
	stream_pdata->is_plugin_stream = 1;
	(*stream)->pdata = stream_pdata;
  }
  else {
	static const StreamInstance fake_StreamInstance = {
	  .stream = NULL,
	  .stream_id = 0
	};
	static const NPStream fake_NPStream = {
	  .pdata = (void *)&fake_StreamInstance,
	  .url = NULL,
	  .end = 0,
	  .lastmodified = 0,
	  .notifyData = NULL
	};
	*stream = (void *)&fake_NPStream;
  }

  return ret;
}

static int handle_NPN_NewStream(rpc_connection_t *connection)
{
  D(bug("handle_NPN_NewStream\n"));

  PluginInstance *plugin;
  char *type;
  char *target;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_STRING, &type,
								  RPC_TYPE_STRING, &target,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_NewStream() get args", error);
	return error;
  }

  NPStream *stream;
  NPError ret = g_NPN_NewStream(PLUGIN_INSTANCE_NPP(plugin), type, target, &stream);

  if (type)
	free(type);
  if (target)
	free(target);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_INT32, ret,
							   RPC_TYPE_UINT32, ((StreamInstance *)stream->pdata)->stream_id,
							   RPC_TYPE_STRING, stream->url,
							   RPC_TYPE_UINT32, stream->end,
							   RPC_TYPE_UINT32, stream->lastmodified,
							   RPC_TYPE_NP_NOTIFY_DATA, stream->notifyData,
							   RPC_TYPE_STRING, NPN_HAS_FEATURE(RESPONSE_HEADERS) ? stream->headers : NULL,
							   RPC_TYPE_INVALID);
}

// NPN_DestroyStream
static NPError
g_NPN_DestroyStream(NPP instance, NPStream *stream, NPReason reason)
{
  if (mozilla_funcs.destroystream == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  if (stream == NULL)
	return NPERR_INVALID_PARAM;

  // Mozilla calls NPP_DestroyStream() for its streams, keep stream
  // info in that case
  if (!PLUGIN_DIRECT_EXEC) {
	StreamInstance *stream_pdata = stream->pdata;
	if (stream_pdata && stream_pdata->is_plugin_stream) {
	  id_remove(stream_pdata->stream_id);
	  free(stream->pdata);
	  stream->pdata = NULL;
	}
  }

  D(bugiI("NPN_DestroyStream instance=%p, stream=%p, reason=%s\n",
		instance, stream, string_of_NPReason(reason)));
  NPError ret = mozilla_funcs.destroystream(instance, stream, reason);
  D(bugiD("NPN_DestroyStream return: %d [%s]\n", ret, string_of_NPError(ret)));

  return ret;
}

static int handle_NPN_DestroyStream(rpc_connection_t *connection)
{
  D(bug("handle_NPN_DestroyStream\n"));

  PluginInstance *plugin;
  NPStream *stream;
  int32_t reason;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_STREAM, &stream,
								  RPC_TYPE_INT32, &reason,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_DestroyStream() get args", error);
	return error;
  }

  NPError ret = g_NPN_DestroyStream(PLUGIN_INSTANCE_NPP(plugin), stream, reason);
  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_Write
static int32_t g_NPN_Write(NPP instance, NPStream *stream, int32_t len, void *buf)
{
  if (mozilla_funcs.write == NULL)
	return -1;

  if (stream == NULL)
	return -1;

  D(bugiI("NPN_Write instance=%p\n", instance));
  int32_t ret = mozilla_funcs.write(instance, stream, len, buf);
  D(bugiD("NPN_Write return: %d\n", ret));
  return ret;
}

static int handle_NPN_Write(rpc_connection_t *connection)
{
  D(bug("handle_NPN_Write\n"));

  PluginInstance *plugin;
  NPStream *stream;
  unsigned char *buf;
  int32_t len;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_STREAM, &stream,
								  RPC_TYPE_ARRAY, RPC_TYPE_CHAR, &len, &buf,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_Write() get args", error);
	return error;
  }

  int32_t ret = g_NPN_Write(PLUGIN_INSTANCE_NPP(plugin), stream, len, buf);

  if (buf)
	free(buf);

  return rpc_method_send_reply(connection, RPC_TYPE_INT32, ret, RPC_TYPE_INVALID);
}

// NPN_PushPopupsEnabledState
static void g_NPN_PushPopupsEnabledState(NPP instance, NPBool enabled)
{
  if (mozilla_funcs.pushpopupsenabledstate == NULL)
	return;

  D(bugiI("NPN_PushPopupsEnabledState instance=%p, enabled=%d\n", instance, enabled));
  mozilla_funcs.pushpopupsenabledstate(instance, enabled);
  D(bugiD("NPN_PushPopupsEnabledState done\n"));
}

static int handle_NPN_PushPopupsEnabledState(rpc_connection_t *connection)
{
  D(bug("handle_NPN_PushPopupsEnabledState\n"));

  PluginInstance *plugin;
  uint32_t enabled;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_UINT32, &enabled,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_PushPopupsEnabledState() get args", error);
	return error;
  }

  g_NPN_PushPopupsEnabledState(PLUGIN_INSTANCE_NPP(plugin), enabled);

  return rpc_method_send_reply (connection, RPC_TYPE_INVALID);
}

// NPN_PopPopupsEnabledState
static void g_NPN_PopPopupsEnabledState(NPP instance)
{
  if (mozilla_funcs.poppopupsenabledstate == NULL)
	return;

  D(bugiI("NPN_PopPopupsEnabledState instance=%p\n", instance));
  mozilla_funcs.poppopupsenabledstate(instance);
  D(bugiD("NPN_PopPopupsEnabledState done\n"));
}

static int handle_NPN_PopPopupsEnabledState(rpc_connection_t *connection)
{
  D(bug("handle_NPN_PopPopupsEnabledState\n"));

  PluginInstance *plugin;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_PopPopupsEnabledState() get args", error);
	return error;
  }

  g_NPN_PopPopupsEnabledState(PLUGIN_INSTANCE_NPP(plugin));

  return rpc_method_send_reply (connection, RPC_TYPE_INVALID);
}


/* ====================================================================== */
/* === NPRuntime glue                                                 === */
/* ====================================================================== */

// NPN_CreateObject
static NPObject *
g_NPN_CreateObject(NPP instance, NPClass *klass)
{
  D(bugiI("NPN_CreateObject instance=%p, aClass=%p\n", instance, klass));
  NPObject *npobj = mozilla_funcs.createobject(instance, klass);
  D(bugiD("NPN_CreateObject return: %p\n", npobj));
  return npobj;
}

// NPN_RetainObject
static NPObject *
g_NPN_RetainObject(NPObject *npobj)
{
  D(bugiI("NPN_RetainObject npobj=%p\n", npobj));
  NPObject *new_npobj = mozilla_funcs.retainobject(npobj);
  D(bugiD("NPN_RetainObject return: %p (refcount: %d)\n", new_npobj, new_npobj->referenceCount));
  return new_npobj;
}

// NPN_ReleaseObject
static void
g_NPN_ReleaseObject(NPObject *npobj)
{
  D(bugiI("NPN_ReleaseObject npobj=%p\n", npobj));
  uint32_t refcount = npobj->referenceCount - 1;
  NPN_ReleaseObject(npobj);
  D(bugiD("NPN_ReleaseObject done (refcount: %d)\n", refcount));
}

// NPN_Invoke
static bool
g_NPN_Invoke(NPP instance, NPObject *npobj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  D(bugiI("NPN_Invoke instance=%p, npobj=%p, methodName=%p\n", instance, npobj, methodName));
  print_npvariant_args(args, argCount);
  bool ret = mozilla_funcs.invoke(instance, npobj, methodName, args, argCount, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPN_Invoke return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

static int handle_NPN_Invoke(rpc_connection_t *connection)
{
  D(bug("handle_NPN_Invoke()\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPIdentifier methodName;
  NPVariant *args;
  uint32_t argCount;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &methodName,
								  RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, &argCount, &args,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_Invoke() get args", error);
	return error;
  }

  NPVariant result;
  VOID_TO_NPVARIANT(result);
  bool ret = g_NPN_Invoke(PLUGIN_INSTANCE_NPP(plugin), npobj, methodName, args, argCount, &result);

  if (npobj)
	NPN_ReleaseObject(npobj);
  if (args) {
	for (int i = 0; i < argCount; i++)
	  NPN_ReleaseVariantValue(&args[i]);
	free(args);
  }

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_NP_VARIANT_PASS_REF, &result,
							   RPC_TYPE_INVALID);
}

// NPN_InvokeDefault
static bool
g_NPN_InvokeDefault(NPP instance, NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  D(bugiI("NPN_InvokeDefault instance=%p, npobj=%p\n", instance, npobj));
  print_npvariant_args(args, argCount);
  bool ret = mozilla_funcs.invokeDefault(instance, npobj, args, argCount, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPN_InvokeDefault return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

static int handle_NPN_InvokeDefault(rpc_connection_t *connection)
{
  D(bug("handle_NPN_InvokeDefault\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPVariant *args;
  uint32_t argCount;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, &argCount, &args,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_InvokeDefault() get args", error);
	return error;
  }

  NPVariant result;
  VOID_TO_NPVARIANT(result);
  bool ret = g_NPN_InvokeDefault(PLUGIN_INSTANCE_NPP(plugin), npobj, args, argCount, &result);

  if (npobj)
	NPN_ReleaseObject(npobj);
  if (args) {
	for (int i = 0; i < argCount; i++)
	  NPN_ReleaseVariantValue(&args[i]);
	free(args);
  }

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_NP_VARIANT_PASS_REF, &result,
							   RPC_TYPE_INVALID);
}

// NPN_Evaluate
static bool
g_NPN_Evaluate(NPP instance, NPObject *npobj, NPString *script, NPVariant *result)
{
  D(bugiI("NPN_Evaluate instance=%p, npobj=%p\n", instance, npobj));
  D(bug("script = '%.*s'\n", script->UTF8Length, script->UTF8Characters));
  bool ret = mozilla_funcs.evaluate(instance, npobj, script, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPN_Evaluate return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

static int handle_NPN_Evaluate(rpc_connection_t *connection)
{
  D(bug("handle_NPN_Evaluate\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPString script;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_STRING, &script,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_Evaluate() get args", error);
	return error;
  }

  NPVariant result;
  VOID_TO_NPVARIANT(result);
  bool ret = g_NPN_Evaluate(PLUGIN_INSTANCE_NPP(plugin), npobj, &script, &result);

  if (npobj)
	NPN_ReleaseObject(npobj);
  if (script.UTF8Characters)
	NPN_MemFree((void *)script.UTF8Characters);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_NP_VARIANT_PASS_REF, &result,
							   RPC_TYPE_INVALID);
}

// NPN_GetProperty
static bool
g_NPN_GetProperty(NPP instance, NPObject *npobj, NPIdentifier propertyName, NPVariant *result)
{
  D(bugiI("NPN_GetProperty instance=%p, npobj=%p, propertyName=%p\n", instance, npobj, propertyName));
  bool ret = mozilla_funcs.getproperty(instance, npobj, propertyName, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPN_GetProperty return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

static int handle_NPN_GetProperty(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetProperty\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPIdentifier propertyName;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &propertyName,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetProperty() get args", error);
	return error;
  }

  NPVariant result;
  VOID_TO_NPVARIANT(result);
  bool ret = g_NPN_GetProperty(PLUGIN_INSTANCE_NPP(plugin), npobj, propertyName, &result);

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_NP_VARIANT_PASS_REF, &result,
							   RPC_TYPE_INVALID);
}

// NPN_SetProperty
static bool
g_NPN_SetProperty(NPP instance, NPObject *npobj, NPIdentifier propertyName, const NPVariant *value)
{
  D(bugiI("NPN_SetProperty instance=%p, npobj=%p, propertyName=%p\n", instance, npobj, propertyName));
  bool ret = mozilla_funcs.setproperty(instance, npobj, propertyName, value);
  D(bugiD("NPN_SetProperty return: %d\n", ret));
  return ret;
}

static int handle_NPN_SetProperty(rpc_connection_t *connection)
{
  D(bug("handle_NPN_SetProperty\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPIdentifier propertyName;
  NPVariant value;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &propertyName,
								  RPC_TYPE_NP_VARIANT, &value,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_SetProperty() get args", error);
	return error;
  }

  bool ret = g_NPN_SetProperty(PLUGIN_INSTANCE_NPP(plugin), npobj, propertyName, &value);

  if (npobj)
	NPN_ReleaseObject(npobj);
  NPN_ReleaseVariantValue(&value);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

// NPN_RemoveProperty
static bool
g_NPN_RemoveProperty(NPP instance, NPObject *npobj, NPIdentifier propertyName)
{
  D(bugiI("NPN_RemoveProperty instance=%p, npobj=%p, propertyName=%p\n", instance, npobj, propertyName));
  bool ret = mozilla_funcs.removeproperty(instance, npobj, propertyName);
  D(bugiD("NPN_RemoveProperty return: %d\n", ret));
  return ret;
}

static int handle_NPN_RemoveProperty(rpc_connection_t *connection)
{
  D(bug("handle_NPN_RemoveProperty\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPIdentifier propertyName;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &propertyName,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_RemoveProperty() get args", error);
	return error;
  }

  bool ret = g_NPN_RemoveProperty(PLUGIN_INSTANCE_NPP(plugin), npobj, propertyName);

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

// NPN_HasProperty
static bool
g_NPN_HasProperty(NPP instance, NPObject *npobj, NPIdentifier propertyName)
{
  D(bugiI("NPN_HasProperty instance=%p, npobj=%p, propertyName=%p\n", instance, npobj, propertyName));
  bool ret = mozilla_funcs.hasproperty(instance, npobj, propertyName);
  D(bugiD("NPN_HasProperty return: %d\n", ret));
  return ret;
}

static int handle_NPN_HasProperty(rpc_connection_t *connection)
{
  D(bug("handle_NPN_HasProperty\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPIdentifier propertyName;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &propertyName,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_HasProperty() get args", error);
	return error;
  }

  bool ret = g_NPN_HasProperty(PLUGIN_INSTANCE_NPP(plugin), npobj, propertyName);

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

// NPN_HasMethod
static bool
g_NPN_HasMethod(NPP instance, NPObject *npobj, NPIdentifier methodName)
{
  D(bugiI("NPN_HasMethod instance=%p, npobj=%p, methodName=%p\n", instance, npobj, methodName));
  bool ret = mozilla_funcs.hasmethod(instance, npobj, methodName);
  D(bugiD("NPN_HasMethod return: %d\n", ret));
  return ret;
}

static int handle_NPN_HasMethod(rpc_connection_t *connection)
{
  D(bug("handle_NPN_HasMethod\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPIdentifier methodName;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &methodName,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_HasMethod() get args", error);
	return error;
  }

  bool ret = g_NPN_HasMethod(PLUGIN_INSTANCE_NPP(plugin), npobj, methodName);

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

// NPN_Enumerate
static bool
g_NPN_Enumerate(NPP instance, NPObject *npobj, NPIdentifier **identifiers,
				uint32_t *count)
{
  if (mozilla_funcs.enumerate == NULL)
	return false;

  D(bugiI("NPN_Enumerate instance=%p, npobj=%p\n", instance, npobj));
  bool ret = mozilla_funcs.enumerate(instance, npobj, identifiers, count);
  D(bugiD("NPN_Enumerate return: %d\n", ret));
  return ret;
}

static int handle_NPN_Enumerate(rpc_connection_t *connection)
{
  D(bug("handle_NPN_Enumerate\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_Enumerate() get args", error);
	return error;
  }

  NPIdentifier *identifiers = NULL;
  uint32_t count = 0;
  bool ret = g_NPN_Enumerate(PLUGIN_INSTANCE_NPP(plugin), npobj, &identifiers, &count);

  if (npobj)
	NPN_ReleaseObject(npobj);

  error = rpc_method_send_reply(connection,
								RPC_TYPE_UINT32, ret,
								RPC_TYPE_ARRAY, RPC_TYPE_NP_IDENTIFIER, count, identifiers,
								RPC_TYPE_INVALID);
  NPN_MemFree(identifiers);
  return error;
}

// NPN_Construct
static bool
g_NPN_Construct(NPP instance, NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
  if (mozilla_funcs.construct == NULL)
	return false;

  D(bugiI("NPN_Construct instance=%p, npobj=%p\n", instance, npobj));
  print_npvariant_args(args, argCount);
  bool ret = mozilla_funcs.construct(instance, npobj, args, argCount, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPN_Construct return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

static int handle_NPN_Construct(rpc_connection_t *connection)
{
  D(bug("handle_NPN_Construct\n"));

  PluginInstance *plugin;
  NPObject *npobj;
  NPVariant *args;
  uint32_t argCount;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, &argCount, &args,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_Construct() get args", error);
	return error;
  }

  NPVariant result;
  VOID_TO_NPVARIANT(result);
  bool ret = g_NPN_Construct(PLUGIN_INSTANCE_NPP(plugin), npobj, args, argCount, &result);

  if (npobj)
	NPN_ReleaseObject(npobj);
  if (args) {
	for (int i = 0; i < argCount; i++)
	  NPN_ReleaseVariantValue(&args[i]);
	free(args);
  }

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_NP_VARIANT_PASS_REF, &result,
							   RPC_TYPE_INVALID);
}

// NPN_SetException
static void
g_NPN_SetException(NPObject *npobj, const char *message)
{
  D(bugiI("NPN_SetException npobj=%p, message='%s'\n", npobj, message));
  mozilla_funcs.setexception(npobj, message);
  D(bugiD("NPN_SetException done\n"));
}

static int handle_NPN_SetException(rpc_connection_t *connection)
{
  D(bug("handle_NPN_SetException\n"));

  NPObject *npobj;
  NPUTF8 *message;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_STRING, &message,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_SetException() get args", error);
	return error;
  }

  g_NPN_SetException(npobj, message);

  if (npobj)
	NPN_ReleaseObject(npobj);
  if (message)
	free(message);

  return rpc_method_send_reply (connection, RPC_TYPE_INVALID);
}

// NPN_GetStringIdentifier
static NPIdentifier
g_NPN_GetStringIdentifier(const char *name)
{
  D(bugiI("NPN_GetStringIdentifier name='%s'\n", name));
  NPIdentifier ident = mozilla_funcs.getstringidentifier(name);
  D(bugiD("NPN_GetStringIdentifier return: %p\n", ident));
  return ident;
}

static int handle_NPN_GetStringIdentifier(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetStringIdentifier\n"));

  char *name;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_STRING, &name,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetStringIdentifier() get args", error);
	return error;
  }

  NPIdentifier ident = g_NPN_GetStringIdentifier(name);

  if (name)
	free(name);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_NP_IDENTIFIER, &ident,
							   RPC_TYPE_INVALID);
}

// NPN_GetStringIdentifiers
static void
g_NPN_GetStringIdentifiers(const NPUTF8 **names, int32_t nameCount, NPIdentifier *idents)
{
  D(bugiI("NPN_GetStringIdentifiers nameCount=%d\n", nameCount));
  mozilla_funcs.getstringidentifiers(names, nameCount, idents);
  D(bugiD("NPN_GetStringIdentifiers done\n"));
}

static int handle_NPN_GetStringIdentifiers(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetStringIdentifiers\n"));

  NPUTF8 **names;
  int32_t nameCount;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_ARRAY, RPC_TYPE_STRING, &nameCount, &names,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetStringIdentifiers() get args", error);
	return error;
  }

  NPIdentifier *idents = NPW_MemNew0(NPIdentifier, nameCount);
  if (idents)
	g_NPN_GetStringIdentifiers((const NPUTF8 **)names, nameCount, idents);

  if (names) {
	for (int i = 0; i < nameCount; i++)
	  free(names[i]);
	free(names);
  }

  int rpc_ret = rpc_method_send_reply(connection,
									  RPC_TYPE_ARRAY, RPC_TYPE_NP_IDENTIFIER, nameCount, idents,
									  RPC_TYPE_INVALID);

  NPW_MemFree(idents);
  return rpc_ret;
}

// NPN_GetIntIdentifier
static NPIdentifier
g_NPN_GetIntIdentifier(int32_t intid)
{
  D(bugiI("NPN_GetIntIdentifier intid=%d\n", intid));
  NPIdentifier ident = mozilla_funcs.getintidentifier(intid);
  D(bugiD("NPN_GetIntIdentifier return: %p\n", ident));
  return ident;
}

static int handle_NPN_GetIntIdentifier(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetIntIdentifier\n"));

  int32_t intid;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_INT32, &intid,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetIntIdentifier() get args", error);
	return error;
  }

  NPIdentifier ident = g_NPN_GetIntIdentifier(intid);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_NP_IDENTIFIER, &ident,
							   RPC_TYPE_INVALID);
}

// NPN_IdentifierIsString
static bool
g_NPN_IdentifierIsString(NPIdentifier ident)
{
  D(bugiI("NPN_IdentifierIsString ident=%p\n", ident));
  bool ret = mozilla_funcs.identifierisstring(ident);
  D(bugiD("NPN_IdentifierIsString return: %s\n", ret ? "true" : "false"));
  return ret;
}

static int handle_NPN_IdentifierIsString(rpc_connection_t *connection)
{
  D(bug("handle_NPN_IdentifierIsString\n"));

  NPIdentifier ident;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_IDENTIFIER, &ident,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_IdentifierIsString() get args", error);
	return error;
  }

  bool ret = g_NPN_IdentifierIsString(ident);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

// NPN_UTF8FromIdentifier
static NPUTF8 *
g_NPN_UTF8FromIdentifier(NPIdentifier ident)
{
  D(bugiI("NPN_UTF8FromIdentifier ident=%p\n", ident));
  NPUTF8 *str = mozilla_funcs.utf8fromidentifier(ident);
  D(bugiD("NPN_UTF8FromIdentifier return: '%s'\n", str));
  return str;
}

static int handle_NPN_UTF8FromIdentifier(rpc_connection_t *connection)
{
  D(bug("handle_NPN_UTF8FromIdentifier\n"));

  NPIdentifier ident;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_IDENTIFIER, &ident,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_UTF8FromIdentifier() get args", error);
	return error;
  }

  NPUTF8 *str = g_NPN_UTF8FromIdentifier(ident);

  error = rpc_method_send_reply(connection,
								RPC_TYPE_NP_UTF8, str,
								RPC_TYPE_INVALID);

  // the caller is responsible for deallocating the memory used by the string
  NPN_MemFree(str);

  return error;
}

// NPN_IntFromIdentifier
static int32_t
g_NPN_IntFromIdentifier(NPIdentifier ident)
{
  D(bugiI("NPN_IntFromIdentifier ident=%p\n", ident));
  int32_t ret = mozilla_funcs.intfromidentifier(ident);
  D(bugiD("NPN_IntFromIdentifier return: %d\n", ret));
  return ret;
}

static int handle_NPN_IntFromIdentifier(rpc_connection_t *connection)
{
  D(bug("handle_NPN_IntFromIdentifier\n"));

  NPIdentifier ident;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_IDENTIFIER, &ident,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_IntFromIdentifier() get args", error);
	return error;
  }

  int32_t ret = g_NPN_IntFromIdentifier(ident);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_INT32, ret,
							   RPC_TYPE_INVALID);
}

// NPN_ReleaseVariantValue
static void
g_NPN_ReleaseVariantValue(NPVariant *variant)
{
  D(bugiI("NPN_ReleaseVariantValue\n"));
  NPN_ReleaseVariantValue(variant);
  D(bugiD("NPN_ReleaseVariantValue done\n"));
}

// NPN_PluginThreadAsyncCall
static void
g_NPN_PluginThreadAsyncCall(NPP instance,
							void (*func)(void *),
							void *userData)
{
  // No debug statements as the debug system is not thread-safe.
  mozilla_funcs.pluginthreadasynccall(instance, func, userData);
}

// NPN_GetValueForURL
static NPError
g_NPN_GetValueForURL(NPP instance, NPNURLVariable variable,
					 const char *url, char **value,
					 uint32_t *len)
{
  if (mozilla_funcs.getvalueforurl == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_GetValueForURL instance=%p, variable=%d [%s], url=%s\n",
		  instance, variable, string_of_NPNURLVariable(variable), url));
  NPError ret = mozilla_funcs.getvalueforurl(instance, variable, url, value, len);
  D(bugiD("NPN_GetValueForURL return: %d [%s] len=%d\n",
		  ret, string_of_NPError(ret), *len));
  return ret;
}

static int handle_NPN_GetValueForURL(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetValueForURL\n"));

  PluginInstance *plugin;
  uint32_t variable;
  char *url;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_UINT32, &variable,
								  RPC_TYPE_STRING, &url,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetValueForURL() get args", error);
	return error;
  }

  char *value = NULL;
  uint32_t len = 0;
  int32_t ret = g_NPN_GetValueForURL(PLUGIN_INSTANCE_NPP(plugin), variable,
									 url, &value, &len);

  if (url)
	free(url);

  error = rpc_method_send_reply(connection,
								RPC_TYPE_INT32, ret,
								RPC_TYPE_ARRAY, RPC_TYPE_CHAR, len, value,
								RPC_TYPE_INVALID);
  NPN_MemFree(value);
  return error;
}

// NPN_SetValueForURL
static NPError
g_NPN_SetValueForURL(NPP instance, NPNURLVariable variable,
					 const char *url, const char *value,
					 uint32_t len)
{
  if (mozilla_funcs.setvalueforurl == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_SetValueForURL instance=%p, variable=%d [%s], url=%s, len=%d\n",
		  instance, variable, string_of_NPNURLVariable(variable), url, len));
  NPError ret = mozilla_funcs.setvalueforurl(instance, variable, url, value, len);
  D(bugiD("NPN_SetValueForURL return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

static int handle_NPN_SetValueForURL(rpc_connection_t *connection)
{
  D(bug("handle_NPN_SetValueForURL\n"));

  PluginInstance *plugin;
  char *url;
  uint32_t variable;
  char *value;
  uint32_t len;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_UINT32, &variable,
								  RPC_TYPE_STRING, &url,
								  RPC_TYPE_ARRAY, RPC_TYPE_CHAR, &len, &value,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_SetValueForURL() get args", error);
	return error;
  }

  int32_t ret = g_NPN_SetValueForURL(PLUGIN_INSTANCE_NPP(plugin), variable,
									 url, value, len);

  if (url)
	free(url);
  if (value)
	free(value);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_INT32, ret,
							   RPC_TYPE_INVALID);
}

// NPN_GetAuthenticationInfo
static NPError
g_NPN_GetAuthenticationInfo(NPP instance, const char *protocol,
							const char *host, int32_t port, const char *scheme,
							const char *realm,
							char **username, uint32_t *ulen,
							char **password, uint32_t *plen)
{
  if (mozilla_funcs.getauthenticationinfo == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  D(bugiI("NPN_GetAuthenticationInfo instance=%p, protocol=%s,"
		  " host=%s, port=%d, scheme=%s, realm=%s\n",
		  instance, protocol, host, port, scheme, realm));
  NPError ret = mozilla_funcs.getauthenticationinfo(instance, protocol, host,
													port, scheme, realm,
													username, ulen,
													password, plen);
  D(bugiD("NPN_GetAuthenticationInfo return: %d [%s] ulen=%d, plen=%d\n",
		  ret, string_of_NPError(ret), *ulen, *plen));
  return ret;
}

static int handle_NPN_GetAuthenticationInfo(rpc_connection_t *connection)
{
  D(bug("handle_NPN_GetAuthenticationInfo\n"));

  PluginInstance *plugin;
  char *protocol, *host;
  int32_t port;
  char *scheme, *realm;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NPW_PLUGIN_INSTANCE, &plugin,
								  RPC_TYPE_STRING, &protocol,
								  RPC_TYPE_STRING, &host,
								  RPC_TYPE_INT32, &port,
								  RPC_TYPE_STRING, &scheme,
								  RPC_TYPE_STRING, &realm,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPN_GetAuthenticationInfo() get args", error);
	return error;
  }

  char *username = NULL, *password = NULL;
  uint32_t ulen = 0, plen = 0;
  int32_t ret = g_NPN_GetAuthenticationInfo(PLUGIN_INSTANCE_NPP(plugin),
											protocol, host, port, scheme, realm,
											&username, &ulen, &password, &plen);

  if (protocol)
	free(protocol);
  if (host)
	free(host);
  if (scheme)
	free(scheme);
  if (realm)
	free(realm);

  error = rpc_method_send_reply(connection,
								RPC_TYPE_INT32, ret,
								RPC_TYPE_ARRAY, RPC_TYPE_CHAR, ulen, username,
								RPC_TYPE_ARRAY, RPC_TYPE_CHAR, plen, password,
								RPC_TYPE_INVALID);

  if (username)
	NPN_MemFree(username);
  if (password)
	NPN_MemFree(password);

  return error;
}

// NPN_ScheduleTimer
static uint32_t
g_NPN_ScheduleTimer(NPP instance, uint32_t interval, NPBool repeat,
					void (*timerFunc)(NPP npp, uint32_t timerID))
{
  if (mozilla_funcs.scheduletimer == NULL)
	return 0;

  D(bugiI("NPN_ScheduleTimer instance=%p, interval=%d, repeat=%d\n",
		  instance, interval, repeat));
  uint32_t ret = mozilla_funcs.scheduletimer(instance, interval, repeat, timerFunc);
  D(bugiD("NPN_ScheduleTimer return: %d\n", ret));
  return ret;
}

// NPN_UnscheduleTimer
static void
g_NPN_UnscheduleTimer(NPP instance, uint32_t timerID)
{
  if (mozilla_funcs.unscheduletimer == NULL)
	return;

  D(bugiI("NPN_UnscheduleTimer instance=%p, timerID=%d\n", instance, timerID));
  mozilla_funcs.unscheduletimer(instance, timerID);
  D(bugiD("NPN_UnscheduleTimer done\n"));
}


/* ====================================================================== */
/* === Plug-in side data                                              === */
/* ====================================================================== */

// Creates a new instance of a plug-in
static NPError
invoke_NPP_New(PluginInstance *plugin, NPMIMEType mime_type,
			   uint16_t mode, int16_t argc, char *argn[], char *argv[],
			   NPSavedData *saved)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.newp(mime_type, plugin->native_instance, mode, argc, argn, argv, saved);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_NEW,
								RPC_TYPE_UINT32, plugin->instance_id,
								RPC_TYPE_STRING, mime_type,
								RPC_TYPE_INT32, (int32_t)mode,
								RPC_TYPE_ARRAY, RPC_TYPE_STRING, (uint32_t)argc, argn,
								RPC_TYPE_ARRAY, RPC_TYPE_STRING, (uint32_t)argc, argv,
								RPC_TYPE_NP_SAVED_DATA, saved,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_New() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_New() wait for reply", error);
	return NPERR_GENERIC_ERROR;
  }

  return ret;
}

static NPError
g_NPP_New(NPMIMEType mime_type, NPP instance,
		  uint16_t mode, int16_t argc, char *argn[], char *argv[],
		  NPSavedData *saved)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;
	
  // Check if we need to restart the plug-in
  NPError ret = plugin_start_if_needed();
  if (ret != NPERR_NO_ERROR)
  	return ret;

  PluginInstance *plugin = npw_plugin_instance_new(&PluginInstanceClass);
  if (plugin == NULL)
	return NPERR_OUT_OF_MEMORY_ERROR;
  plugin->instance = instance;
  plugin->instance_id = id_create(plugin);
  plugin->connection = rpc_connection_ref(g_rpc_connection);
  instance->pdata = plugin;

  if (PLUGIN_DIRECT_EXEC) {
	if ((plugin->native_instance = NPW_MemNew0(NPP_t, 1)) == NULL)
	  return NPERR_OUT_OF_MEMORY_ERROR;
	plugin->native_instance->ndata = instance->ndata;
  }

  D(bugiI("NPP_New instance=%p\n", instance));
  ret = invoke_NPP_New(plugin, mime_type, mode, argc, argn, argv, saved);
  D(bugiD("NPP_New return: %d [%s]\n", ret, string_of_NPError(ret)));

  if (saved) {
	if (saved->buf)
	  free(saved->buf);
	free(saved);
  }

  return ret;
}

// Deletes a specific instance of a plug-in
static NPError
invoke_NPP_Destroy(PluginInstance *plugin, NPSavedData **save)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.destroy(plugin->native_instance, save);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_DESTROY,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_Destroy() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  NPSavedData *save_area = NULL;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_NP_SAVED_DATA, &save_area,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_Destroy() wait for reply", error);
	return NPERR_GENERIC_ERROR;
  }

  if (save)
	*save = save_area;
  else if (save_area) {
	if (save_area->len > 0 && save_area->buf)
	  free(save_area->buf);
	free(save_area);
  }

  return ret;
}

static NPError
g_NPP_Destroy(NPP instance, NPSavedData **save)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  D(bugiI("NPP_Destroy instance=%p\n", instance));
  NPError ret = invoke_NPP_Destroy(plugin, save);
  D(bugiD("NPP_Destroy return: %d [%s]\n", ret, string_of_NPError(ret)));

  if (PLUGIN_DIRECT_EXEC) {
	if (plugin->native_instance) {
	  NPW_MemFree(plugin->native_instance);
	  plugin->native_instance = NULL;
	}
  }

  npw_plugin_instance_invalidate(plugin);
  npw_plugin_instance_unref(plugin);
  return ret;
}

// Tells the plug-in when a window is created, moved, sized, or destroyed
static NPError
invoke_NPP_SetWindow(PluginInstance *plugin, NPWindow *window)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.setwindow(plugin->native_instance, window);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_SET_WINDOW,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_NP_WINDOW, window,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_SetWindow() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_SetWindow() wait for reply", error);
	return NPERR_GENERIC_ERROR;
  }

  return ret;
}

static NPError
g_NPP_SetWindow(NPP instance, NPWindow *window)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  D(bugiI("NPP_SetWindow instance=%p\n", instance));
  NPError ret = invoke_NPP_SetWindow(plugin, window);
  D(bugiD("NPP_SetWindow return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

// Allows the browser to query the plug-in for information
static NPError
invoke_NPP_GetValue(PluginInstance *plugin, NPPVariable variable, void *value)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.getvalue(plugin->native_instance, variable, value);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_GET_VALUE,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_INT32, variable,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_GetValue() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  switch (rpc_type_of_NPPVariable(variable)) {
  case RPC_TYPE_STRING:
	{
	  char *str = NULL;
	  error = rpc_method_wait_for_reply(plugin->connection, RPC_TYPE_INT32, &ret, RPC_TYPE_STRING, &str, RPC_TYPE_INVALID);
	  if (error != RPC_ERROR_NO_ERROR) {
		npw_perror("NPP_GetValue() wait for reply", error);
		ret = NPERR_GENERIC_ERROR;
	  }
	  D(bug("-> value: %s\n", str));
	  switch (variable) {
	  case NPPVformValue:
		// this is a '\0'-terminated UTF-8 string data allocated by NPN_MemAlloc()
		if (ret == NPERR_NO_ERROR) {
		  char *npn_str = NULL;
		  ret = NPW_ReallocData(str, strlen(str) + 1, (void**)&npn_str);
		  free(str);
		  str = npn_str;
		}
		break;
	  default:
		// XXX memory leak (add to a deallocation pool?)
		break;
	  }
	  *((char **)value) = str;
	  break;
	}
  case RPC_TYPE_INT32:
	{
	  int32_t n = 0;
	  error = rpc_method_wait_for_reply(plugin->connection, RPC_TYPE_INT32, &ret, RPC_TYPE_INT32, &n, RPC_TYPE_INVALID);
	  if (error != RPC_ERROR_NO_ERROR) {
		npw_perror("NPP_GetValue() wait for reply", error);
		ret = NPERR_GENERIC_ERROR;
	  }
	  D(bug("-> value: %d\n", n));
	  *((int *)value) = n;
	  break;
	}
  case RPC_TYPE_BOOLEAN:
	{
	  uint32_t b = 0;
	  error = rpc_method_wait_for_reply(plugin->connection, RPC_TYPE_INT32, &ret, RPC_TYPE_BOOLEAN, &b, RPC_TYPE_INVALID);
	  if (error != RPC_ERROR_NO_ERROR) {
		npw_perror("NPP_GetValue() wait for reply", error);
		ret = NPERR_GENERIC_ERROR;
	  }
	  D(bug("-> value: %s\n", b ? "true" : "false"));
	  *((NPBool *)value) = b ? TRUE : FALSE;
	  break;
	}
  case RPC_TYPE_NP_OBJECT:
	{
	  NPObject *npobj = NULL;
	  error = rpc_method_wait_for_reply(plugin->connection,
										RPC_TYPE_INT32, &ret,
										RPC_TYPE_NP_OBJECT_PASS_REF, &npobj,
										RPC_TYPE_INVALID);
	  if (error != RPC_ERROR_NO_ERROR) {
		npw_perror("NPP_GetValue() wait for reply", error);
		ret = NPERR_GENERIC_ERROR;
	  }
	  D(bug("-> value: <object %p>\n", npobj));
	  *((NPObject **)value) = npobj;
	  // Caller is responsible for releasing reference.
	  break;
	}
  }

  return ret;
}

static NPError
g_NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
  // Firefox sometimes requests NP_GetValue values with NPP_GetValue
  // for some reason. Notably it requests NPPVpluginDescriptionString
  // with it when determining whether or not to apply a Flash-specific quirk.
  //
  // XXX: The more correct way to fix this may be to accept an NPP
  // with NULL pdata and let the plugin decide whether to crash or
  // not. Unfortunately, we get the connection out of the PluginInstance on
  // the wrapper side, so it's best to avoid a NULL instance
  // here. This way also avoids extra IPC.
  if (variable == NPPVpluginNameString || variable == NPPVpluginDescriptionString) {
	D(bugiI("NPP_GetValue instance=%p, variable=%d [%s]\n",
			instance, variable, string_of_NPPVariable(variable)));
	npw_printf("WARNING: browser requested NP_GetValue variable via NPP_GetValue.\n");
	NPError ret = NP_GetValue(NULL, variable, value);
	D(bugiD("NPP_GetValue return: %d [%s]\n", ret, string_of_NPError(ret)));
	return ret;
  }

  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  switch (rpc_type_of_NPPVariable(variable)) {
  case RPC_TYPE_STRING:
  case RPC_TYPE_INT32:
  case RPC_TYPE_BOOLEAN:
  case RPC_TYPE_NP_OBJECT:
	break;
  default:
	D(bug("WARNING: unhandled variable %d in NPP_GetValue()\n", variable));
	return NPERR_INVALID_PARAM;
  }

  D(bugiI("NPP_GetValue instance=%p, variable=%d [%s]\n", instance, variable, string_of_NPPVariable(variable)));
  NPError ret = invoke_NPP_GetValue(plugin, variable, value);
  D(bugiD("NPP_GetValue return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

// Sets information about the plug-in
static NPError
invoke_NPP_SetValue(PluginInstance *plugin, NPNVariable variable, void *value)
{
  NPW_UNIMPLEMENTED();

  return NPERR_GENERIC_ERROR;
}

static NPError
g_NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  D(bugiI("NPP_SetValue instance=%p, variable=%d [%s]\n", instance, variable, string_of_NPPVariable(variable)));
  NPError ret = invoke_NPP_SetValue(plugin, variable, value);
  D(bugiD("NPP_SetValue return: %d [%s]\n", ret, string_of_NPError(ret)));
  return NPERR_GENERIC_ERROR;
}

// Notifies the instance of the completion of a URL request
static void
invoke_NPP_URLNotify(PluginInstance *plugin, const char *url, NPReason reason, void *notifyData)
{
  if (PLUGIN_DIRECT_EXEC) {
	plugin_funcs.urlnotify(plugin->native_instance, url, reason, notifyData);
	return;
  }

  npw_return_if_fail(rpc_method_invoke_possible(plugin->connection));

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_URL_NOTIFY,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_STRING, url,
								RPC_TYPE_INT32, reason,
								RPC_TYPE_NP_NOTIFY_DATA, notifyData,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_URLNotify() invoke", error);
	return;
  }

  error = rpc_method_wait_for_reply(plugin->connection, RPC_TYPE_INVALID);
  
  if (error != RPC_ERROR_NO_ERROR)
	npw_perror("NPP_URLNotify() wait for reply", error);
}

static void
g_NPP_URLNotify(NPP instance, const char *url, NPReason reason, void *notifyData)
{
  if (instance == NULL)
	return;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return;

  D(bugiI("NPP_URLNotify instance=%p, url='%s', reason=%s, notifyData=%p\n", instance, url, string_of_NPReason(reason), notifyData));
  invoke_NPP_URLNotify(plugin, url, reason, notifyData);
  D(bugiD("NPP_URLNotify done\n"));
}

// Notifies a plug-in instance of a new data stream
static NPError
invoke_NPP_NewStream(PluginInstance *plugin, NPMIMEType type, NPStream *stream, NPBool seekable, uint16_t *stype)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.newstream(plugin->native_instance, type, stream, seekable, stype);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_NEW_STREAM,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_STRING, type,
								RPC_TYPE_UINT32, ((StreamInstance *)stream->pdata)->stream_id,
								RPC_TYPE_STRING, stream->url,
								RPC_TYPE_UINT32, stream->end,
								RPC_TYPE_UINT32, stream->lastmodified,
								RPC_TYPE_NP_NOTIFY_DATA, stream->notifyData,
								RPC_TYPE_STRING, NPN_HAS_FEATURE(RESPONSE_HEADERS) ? stream->headers : NULL,
								RPC_TYPE_BOOLEAN, seekable,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_NewStream() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  uint32_t r_stype;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_UINT32, &r_stype,
									RPC_TYPE_NP_NOTIFY_DATA, &stream->notifyData,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_NewStream() wait for reply", error);
	return NPERR_GENERIC_ERROR;
  }

  *stype = r_stype;
  return ret;
}

static NPError
g_NPP_NewStream(NPP instance, NPMIMEType type, NPStream *stream, NPBool seekable, uint16_t *stype)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  if (!PLUGIN_DIRECT_EXEC) {
	StreamInstance *stream_pdata = malloc(sizeof(*stream_pdata));
	if (stream_pdata == NULL)
	  return NPERR_OUT_OF_MEMORY_ERROR;
	memset(stream_pdata, 0, sizeof(*stream_pdata));
	stream_pdata->stream = stream;
	stream_pdata->stream_id = id_create(stream_pdata);
	stream_pdata->is_plugin_stream = 0;
	stream->pdata = stream_pdata;
  }

  D(bugiI("NPP_NewStream instance=%p\n", instance));
  NPError ret = invoke_NPP_NewStream(plugin, type, stream, seekable, stype);
  D(bugiD("NPP_NewStream return: %d [%s], stype=%s\n", ret, string_of_NPError(ret), string_of_NPStreamType(*stype)));
  return ret;
}

// Tells the plug-in that a stream is about to be closed or destroyed
static NPError
invoke_NPP_DestroyStream(PluginInstance *plugin, NPStream *stream, NPReason reason)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.destroystream(plugin->native_instance, stream, reason);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_DESTROY_STREAM,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_NP_STREAM, stream,
								RPC_TYPE_INT32, reason,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_DestroyStream() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_DestroyStream() wait for reply", error);
	return NPERR_GENERIC_ERROR;
  }

  return ret;
}

static NPError
g_NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  D(bugiI("NPP_DestroyStream instance=%p\n", instance));
  NPError ret = invoke_NPP_DestroyStream(plugin, stream, reason);
  D(bugiD("NPP_DestroyStream return: %d [%s]\n", ret, string_of_NPError(ret)));

  if (!PLUGIN_DIRECT_EXEC) {
	StreamInstance *stream_pdata = stream->pdata;
	if (stream_pdata) {
	  id_remove(stream_pdata->stream_id);
	  free(stream->pdata);
	  stream->pdata = NULL;
	}
  }

  return ret;
}

// Provides a local file name for the data from a stream
static void
invoke_NPP_StreamAsFile(PluginInstance *plugin, NPStream *stream, const char *fname)
{
  if (PLUGIN_DIRECT_EXEC) {
	plugin_funcs.asfile(plugin->native_instance, stream, fname);
	return;
  }

  npw_return_if_fail(rpc_method_invoke_possible(plugin->connection));

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_STREAM_AS_FILE,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_NP_STREAM, stream,
								RPC_TYPE_STRING, fname,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_StreamAsFile() invoke", error);
	return;
  }

  error = rpc_method_wait_for_reply(plugin->connection, RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR)
	npw_perror("NPP_StreamAsFile() wait for reply", error);
}

static void
g_NPP_StreamAsFile(NPP instance, NPStream *stream, const char *fname)
{
  if (instance == NULL)
	return;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return;

  D(bugiI("NPP_StreamAsFile instance=%p\n", instance));
  invoke_NPP_StreamAsFile(plugin, stream, fname);
  D(bugiD("NPP_StreamAsFile done\n"));
}

// Determines maximum number of bytes that the plug-in can consume
static int32_t
invoke_NPP_WriteReady(PluginInstance *plugin, NPStream *stream)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.writeready(plugin->native_instance, stream);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection),
						 NPERR_STREAM_BUFSIZ);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_WRITE_READY,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_NP_STREAM, stream,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_WriteReady() invoke", error);
	return NPERR_STREAM_BUFSIZ;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_WriteReady() wait for reply", error);
	return NPERR_STREAM_BUFSIZ;
  }

  return ret;
}

static int32_t
g_NPP_WriteReady(NPP instance, NPStream *stream)
{
  if (instance == NULL)
	return 0;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return 0;

  D(bugiI("NPP_WriteReady instance=%p\n", instance));
  int32_t ret = invoke_NPP_WriteReady(plugin, stream);
  D(bugiD("NPP_WriteReady return: %d\n", ret));
  return ret;
}


// Delivers data to a plug-in instance
static int32_t
invoke_NPP_Write(PluginInstance *plugin, NPStream *stream, int32_t offset, int32_t len, void *buf)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.write(plugin->native_instance, stream, offset, len, buf);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection), -1);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_WRITE,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_NP_STREAM, stream,
								RPC_TYPE_INT32, offset,
								RPC_TYPE_ARRAY, RPC_TYPE_CHAR, len, buf,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_Write() invoke", error);
	return -1;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_Write() wait for reply", error);
	return -1;
  }

  return ret;
}

static int32_t
g_NPP_Write(NPP instance, NPStream *stream, int32_t offset, int32_t len, void *buf)
{
  if (instance == NULL)
	return -1;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return -1;

  /* Don't try to propagate erroneous buffers.
   *
   * Actually, we can get to that case if NPP_WriteReady() returned -1
   * or another negative value, in general. Some browsers (Konqueror,
   * Google Chrome) send data through NPP_Write() anyway. Others
   * (Firefox, WebKit) actually suspend the stream temporarily.
   *
   * Note that we could also return -1 here to destroy the stream
   * right away. However, some plugins may want to handle that case
   * themselves. i.e. it's not our role to idealize the plugin
   * intents, we are just a "passthrough". On the other hand, the only
   * useful way to handle that case is to range check the arguments
   * and return -1. Is there a "real" plugin that wants to do more
   * than that (except an explicit NPN_DestroyStream())?
   */
  if (len <= 0)
	buf = NULL;

  D(bugiI("NPP_Write instance=%p\n", instance));
  int32_t ret = invoke_NPP_Write(plugin, stream, offset, len, buf);
  D(bugiD("NPP_Write return: %d\n", ret));
  return ret;
}


// Requests a platform-specific print operation for an embedded or full-screen plug-in
static void invoke_NPP_Print(PluginInstance *plugin, NPPrint *PrintInfo)
{
  if (PLUGIN_DIRECT_EXEC) {
	plugin_funcs.print(plugin->native_instance, PrintInfo);
	return;
  }

  NPPrintCallbackStruct *platformPrint;
  switch (PrintInfo->mode) {
  case NP_FULL:
	platformPrint = PrintInfo->print.fullPrint.platformPrint;
	break;
  case NP_EMBED:
	platformPrint = PrintInfo->print.embedPrint.platformPrint;
	break;
  default:
	D(bug("WARNING: PrintInfo mode %d is not supported\n", PrintInfo->mode));
	return;
  }
  uint32_t platform_print_id = 0;
  if (platformPrint)
	platform_print_id = id_create(platformPrint);
  D(bug(" platformPrint=%p\n", platformPrint));

  npw_return_if_fail(rpc_method_invoke_possible(plugin->connection));

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_PRINT,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_UINT32, platform_print_id,
								RPC_TYPE_NP_PRINT, PrintInfo,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_Print() invoke", error);
	return;
  }

  uint32_t pluginPrinted;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_BOOLEAN, &pluginPrinted,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_Print() wait for reply", error);
	return;
  }

  // update browser-side NPPrint struct
  if (PrintInfo->mode == NP_FULL)
	PrintInfo->print.fullPrint.pluginPrinted = pluginPrinted;

  if (platform_print_id)
	id_remove(platform_print_id);
}

static void g_NPP_Print(NPP instance, NPPrint *PrintInfo)
{
  if (instance == NULL)
	return;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return;

  if (PrintInfo == NULL)
	return;

  D(bugiI("NPP_Print instance=%p\n", instance));
  invoke_NPP_Print(plugin, PrintInfo);
  D(bugiD("NPP_Print done\n"));
}

// Delivers a platform-specific window event to the instance
static int16_t invoke_NPP_HandleEvent(PluginInstance *plugin, void *event)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.event(plugin->native_instance, event);

  npw_return_val_if_fail(rpc_method_invoke_possible(plugin->connection), false);

  int error = rpc_method_invoke(plugin->connection,
								RPC_METHOD_NPP_HANDLE_EVENT,
								RPC_TYPE_NPW_PLUGIN_INSTANCE, plugin,
								RPC_TYPE_NP_EVENT, event,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_HandleEvent() invoke", error);
	return false;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(plugin->connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_HandleEvent() wait for reply", error);
	return false;
  }

  return ret;
}

static int16_t g_NPP_HandleEvent(NPP instance, void *event)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  PluginInstance *plugin = PLUGIN_INSTANCE(instance);
  if (plugin == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  NPEvent *npevent = event;
  if (npevent->type == GraphicsExpose) {
	/* XXX: flush the X output buffer so that the call to
	   gdk_pixmap_foreign_new() in the viewer can work */
	toolkit_flush(instance);
  }

  if (npevent->type == ButtonPress) {
	// Release any implicit passive grabs we have so Flash can show a
	// menu. This is only relevant if your browser does not do
	// out-of-process plugins. Otherwise, we need the browser to do it
	// instead.
	pointer_ungrab(instance, npevent->xbutton.time);
	toolkit_flush(instance);
  }

  D(bugiI("NPP_HandleEvent instance=%p\n", instance));
  int16_t ret = invoke_NPP_HandleEvent(plugin, event);
  D(bugiD("NPP_HandleEvent return: %d\n", ret));
  return ret;
}

// Clears site-data stored by the plug-in
static NPError
invoke_NPP_ClearSiteData(const char* site, uint64_t flags, uint64_t maxAge)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.clearsitedata(site, flags, maxAge);

  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPP_CLEAR_SITE_DATA,
								RPC_TYPE_STRING, site,
								RPC_TYPE_UINT64, flags,
								RPC_TYPE_UINT64, maxAge,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_ClearSiteData() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_INT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_ClearSiteData() wait for reply", error);
	return NPERR_GENERIC_ERROR;
  }

  return ret;
}

static NPError
g_NPP_ClearSiteData(const char* site, uint64_t flags, uint64_t maxAge)
{
  D(bugiI("NPP_ClearSiteData site=%s, flags=%" G_GUINT64_FORMAT
		  ", maxAge=%" G_GUINT64_FORMAT "\n",
		  site ? site : "<null>", flags, maxAge));
  NPError ret = invoke_NPP_ClearSiteData(site, flags, maxAge);
  D(bugiD("NPP_ClearSiteData return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

// Get sites with data stored by the plug-in
static char **
invoke_NPP_GetSitesWithData(void)
{
  if (PLUGIN_DIRECT_EXEC)
	return plugin_funcs.getsiteswithdata();

  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), NULL);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPP_GET_SITES_WITH_DATA,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_GetSitesWithData() invoke", error);
	return NULL;
  }

  char **sites = NULL;
  uint32_t siteCount = 0;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_ARRAY, RPC_TYPE_STRING,
									&siteCount, &sites,
									RPC_TYPE_INVALID);
  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPP_GetSitesWithData() wait for reply", error);
	return NULL;
  }

  // Convert this to the format NPAPI wants... ugh.
  char **sites_ret = NULL;
  if (siteCount > 0) {
	sites_ret = NPN_MemAlloc(sizeof(char*) * (siteCount+1));
	if (sites_ret) {
	  for (int i = 0; i < siteCount; i++) {
		// Ignore allocation failures here.
		NPW_ReallocData(sites[i], strlen(sites[i]), (void**)&sites_ret[i]);
	  }
	  sites_ret[siteCount] = NULL;
	}
  }

  // Delete the other copy.
  if (sites) {
	for (int i = 0; i < siteCount; i++) {
	  free(sites[i]);
	}
	free(sites);
  }

  return sites_ret;
}

static char **
g_NPP_GetSitesWithData(void)
{
  D(bugiI("NPP_GetSitesWithData\n"));
  char **ret = invoke_NPP_GetSitesWithData();
  D(bugiD("NPP_GetSitesWithData return: %d sites\n",
		  ret ? g_strv_length(ret) : 0));
  return ret;
}

// Allows the browser to query the plug-in for information
static NPError
g_NP_GetValue(void *future, NPPVariable variable, void *value)
{
  if (g_plugin.initialized == 0)
	plugin_init(0);
  if (g_plugin.initialized <= 0)
	return NPERR_GENERIC_ERROR;

  if (PLUGIN_DIRECT_EXEC)
	return g_plugin_NP_GetValue(future, variable, value);

  char *str = NULL;
  int ret = NPERR_GENERIC_ERROR;
  switch (variable) {
  case NPPVpluginNameString:
	if (g_plugin.is_wrapper) {
	  str = "NPAPI Plugins Wrapper " NPW_VERSION;
	  ret = NPERR_NO_ERROR;
	}
	else if (g_plugin.name) {
	  str = g_plugin.name;
	  ret = NPERR_NO_ERROR;
	}
	break;
  case NPPVpluginDescriptionString:
	if (g_plugin.is_wrapper) {
	  str =
		"<a href=\"http://gwenole.beauchesne.info/projects/nspluginwrapper/\">nspluginwrapper</a> "
		" is a cross-platform NPAPI plugin viewer, in particular for linux/i386 plugins.<br>"
		"This <b>beta</b> software is available under the terms of the GNU General Public License.<br>"
		;
	  ret = NPERR_NO_ERROR;
	}
	else if (g_plugin.description) {
	  str = g_plugin.description;
	  ret = NPERR_NO_ERROR;
	}
	break;
  default:
	return NPERR_INVALID_PARAM;
  }
  *((char **)value) = str;

  return ret;
}

NPError
NP_GetValue(void *future, NPPVariable variable, void *value)
{
  D(bugiI("NP_GetValue variable=%d [%s]\n", variable, string_of_NPPVariable(variable)));
  NPError ret = g_NP_GetValue(future, variable, value);
  D(bugiD("NP_GetValue return: %d [%s]\n", ret, string_of_NPError(ret)));
  return ret;
}

// Allows the browser to query the plug-in supported formats
static const char *
g_NP_GetMIMEDescription(void)
{
  if (g_plugin.initialized == 0)
	plugin_init(0);
	
  if (g_plugin.initialized <= 0)
	return NULL;

  if (PLUGIN_DIRECT_EXEC)
	return g_plugin_NP_GetMIMEDescription();

  if (g_plugin.is_wrapper)
	return "unknown/mime-type:none:Do not open";

  return g_plugin.formats;
}

const char *
NP_GetMIMEDescription(void)
{
  D(bugiI("NP_GetMIMEDescription\n"));
  const char *formats = g_NP_GetMIMEDescription();
  D(bugiD("NP_GetMIMEDescription return: '%s'\n", formats));
  return formats;
}


/* ====================================================================== */
/* === LONG64 NPAPI support                                           === */
/* ====================================================================== */

/*
 * Dependent on NPSavedData
 *  NPP_New
 *  NPP_Destroy
 * NOTE: the browsers don't seem to care about NPSavedData
 *
 * Dependent on NPWindow / NPSetWindowCallbackStruct
 *  NPP_SetWindow
 *
 * Dependent on NPStream (plug-in side)
 *  NPP_NewStream
 *  NPP_DestroyStream
 *  NPP_WriteReady
 *  NPP_Write
 *  NPP_StreamAsFile
 *
 * Dependent on NPStream (browser-side)
 *  NPN_RequestRead
 *  NPN_NewStream
 *  NPN_DestroyStream
 *  NPN_Write
 * NOTE: Konqueror does not implement those
 *
 * Dependent on NPPrintCallbackStruct
 *  NPP_Print
 */

// Check if another thunking layer is necessary
static int g_use_long64_thunks = -1;

static void set_use_long64_thunks(bool enabled)
{
  g_use_long64_thunks = enabled;

  if (g_use_long64_thunks) {
	// XXX update mozilla_funcs with g_LONG64_*() variants
  }
}

#define NP_CVT32(VAL) ptr32->VAL = ptr64->VAL
#define NP_CVT64(VAL) ptr64->VAL = ptr32->VAL

// Check display is valid
static bool is_browser_display(Display *display)
{
  Display *browser_display = NULL;
  if (mozilla_funcs.getvalue == NULL)
	return 0;
  if (mozilla_funcs.getvalue(NULL, NPNVxDisplay, (void *)&browser_display) != NPERR_NO_ERROR)
	return 0;
  return display == browser_display;
}

// NPStream
typedef struct _LONG64_NPStream {
  void*  pdata;
  void*  ndata;
  const  char* url;
  uint64_t end;
  uint64_t lastmodified;
  void*  notifyData;
  const  char* headers;
} LONG64_NPStream;

static void convert_from_LONG64_NPStream(NPStream *ptr32, const LONG64_NPStream *ptr64)
{
  NP_CVT32(pdata);
  NP_CVT32(ndata);
  NP_CVT32(url);
  NP_CVT32(end);
  NP_CVT32(lastmodified);
  NP_CVT32(notifyData);
  NP_CVT32(headers);
}

#define NP_STREAM32(STREAM) get_stream32(STREAM)

static inline NPStream *get_stream32(LONG64_NPStream *stream64)
{
  NPStream *stream32 = stream64->pdata;
  if (stream32 && stream32->ndata == stream64)
	return stream32;
  return (NPStream *)stream64;
}

// NPByteRange
typedef struct _LONG64_NPByteRange {
  int64_t  offset;
  uint64_t length;
  struct _LONG64_NPByteRange* next;
} LONG64_NPByteRange;

// NPSavedData
typedef struct _LONG64_NPSavedData {
  int64_t	len;
  void*	buf;
} LONG64_NPSavedData;

static void convert_from_LONG64_NPSavedData(NPSavedData *ptr32, const LONG64_NPSavedData *ptr64)
{
  NP_CVT32(len);
  NP_CVT32(buf);
}

static void convert_from_NPSavedData(LONG64_NPSavedData *ptr64, const NPSavedData *ptr32)
{
  NP_CVT64(len);
  NP_CVT64(buf);
}

// NPSetWindowCallbackStruct
typedef struct {
  int64_t      type;
#ifdef MOZ_X11
  Display*     display;
  Visual*      visual;
  Colormap     colormap;
  unsigned int depth;
#endif
} LONG64_NPSetWindowCallbackStruct;

static bool is_LONG64_NPSetWindowCallbackStruct(void *ws_info)
{
  LONG64_NPSetWindowCallbackStruct *ws_info64 = (LONG64_NPSetWindowCallbackStruct *)ws_info;

  return (/* LONG64_NPSetWindowCallbacStruct.type valid? */
		  (ws_info64->type == 0 || ws_info64->type == NP_SETWINDOW) &&
#ifdef MOZ_X11
		  /* LONG64_NPSetWindowCallbacStruct.display valid? */
		  is_browser_display(ws_info64->display) &&
#endif
		  1);
}

static void convert_from_LONG64_NPSetWindowCallbackStruct(NPSetWindowCallbackStruct *ptr32,
														  const LONG64_NPSetWindowCallbackStruct *ptr64)
{
  NP_CVT32(type);
#ifdef MOZ_X11
  NP_CVT32(display);
  NP_CVT32(visual);
  NP_CVT32(colormap);
  NP_CVT32(depth);
#endif
}

// NPPrintCallbackStruct
typedef struct {
  int64_t type;
  FILE* fp;
} LONG64_NPPrintCallbackStruct;

static bool is_LONG64_NPPrintCallbackStruct(void *platformPrint)
{
  LONG64_NPPrintCallbackStruct *platformPrint64 = (LONG64_NPPrintCallbackStruct *)platformPrint;

  return (/* LONG64_NPPrintCallbackStruct.type valid? */
		  platformPrint64->type == NP_PRINT &&
		  /* LONG64_NPPrintCallbackStruct.file valid? */
		  platformPrint64->fp != NULL);
}

static void convert_from_LONG64_NPPrintCallbackStruct(NPPrintCallbackStruct *ptr32,
													  const LONG64_NPPrintCallbackStruct *ptr64)
{
  NP_CVT32(type);
  NP_CVT32(fp);
}

// NPWindow
typedef struct _LONG64_NPWindow {
  void* window;
  int64_t x;
  int64_t y;
  uint64_t width;
  uint64_t height;
  NPRect clipRect;
#if defined(XP_UNIX) && !defined(XP_MACOSX)
  void * ws_info;
#endif /* XP_UNIX */
  NPWindowType type;
} LONG64_NPWindow;

static bool is_LONG64_NPWindow(void *window)
{
  NPWindow *window32 = (NPWindow *)window;
  LONG64_NPWindow *window64 = (LONG64_NPWindow *)window;

  return (/* MSW32(LONG64_NPWindow.x) */
		  (window32->x == 0 || window32->x == 0xffffffff) &&
		  /* MSW32(LONG64_NPWindow.y) */
		  (window32->width == 0 || window32->width == 0xffffffff) &&
		  /* LONG64_NPWindow.clipRect.top, LONG64_NPWindow.clipRect.left */
		  (window32->type != NPWindowTypeWindow && window32->type != NPWindowTypeDrawable) &&
		  /* LONG64_NPWindow.type valid? */
		  (window64->type == NPWindowTypeWindow || window64->type == NPWindowTypeDrawable) &&
		  /* LONG64_NPWindow.ws_info valid? */
		  is_LONG64_NPSetWindowCallbackStruct(window64->ws_info));
}

static void convert_from_LONG64_NPWindow(NPWindow *ptr32, const LONG64_NPWindow *ptr64)
{
  NP_CVT32(type);
  NP_CVT32(window);
  NP_CVT32(x);
  NP_CVT32(y);
  NP_CVT32(width);
  NP_CVT32(height);
  NP_CVT32(clipRect.top);
  NP_CVT32(clipRect.left);
  NP_CVT32(clipRect.bottom);
  NP_CVT32(clipRect.right);
  convert_from_LONG64_NPSetWindowCallbackStruct(ptr32->ws_info, ptr64->ws_info);
}

// NPP_SetWindow (LONG64)
static NPError
g_LONG64_NPP_SetWindow(NPP instance, void *window)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  // Detect broken 64-bit NPAPI
  if (g_use_long64_thunks < 0)
	set_use_long64_thunks(is_LONG64_NPWindow(window));

  NPWindow window32;
  NPSetWindowCallbackStruct ws_info32;
  if (g_use_long64_thunks) {
	window32.ws_info = &ws_info32;
	convert_from_LONG64_NPWindow(&window32, window);
	window = &window32;
  }

  return g_NPP_SetWindow(instance, window);
}

// NPP_New (LONG64)
static NPError
g_LONG64_NPP_New(NPMIMEType mime_type, NPP instance,
				 uint16_t mode, int16_t argc, char *argn[], char *argv[],
				 void *saved)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  NPSavedData saved32;
  if (saved && g_use_long64_thunks > 0) {
	convert_from_LONG64_NPSavedData(&saved32, saved);
	saved = &saved32;
  }

  return g_NPP_New(mime_type, instance, mode, argc, argn, argv, saved);
}

// NPP_Destroy (LONG64)
static NPError
g_LONG64_NPP_Destroy(NPP instance, void **save)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  NPSavedData *save_area32 = NULL;
  NPError ret = g_NPP_Destroy(instance, &save_area32);

  if (save && g_use_long64_thunks > 0) {
	LONG64_NPSavedData *save_area64 = NULL;
	if (ret == NPERR_NO_ERROR && save_area32) {
	  if ((save_area64 = g_NPN_MemAlloc(save_area32->len)) != NULL)
		convert_from_NPSavedData(save_area64, save_area32);
	  free(save_area32);
	}
	*save = save_area64;
  }

  return ret;
}

// NPP_NewStream (LONG64)
static NPError
g_LONG64_NPP_NewStream(NPP instance, NPMIMEType type, void *stream, NPBool seekable, uint16_t *stype)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  // Detect broken 64-bit NPAPI
  if (g_use_long64_thunks < 0) {
	D(bug("WARNING: function using an NPStream was called too early, could not determine LONG64 data structure\n"));
	set_use_long64_thunks(false);
  }

  if (g_use_long64_thunks) {
	NPStream *stream32;
	if ((stream32 = malloc(sizeof(*stream32))) == NULL)
	  return NPERR_OUT_OF_MEMORY_ERROR;
	convert_from_LONG64_NPStream(stream32, stream);
	stream32->ndata = stream;
	((NPStream *)stream)->pdata = stream32;
  }

  return g_NPP_NewStream(instance, type, NP_STREAM32(stream), seekable, stype);
}

// NPP_DestroyStream (LONG64)
static NPError
g_LONG64_NPP_DestroyStream(NPP instance, void *stream, NPReason reason)
{
  if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

  if (stream == NULL)
	return NPERR_INVALID_PARAM;

  NPError ret = g_NPP_DestroyStream(instance, NP_STREAM32(stream), reason);

  if (g_use_long64_thunks) {
	free(((NPStream *)stream)->pdata);
	((NPStream *)stream)->pdata = NULL;
  }

  return ret;
}

// NPP_WriteReady (LONG64)
static int64_t
g_LONG64_NPP_WriteReady(NPP instance, void *stream)
{
  if (instance == NULL)
	return 0;

  if (stream == NULL)
	return 0;

  return (int64_t)(int32_t)g_NPP_WriteReady(instance, NP_STREAM32(stream));
}

// NPP_Write (LONG64)
static int64_t
g_LONG64_NPP_Write(NPP instance, void *stream, int64_t offset, int64_t len, void *buf)
{
  if (instance == NULL)
	return -1L;

  if (stream == NULL)
	return -1L;

  return (int64_t)(int32_t)g_NPP_Write(instance, NP_STREAM32(stream), offset, len, buf);
}

// NPP_StreamAsFile (LONG64)
static void
g_LONG64_NPP_StreamAsFile(NPP instance, void *stream, const char *fname)
{
  if (instance == NULL)
	return;

  if (stream == NULL)
	return;

  g_NPP_StreamAsFile(instance, NP_STREAM32(stream), fname);
}

// NPP_Print (LONG64)
static void g_LONG64_NPP_Print(NPP instance, void *PrintInfo)
{
  if (instance == NULL)
	return;

  if (PrintInfo == NULL)
	return;

  // Detect broken 64-bit NPAPI
  if (g_use_long64_thunks < 0)
	set_use_long64_thunks(is_LONG64_NPPrintCallbackStruct(PrintInfo));

  NPPrint PrintInfo32;
  NPPrintCallbackStruct platformPrint32;
  if (g_use_long64_thunks) {
	memcpy(&PrintInfo32, PrintInfo, sizeof(PrintInfo32));
	void *platformPrint;
	switch (((NPPrint *)PrintInfo)->mode) {
	case NP_FULL:
	  platformPrint = ((NPPrint *)PrintInfo)->print.fullPrint.platformPrint;
	  convert_from_LONG64_NPPrintCallbackStruct(&platformPrint32, platformPrint);
	  PrintInfo32.print.fullPrint.platformPrint = &platformPrint32;
	  break;
	case NP_EMBED:
	  platformPrint = ((NPPrint *)PrintInfo)->print.embedPrint.platformPrint;
	  convert_from_LONG64_NPPrintCallbackStruct(&platformPrint32, platformPrint);
	  PrintInfo32.print.embedPrint.platformPrint = &platformPrint32;
	  break;
	}
	PrintInfo = &PrintInfo32;
  }

  g_NPP_Print(instance, PrintInfo);
}


/* ====================================================================== */
/* === Plug-in initialization                                         === */
/* ====================================================================== */

// Detect Konqueror
static bool is_konqueror(void)
{
  if (dlsym(RTLD_DEFAULT, "qApp") == NULL)
	return false;
  if (mozilla_funcs.getvalue == NULL)
	return false;
  Display *x_display = NULL;
  if (mozilla_funcs.getvalue(NULL, NPNVxDisplay, (void *)&x_display) != NPERR_NO_ERROR)
	return false;
  XtAppContext x_app_context = NULL;
  if (mozilla_funcs.getvalue(NULL, NPNVxtAppContext, (void *)&x_app_context) != NPERR_NO_ERROR)
	return false;
  if (x_display == NULL || x_app_context == NULL)
	return false;
  String name, class;
  XtGetApplicationNameAndClass(x_display, &name, &class);
  if (strcmp(name, "nspluginviewer") == 0)
	return true;
  // XXX user-agent string can be changed, but it's still an heuristic
  const char *user_agent = g_NPN_UserAgent(NULL);
  if (user_agent == NULL)
	return false;
  if (strstr(user_agent, "Konqueror") != NULL)
	return true;
  return false;
}

// Provides global initialization for a plug-in
static NPError
invoke_NP_Initialize(uint32_t npapi_version,
					 uint32_t *plugin_version,
					 uint32_t **plugin_capabilities,
					 uint32_t *plugin_capabilities_len)
{
  if (PLUGIN_DIRECT_EXEC) {
	NPNetscapeFuncs wrapped_mozilla_funcs;
	memset(&wrapped_mozilla_funcs, 0, sizeof(wrapped_mozilla_funcs));
	wrapped_mozilla_funcs.size = sizeof(wrapped_mozilla_funcs);
	wrapped_mozilla_funcs.version = npapi_version;
#define BROWSER_FUNC(func, member)					\
	if (mozilla_funcs.member != NULL)				\
	  wrapped_mozilla_funcs.member = g_ ## func;
#include "browser-funcs.h"
#undef BROWSER_FUNC
	NPError error = g_plugin_NP_Initialize(&wrapped_mozilla_funcs, &plugin_funcs);
    *plugin_version = plugin_funcs.version;
    return error;
  }

  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection),
						 NPERR_MODULE_LOAD_FAILED_ERROR);

  // Allocate browser capabilities.
  uint32_t browser_capabilities[] = {
#define BROWSER_FUNC(func, member)				\
	(mozilla_funcs.member != NULL) ? 1 : 0,
#include "browser-funcs.h"
#undef BROWSER_FUNC
  };

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NP_INITIALIZE,
								RPC_TYPE_UINT32, npapi_version,
								RPC_TYPE_ARRAY, RPC_TYPE_UINT32,
								G_N_ELEMENTS(browser_capabilities),
								browser_capabilities,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NP_Initialize() invoke", error);
	return NPERR_MODULE_LOAD_FAILED_ERROR;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection,
                                    RPC_TYPE_INT32, &ret,
                                    RPC_TYPE_UINT32, plugin_version,
									RPC_TYPE_ARRAY, RPC_TYPE_UINT32,
									plugin_capabilities_len,
									plugin_capabilities,
                                    RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NP_Initialize() wait for reply", error);
	return NPERR_MODULE_LOAD_FAILED_ERROR;
  }

  return ret;
}

static NPError
g_NP_Initialize(uint32_t npapi_version,
				uint32_t *plugin_version,
				uint32_t **plugin_capabilities, uint32_t *plugin_capabilities_len)
{
  D(bugiI("NP_Initialize\n"));
  NPError ret = invoke_NP_Initialize(npapi_version, plugin_version,
									 plugin_capabilities, plugin_capabilities_len);
  D(bugiD("NP_Initialize return: %d [%s], plugin_version=%d\n",
          ret, string_of_NPError(ret), *plugin_version));
  return ret;
}

NPError
NP_Initialize(NPNetscapeFuncs *moz_funcs, NPPluginFuncs *plugin_funcs)
{
  D(bug("NP_Initialize\n"));

  if (moz_funcs == NULL || plugin_funcs == NULL)
	return NPERR_INVALID_FUNCTABLE_ERROR;

  if ((moz_funcs->version >> 8) != NP_VERSION_MAJOR)
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  // for now, we only need fields up to including forceRedraw
  if (moz_funcs->size < (offsetof(NPNetscapeFuncs, forceredraw) + sizeof(NPN_ForceRedrawProcPtr)))
    return NPERR_INVALID_FUNCTABLE_ERROR;

  if (g_plugin.initialized == 0)
	plugin_init(0);
  if (g_plugin.initialized <= 0)
	return NPERR_GENERIC_ERROR;

  if (g_plugin.is_wrapper)
	return NPERR_NO_ERROR;

  // copy mozilla_funcs table here as plugin_init() will need it
  memcpy(&mozilla_funcs, moz_funcs, MIN(moz_funcs->size, sizeof(mozilla_funcs)));

  static NPPluginFuncs full_plugin_funcs;
  memset(&full_plugin_funcs, 0, sizeof(full_plugin_funcs));
  full_plugin_funcs.size = sizeof(NPPluginFuncs);
  full_plugin_funcs.version = NPW_NPAPI_VERSION;
#define PLUGIN_FUNC(func, member)				\
  full_plugin_funcs.member = g_ ## func;
#include "plugin-funcs.h"
#undef PLUGIN_FUNC
  full_plugin_funcs.javaClass = NULL;

  // override function table with an additional thunking layer for
  // possibly broken 64-bit Konqueror versions (NPAPI 0.11)
  if (sizeof(void *) == 8 && ! NPN_HAS_FEATURE(NPRUNTIME_SCRIPTING) && is_konqueror()) {
    D(bug("Installing Konqueror workarounds\n"));
    // We're doing sketchy pointer casts, so just cast the function pointers to
    // make the compiler be quiet. We're doing this intentionally.
	full_plugin_funcs.setwindow = (NPP_SetWindowProcPtr)g_LONG64_NPP_SetWindow;
	full_plugin_funcs.newstream = (NPP_NewStreamProcPtr)g_LONG64_NPP_NewStream;
	full_plugin_funcs.destroystream = (NPP_DestroyStreamProcPtr)g_LONG64_NPP_DestroyStream;
	full_plugin_funcs.asfile = (NPP_StreamAsFileProcPtr)g_LONG64_NPP_StreamAsFile;
	full_plugin_funcs.writeready = (NPP_WriteReadyProcPtr)g_LONG64_NPP_WriteReady;
	full_plugin_funcs.write = (NPP_WriteProcPtr)g_LONG64_NPP_Write;
	full_plugin_funcs.print = (NPP_PrintProcPtr)g_LONG64_NPP_Print;
	full_plugin_funcs.newp = (NPP_NewProcPtr)g_LONG64_NPP_New;
	full_plugin_funcs.destroy = (NPP_DestroyProcPtr)g_LONG64_NPP_Destroy;
  }

  // Initialize function tables
  // XXX: remove the local copies from this file
  // XXX: This doesn't get the adjust plugin version number below, but don't
  // use it and someone may try to call something in npw-common.c in the
  // meantime.
  NPW_InitializeFuncs(moz_funcs, &full_plugin_funcs);

  if (g_plugin.initialized == 0 || g_plugin.initialized == 1)
	plugin_init(1);
  if (g_plugin.initialized <= 0)
	return NPERR_MODULE_LOAD_FAILED_ERROR;

  if (!id_init())
	return NPERR_MODULE_LOAD_FAILED_ERROR;

  if (!npobject_bridge_new())
	return NPERR_MODULE_LOAD_FAILED_ERROR;

  // pass down common NPAPI version supported by both the underlying
  // browser and the thunking capabilities of nspluginwrapper
  D(bug("Thunking layer supports NPAPI %d\n", NPW_NPAPI_VERSION));
  npapi_version = MIN(moz_funcs->version, NPW_NPAPI_VERSION);
  D(bug("Browser supports NPAPI %d, advertising version %d to plugin\n",
        moz_funcs->version, npapi_version));
  uint32_t plugin_version = 0;
  uint32_t *plugin_capabilities = NULL;
  uint32_t plugin_capabilities_len = 0;
  NPError error = g_NP_Initialize(npapi_version, &plugin_version,
								  &plugin_capabilities, &plugin_capabilities_len);

  // Likewise, advertise the common NPAPI version between the plugin and our
  // thunking capabilities.
  full_plugin_funcs.version = MIN(plugin_version, NPW_NPAPI_VERSION);
  D(bug("Plugin supports NPAPI %d, advertising version %d to browser\n",
        plugin_version, full_plugin_funcs.version));

  if (plugin_capabilities) {
	// Don't advertise any functions the plugin doesn't support.
	int num = 0;
#define PLUGIN_FUNC(func, member)								\
	if (num >= plugin_capabilities_len)	{						\
	  D(bug("ERROR: provided array was too small.\n"));			\
	  goto plugin_func_done;									\
	}															\
	if (!plugin_capabilities[num]) {							\
	  D(bug("plugin does not support " #func "\n"));			\
	  full_plugin_funcs.member = NULL;							\
	}															\
	num++;
#include "plugin-funcs.h"
#undef PLUGIN_FUNC
  plugin_func_done:
	free(plugin_capabilities);
  }

  // Copy only the portion of full_plugin_funcs that the browser
  // understands.
  uint16_t plugin_funcs_size = MIN(plugin_funcs->size, sizeof(full_plugin_funcs));
  memcpy(plugin_funcs, &full_plugin_funcs, plugin_funcs_size);
  plugin_funcs->size = plugin_funcs_size;

  return error;
}

// Provides global deinitialization for a plug-in
static NPError
invoke_NP_Shutdown(void)
{
  if (PLUGIN_DIRECT_EXEC)
	return g_plugin_NP_Shutdown();

  if (g_rpc_connection == NULL)
	return NPERR_NO_ERROR;

  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection),
						 NPERR_GENERIC_ERROR);

  int error = rpc_method_invoke(g_rpc_connection, RPC_METHOD_NP_SHUTDOWN, RPC_TYPE_INVALID);
  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NP_Shutdown() invoke", error);
	return NPERR_GENERIC_ERROR;
  }

  int32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection, RPC_TYPE_INT32, &ret, RPC_TYPE_INVALID);
  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NP_Shutdown() wait for reply", error);
	return NPERR_GENERIC_ERROR;
  }

  return ret;
}

static NPError
g_NP_Shutdown(void)
{
  D(bugiI("NP_Shutdown\n"));
  NPError ret = invoke_NP_Shutdown();
  D(bugiD("NP_Shutdown return: %d [%s]\n", ret, string_of_NPError(ret)));

  if (!g_plugin.is_wrapper)
	plugin_exit();

  return ret;
}

NPError
NP_Shutdown(void)
{
  NPError ret = g_NP_Shutdown();

  npobject_bridge_destroy();

  id_kill();

  return ret;
}

static bool plugin_init_info()
{
	char *command = g_strdup_printf("%s --info --plugin %s", plugin_viewer_path, plugin_path);
	D(bug("command [%s]\n", command));
	
	FILE *viewer_fp = popen(command, "r");
	g_free(command);
	
	if (viewer_fp == NULL)
	{
	  D(bug("viewer_fp us Null\n"));
	  return false;
    }
  
	char line[256];
	while (fgets(line, sizeof(line), viewer_fp)) {
	  // Read line
	  int len = strlen(line);
	  if (len == 0)
		continue;
	  if (line[len - 1] != '\n') {
		// Consume the whole line, we can't see our tags here
		while (fgets(line, sizeof(line), viewer_fp)) {
		  len = strlen(line);
		  if (len > 0 && line[len - 1] == '\n')
			break;
		}
		continue;
	  }
	  line[len - 1] = '\0';

	  // Parse line
	  char tag[sizeof(line)];
	  if (sscanf(line, "%s %d", tag, &len) == 2) {
		char *str = malloc(++len);
		if (str && fgets(str, len, viewer_fp)) {
		  char **ptag = NULL;
		  if (strcmp(tag, "PLUGIN_NAME") == 0)
			ptag = &g_plugin.name;
		  else if (strcmp(tag, "PLUGIN_DESC") == 0)
			ptag = &g_plugin.description;
		  else if (strcmp(tag, "PLUGIN_MIME") == 0)
			ptag = &g_plugin.formats;
		  if (ptag)
			*ptag = str;
		}
	  }
	}
	
	pclose(viewer_fp);
	return true;
}


// Initialize wrapper plugin and execute viewer
static void plugin_init(int is_NP_Initialize)
{
  if (g_plugin.initialized < 0)
	return;

  g_plugin.initialized = -1;

  D(bug("plugin_init for %s [%s]\n", plugin_path, NPW_DEFAULT_PLUGIN_PATH));
  
  if (strcmp(plugin_path, NPW_DEFAULT_PLUGIN_PATH) == 0) {
	g_plugin.is_wrapper = 1;
	g_plugin.initialized = 1 + is_NP_Initialize;
	return;
  }

  if (PLUGIN_DIRECT_EXEC){
	g_plugin.initialized = 1;
	return;
  }

  static const char *plugin_file_name = NULL;
  if (plugin_file_name == NULL) {
	  
	plugin_file_name = plugin_path;
	  
	const char *p;
	for (p = &plugin_path[strlen(plugin_path) - 1]; p > plugin_path; p--) {
	  if (*p == '/') {
		plugin_file_name = p + 1;
		break;
	  }
	}
	
	D(bug("plugin_file_name %s\n", plugin_file_name));
	
	if (plugin_file_name == NULL)
	  return;
  }

  static int init_count = 0;
  ++init_count;

  // Cache MIME info and plugin name/description
  if (g_plugin.name == NULL && g_plugin.description == NULL && g_plugin.formats == NULL) 
  {
	  if (!plugin_init_info())
		return;
  }

  g_plugin.initialized = 1;

  if (!is_NP_Initialize)
	return;

  char *connection_path = g_strdup_printf("%s/%s/%d-%d/%ld", NPW_CONNECTION_PATH, plugin_file_name, getpid(), init_count, random());

  // Start plug-in viewer
  if ((g_plugin.viewer_pid = fork()) == 0) {
	char *argv[8];
	int argc = 0;

	argv[argc++] = NPW_VIEWER;
	argv[argc++] = "--plugin";
	argv[argc++] = (char *)plugin_path;
	argv[argc++] = "--connection";
	argv[argc++] = connection_path;
	argv[argc] = NULL;

	npw_close_all_open_files();

	execv(plugin_viewer_path, argv);
	npw_printf("ERROR: failed to execute NSPlugin viewer\n");
	_Exit(255);
  }

  // Initialize browser-side RPC communication channel
  if ((g_rpc_connection = rpc_init_client(connection_path)) == NULL) {
	npw_printf("ERROR: failed to initialize plugin-side RPC client connection\n");
	g_free(connection_path);
	return;
  }
  g_free(connection_path);
  if (rpc_add_np_marshalers(g_rpc_connection) < 0) {
	npw_printf("ERROR: failed to initialize browser-side marshalers\n");
	return;
  }
  static const rpc_method_descriptor_t vtable[] = {
	{ RPC_METHOD_NPN_USER_AGENT,						handle_NPN_UserAgent },
	{ RPC_METHOD_NPN_GET_VALUE,							handle_NPN_GetValue },
	{ RPC_METHOD_NPN_SET_VALUE,							handle_NPN_SetValue },
	{ RPC_METHOD_NPN_GET_URL,							handle_NPN_GetURL },
	{ RPC_METHOD_NPN_GET_URL_NOTIFY,					handle_NPN_GetURLNotify },
	{ RPC_METHOD_NPN_POST_URL,							handle_NPN_PostURL },
	{ RPC_METHOD_NPN_POST_URL_NOTIFY,					handle_NPN_PostURLNotify },
	{ RPC_METHOD_NPN_STATUS,							handle_NPN_Status },
	{ RPC_METHOD_NPN_PRINT_DATA,						handle_NPN_PrintData },
	{ RPC_METHOD_NPN_REQUEST_READ,						handle_NPN_RequestRead },
	{ RPC_METHOD_NPN_NEW_STREAM,						handle_NPN_NewStream },
	{ RPC_METHOD_NPN_DESTROY_STREAM,					handle_NPN_DestroyStream },
	{ RPC_METHOD_NPN_WRITE,								handle_NPN_Write },
	{ RPC_METHOD_NPN_PUSH_POPUPS_ENABLED_STATE,			handle_NPN_PushPopupsEnabledState },
	{ RPC_METHOD_NPN_POP_POPUPS_ENABLED_STATE,			handle_NPN_PopPopupsEnabledState },
	{ RPC_METHOD_NPN_INVALIDATE_RECT,					handle_NPN_InvalidateRect },
	{ RPC_METHOD_NPN_GET_VALUE_FOR_URL,					handle_NPN_GetValueForURL },
	{ RPC_METHOD_NPN_SET_VALUE_FOR_URL,					handle_NPN_SetValueForURL },
	{ RPC_METHOD_NPN_GET_AUTHENTICATION_INFO,			handle_NPN_GetAuthenticationInfo },
	{ RPC_METHOD_NPN_INVOKE,							handle_NPN_Invoke },
	{ RPC_METHOD_NPN_INVOKE_DEFAULT,					handle_NPN_InvokeDefault },
	{ RPC_METHOD_NPN_EVALUATE,							handle_NPN_Evaluate },
	{ RPC_METHOD_NPN_GET_PROPERTY,						handle_NPN_GetProperty },
	{ RPC_METHOD_NPN_SET_PROPERTY,						handle_NPN_SetProperty },
	{ RPC_METHOD_NPN_REMOVE_PROPERTY,					handle_NPN_RemoveProperty },
	{ RPC_METHOD_NPN_HAS_PROPERTY,						handle_NPN_HasProperty },
	{ RPC_METHOD_NPN_HAS_METHOD,						handle_NPN_HasMethod },
	{ RPC_METHOD_NPN_ENUMERATE,							handle_NPN_Enumerate },
	{ RPC_METHOD_NPN_CONSTRUCT,							handle_NPN_Construct },
	{ RPC_METHOD_NPN_SET_EXCEPTION,						handle_NPN_SetException },
	{ RPC_METHOD_NPN_GET_STRING_IDENTIFIER,				handle_NPN_GetStringIdentifier },
	{ RPC_METHOD_NPN_GET_STRING_IDENTIFIERS,			handle_NPN_GetStringIdentifiers },
	{ RPC_METHOD_NPN_GET_INT_IDENTIFIER,				handle_NPN_GetIntIdentifier },
	{ RPC_METHOD_NPN_IDENTIFIER_IS_STRING,				handle_NPN_IdentifierIsString },
	{ RPC_METHOD_NPN_UTF8_FROM_IDENTIFIER,				handle_NPN_UTF8FromIdentifier },
	{ RPC_METHOD_NPN_INT_FROM_IDENTIFIER,				handle_NPN_IntFromIdentifier },
  };
  if (rpc_connection_add_method_descriptors(g_rpc_connection, vtable, sizeof(vtable) / sizeof(vtable[0])) < 0) {
	npw_printf("ERROR: failed to setup NPN method callbacks\n");
	return;
  }
  if (npclass_add_method_descriptors(g_rpc_connection) < 0) {
	npw_printf("ERROR: failed to setup NPClass method callbacks\n");
	return;
  }

  // Retrieve toolkit information
  if (mozilla_funcs.getvalue == NULL)
	return;
  NPNToolkitType toolkit = 0;
  mozilla_funcs.getvalue(NULL, NPNVToolkit, (void *)&toolkit);

  // Initialize RPC events listener, try to attach it to the main event loop
  if (toolkit == NPNVGtk12 || toolkit == NPNVGtk2
	  || toolkit == 0xFEEDABEE) {  // GLib
	// We use the glib event loop in Konqueror (0xFEEDABEE) because
	// its Xt event loop bridge is completely broken and non-functional.
	D(bug("  trying to attach RPC listener to main GLib event loop\n"));
	g_rpc_source = rpc_event_source_new(g_rpc_connection);
	g_source_set_priority(g_rpc_source, G_PRIORITY_LOW);
	g_source_attach(g_rpc_source, NULL);

	g_rpc_sync_source = rpc_sync_source_new(g_rpc_connection);
	g_source_set_priority(g_rpc_sync_source, G_PRIORITY_HIGH);
	g_source_attach(g_rpc_sync_source, NULL);
  } else {							// X11
	D(bug("  trying to attach RPC listener to main X11 event loop\n"));
	XtAppContext x_app_context = NULL;
	int error = mozilla_funcs.getvalue(NULL, NPNVxtAppContext, (void *)&x_app_context);
	if (error != NPERR_NO_ERROR || x_app_context == NULL) {
	  D(bug("  ... getting raw application context through X display\n"));
	  Display *x_display = NULL;
	  error = mozilla_funcs.getvalue(NULL, NPNVxDisplay, (void *)&x_display);
	  if (error == NPERR_NO_ERROR && x_display)
		x_app_context = XtDisplayToApplicationContext(x_display);
	}
	if (x_app_context) {
	  xt_rpc_source_id = XtAppAddInput(x_app_context,
									   rpc_socket(g_rpc_connection),
									   (XtPointer)XtInputReadMask,
									   (XtInputCallbackProc)rpc_dispatch, g_rpc_connection);
	  xt_rpc_sync_id = XtAppAddBlockHook(x_app_context,
										 (XtBlockHookProc)rpc_dispatch_pending_sync,
										 g_rpc_connection);
	}
  }
  if (g_rpc_source == NULL && (xt_rpc_source_id == 0 || xt_rpc_sync_id == 0)) {
	npw_printf("ERROR: failed to initialize brower-side RPC events listener\n");
	return;
  }

  // Set error handler - stop plugin if there's a connection error
  rpc_connection_set_error_callback(g_rpc_connection, plugin_kill_cb, NULL);

  g_plugin.initialized = 1 + is_NP_Initialize;
  D(bug("--- INIT ---\n"));
}

// Kill NSPlugin Viewer process
static void plugin_exit(void)
{
  D(bug("plugin_exit\n"));

  if (xt_rpc_source_id) {
	XtRemoveInput(xt_rpc_source_id);
	xt_rpc_source_id = 0;
  }
  if (xt_rpc_sync_id) {
	XtRemoveBlockHook(xt_rpc_sync_id);
	xt_rpc_sync_id = 0;
  }

  if (g_rpc_source) {
	g_source_destroy(g_rpc_source);
	g_rpc_source = NULL;
  }
  if (g_rpc_sync_source) {
	g_source_destroy(g_rpc_sync_source);
	g_rpc_sync_source = NULL;
  }

  if (g_rpc_connection) {
	rpc_connection_unref(g_rpc_connection);
	g_rpc_connection = NULL;
  }

  if (g_plugin.viewer_pid != -1) {
	// let it shutdown gracefully, then kill it gently to no mercy
	const int WAITPID_DELAY_TO_SIGTERM = 3;
	const int WAITPID_DELAY_TO_SIGKILL = 3;
	int counter = 0;
	while (waitpid(g_plugin.viewer_pid, NULL, WNOHANG) == 0) {
	  if (++counter > WAITPID_DELAY_TO_SIGTERM) {
		kill(g_plugin.viewer_pid, SIGTERM);
		counter = 0;
		while (waitpid(g_plugin.viewer_pid, NULL, WNOHANG) == 0) {
		  if (++counter > WAITPID_DELAY_TO_SIGKILL) {
			kill(g_plugin.viewer_pid, SIGKILL);
			break;
		  }
		  sleep(1);
		}
		break;
	  }
	  sleep(1);
	}
	g_plugin.viewer_pid = -1;
  }

  g_plugin.initialized = 0;
}

static void __attribute__((destructor)) plugin_exit_sentinel(void)
{
  plugin_exit();

  if (plugin_handle) {
	dlclose(plugin_handle);
	plugin_handle = NULL;
  }

  if (g_plugin.formats) {
	free(g_plugin.formats);
	g_plugin.formats = NULL;
  }

  if (g_plugin.name) {
	free(g_plugin.name);
	g_plugin.name = NULL;
  }

  if (g_plugin.description) {
	free(g_plugin.description);
	g_plugin.description = NULL;
  }
}

static void plugin_kill(void)
{
  if (g_plugin.is_wrapper)
	return;  

  // Kill viewer and plugin
  plugin_exit();

  // Clear-up
  g_plugin.initialized = 0;
  g_plugin.viewer_pid = -1;
  g_plugin.is_wrapper = 0;

  npruntime_deactivate();

  // Set the kill flag
  plugin_killed = 1;
}

static void plugin_kill_cb(rpc_connection_t *connection, void *user_data)
{
  D(bug("plugin_kill, connection %p\n", connection));

  // Don't kill the plugin again through another instance
  rpc_connection_set_error_callback(connection, NULL, NULL);

  plugin_kill();
}

static NPError plugin_start(void)
{
  D(bug("plugin_start\n"));

  if (!plugin_killed) {
    // Plugin is still active, terminate it before the restart
    D(bug("plugin_start: plugin_killed == 0!\n"));
    plugin_kill();
  }
  plugin_killed = 0;

  // And start it again
  plugin_init(1);
  if (g_plugin.initialized <= 0)
	return NPERR_MODULE_LOAD_FAILED_ERROR;

  uint32_t plugin_version;
  uint32_t *plugin_capabilities = NULL;
  uint32_t plugin_capabilities_len;
  NPError ret = g_NP_Initialize(npapi_version, &plugin_version,
								&plugin_capabilities, &plugin_capabilities_len);
  // Assume capabilities unchanged.
  if (plugin_capabilities)
	free(plugin_capabilities);
  return ret;
}

static NPError plugin_start_if_needed(void)
{
  if (PLUGIN_DIRECT_EXEC)
	return NPERR_NO_ERROR;

  if (rpc_status(g_rpc_connection) != RPC_STATUS_ACTIVE) {
	static time_t last_restart = 0;
	time_t now = time(NULL);
	if (now - last_restart < MIN_RESTART_INTERVAL)
	  return NPERR_GENERIC_ERROR;
	last_restart = now;

	D(bug("Restart plugins viewer\n"));
	NPError ret = plugin_start();
	D(bug(" return: %d [%s]\n", ret, string_of_NPError(ret)));
	return ret;
  }

  return NPERR_NO_ERROR;
}
