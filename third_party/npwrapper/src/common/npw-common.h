/*
 *  npw-common.h - Common code for both the wrapper and the plugin
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

#ifndef NPW_COMMON_H
#define NPW_COMMON_H

#include <assert.h>

#include "rpc.h"
#include "npw-rpc.h"

#define XP_UNIX 1
#define MOZ_X11 1
#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>

/* Supported NPAPI interfaces */
#define NPW_NPAPI_VERSION		27
#define NPW_NP_CLASS_STRUCT_VERSION	3
#define NPW_TOOLKIT			NPNVGtk2

/* What are we building? */
#if defined BUILD_WRAPPER
# define NPW_IS_BROWSER (1)
#elif defined BUILD_VIEWER
# define NPW_IS_BROWSER (0)
#else
# error "Could not determine what we are building (browser|plugin)-side?"
#endif
#define NPW_IS_PLUGIN (!NPW_IS_BROWSER)

#include "npruntime-impl.h"

#if NPW_IS_BROWSER
# define _NPW_INSTANCE_PRIVATE_DATA pdata
#else
# define _NPW_INSTANCE_PRIVATE_DATA ndata
#endif

/* PluginInstance */
#define NPW_DECL_PLUGIN_INSTANCE		\
  NPW_PluginInstanceClass *klass;		\
  volatile int             refcount;		\
  NPP                      instance;		\
  uint32_t                 instance_id;		\
  bool                     is_valid

typedef struct _NPW_PluginInstance	NPW_PluginInstance;
typedef struct _NPW_PluginInstanceClass	NPW_PluginInstanceClass;

struct _NPW_PluginInstance
{
  NPW_DECL_PLUGIN_INSTANCE;
};

typedef void *
(*NPW_PluginInstanceAllocateFunctionPtr) (void);

typedef void
(*NPW_PluginInstanceDeallocateFunctionPtr) (NPW_PluginInstance *plugin);

typedef void
(*NPW_PluginInstanceFinalizeFunctionPtr) (NPW_PluginInstance *plugin);

typedef void
(*NPW_PluginInstanceInvalidateFunctionPtr) (NPW_PluginInstance *plugin);

struct _NPW_PluginInstanceClass
{
  NPW_PluginInstanceAllocateFunctionPtr   allocate;
  NPW_PluginInstanceDeallocateFunctionPtr deallocate;
  NPW_PluginInstanceFinalizeFunctionPtr   finalize;
  NPW_PluginInstanceInvalidateFunctionPtr invalidate;
};

void *
npw_plugin_instance_new(NPW_PluginInstanceClass *klass) attribute_hidden;

void *
npw_plugin_instance_ref(void *ptr) attribute_hidden;

void
npw_plugin_instance_unref(void *ptr) attribute_hidden;

void
npw_plugin_instance_invalidate(void *ptr) attribute_hidden;

static inline bool
npw_plugin_instance_is_valid(void *ptr)
{
  NPW_PluginInstance *plugin = (NPW_PluginInstance *)ptr;
  return plugin && plugin->is_valid;
}

#define NPW_PLUGIN_INSTANCE(instance)	npw_get_plugin_instance (instance)
#define NPW_PLUGIN_INSTANCE_NPP(plugin)	npw_get_plugin_instance_npp (plugin)

static inline NPW_PluginInstance *
_npw_get_plugin_instance (NPP instance)
{
  return instance ? (NPW_PluginInstance *)instance->_NPW_INSTANCE_PRIVATE_DATA : NULL;
}

static inline NPW_PluginInstance *
npw_get_plugin_instance (NPP instance)
{
  if (instance)
    {
      NPW_PluginInstance *plugin;
      if ((plugin = _npw_get_plugin_instance (instance)) != NULL)
	{
	  if (plugin->instance == instance)
	    return plugin;
#if NPW_IS_PLUGIN
	  /* XXX: `instance' is probably a copy of the original
	     plugin->instance with the same `plugin' as private data
	     (CrossOver Plugin) */
	  if (_npw_get_plugin_instance (plugin->instance) == plugin)
	    return plugin;
#endif
	  assert (plugin->instance == instance);
	}
    }
  return NULL;
}

static inline NPP
npw_get_plugin_instance_npp (NPW_PluginInstance *plugin)
{
  return plugin ? plugin->instance : NULL;
}

/* StreamInstance */
#define NPW_DECL_STREAM_INSTANCE		\
  NPStream *stream;				\
  uint32_t  stream_id;				\
  int       is_plugin_stream

typedef struct _NPW_StreamInstance NPW_StreamInstance;
struct _NPW_StreamInstance
{
  NPW_DECL_STREAM_INSTANCE;
};

#define NPW_STREAM_INSTANCE(stream) npw_get_stream_instance (stream)

static inline NPW_StreamInstance *
_npw_get_stream_instance (NPStream *np_stream)
{
  return (NPW_StreamInstance *)np_stream->_NPW_INSTANCE_PRIVATE_DATA;
}

static inline NPW_StreamInstance *
npw_get_stream_instance (NPStream *np_stream)
{
  if (np_stream)
    {
      NPW_StreamInstance *stream;
      if ((stream = _npw_get_stream_instance (np_stream)) != NULL)
	{
	  assert (stream->stream == np_stream);
	  return stream;
	}
    }
  return NULL;
}

/* Unimplemented functions */
#define NPW_UNIMPLEMENTED()						\
  npw_printf ("WARNING: Unimplemented function %s() at %s:%d\n",	\
			  __func__, __FILE__, __LINE__)

/* Initialize NPAPI hooks */
void
NPW_InitializeFuncs (NPNetscapeFuncs *mozilla_funcs,
		     NPPluginFuncs   *plugin_funcs)
  attribute_hidden;

enum {
  NPW_IdentifierType_Integer = 1,
  NPW_IdentifierType_String
};

typedef struct _NPW_Identifier *NPW_Identifier;
struct _NPW_Identifier
{
  uint32_t   type;	// type of the identifier
  uint32_t   id;	// numeric id to pass back to the browser
  union {
    int32_t  i;
    char    *s;
  }          value;	// local value cache
};

/* Create identifier from an integer */
NPW_Identifier
NPW_CreateIntIdentifier (int32_t value) attribute_hidden;

/* Create identifier from a string (that is copied) */
NPW_Identifier
NPW_CreateStringIdentifier (const char *str) attribute_hidden;

/* Create identifier from a string (that is now owned by the identifier) */
NPW_Identifier
NPW_CreateStringIdentifierSink (char *str) attribute_hidden;

/* Destroy identifier */
void
NPW_DestroyIdentifier (NPW_Identifier id) attribute_hidden;

/* Check whether identifier is an integer */
static inline bool
NPW_IsIntIdentifier (NPW_Identifier id)
{
  return id && id->type == NPW_IdentifierType_Integer;
}

/* Check whether identifier is a string */
static inline bool
NPW_IsStringIdentifier (NPW_Identifier id)
{
  return id && id->type == NPW_IdentifierType_String;
}

/* Return integer value from the identifier */
static inline int32_t
NPW_GetIntIdentifierValue (NPW_Identifier id)
{
  return NPW_IsIntIdentifier (id) ? id->value.i : 0;
}

/* Return string value from the identifier */
static inline const char *
NPW_GetStringIdentifierValue (NPW_Identifier id)
{
  return NPW_IsStringIdentifier (id) ? id->value.s : 0;
}

/* Reallocates a buffer with NPN_MemAlloc. Returns
 * NPERR_OUT_OF_MEMORY_ERROR if NPN_MemAlloc fails. */
NPError
NPW_ReallocData(void *ptr, uint32_t size, void **out) attribute_hidden;

#endif /* NPW_COMMON_H */
