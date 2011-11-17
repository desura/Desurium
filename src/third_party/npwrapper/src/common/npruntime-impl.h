/*
 *  npruntime.c - Scripting plugins support
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

#ifndef NPRUNTIME_IMPL_H
#define NPRUNTIME_IMPL_H

// npruntime bridge system inspired by Chromium's proxy/stub setup.

// Init and shutdown of the NPObject bridge system.
extern bool npobject_bridge_new(void) attribute_hidden;
extern void npobject_bridge_destroy(void) attribute_hidden;

extern int npclass_add_method_descriptors(rpc_connection_t *connection) attribute_hidden;

// Management of stubs, objects which live on the side that owns the
// NPObject and holds a reference to it on behalf of a proxy.
extern uint32_t npobject_create_stub(NPObject *npobj) attribute_hidden;
extern void npobject_destroy_stub(uint32_t id) attribute_hidden;
extern NPObject *npobject_lookup_local(uint32_t id) attribute_hidden;

// Create a proxy object. The received id must correspond to a live
// stub in the other process. Deallocating this object releases its
// corresponding stub. Holds a reference to the other NPObject on via
// its stub.
extern NPObject *npobject_create_proxy(NPP npp, uint32_t id) attribute_hidden;
extern bool npobject_is_proxy(NPObject *npobj) attribute_hidden;
extern uint32_t npobject_get_proxy_id(NPObject *npobj) attribute_hidden;
extern void npobject_destroy_proxy(NPObject *npobj, bool release_stub) attribute_hidden;

#if NPW_IS_PLUGIN
// Firefox requires that NPN_CreateObject be called with a real NPP
// pointer, so we keep track of the ownership of NPObjects. This
// mapping also doubles as a way of tracking if they've been invalidated.
extern void npobject_register(NPObject *npobj, void *plugin) attribute_hidden;
extern bool npobject_is_registered(NPObject *npobj) attribute_hidden;
extern void *npobject_get_owner(NPObject *npobj) attribute_hidden;
extern void npobject_unregister(NPObject *npobj) attribute_hidden;
#endif

struct _NPVariant;
extern void npvariant_clear(struct _NPVariant *variant) attribute_hidden;
extern char *string_of_NPVariant(const struct _NPVariant *arg) attribute_hidden;
extern void print_npvariant_args(const struct _NPVariant *args, uint32_t nargs) attribute_hidden;

#if NPW_IS_BROWSER
// Deactivate all NPObject instances
extern void npruntime_deactivate(void) attribute_hidden;
#endif

// Check whether to use NPRuntime data caching
// (on by default, disabled with NPW_NPRUNTIME_CACHE=0|no)
extern bool npruntime_use_cache(void) attribute_hidden;

#endif /* NPRUNTIME_IMPL_H */
