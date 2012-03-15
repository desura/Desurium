/*
 *  browser-funcs.h - Browser functions
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

BROWSER_FUNC(NPN_GetURL, geturl)
BROWSER_FUNC(NPN_PostURL, posturl)
BROWSER_FUNC(NPN_RequestRead, requestread)
BROWSER_FUNC(NPN_NewStream, newstream)
BROWSER_FUNC(NPN_Write, write)
BROWSER_FUNC(NPN_DestroyStream, destroystream)
BROWSER_FUNC(NPN_Status, status)
BROWSER_FUNC(NPN_UserAgent, uagent)
BROWSER_FUNC(NPN_MemAlloc, memalloc)
BROWSER_FUNC(NPN_MemFree, memfree)
BROWSER_FUNC(NPN_MemFlush, memflush)
BROWSER_FUNC(NPN_ReloadPlugins, reloadplugins)
BROWSER_FUNC(NPN_GetJavaEnv, getJavaEnv)
BROWSER_FUNC(NPN_GetJavaPeer, getJavaPeer)
BROWSER_FUNC(NPN_GetURLNotify, geturlnotify)
BROWSER_FUNC(NPN_PostURLNotify, posturlnotify)
BROWSER_FUNC(NPN_GetValue, getvalue)
BROWSER_FUNC(NPN_SetValue, setvalue)
BROWSER_FUNC(NPN_InvalidateRect, invalidaterect)
BROWSER_FUNC(NPN_InvalidateRegion, invalidateregion)
BROWSER_FUNC(NPN_ForceRedraw, forceredraw)
BROWSER_FUNC(NPN_GetStringIdentifier, getstringidentifier)
BROWSER_FUNC(NPN_GetStringIdentifiers, getstringidentifiers)
BROWSER_FUNC(NPN_GetIntIdentifier, getintidentifier)
BROWSER_FUNC(NPN_IdentifierIsString, identifierisstring)
BROWSER_FUNC(NPN_UTF8FromIdentifier, utf8fromidentifier)
BROWSER_FUNC(NPN_IntFromIdentifier, intfromidentifier)
BROWSER_FUNC(NPN_CreateObject, createobject)
BROWSER_FUNC(NPN_RetainObject, retainobject)
BROWSER_FUNC(NPN_ReleaseObject, releaseobject)
BROWSER_FUNC(NPN_Invoke, invoke)
BROWSER_FUNC(NPN_InvokeDefault, invokeDefault)
BROWSER_FUNC(NPN_Evaluate, evaluate)
BROWSER_FUNC(NPN_GetProperty, getproperty)
BROWSER_FUNC(NPN_SetProperty, setproperty)
BROWSER_FUNC(NPN_RemoveProperty, removeproperty)
BROWSER_FUNC(NPN_HasProperty, hasproperty)
BROWSER_FUNC(NPN_HasMethod, hasmethod)
BROWSER_FUNC(NPN_ReleaseVariantValue, releasevariantvalue)
BROWSER_FUNC(NPN_SetException, setexception)
BROWSER_FUNC(NPN_PushPopupsEnabledState, pushpopupsenabledstate)
BROWSER_FUNC(NPN_PopPopupsEnabledState, poppopupsenabledstate)
BROWSER_FUNC(NPN_Enumerate, enumerate)
BROWSER_FUNC(NPN_PluginThreadAsyncCall, pluginthreadasynccall)
BROWSER_FUNC(NPN_Construct, construct)
BROWSER_FUNC(NPN_GetValueForURL, getvalueforurl)
BROWSER_FUNC(NPN_SetValueForURL, setvalueforurl)
BROWSER_FUNC(NPN_GetAuthenticationInfo, getauthenticationinfo)
BROWSER_FUNC(NPN_ScheduleTimer, scheduletimer)
BROWSER_FUNC(NPN_UnscheduleTimer, unscheduletimer)
// BROWSER_FUNC(NPN_PopUpContextMenu, popupcontextmenu)
// BROWSER_FUNC(NPN_ConvertPoint, convertpoint)
// BROWSER_FUNC(NPN_HandleEvent, handleevent)
// BROWSER_FUNC(NPN_UnfocusInstance, unfocusinstance)
// BROWSER_FUNC(NPN_URLRedirectResponse, urlredirectresponse)
