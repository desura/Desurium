/*
 *  npw-rpc.c - Remote Procedure Calls (NPAPI specialisation)
 *
 *  nspluginwrapper (C) 2005-2009 Gwenole Beauchesne
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

#include "sysdeps.h"
#include <assert.h>

#include "utils.h"
#include "npw-common.h"

#define DEBUG 1
#include "debug.h"


/*
 *  RPC types of NPP/NPN variables
 */

int rpc_type_of_NPNVariable(int variable)
{
  int type;
  switch (variable) {
  case NPNVjavascriptEnabledBool:
  case NPNVasdEnabledBool:
  case NPNVisOfflineBool:
  case NPNVSupportsXEmbedBool:
  case NPNVSupportsWindowless:
  case NPNVprivateModeBool:
  case NPNVsupportsAdvancedKeyHandling:
	type = RPC_TYPE_BOOLEAN;
	break;
  case NPNVToolkit:
  case NPNVnetscapeWindow:
	type = RPC_TYPE_UINT32;
	break;
  case NPNVWindowNPObject:
  case NPNVPluginElementNPObject:
	type = RPC_TYPE_NP_OBJECT;
	break;
  default:
	type = RPC_ERROR_GENERIC;
	break;
  }
  return type;
}

int rpc_type_of_NPPVariable(int variable)
{
  int type;
  switch (variable) {
  case NPPVpluginNameString:
  case NPPVpluginDescriptionString:
  case NPPVformValue: // byte values of 0 does not appear in the UTF-8 encoding but for U+0000
  case NPPVpluginNativeAccessibleAtkPlugId:
	type = RPC_TYPE_STRING;
	break;
  case NPPVpluginWindowSize:
  case NPPVpluginTimerInterval:
	type = RPC_TYPE_INT32;
	break;
  case NPPVpluginNeedsXEmbed:
  case NPPVpluginWindowBool:
  case NPPVpluginTransparentBool:
  case NPPVjavascriptPushCallerBool:
  case NPPVpluginKeepLibraryInMemory:
  case NPPVpluginUrlRequestsDisplayedBool:
  case NPPVpluginWantsAllNetworkStreams:
  case NPPVpluginCancelSrcStream:
  case NPPVsupportsAdvancedKeyHandling:
  case NPPVpluginUsesDOMForCursorBool:
	type = RPC_TYPE_BOOLEAN;
	break;
  case NPPVpluginScriptableNPObject:
	type = RPC_TYPE_NP_OBJECT;
	break;
  default:
	type = RPC_ERROR_GENERIC;
	break;
  }
  return type;
}


/*
 *  Process NPW_PluginInstance objects
 */

static int do_send_NPW_PluginInstance(rpc_message_t *message, void *p_value)
{
  NPW_PluginInstance *plugin = (NPW_PluginInstance *)p_value;
  uint32_t instance_id = 0;
  if (plugin)
	instance_id = plugin->instance_id;
  return rpc_message_send_uint32(message, instance_id);
}

static int do_recv_NPW_PluginInstance(rpc_message_t *message, void *p_value)
{
  int error;
  uint32_t instance_id;

  if ((error = rpc_message_recv_uint32(message, &instance_id)) < 0)
	return error;

  NPW_PluginInstance *plugin = id_lookup(instance_id);
  if (instance_id && plugin == NULL)
	npw_printf("ERROR: no valid NPP -> PluginInstance mapping found\n");
  else if (plugin && plugin->instance == NULL)
	npw_printf("ERROR: no valid PluginInstance -> NPP mapping found\n");
  else if (plugin && !npw_plugin_instance_is_valid(plugin))
	npw_printf("ERROR: received PluginInstance was invalidated earlier\n");
  *((NPW_PluginInstance **)p_value) = plugin;
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPP objects
 */

static int do_send_NPP(rpc_message_t *message, void *p_value)
{
  NPP instance = (NPP)p_value;
  NPW_PluginInstance *plugin = NULL;
  if (instance)
	plugin = NPW_PLUGIN_INSTANCE(instance);
  return do_send_NPW_PluginInstance(message, plugin);
}

static int do_recv_NPP(rpc_message_t *message, void *p_value)
{
  int error;
  NPW_PluginInstance *plugin;

  if ((error = do_recv_NPW_PluginInstance(message, &plugin)) < 0)
	return error;

  *((NPP *)p_value) = plugin ? plugin->instance : NULL;
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPStream objects
 */

static int do_send_NPStream(rpc_message_t *message, void *p_value)
{
  uint32_t stream_id = 0;
  NPStream *stream = (NPStream *)p_value;
  if (stream) {
	NPW_StreamInstance *sip = NPW_STREAM_INSTANCE(stream);
	if (sip)
	  stream_id = sip->stream_id;
  }
  return rpc_message_send_uint32(message, stream_id);
}

static int do_recv_NPStream(rpc_message_t *message, void *p_value)
{
  int error;
  uint32_t stream_id;

  if ((error = rpc_message_recv_uint32(message, &stream_id)) < 0)
	return error;

  NPW_StreamInstance *stream = id_lookup(stream_id);
  *((NPStream **)p_value) = stream ? stream->stream : NULL;
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPByteRange objects
 */

static int do_send_NPByteRange(rpc_message_t *message, void *p_value)
{
  NPByteRange *range = (NPByteRange *)p_value;
  while (range) {
	int error;
	if ((error = rpc_message_send_uint32(message, 1)) < 0)
	  return error;
	if ((error = rpc_message_send_int32(message, range->offset)) < 0)
	  return error;
	if ((error = rpc_message_send_uint32(message, range->length)) < 0)
	  return error;
	range = range->next;
  }
  return rpc_message_send_uint32(message, 0);
}

static int do_recv_NPByteRange(rpc_message_t *message, void *p_value)
{
  NPByteRange **rangeListPtr = (NPByteRange **)p_value;
  if (rangeListPtr == NULL)
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;
  *rangeListPtr = NULL;

  for (;;) {
	int error;
	uint32_t cont;

	if ((error = rpc_message_recv_uint32(message, &cont)) < 0)
	  return error;
	if (!cont)
	  break;
	NPByteRange *range = malloc(sizeof(*range));
	if (range == NULL)
	  return RPC_ERROR_NO_MEMORY;
	range->next = NULL;
	if ((error = rpc_message_recv_int32(message, &range->offset)) < 0)
	  return error;
	if ((error = rpc_message_recv_uint32(message, &range->length)) < 0)
	  return error;
	*rangeListPtr = range;
	rangeListPtr = &range->next;
  }
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPSavedData objects
 */

static int do_send_NPSavedData(rpc_message_t *message, void *p_value)
{
  NPSavedData *save_area = (NPSavedData *)p_value;
  int error;

  if (save_area == NULL) {
	if ((error = rpc_message_send_int32(message, 0)) < 0)
	  return error;
  }
  else {
	if ((error = rpc_message_send_int32(message, save_area->len)) < 0)
	  return error;
	if ((error = rpc_message_send_bytes(message, save_area->buf, save_area->len)) < 0)
	  return error;
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPSavedData(rpc_message_t *message, void *p_value)
{
  NPSavedData *save_area;
  int error;
  int32_t len;
  unsigned char *buf;

  if ((error = rpc_message_recv_int32(message, &len)) < 0)
	return error;
  if (len == 0)
	save_area = NULL;
  else {
	if ((save_area = NPN_MemAlloc(sizeof(*save_area))) == NULL)
	  return RPC_ERROR_NO_MEMORY;
	if ((buf = NPN_MemAlloc(len)) == NULL)
	  return RPC_ERROR_NO_MEMORY;
	if ((error = rpc_message_recv_bytes(message, buf, len)) < 0)
	  return error;
	save_area->len = len;
	save_area->buf = buf;
  }

  if (p_value)
	*((NPSavedData **)p_value) = save_area;
  else if (save_area) {
	NPN_MemFree(save_area->buf);
	NPN_MemFree(save_area);
  }

  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NotifyData objects
 */

// Rationale: NotifyData objects are allocated on the plugin side
// only. IDs are passed through to the browser, and they have no
// meaning on that side as they are only used to get passed back to
// the plugin side
//
// XXX 64-bit viewers in 32-bit wrappers are not supported
static int do_send_NotifyData(rpc_message_t *message, void *p_value)
{
  void *notifyData = (void *)p_value;
  return rpc_message_send_uint64(message, (uintptr_t)notifyData);
}

static int do_recv_NotifyData(rpc_message_t *message, void *p_value)
{
  int error;
  uint64_t id;

  if ((error = rpc_message_recv_uint64(message, &id)) < 0)
	return error;

  if (sizeof(void *) == 4 && ((uint32_t)(id >> 32)) != 0) {
	npw_printf("ERROR: 64-bit viewers in 32-bit wrappers are not supported\n");
	abort();
  }

  *((void **)p_value) = (void *)(uintptr_t)id;
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPRect objects
 */

static int do_send_NPRect(rpc_message_t *message, void *p_value)
{
  NPRect *rect = (NPRect *)p_value;
  int error;

  if ((error = rpc_message_send_uint32(message, rect->top)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, rect->left)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, rect->bottom)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, rect->right)) < 0)
	return error;

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPRect(rpc_message_t *message, void *p_value)
{
  NPRect *rect = (NPRect *)p_value;
  uint32_t top, left, bottom, right;
  int error;

  if ((error = rpc_message_recv_uint32(message, &top)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &left)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &bottom)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &right)) < 0)
	return error;

  rect->top = top;
  rect->left = left;
  rect->bottom = bottom;
  rect->right = right;
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPWindow objects
 */

static int do_send_NPSetWindowCallbackStruct(rpc_message_t *message, void *p_value)
{
  NPSetWindowCallbackStruct *ws_info = (NPSetWindowCallbackStruct *)p_value;
  int error;

  if (ws_info) {
	if ((error = rpc_message_send_uint32(message, 1)) < 0)
	  return error;
	if ((error = rpc_message_send_int32(message, ws_info->type)) < 0)
	  return error;
	if ((error = rpc_message_send_uint32(message, ws_info->visual ? XVisualIDFromVisual(ws_info->visual) : 0)) < 0)
	  return error;
	if ((error = rpc_message_send_uint32(message, ws_info->colormap)) < 0)
	  return error;
	if ((error = rpc_message_send_uint32(message, ws_info->depth)) < 0)
	  return error;
  }
  else {
	if ((error = rpc_message_send_uint32(message, 0)) < 0)
	  return error;
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPSetWindowCallbackStruct(rpc_message_t *message, void *p_value)
{
  NPSetWindowCallbackStruct **ws_info_p = (NPSetWindowCallbackStruct **)p_value;
  NPSetWindowCallbackStruct *ws_info;
  int32_t type;
  uint32_t has_ws_info, visual_id, colormap, depth;
  int error;

  if (ws_info_p)
	*ws_info_p = NULL;

  if ((error = rpc_message_recv_uint32(message, &has_ws_info)) < 0)
	return error;

  if (has_ws_info) {
	if ((error = rpc_message_recv_int32(message, &type)) < 0)
	  return error;
	if ((error = rpc_message_recv_uint32(message, &visual_id)) < 0)
	  return error;
	if ((error = rpc_message_recv_uint32(message, &colormap)) < 0)
	  return error;
	if ((error = rpc_message_recv_uint32(message, &depth)) < 0)
	  return error;

	if (ws_info_p) {
	  if ((ws_info = calloc(1, sizeof(*ws_info))) == NULL)
		return RPC_ERROR_NO_MEMORY;
	  ws_info->type = type;
	  // display shall be filled in by the plugin
	  // visual shall be reconstructed by the plugin based on the visualID
	  ws_info->visual = (void *)(uintptr_t)visual_id;
	  ws_info->colormap = colormap;
	  ws_info->depth = depth;
	  *ws_info_p = ws_info;
	}
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_send_NPWindowData(rpc_message_t *message, void *p_value)
{
  NPWindow *window = (NPWindow *)p_value;
  int error;

  if (window == NULL)
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;

  if ((error = rpc_message_send_uint32(message, (Window)window->window)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, window->x)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, window->y)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, window->width)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, window->height)) < 0)
	return error;
  if ((error = do_send_NPRect(message, &window->clipRect)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, window->type)) < 0)
	return error;
  if ((error = do_send_NPSetWindowCallbackStruct(message, window->ws_info)) < 0)
	return 0;

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPWindowData(rpc_message_t *message, void *p_value)
{
  NPWindow *window = (NPWindow *)p_value;
  NPSetWindowCallbackStruct *ws_info;
  uint32_t window_id;
  int32_t window_type;
  int error;

  if (window == NULL)
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;

  if ((error = rpc_message_recv_uint32(message, &window_id)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &window->x)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &window->y)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &window->width)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &window->height)) < 0)
	return error;
  if ((error = do_recv_NPRect(message, &window->clipRect)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &window_type)) < 0)
	return error;
  if ((error = do_recv_NPSetWindowCallbackStruct(message, &ws_info)) < 0)
	return error;
  window->type = window_type;
  window->window = (void *)(Window)window_id;
  window->ws_info = ws_info;

  return RPC_ERROR_NO_ERROR;
}

static int do_send_NPWindow(rpc_message_t *message, void *p_value)
{
  NPWindow *window = (NPWindow *)p_value;
  int error;

  if (window == NULL) {
	if ((error = rpc_message_send_uint32(message, 0)) < 0)
	  return error;
  }
  else {
	if ((error = rpc_message_send_uint32(message, 1)) < 0)
	  return error;
	if ((error = do_send_NPWindowData(message, window)) < 0)
	  return error;
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPWindow(rpc_message_t *message, void *p_value)
{
  NPWindow **window_p = (NPWindow **)p_value;
  NPWindow *window;
  uint32_t window_valid;
  int error;

  if (window_p)
	*window_p = NULL;
  if ((error = rpc_message_recv_uint32(message, &window_valid)) < 0)
	return error;
  if (window_valid) {
	if ((window = malloc(sizeof(NPWindow))) == NULL)
	  return RPC_ERROR_NO_MEMORY;
	if ((error = do_recv_NPWindowData(message, window)) < 0) {
	  free(window);
	  return error;
	}
	if (window_p)
	  *window_p = window;
  }

  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPEvent objects
 */

// XXX: those are not real XEvent generated by the X server, i.e. some
// fields can be optimized out, which is what Firefox does
static bool is_valid_NPEvent_type(NPEvent *event)
{
  switch (event->type) {
  case GraphicsExpose:
  case FocusIn:
  case FocusOut:
  case EnterNotify:
  case LeaveNotify:
  case MotionNotify:
  case ButtonPress:
  case ButtonRelease:
  case KeyPress:
  case KeyRelease:
	return true;
  default:
	break;
  }
  return false;
}

static int do_send_XAnyEvent(rpc_message_t *message, XEvent *xevent)
{
  int error;
  if ((error = rpc_message_send_uint32(message, xevent->xany.serial)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xany.send_event)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xany.window)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

static int do_send_XGraphicsExposeEvent(rpc_message_t *message, XEvent *xevent)
{
  int error;
  if ((error = do_send_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xgraphicsexpose.x)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xgraphicsexpose.y)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xgraphicsexpose.width)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xgraphicsexpose.height)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

static int do_send_XFocusChangeEvent(rpc_message_t *message, XEvent *xevent)
{
  return RPC_ERROR_NO_ERROR;
}

static int do_send_XCrossingEvent(rpc_message_t *message, XEvent *xevent)
{
  int error;
  if ((error = do_send_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xcrossing.root)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xcrossing.subwindow)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xcrossing.time)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.x)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.y)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.x_root)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.y_root)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.mode)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.detail)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.same_screen)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xcrossing.focus)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xcrossing.state)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

static int do_send_XMotionEvent(rpc_message_t *message, XEvent *xevent)
{
  int error;
  if ((error = do_send_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xmotion.root)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xmotion.subwindow)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xmotion.time)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xmotion.x)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xmotion.y)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xmotion.x_root)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xmotion.y_root)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xmotion.state)) < 0)
	return error;
  if ((error = rpc_message_send_char(message, xevent->xmotion.is_hint)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xmotion.same_screen)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

static int do_send_XButtonEvent(rpc_message_t *message, XEvent *xevent)
{
  int error;
  if ((error = do_send_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xbutton.root)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xbutton.subwindow)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xbutton.time)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xbutton.x)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xbutton.y)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xbutton.x_root)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xbutton.y_root)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xbutton.state)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xbutton.button)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xbutton.same_screen)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

static int do_send_XKeyEvent(rpc_message_t *message, XEvent *xevent)
{
  int error;
  if ((error = do_send_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xkey.root)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xkey.subwindow)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xkey.time)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xkey.x)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xkey.y)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xkey.x_root)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xkey.y_root)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xkey.state)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, xevent->xkey.keycode)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, xevent->xkey.same_screen)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

static int do_send_NPEvent(rpc_message_t *message, void *p_value)
{
  NPEvent *event = (NPEvent *)p_value;
  int error;

  if (event == NULL || !is_valid_NPEvent_type(event))
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;

  if ((error = rpc_message_send_int32(message, event->type)) < 0)
	return error;

  switch (event->type) {
  case GraphicsExpose:
	if ((error = do_send_XGraphicsExposeEvent(message, event)) < 0)
	  return error;
	break;
  case FocusIn:
  case FocusOut:
	if ((error = do_send_XFocusChangeEvent(message, event)) < 0)
	  return error;
	break;
  case EnterNotify:
  case LeaveNotify:
	if ((error = do_send_XCrossingEvent(message, event)) < 0)
	  return error;
	break;
  case MotionNotify:
	if ((error = do_send_XMotionEvent(message, event)) < 0)
	  return error;
	break;
  case ButtonPress:
  case ButtonRelease:
	if ((error = do_send_XButtonEvent(message, event)) < 0)
	  return error;
	break;
  case KeyPress:
  case KeyRelease:
	if ((error = do_send_XKeyEvent(message, event)) < 0)
	  return error;
	break;
  default:
	return RPC_ERROR_GENERIC;
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_XAnyEvent(rpc_message_t *message, XEvent *xevent)
{
  uint32_t serial, send_event, window;
  int error;
  if ((error = rpc_message_recv_uint32(message, &serial)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &send_event)) < 0)
	return error;
  // display shall be filled in by the plugin
  if ((error = rpc_message_recv_uint32(message, &window)) < 0)
	return error;
  xevent->xany.serial = serial;
  xevent->xany.send_event = send_event;
  xevent->xany.window = window;
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_XGraphicsExposeEvent(rpc_message_t *message, XEvent *xevent)
{
  int32_t x, y;
  uint32_t width, height;
  int error;
  if ((error = do_recv_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &width)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &height)) < 0)
	return error;
  xevent->xgraphicsexpose.x = x;
  xevent->xgraphicsexpose.y = y;
  xevent->xgraphicsexpose.width = width;
  xevent->xgraphicsexpose.height = height;
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_XFocusChangeEvent(rpc_message_t *message, XEvent *xevent)
{
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_XCrossingEvent(rpc_message_t *message, XEvent *xevent)
{
  int32_t x, y, x_root, y_root, mode, detail, same_screen, focus;
  uint32_t root, subwindow, time, state;
  int error;
  if ((error = do_recv_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &root)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &subwindow)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &time)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x_root)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y_root)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &mode)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &detail)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &same_screen)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &focus)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &state)) < 0)
	return error;
  xevent->xcrossing.root = root;
  xevent->xcrossing.subwindow = subwindow;
  xevent->xcrossing.time = time;
  xevent->xcrossing.x = x;
  xevent->xcrossing.y = y;
  xevent->xcrossing.x_root = x_root;
  xevent->xcrossing.y_root = y_root;
  xevent->xcrossing.mode = mode;
  xevent->xcrossing.detail = detail;
  xevent->xcrossing.same_screen = same_screen;
  xevent->xcrossing.focus = focus;
  xevent->xcrossing.state = state;
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_XMotionEvent(rpc_message_t *message, XEvent *xevent)
{
  char is_hint;
  int32_t x, y, x_root, y_root, same_screen;
  uint32_t root, subwindow, time, state;
  int error;
  if ((error = do_recv_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &root)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &subwindow)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &time)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x_root)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y_root)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &state)) < 0)
	return error;
  if ((error = rpc_message_recv_char(message, &is_hint)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &same_screen)) < 0)
	return error;
  xevent->xmotion.root = root;
  xevent->xmotion.subwindow = subwindow;
  xevent->xmotion.time = time;
  xevent->xmotion.x = x;
  xevent->xmotion.y = y;
  xevent->xmotion.x_root = x_root;
  xevent->xmotion.y_root = y_root;
  xevent->xmotion.state = state;
  xevent->xmotion.is_hint = is_hint;
  xevent->xmotion.same_screen = same_screen;
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_XButtonEvent(rpc_message_t *message, XEvent *xevent)
{
  int32_t x, y, x_root, y_root, same_screen;
  uint32_t root, subwindow, time, state, button;
  int error;
  if ((error = do_recv_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &root)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &subwindow)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &time)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x_root)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y_root)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &state)) < 0)
	return error; 
  if ((error = rpc_message_recv_uint32(message, &button)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &same_screen)) < 0)
	return error;
  xevent->xbutton.root = root;
  xevent->xbutton.subwindow = subwindow;
  xevent->xbutton.time = time;
  xevent->xbutton.x = x;
  xevent->xbutton.y = y;
  xevent->xbutton.x_root = x_root;
  xevent->xbutton.y_root = y_root;
  xevent->xbutton.state = state;
  xevent->xbutton.button = button;
  xevent->xbutton.same_screen = same_screen;
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_XKeyEvent(rpc_message_t *message, XEvent *xevent)
{
  int32_t x, y, x_root, y_root, same_screen;
  uint32_t root, subwindow, time, state, keycode;
  int error;
  if ((error = do_recv_XAnyEvent(message, xevent)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &root)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &subwindow)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &time)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &x_root)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &y_root)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &state)) < 0)
	return error; 
  if ((error = rpc_message_recv_uint32(message, &keycode)) < 0)
	return error;
  if ((error = rpc_message_recv_int32(message, &same_screen)) < 0)
	return error;
  xevent->xkey.root = root;
  xevent->xkey.subwindow = subwindow;
  xevent->xkey.time = time;
  xevent->xkey.x = x;
  xevent->xkey.y = y;
  xevent->xkey.x_root = x_root;
  xevent->xkey.y_root = y_root;
  xevent->xkey.state = state;
  xevent->xkey.keycode = keycode;
  xevent->xkey.same_screen = same_screen;
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPEvent(rpc_message_t *message, void *p_value)
{
  NPEvent *event = (NPEvent *)p_value;
  int32_t event_type;
  int error;

  if ((error = rpc_message_recv_int32(message, &event_type)) < 0)
	return error;
  memset(event, 0, sizeof(*event));
  event->type = event_type;

  switch (event->type) {
  case GraphicsExpose:
	if ((error = do_recv_XGraphicsExposeEvent(message, event)) < 0)
	  return error;
	break;
  case FocusIn:
  case FocusOut:
	if ((error = do_recv_XFocusChangeEvent(message, event)) < 0)
	  return error;
	break;
  case EnterNotify:
  case LeaveNotify:
	if ((error = do_recv_XCrossingEvent(message, event)) < 0)
	  return error;
	break;
  case MotionNotify:
	if ((error = do_recv_XMotionEvent(message, event)) < 0)
	  return error;
	break;
  case ButtonPress:
  case ButtonRelease:
	if ((error = do_recv_XButtonEvent(message, event)) < 0)
	  return error;
	break;
  case KeyPress:
  case KeyRelease:
	if ((error = do_recv_XKeyEvent(message, event)) < 0)
	  return error;
	break;
  default:
	return RPC_ERROR_GENERIC;
  }

  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPFullPrint objects
 */

static int do_send_NPFullPrint(rpc_message_t *message, void *p_value)
{
  NPFullPrint *fullPrint = (NPFullPrint *)p_value;
  int error;

  if ((error = rpc_message_send_uint32(message, fullPrint->pluginPrinted)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, fullPrint->printOne)) < 0)
	return error;

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPFullPrint(rpc_message_t *message, void *p_value)
{
  NPFullPrint *fullPrint = (NPFullPrint *)p_value;
  uint32_t pluginPrinted, printOne;
  int error;

  if ((error = rpc_message_recv_uint32(message, &pluginPrinted)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &printOne)) < 0)
	return error;

  fullPrint->pluginPrinted = pluginPrinted;
  fullPrint->printOne = printOne;
  fullPrint->platformPrint = NULL; // to be filled in by the plugin
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPEmbedPrint objects
 */

static int do_send_NPEmbedPrint(rpc_message_t *message, void *p_value)
{
  NPEmbedPrint *embedPrint = (NPEmbedPrint *)p_value;
  int error;

  if ((error = do_send_NPWindowData(message, &embedPrint->window)) < 0)
	return error;

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPEmbedPrint(rpc_message_t *message, void *p_value)
{
  NPEmbedPrint *embedPrint = (NPEmbedPrint *)p_value;
  int error;

  if ((error = do_recv_NPWindowData(message, &embedPrint->window)) < 0)
	return error;

  embedPrint->platformPrint = NULL; // to be filled in by the plugin
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPPrint objects
 */

static int do_send_NPPrint(rpc_message_t *message, void *p_value)
{
  NPPrint *printInfo = (NPPrint *)p_value;
  int error;

  if ((error = rpc_message_send_uint32(message, printInfo->mode)) < 0)
	return error;
  switch (printInfo->mode) {
  case NP_FULL:
	if ((error = do_send_NPFullPrint(message, &printInfo->print.fullPrint)) < 0)
	  return error;
	break;
  case NP_EMBED:
	if ((error = do_send_NPEmbedPrint(message, &printInfo->print.embedPrint)) < 0)
	  return error;
	break;
  default:
	return RPC_ERROR_GENERIC;
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPPrint(rpc_message_t *message, void *p_value)
{
  NPPrint *printInfo = (NPPrint *)p_value;
  uint32_t print_mode;
  int error;

  if ((error = rpc_message_recv_uint32(message, &print_mode)) < 0)
	return error;
  switch (print_mode) {
  case NP_FULL:
	if ((error = do_recv_NPFullPrint(message, &printInfo->print.fullPrint)) < 0)
	  return error;
	break;
  case NP_EMBED:
	if ((error = do_recv_NPEmbedPrint(message, &printInfo->print.embedPrint)) < 0)
	  return error;
	break;
  default:
	return RPC_ERROR_GENERIC;
  }

  printInfo->mode = print_mode;
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPPrintData objects
 */

static int do_send_NPPrintData(rpc_message_t *message, void *p_value)
{
  NPPrintData *printData = (NPPrintData *)p_value;
  int error;

  if ((error = rpc_message_send_uint32(message, printData->size)) < 0)
	return error;
  if ((error = rpc_message_send_bytes(message, printData->data, printData->size)) < 0)
	return error;

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPPrintData(rpc_message_t *message, void *p_value)
{
  NPPrintData *printData = (NPPrintData *)p_value;
  int error;

  if ((error = rpc_message_recv_uint32(message, &printData->size)) < 0)
	return error;
  if ((error = rpc_message_recv_bytes(message, printData->data, printData->size)) < 0)
	return error;

  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPObject objects
 */

enum NPObjectType {
  NPOBJECT_NULL = 0,
  NPOBJECT_SENDER_OWNED,
  NPOBJECT_RECEIVER_OWNED
};

static int do_send_NPObject_helper(rpc_message_t *message, void *p_value,
								   bool pass_ref)
{
  NPObject *npobj = (NPObject *)p_value;

  int error;
  uint32_t type = NPOBJECT_NULL;
  NPW_PluginInstance *plugin = NULL;
  uint32_t npobj_id = 0;
  bool release_stub = false;

  if (npobj) {
	npobj_id = npobject_get_proxy_id(npobj);
	if (npobj_id == 0) {
	  // Sending an object on our side. Allocate a stub so the other
	  // side can make a proxy.
	  type = NPOBJECT_SENDER_OWNED;
	  npobj_id = npobject_create_stub(npobj);
	  if (pass_ref) {
		// Release our reference; the stub protects the object from
		// deallocation.
		NPN_ReleaseObject(npobj);
	  }
#if NPW_IS_PLUGIN
	  // Get the owning NPP so we can figure out who the owner is.
	  plugin = npobject_get_owner(npobj);
#endif
	} else {
	  // This is a proxy for the object on the other side. Just pass
	  // the id along.
	  type = NPOBJECT_RECEIVER_OWNED;
	  if (pass_ref) {
		// As above, we release out reference. If this is not the last
		// reference, we may just do so. Otherwise, we destroy it, but
		// do /not/ send the corresponding RPC. That gets merged into
		// this one.
		if (npobj->referenceCount == 1) {
		  npobject_destroy_proxy(npobj, false);
		  // Tell the other side to destroy the stub after taking a ref.
		  release_stub = true;
		} else {
		  NPN_ReleaseObject(npobj);
		}
	  }
	}
	assert(npobj_id != 0);
  }

  // This could be significantly trimmed down, but it really doesn't
  // matter. Latency, not bandwidth, is what we care about.
  if ((error = rpc_message_send_uint32(message, type)) < 0)
	return error;
  if ((error = do_send_NPW_PluginInstance(message, plugin)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, npobj_id)) < 0)
	return error;
  if (pass_ref) {
	if ((error = rpc_message_send_uint32(message, release_stub)) < 0)
	  return error;
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPObject_helper(rpc_message_t *message, void *p_value,
								   bool pass_ref)
{
  int error;
  uint32_t type = NPOBJECT_NULL;
  uint32_t npobj_id = 0;
  NPW_PluginInstance *plugin = NULL;
  uint32_t release_stub = 0;

  if ((error = rpc_message_recv_uint32(message, &type)) < 0)
	return error;
  if ((error = do_recv_NPW_PluginInstance(message, &plugin)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &npobj_id)) < 0)
	return error;
  if (pass_ref) {
	if ((error = rpc_message_recv_uint32(message, &release_stub)) < 0)
	  return error;
  }

  NPObject *npobj = NULL;
  if (type == NPOBJECT_NULL) {
	// Do nothing.
  } else if (type == NPOBJECT_SENDER_OWNED) {
	// We got a newly-created remote stub. Create a matching proxy.
	npobj = npobject_create_proxy(NPW_PLUGIN_INSTANCE_NPP(plugin), npobj_id);
	if (release_stub) {
	  npw_printf("ERROR: received release_stub for proxy NPObject.\n");
	  return RPC_ERROR_GENERIC;
	}
  } else if (type == NPOBJECT_RECEIVER_OWNED) {
	npobj = npobject_lookup_local(npobj_id);
	assert(npobj != NULL);
	// This is an object on our side. Retain it; receiver must
	// release all received NPObjects.
	NPN_RetainObject(npobj);
	if (release_stub) {
	  // We just retained the object, so it won't be destroyed.
	  npobject_destroy_stub(npobj_id);
	}
  } else {
	npw_printf("ERROR: unknown NPObject type %d\n", type);
	return RPC_ERROR_GENERIC;
  }

  *((NPObject **)p_value) = npobj;
  return RPC_ERROR_NO_ERROR;
}

static int do_send_NPObject(rpc_message_t *message, void *p_value)
{
  return do_send_NPObject_helper(message, p_value, false);
}

static int do_recv_NPObject(rpc_message_t *message, void *p_value)
{
  return do_recv_NPObject_helper(message, p_value, false);
}

static int do_send_NPObject_pass_ref(rpc_message_t *message, void *p_value)
{
  return do_send_NPObject_helper(message, p_value, true);
}

static int do_recv_NPObject_pass_ref(rpc_message_t *message, void *p_value)
{
  return do_recv_NPObject_helper(message, p_value, true);
}


/*
 *  Process NPIdentifier objects
 */

// Rationale: NPIdentifiers are allocated on the browser side
// only. IDs are passed through to the viewer, and they have no
// meaning on that side as they are only used to get passed back to
// the browser side
static int do_send_NPIdentifier(rpc_message_t *message, void *p_value)
{
  NPIdentifier ident = *(NPIdentifier *)p_value;
  int id = 0;
  if (ident) {
#ifdef BUILD_WRAPPER
	id = id_lookup_value(ident);
	if (id < 0)
	  id = id_create(ident);
#endif
#ifdef BUILD_VIEWER
	id = (uintptr_t)ident;
#endif
	assert(id != 0);
  }
  return rpc_message_send_uint32(message, id);
}

static int do_recv_NPIdentifier(rpc_message_t *message, void *p_value)
{
  int error;
  uint32_t id;

  if ((error = rpc_message_recv_uint32(message, &id)) < 0)
	return error;

  NPIdentifier ident = NULL;
  if (id) {
#ifdef BUILD_WRAPPER
	ident = id_lookup(id);
#endif
#ifdef BUILD_VIEWER
	ident = (void *)(uintptr_t)id;
#endif
	assert(ident != NULL);
  }
  *((NPIdentifier *)p_value) = ident;
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPUTF8 strings
 */

static int do_send_NPUTF8(rpc_message_t *message, void *p_value)
{
  NPUTF8 *string = (NPUTF8 *)p_value;
  if (string == NULL)
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;

  int len = strlen(string) + 1;
  int error = rpc_message_send_uint32(message, len);
  if (error < 0)
	return error;
  if (len > 0)
	return rpc_message_send_bytes(message, (unsigned char *)string, len);

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPUTF8(rpc_message_t *message, void *p_value)
{
  NPUTF8 **string_p = (NPUTF8 **)p_value;
  NPUTF8 *string = NULL;

  uint32_t len;
  int error = rpc_message_recv_uint32(message, &len);
  if (error < 0)
	return error;
  if ((string = NPN_MemAlloc(len)) == NULL)
	return RPC_ERROR_NO_MEMORY;
  if (len > 0) {
	if ((error = rpc_message_recv_bytes(message, (unsigned char *)string, len)) < 0)
	  return error;
  }

  if (string_p)
	*string_p = string;
  else if (string)
	NPN_MemFree(string);
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPString objects
 */

static int do_send_NPString(rpc_message_t *message, void *p_value)
{
  NPString *string = (NPString *)p_value;
  if (string == NULL)
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;

  int error = rpc_message_send_uint32(message, string->UTF8Length);
  if (error < 0)
	return error;
  if (string->UTF8Length && string->UTF8Characters)
	return rpc_message_send_bytes(message, (unsigned char *)string->UTF8Characters, string->UTF8Length);
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPString(rpc_message_t *message, void *p_value)
{
  NPString *string = (NPString *)p_value;
  if (string == NULL)
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;
  string->UTF8Length = 0;
  string->UTF8Characters = NULL;

  int error = rpc_message_recv_uint32(message, &string->UTF8Length);
  if (error < 0)
	return error;

  if ((string->UTF8Characters = NPN_MemAlloc(string->UTF8Length + 1)) == NULL)
	return RPC_ERROR_NO_MEMORY;
  if (string->UTF8Length > 0) {
	if ((error = rpc_message_recv_bytes(message, (unsigned char *)string->UTF8Characters, string->UTF8Length)) < 0)
	  return error;
  }
  ((char *)string->UTF8Characters)[string->UTF8Length] = '\0';
  
  return RPC_ERROR_NO_ERROR;
}


/*
 *  Process NPVariant objects
 */

static int do_send_NPVariant_helper(rpc_message_t *message, void *p_value,
									bool pass_ref)
{
  NPVariant *variant = (NPVariant *)p_value;
  if (variant == NULL)
	return RPC_ERROR_MESSAGE_ARGUMENT_INVALID;

  int error = rpc_message_send_uint32(message, variant->type);
  if (error < 0)
	return error;

  switch (variant->type) {
  case NPVariantType_Void:
  case NPVariantType_Null:
	// nothing to do (initialized in receiver)
	break;
  case NPVariantType_Bool:
	if ((error = rpc_message_send_uint32(message, variant->value.boolValue)) < 0)
	  return error;
	break;
  case NPVariantType_Int32:
	if ((error = rpc_message_send_int32(message, variant->value.intValue)) < 0)
	  return error;
	break;
  case NPVariantType_Double:
	if ((error = rpc_message_send_double(message, variant->value.doubleValue)) < 0)
	  return error;
	break;
  case NPVariantType_String:
	if ((error = do_send_NPString(message, &variant->value.stringValue)) < 0)
	  return error;
	break;
  case NPVariantType_Object:
	if ((error = do_send_NPObject_helper(message, variant->value.objectValue,
										 pass_ref)) < 0)
	  return error;
	break;
  }

  // Clean up local data. If we had an NPObject,
  // do_send_NPObject_pass_ref took care of it.
  if (pass_ref && variant->type != NPVariantType_Object) {
	NPN_ReleaseVariantValue(variant);
  }

  return RPC_ERROR_NO_ERROR;
}

static int do_recv_NPVariant_helper(rpc_message_t *message, void *p_value,
									bool pass_ref)
{
  NPVariant *variant = (NPVariant *)p_value;
  if (variant)
	VOID_TO_NPVARIANT(*variant);

  uint32_t type;
  int error = rpc_message_recv_uint32(message, &type);
  if (error < 0)
	return error;

  NPVariant result;
  VOID_TO_NPVARIANT(result);
  switch (type) {
  case NPVariantType_Void:
	VOID_TO_NPVARIANT(result);
	break;
  case NPVariantType_Null:
	NULL_TO_NPVARIANT(result);
	break;
  case NPVariantType_Bool: {
	uint32_t value;
	if ((error = rpc_message_recv_uint32(message, &value)) < 0)
	  return error;
	result.value.boolValue = value;
	break;
  }
  case NPVariantType_Int32:
	if ((error = rpc_message_recv_int32(message, &result.value.intValue)) < 0)
	  return error;
	break;
  case NPVariantType_Double:
	if ((error = rpc_message_recv_double(message, &result.value.doubleValue)) < 0)
	  return error;
	break;
  case NPVariantType_String:
	if ((error = do_recv_NPString(message, &result.value.stringValue)) < 0)
	  return error;
	break;
  case NPVariantType_Object:
	if ((error = do_recv_NPObject_helper(message, &result.value.objectValue,
										 pass_ref)) < 0)
	  return error;
	// NPVariant owns reference from do_recv_NPObject_helper.
	break;
  }

  if (variant) {
	*variant = result;
	variant->type = type;
  }
  
  return RPC_ERROR_NO_ERROR;
}

static int do_send_NPVariant(rpc_message_t *message, void *p_value)
{
  return do_send_NPVariant_helper(message, p_value, false);
}

static int do_recv_NPVariant(rpc_message_t *message, void *p_value)
{
  return do_recv_NPVariant_helper(message, p_value, false);
}

static int do_send_NPVariant_pass_ref(rpc_message_t *message, void *p_value)
{
  return do_send_NPVariant_helper(message, p_value, true);
}

static int do_recv_NPVariant_pass_ref(rpc_message_t *message, void *p_value)
{
  return do_recv_NPVariant_helper(message, p_value, true);
}

/*
 *  Initialize marshalers for NPAPI types
 */

static const rpc_message_descriptor_t message_descs[] = {
  {
	RPC_TYPE_NPP,
	sizeof(NPP),
	do_send_NPP,
	do_recv_NPP
  },
  {
	RPC_TYPE_NPW_PLUGIN_INSTANCE,
	sizeof(NPW_PluginInstance *),
	do_send_NPW_PluginInstance,
	do_recv_NPW_PluginInstance
  },
  {
	RPC_TYPE_NP_STREAM,
	sizeof(NPStream *),
	do_send_NPStream,
	do_recv_NPStream
  },
  {
	RPC_TYPE_NP_BYTE_RANGE,
	sizeof(NPByteRange *),
	do_send_NPByteRange,
	do_recv_NPByteRange
  },
  {
	RPC_TYPE_NP_SAVED_DATA,
	sizeof(NPSavedData *),
	do_send_NPSavedData,
	do_recv_NPSavedData
  },
  {
	RPC_TYPE_NP_NOTIFY_DATA,
	sizeof(void *),
	do_send_NotifyData,
	do_recv_NotifyData
  },
  {
	RPC_TYPE_NP_RECT,
	sizeof(NPRect),
	do_send_NPRect,
	do_recv_NPRect
  },
  {
	RPC_TYPE_NP_WINDOW,
	sizeof(NPWindow *),
	do_send_NPWindow,
	do_recv_NPWindow
  },
  {
	RPC_TYPE_NP_EVENT,
	sizeof(NPEvent),
	do_send_NPEvent,
	do_recv_NPEvent
  },
  {
	RPC_TYPE_NP_PRINT,
	sizeof(NPPrint),
	do_send_NPPrint,
	do_recv_NPPrint
  },
  {
	RPC_TYPE_NP_FULL_PRINT,
	sizeof(NPFullPrint),
	do_send_NPFullPrint,
	do_recv_NPFullPrint
  },
  {
	RPC_TYPE_NP_EMBED_PRINT,
	sizeof(NPEmbedPrint),
	do_send_NPEmbedPrint,
	do_recv_NPEmbedPrint
  },
  {
	RPC_TYPE_NP_PRINT_DATA,
	sizeof(NPPrintData),
	do_send_NPPrintData,
	do_recv_NPPrintData
  },
  {
	RPC_TYPE_NP_OBJECT,
	sizeof(NPObject *),
	do_send_NPObject,
	do_recv_NPObject
  },
  {
	RPC_TYPE_NP_OBJECT_PASS_REF,
	sizeof(NPObject *),
	do_send_NPObject_pass_ref,
	do_recv_NPObject_pass_ref
  },
  {
	RPC_TYPE_NP_IDENTIFIER,
	sizeof(NPIdentifier),
	do_send_NPIdentifier,
	do_recv_NPIdentifier
  },
  {
	RPC_TYPE_NP_UTF8,
	sizeof(NPUTF8 *),
	do_send_NPUTF8,
	do_recv_NPUTF8
  },
  {
	RPC_TYPE_NP_STRING,
	sizeof(NPString),
	do_send_NPString,
	do_recv_NPString
  },
  {
	RPC_TYPE_NP_VARIANT,
	sizeof(NPVariant),
	do_send_NPVariant,
	do_recv_NPVariant
  },
  {
	RPC_TYPE_NP_VARIANT_PASS_REF,
	sizeof(NPVariant),
	do_send_NPVariant_pass_ref,
	do_recv_NPVariant_pass_ref
  }
};

int rpc_add_np_marshalers(rpc_connection_t *connection)
{
  return rpc_connection_add_message_descriptors(connection, message_descs, sizeof(message_descs) / sizeof(message_descs[0]));
}
