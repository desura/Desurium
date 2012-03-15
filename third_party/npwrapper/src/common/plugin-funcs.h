/*
 *  plugin-funcs.h - Plugin functions
 *
 *  nspluginwrapper (C) 2011 David Benjamin
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

PLUGIN_FUNC(NPP_New, newp)
PLUGIN_FUNC(NPP_Destroy, destroy)
PLUGIN_FUNC(NPP_SetWindow, setwindow)
PLUGIN_FUNC(NPP_NewStream, newstream)
PLUGIN_FUNC(NPP_DestroyStream, destroystream)
PLUGIN_FUNC(NPP_StreamAsFile, asfile)
PLUGIN_FUNC(NPP_WriteReady, writeready)
PLUGIN_FUNC(NPP_Write, write)
PLUGIN_FUNC(NPP_Print, print)
PLUGIN_FUNC(NPP_HandleEvent, event)
PLUGIN_FUNC(NPP_URLNotify, urlnotify)
PLUGIN_FUNC(NPP_GetValue, getvalue)
PLUGIN_FUNC(NPP_SetValue, setvalue)
// PLUGIN_FUNC(NPP_GotFocus, gotfocus)
// PLUGIN_FUNC(NPP_LostFocus, lostfocus)
// PLUGIN_FUNC(NPP_URLRedirectNotify, urlredirectnotify)
PLUGIN_FUNC(NPP_ClearSiteData, clearsitedata)
PLUGIN_FUNC(NPP_GetSitesWithData, getsiteswithdata)
