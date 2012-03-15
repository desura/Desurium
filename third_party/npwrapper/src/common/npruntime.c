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

#include "sysdeps.h"

#include <assert.h>
#include <glib.h> /* <glib/ghash.h> */
#include "utils.h"
#include "npw-common.h"
#include "npw-malloc.h"

#define DEBUG 1
#include "debug.h"


// Defined in npw-{wrapper,viewer}.c
extern rpc_connection_t *g_rpc_connection attribute_hidden;

// Defined in npw-viewer.c
#if defined(ENABLE_THREAD_CHECK) && NPW_IS_PLUGIN
extern bool thread_check(void);
#else
#define thread_check() true
#endif


/* ====================================================================== */
/* === Helpers                                                        === */
/* ====================================================================== */

static inline bool get_use_npruntime_cache_env(void)
{
  const gchar *env = getenv("NPW_NPRUNTIME_CACHE");
  return env == NULL || (strcmp(env, "no") != 0 && strcmp(env, "0") != 0);
}

bool npruntime_use_cache(void)
{
  static int use_cache = -1;
  if (G_UNLIKELY(use_cache < 0))
	use_cache = get_use_npruntime_cache_env();
  return use_cache;
}

/* ====================================================================== */
/* === NPObject stubs                                                 === */
/* ====================================================================== */

static GHashTable *g_stubs = NULL;  // uint32_t  -> (NPObjectStub *)

typedef struct _NPObjectStub {
  NPObject *npobject;
  uint32_t id;
} NPObjectStub;

uint32_t npobject_create_stub(NPObject *npobj)
{
  npw_return_val_if_fail(npobj != NULL, 0);

  static uint32_t next_id = 0;

  // Allocate an id. Client and server get separate id spaces because
  // of type field.
  uint32_t id = ++next_id;

  D(bug("npobject_create_stub: npobj=%p, id=0x%x\n", npobj, id));
  NPObjectStub *stub = g_new0(NPObjectStub, 1);
  stub->npobject = NPN_RetainObject(npobj);
  stub->id = id;
  g_hash_table_insert(g_stubs, GINT_TO_POINTER(stub->id), stub);

  return stub->id;
}

static NPObjectStub *npobject_lookup_stub(uint32_t id)
{
  return g_hash_table_lookup(g_stubs, GINT_TO_POINTER(id));
}

NPObject *npobject_lookup_local(uint32_t id)
{
  NPObjectStub *stub = npobject_lookup_stub(id);
  return stub ? stub->npobject : NULL;
}

static void npobject_destroy_stub_obj(NPObjectStub *stub)
{
  D(bugiI("npobject_destroy_stub: id=0x%x\n", stub->id));
  g_hash_table_remove(g_stubs, GINT_TO_POINTER(stub->id));
  NPN_ReleaseObject(stub->npobject);
  g_free(stub);
  D(bugiD("npobject_destroy_stub done\n"));
}

void npobject_destroy_stub(uint32_t id)
{
  NPObjectStub *stub = npobject_lookup_stub(id);
  assert(stub != NULL);
  npobject_destroy_stub_obj(stub);
}

/* ====================================================================== */
/* === NPObject proxies                                               === */
/* ====================================================================== */

static GHashTable *g_proxies = NULL;  // uint32_t  -> (NPObjectProxy *)

static NPClass npclass_bridge;

typedef struct _NPObjectProxy {
  NPObject parent;
  uint32_t id;
  bool is_valid;
} NPObjectProxy;

static NPObjectProxy *npobject_get_proxy(NPObject *npobj)
{
  if (npobj->_class != &npclass_bridge)
	return NULL;
  return (NPObjectProxy *)npobj;
}

NPObject *npobject_create_proxy(NPP instance, uint32_t id)
{
  D(bugiI("npobject_create_proxy: NPP=%p, id=0x%x\n", instance, id));

  NPObject *object = NPN_CreateObject(instance, &npclass_bridge);
  NPObjectProxy *proxy = npobject_get_proxy(object);
  proxy->id = id;
  proxy->is_valid = true;
  // There isn't a huge need to track them by id. Any, really. But it
  // does let us invalidate them all.
  g_hash_table_insert(g_proxies, GINT_TO_POINTER(id), proxy);
  D(bugiD("npobject_create_proxy done: obj=%p\n", object));
  return object;
}

bool npobject_is_proxy(NPObject *npobj)
{
  return npobject_get_proxy(npobj) != NULL;
}

uint32_t npobject_get_proxy_id(NPObject *npobj)
{
  NPObjectProxy *proxy = npobject_get_proxy(npobj);
  if (proxy == NULL)
	return 0;
  return proxy->id;
}

static void npclass_invoke_Deallocate(NPObjectProxy *proxy);

void npobject_destroy_proxy(NPObject *npobj, bool release_stub)
{
  // Unregister the proxy.
  D(bugiI("npobject_destroy_proxy: npobj=%p, release_stub=%d\n",
		  npobj, release_stub));
  NPObjectProxy *proxy = npobject_get_proxy(npobj);
  assert(proxy != NULL);
  if (release_stub && proxy->is_valid) {
	npclass_invoke_Deallocate(proxy);
  }
  g_hash_table_remove(g_proxies, GINT_TO_POINTER(proxy->id));
  free(npobj);
  D(bugiD("npobject_destroy_proxy done\n"));
}

static inline bool is_valid_npobject_proxy(NPObject *npobj)
{
  if (npobj == NULL)
	return false;
  NPObjectProxy *proxy = npobject_get_proxy(npobj);
  if (proxy == NULL)
	return false;
  if (!proxy->is_valid)
	npw_printf("ERROR: NPObject proxy %p is no longer valid!\n", npobj);
  return proxy->is_valid;
}

/* ====================================================================== */
/* === NPClass Bridge                                                 === */
/* ====================================================================== */

static NPObject *g_NPClass_Allocate(NPP npp, NPClass *aclass);
static void g_NPClass_Deallocate(NPObject *npobj);
static void g_NPClass_Invalidate(NPObject *npobj);
static bool g_NPClass_HasMethod(NPObject *npobj, NPIdentifier name);
static bool g_NPClass_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
static bool g_NPClass_InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);
static bool g_NPClass_HasProperty(NPObject *npobj, NPIdentifier name);
static bool g_NPClass_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result);
static bool g_NPClass_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value);
static bool g_NPClass_RemoveProperty(NPObject *npobj, NPIdentifier name);
static bool g_NPClass_Enumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count);
static bool g_NPClass_Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);

static NPClass npclass_bridge = {
  NPW_NP_CLASS_STRUCT_VERSION,
  g_NPClass_Allocate,
  g_NPClass_Deallocate,
  g_NPClass_Invalidate,
  g_NPClass_HasMethod,
  g_NPClass_Invoke,
  g_NPClass_InvokeDefault,
  g_NPClass_HasProperty,
  g_NPClass_GetProperty,
  g_NPClass_SetProperty,
  g_NPClass_RemoveProperty,
  g_NPClass_Enumerate,
  g_NPClass_Construct
};

static inline bool is_valid_npobject_class(NPObject *npobj)
{
  if (npobj == NULL || npobj->_class == NULL)
	return false;
#if NPW_IS_PLUGIN
  // Reject requests on invalidated objects.
  if (!npobject_is_registered(npobj)) {
	npw_printf("ERROR: accessed invalidated NPObject\n");
	return false;
  }
#endif
  return true;
}

// NPClass::Allocate
NPObject *g_NPClass_Allocate(NPP npp, NPClass *aclass)
{
  return malloc(sizeof(NPObjectProxy));
}

// NPClass::Deallocate
static int npclass_handle_Deallocate(rpc_connection_t *connection)
{
  D(bug("npclass_handle_Deallocate\n"));

  uint32_t id;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_UINT32, &id,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Deallocate() get args", error);
	return error;
  }

  D(bugiI("NPClass:Deallocate: id=0x%x\n", id));
  NPObjectStub *stub = npobject_lookup_stub(id);
  if (stub != NULL) {
	npobject_destroy_stub_obj(stub);
  }
  D(bugiD("NPClass:Deallocate done\n"));

  return rpc_method_send_reply(connection, RPC_TYPE_INVALID);
}

static void npclass_invoke_Deallocate(NPObjectProxy *proxy)
{
  npw_return_if_fail(rpc_method_invoke_possible(g_rpc_connection));

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_DEALLOCATE,
								RPC_TYPE_UINT32, proxy->id,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Deallocate() invoke", error);
	return;
  }

  // FIXME: This really could be asynchronous...
  error = rpc_method_wait_for_reply(g_rpc_connection, RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Deallocate() wait for reply", error);
	return;
  }
}

void g_NPClass_Deallocate(NPObject *npobj)
{
  npobject_destroy_proxy(npobj, true);
}

void g_NPClass_Invalidate(NPObject *npobj)
{
  if (!is_valid_npobject_proxy(npobj))
	return;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::Invalidate not called from the main thread\n");
	return;
  }

  D(bugiI("NPClass::Invalidate(npobj %p)\n", npobj));
  // Just invalidate the proxy itself. There may be multiple proxies
  // for a plugin-side NPObject. We'll invalidate them viewer-side.
  NPObjectProxy *proxy = npobject_get_proxy(npobj);
  proxy->is_valid = false;
  // Release the underlying stub now, otherwise the is_valid check
  // will refuse to do so. The stub is of no use to us now.
  npclass_invoke_Deallocate(proxy);
  D(bugiD("NPClass::Invalidate done\n"));
}

// NPClass::HasMethod
static int npclass_handle_HasMethod(rpc_connection_t *connection)
{
  D(bug("npclass_handle_HasMethod\n"));

  NPObject *npobj;
  NPIdentifier name;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &name,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::HasMethod() get args", error);
	return error;
  }

  uint32_t ret = false;
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->hasMethod) {
	D(bugiI("NPClass::HasMethod(npobj %p, name id %p)\n", npobj, name));
	ret = npobj->_class->hasMethod(npobj, name);
	D(bugiD("NPClass::HasMethod return: %d\n", ret));
  }

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

static bool npclass_invoke_HasMethod(NPObject *npobj, NPIdentifier name)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_HAS_METHOD,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_NP_IDENTIFIER, &name,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::HasMethod() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection, RPC_TYPE_UINT32, &ret, RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::HasMethod() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_HasMethod(NPObject *npobj, NPIdentifier name)
{
  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::HasMethod not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::HasMethod(npobj %p, name id %p)\n", npobj, name));
  bool ret = npclass_invoke_HasMethod(npobj, name);
  D(bugiD("NPClass::HasMethod return: %d\n", ret));
  return ret;
}

// NPClass::Invoke
static int npclass_handle_Invoke(rpc_connection_t *connection)
{
  D(bug("npclass_handle_Invoke\n"));

  NPObject *npobj;
  NPIdentifier name;
  uint32_t argCount;
  NPVariant *args;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &name,
								  RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, &argCount, &args,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Invoke() get args", error);
	return error;
  }

  uint32_t ret = false;
  NPVariant result;
  VOID_TO_NPVARIANT(result);
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->invoke) {
	D(bugiI("NPClass::Invoke(npobj %p, name id %p)\n", npobj, name));
	print_npvariant_args(args, argCount);
	ret = npobj->_class->invoke(npobj, name, args, argCount, &result);
	gchar *result_str = string_of_NPVariant(&result);
	D(bugiD("NPClass::Invoke return: %d (%s)\n", ret, result_str));
	g_free(result_str);
  }

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

static bool npclass_invoke_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount,
								  NPVariant *result)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_INVOKE,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_NP_IDENTIFIER, &name,
								RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, argCount, args,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Invoke() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_UINT32, &ret,
									RPC_TYPE_NP_VARIANT_PASS_REF, result,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Invoke() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount,
					  NPVariant *result)
{
  if (result == NULL)
	return false;
  VOID_TO_NPVARIANT(*result);

  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::Invoke not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::Invoke(npobj %p, name id %p)\n", npobj, name));
  print_npvariant_args(args, argCount);
  bool ret = npclass_invoke_Invoke(npobj, name, args, argCount, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPClass::Invoke return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

// NPClass::InvokeDefault
static int npclass_handle_InvokeDefault(rpc_connection_t *connection)
{
  D(bug("npclass_handle_InvokeDefault\n"));

  NPObject *npobj;
  uint32_t argCount;
  NPVariant *args;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, &argCount, &args,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::InvokeDefault() get args", error);
	return error;
  }

  uint32_t ret = false;
  NPVariant result;
  VOID_TO_NPVARIANT(result);
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->invokeDefault) {
	D(bugiI("NPClass::InvokeDefault(npobj %p)\n", npobj));
	print_npvariant_args(args, argCount);
	ret = npobj->_class->invokeDefault(npobj, args, argCount, &result);
	gchar *result_str = string_of_NPVariant(&result);
	D(bugiD("NPClass::InvokeDefault return: %d (%s)\n", ret, result_str));
	g_free(result_str);
  }

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

static bool npclass_invoke_InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount,
										 NPVariant *result)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_INVOKE_DEFAULT,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, argCount, args,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::InvokeDefault() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_UINT32, &ret,
									RPC_TYPE_NP_VARIANT_PASS_REF, result,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::InvokeDefault() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount,
							 NPVariant *result)
{
  if (result == NULL)
	return false;
  VOID_TO_NPVARIANT(*result);

  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::InvokeDefault not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::InvokeDefault(npobj %p)\n", npobj));
  print_npvariant_args(args, argCount);
  bool ret = npclass_invoke_InvokeDefault(npobj, args, argCount, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPClass::InvokeDefault return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

// NPClass::HasProperty
static int npclass_handle_HasProperty(rpc_connection_t *connection)
{
  D(bug("npclass_handle_HasProperty\n"));

  NPObject *npobj;
  NPIdentifier name;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &name,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::HasProperty() get args", error);
	return error;
  }

  uint32_t ret = false;
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->hasProperty) {
	D(bugiI("NPClass::HasProperty(npobj %p, name id %p)\n", npobj, name));
	ret = npobj->_class->hasProperty(npobj, name);
	D(bugiD("NPClass::HasProperty return: %d\n", ret));
  }

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

static bool npclass_invoke_HasProperty(NPObject *npobj, NPIdentifier name)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_HAS_PROPERTY,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_NP_IDENTIFIER, &name,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::HasProperty() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection, RPC_TYPE_UINT32, &ret, RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::HasProperty() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_HasProperty(NPObject *npobj, NPIdentifier name)
{
  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::HasProperty not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::HasProperty(npobj %p, name id %p)\n", npobj, name));
  bool ret = npclass_invoke_HasProperty(npobj, name);
  D(bugiD("NPClass::HasProperty return: %d\n", ret));
  return ret;
}
  
// NPClass::GetProperty
static int npclass_handle_GetProperty(rpc_connection_t *connection)
{
  D(bug("npclass_handle_GetProperty\n"));

  NPObject *npobj;
  NPIdentifier name;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &name,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::GetProperty() get args", error);
	return error;
  }

  uint32_t ret = false;
  NPVariant result;
  VOID_TO_NPVARIANT(result);
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->getProperty) {
	D(bugiI("NPClass::GetProperty(npobj %p, name id %p)\n", npobj, name));
	ret = npobj->_class->getProperty(npobj, name, &result);
	gchar *result_str = string_of_NPVariant(&result);
	D(bugiD("NPClass::GetProperty return: %d (%s)\n", ret, result_str));
	g_free(result_str);
  }

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_NP_VARIANT_PASS_REF, &result,
							   RPC_TYPE_INVALID);
}

static bool npclass_invoke_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_GET_PROPERTY,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_NP_IDENTIFIER, &name,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::GetProperty() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_UINT32, &ret,
									RPC_TYPE_NP_VARIANT_PASS_REF, result,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::GetProperty() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
  if (result == NULL)
	return false;
  VOID_TO_NPVARIANT(*result);

  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::GetProperty not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::GetProperty(npobj %p, name id %p)\n", npobj, name));
  bool ret = npclass_invoke_GetProperty(npobj, name, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPClass::GetProperty return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}
  
// NPClass::SetProperty
static int npclass_handle_SetProperty(rpc_connection_t *connection)
{
  D(bug("npclass_handle_SetProperty\n"));

  NPObject *npobj;
  NPIdentifier name;
  NPVariant value;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &name,
								  RPC_TYPE_NP_VARIANT, &value,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::SetProperty() get args", error);
	return error;
  }

  uint32_t ret = false;
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->setProperty) {
	D(bugiI("NPClass::SetProperty(npobj %p, name id %p)\n", npobj, name));
	ret = npobj->_class->setProperty(npobj, name, &value);
	D(bugiD("NPClass::SetProperty return: %d\n", ret));
  }

  if (npobj)
	NPN_ReleaseObject(npobj);
  NPN_ReleaseVariantValue(&value);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

static bool npclass_invoke_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_SET_PROPERTY,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_NP_IDENTIFIER, &name,
								RPC_TYPE_NP_VARIANT, value,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::SetProperty() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_UINT32, &ret,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::SetProperty() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
  if (value == NULL) {
	npw_printf("WARNING: NPClass::SetProperty() called with a NULL value\n");
	return false;
  }

  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::SetProperty not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::SetProperty(npobj %p, name id %p)\n", npobj, name));
  bool ret = npclass_invoke_SetProperty(npobj, name, value);
  D(bugiD("NPClass::SetProperty return: %d\n", ret));
  return ret;
}

// NPClass::RemoveProperty
static int npclass_handle_RemoveProperty(rpc_connection_t *connection)
{
  D(bug("npclass_handle_RemoveProperty\n"));

  NPObject *npobj;
  NPIdentifier name;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_NP_IDENTIFIER, &name,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::RemoveProperty() get args", error);
	return error;
  }

  uint32_t ret = false;
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->removeProperty) {
	D(bugiI("NPClass::RemoveProperty(npobj %p, name id %p)\n", npobj, name));
	ret = npobj->_class->removeProperty(npobj, name);
	D(bugiD("NPClass::RemoveProperty return: %d\n", ret));
  }

  if (npobj)
	NPN_ReleaseObject(npobj);

  return rpc_method_send_reply(connection,
							   RPC_TYPE_UINT32, ret,
							   RPC_TYPE_INVALID);
}

static bool npclass_invoke_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_REMOVE_PROPERTY,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_NP_IDENTIFIER, &name,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::RemoveProperty() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection, RPC_TYPE_UINT32, &ret, RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::RemoveProperty() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::RemoveProperty not called from the main thread\n");
	return false;
  }
  
  D(bugiI("NPClass::RemoveProperty(npobj %p, name id %p)\n", npobj, name));
  bool ret = npclass_invoke_RemoveProperty(npobj, name);
  D(bugiD("NPClass::RemoveProperty return: %d\n", ret));
  return ret;
}

// NPClass::Enumerate
static int npclass_handle_Enumerate(rpc_connection_t *connection)
{
  D(bug("npclass_handle_Enumerate\n"));

  NPObject *npobj;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Enumerate() get args", error);
	return error;
  }

  uint32_t ret = false;
  NPIdentifier *idents = NULL;
  uint32_t argCount = 0;
  if (npobj && is_valid_npobject_class(npobj) &&
	  NP_CLASS_STRUCT_VERSION_HAS_ENUM(npobj->_class) &&
	  npobj->_class->enumerate) {
	D(bugiI("NPClass::Enumerate(npobj %p)\n", npobj));
	ret = npobj->_class->enumerate(npobj, &idents, &argCount);
	// TODO: Print the identifiers to debug
	D(bugiD("NPClass::Enumerate return: %d (%d identifiers)\n", ret, argCount));
  }

  if (npobj)
	NPN_ReleaseObject(npobj);

  int rpc_ret = rpc_method_send_reply(connection,
									  RPC_TYPE_UINT32, ret,
									  RPC_TYPE_ARRAY, RPC_TYPE_NP_IDENTIFIER, argCount, idents,
									  RPC_TYPE_INVALID);
  if (idents) NPN_MemFree(idents);

  return rpc_ret;
}

static bool npclass_invoke_Enumerate(NPObject *npobj,
									 NPIdentifier **idents, uint32_t *count)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_ENUMERATE,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Enumerate() invoke", error);
	return false;
  }

  uint32_t ret;
  uint32_t myCount = 0;
  NPIdentifier *myIdentifiers = NULL;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_UINT32, &ret,
									RPC_TYPE_ARRAY, RPC_TYPE_NP_IDENTIFIER, &myCount, &myIdentifiers,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Enumerate() wait for reply", error);
	return false;
  }

  *count = myCount;
  if (ret) {
	ret = NPW_ReallocData(myIdentifiers,
						  sizeof(**idents) * myCount,
						  (void**)idents) == NPERR_NO_ERROR;
  }
  if (myIdentifiers)
	free(myIdentifiers);

  return ret;
}

bool g_NPClass_Enumerate(NPObject *npobj,
						 NPIdentifier **idents, uint32_t *count)
{
  if (count == NULL || idents == NULL)
	return false;

  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::Enumerate not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::Enumerate(npobj %p)\n", npobj));
  bool ret = npclass_invoke_Enumerate(npobj, idents, count);
  // TODO: print the identifiers to debug
  D(bugiD("NPClass::Enumerate return: %d (%d)\n", ret, *count));
  return ret;
}

// NPClass::Construct
static int npclass_handle_Construct(rpc_connection_t *connection)
{
  D(bug("npclass_handle_Construct\n"));

  NPObject *npobj;
  uint32_t argCount;
  NPVariant *args;
  int error = rpc_method_get_args(connection,
								  RPC_TYPE_NP_OBJECT, &npobj,
								  RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, &argCount, &args,
								  RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Construct() get args", error);
	return error;
  }

  uint32_t ret = false;
  NPVariant result;
  VOID_TO_NPVARIANT(result);
  if (npobj && is_valid_npobject_class(npobj) && npobj->_class->construct) {
	D(bugiI("NPClass::Construct(npobj %p)\n", npobj));
	print_npvariant_args(args, argCount);
	ret = npobj->_class->construct(npobj, args, argCount, &result);
	gchar *result_str = string_of_NPVariant(&result);
	D(bugiD("NPClass::Construct return: %d (%s)\n", ret, result_str));
	g_free(result_str);
  }

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

static bool npclass_invoke_Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount,
										 NPVariant *result)
{
  npw_return_val_if_fail(rpc_method_invoke_possible(g_rpc_connection), false);

  int error = rpc_method_invoke(g_rpc_connection,
								RPC_METHOD_NPCLASS_CONSTRUCT,
								RPC_TYPE_NP_OBJECT, npobj,
								RPC_TYPE_ARRAY, RPC_TYPE_NP_VARIANT, argCount, args,
								RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Construct() invoke", error);
	return false;
  }

  uint32_t ret;
  error = rpc_method_wait_for_reply(g_rpc_connection,
									RPC_TYPE_UINT32, &ret,
									RPC_TYPE_NP_VARIANT_PASS_REF, result,
									RPC_TYPE_INVALID);

  if (error != RPC_ERROR_NO_ERROR) {
	npw_perror("NPClass::Construct() wait for reply", error);
	return false;
  }

  return ret;
}

bool g_NPClass_Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount,
						 NPVariant *result)
{
  if (result == NULL)
	return false;
  VOID_TO_NPVARIANT(*result);

  if (!is_valid_npobject_proxy(npobj))
	return false;

  if (!thread_check()) {
	npw_printf("WARNING: NPClass::Construct not called from the main thread\n");
	return false;
  }

  D(bugiI("NPClass::Construct(npobj %p)\n", npobj));
  print_npvariant_args(args, argCount);
  bool ret = npclass_invoke_Construct(npobj, args, argCount, result);
  gchar *result_str = string_of_NPVariant(result);
  D(bugiD("NPClass::Construct return: %d (%s)\n", ret, result_str));
  g_free(result_str);
  return ret;
}

int npclass_add_method_descriptors(rpc_connection_t *connection)
{
  static const rpc_method_descriptor_t vtable[] = {
	{ RPC_METHOD_NPCLASS_HAS_METHOD,		npclass_handle_HasMethod },
	{ RPC_METHOD_NPCLASS_INVOKE,			npclass_handle_Invoke },
	{ RPC_METHOD_NPCLASS_INVOKE_DEFAULT,	npclass_handle_InvokeDefault },
	{ RPC_METHOD_NPCLASS_HAS_PROPERTY,		npclass_handle_HasProperty },
	{ RPC_METHOD_NPCLASS_GET_PROPERTY,		npclass_handle_GetProperty },
	{ RPC_METHOD_NPCLASS_SET_PROPERTY,		npclass_handle_SetProperty },
	{ RPC_METHOD_NPCLASS_REMOVE_PROPERTY,	npclass_handle_RemoveProperty },
	{ RPC_METHOD_NPCLASS_ENUMERATE,			npclass_handle_Enumerate },
	{ RPC_METHOD_NPCLASS_CONSTRUCT,			npclass_handle_Construct },
	{ RPC_METHOD_NPCLASS_DEALLOCATE,		npclass_handle_Deallocate },
  };
  return rpc_connection_add_method_descriptors(g_rpc_connection, vtable,
											   sizeof(vtable) / sizeof(vtable[0]));
}

#if NPW_IS_PLUGIN
/* ====================================================================== */
/* === NPObject Registration                                          === */
/* ====================================================================== */

static GHashTable *g_npobj_owners = NULL;

void npobject_register(NPObject *npobj, void *plugin)
{
  assert(!npobject_is_registered(npobj));
  g_hash_table_insert(g_npobj_owners, npobj, plugin);
}

bool npobject_is_registered(NPObject *npobj)
{
  return g_hash_table_lookup_extended(g_npobj_owners, npobj, NULL, NULL);
}

void *npobject_get_owner(NPObject *npobj)
{
  return g_hash_table_lookup(g_npobj_owners, npobj);
}

void npobject_unregister(NPObject *npobj)
{
  g_hash_table_remove(g_npobj_owners, npobj);
}
#endif

/* ====================================================================== */
/* === NPObject Bridge System                                         === */
/* ====================================================================== */

bool npobject_bridge_new(void)
{
  g_stubs = g_hash_table_new(NULL, NULL);
  g_proxies = g_hash_table_new(NULL, NULL);
#if NPW_IS_PLUGIN
  g_npobj_owners = g_hash_table_new(NULL, NULL);
#endif
  return true;
}

void npobject_bridge_destroy(void)
{
  if (g_stubs) {
	g_hash_table_destroy(g_stubs);
	g_stubs = NULL;
  }
  if (g_proxies) {
	g_hash_table_destroy(g_proxies);
	g_proxies = NULL;
  }
#if NPW_IS_PLUGIN
  if (g_npobj_owners) {
	g_hash_table_destroy(g_npobj_owners);
	g_npobj_owners = NULL;
  }
#endif
}

#if NPW_IS_BROWSER
static void proxy_deactivate_func(gpointer key, gpointer value, gpointer user_data)
{
  NPObjectProxy *proxy = (NPObjectProxy *)value;
  proxy->is_valid = false;
  // No need to release the stub. At this point the viewer is dead.
}

static void stub_destroy_func(gpointer key, gpointer value, gpointer user_data)
{
  NPObjectStub *stub = (NPObjectStub *)value;
  npobject_destroy_stub_obj(stub);
}

void npruntime_deactivate(void)
{
  // Invalidate any proxies the wrapper may still be holding on to.
  g_hash_table_foreach(g_proxies, proxy_deactivate_func, NULL);
  g_hash_table_foreach(g_stubs, stub_destroy_func, NULL);
  // Reset both tables.
  npobject_bridge_destroy();
  npobject_bridge_new();
}
#endif


/* ====================================================================== */
/* === NPVariant helpers                                              === */
/* ====================================================================== */

void
npvariant_clear(NPVariant *variant)
{
  switch (variant->type) {
  case NPVariantType_Void:
  case NPVariantType_Null:
  case NPVariantType_Bool:
  case NPVariantType_Int32:
  case NPVariantType_Double:
	break;
  case NPVariantType_String:
	{
	  NPString *s = &NPVARIANT_TO_STRING(*variant);
	  if (s->UTF8Characters)
		NPN_MemFree((void *)s->UTF8Characters);
	  break;
	}
  case NPVariantType_Object:
	{
	  NPObject *npobj = NPVARIANT_TO_OBJECT(*variant);
	  if (npobj)
		NPN_ReleaseObject(npobj);
	  break;
	}
  }
  VOID_TO_NPVARIANT(*variant);
}

// Make sure to deallocate with g_free() since it comes from a GString
gchar *
string_of_NPVariant(const NPVariant *arg)
{
#if DEBUG
  if (arg == NULL)
	return NULL;
  GString *str = g_string_new(NULL);
  switch (arg->type)
	{
	case NPVariantType_Void:
	  g_string_append_printf(str, "void");
	  break;
	case NPVariantType_Null:
	  g_string_append_printf(str, "null");
	  break;
	case NPVariantType_Bool:
	  g_string_append(str, arg->value.boolValue ? "true" : "false");
	  break;
	case NPVariantType_Int32:
	  g_string_append_printf(str, "%d", arg->value.intValue);
	  break;
	case NPVariantType_Double:
	  g_string_append_printf(str, "%f", arg->value.doubleValue);
	  break;
	case NPVariantType_String:
	  g_string_append_c(str, '\'');
	  g_string_append_len(str,
						  arg->value.stringValue.UTF8Characters,
						  arg->value.stringValue.UTF8Length);
	  g_string_append_c(str, '\'');
	  break;
	case NPVariantType_Object:
	  g_string_append_printf(str, "<object %p>", arg->value.objectValue);
	  break;
	default:
	  g_string_append_printf(str, "<invalid type %d>", arg->type);
	  break;
	}
  return g_string_free(str, FALSE);
#endif
  return NULL;
}

void
print_npvariant_args(const NPVariant *args, uint32_t nargs)
{
#if DEBUG
  GString *str = g_string_new(NULL);
  for (int i = 0; i < nargs; i++) {
	if (i > 0)
	  g_string_append(str, ", ");
	gchar *argstr = string_of_NPVariant(&args[i]);
	g_string_append(str, argstr);
	g_free(argstr);
  }
  D(bug("%u args (%s)\n", nargs, str->str));
  g_string_free(str, TRUE);
#endif
}
