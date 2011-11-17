/*
 *  npw-malloc.c - Memory allocation
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

/* Hack to workaround the NPW_MemAlloc macros. TODO: don't make macros
 * and function names conflict. It's insane. */
#undef ENABLE_MALLOC_CHECK
#include "npw-malloc.h"

#define DEBUG 1
#include "debug.h"

typedef void *(*NPW_MemAllocProcPtr) (uint32_t);
typedef void  (*NPW_MemFreeProcPtr)  (void *, uint32_t);

typedef struct _NPW_MallocHooks NPW_MallocHooks;
struct _NPW_MallocHooks
{
  NPW_MemAllocProcPtr memalloc;
  NPW_MemAllocProcPtr memalloc0;
  NPW_MemFreeProcPtr  memfree;
};

#define NPW_MALLOC_MAGIC 0x4e50574d /* 'NPWM' */

typedef struct _NPW_MemBlock NPW_MemBlock;
struct _NPW_MemBlock
{
  uint32_t    magic;
  uint32_t    real_size;
  uint32_t    alloc_size;
  uint32_t    alloc_lineno;
  const char *alloc_file;
};

static void *npw_mem_alloc      (uint32_t size, const char *file, int lineno);
static void *npw_mem_alloc0     (uint32_t size, const char *file, int lineno);
static void *npw_mem_alloc_copy (uint32_t size, const void *ptr, const char *file, int lineno);
static void npw_mem_free        (void *ptr, const char *file, int lineno);

/* ====================================================================== */
/* === Standard C library                                             === */
/* ====================================================================== */

#ifndef USE_MALLOC_LIBC
#define USE_MALLOC_LIBC 0
#endif

#if USE_MALLOC_LIBC
#include <stdlib.h>

static void *
NPW_Libc_MemAlloc (uint32_t size)
{
  return malloc (size);
}

static void *
NPW_Libc_MemAlloc0 (uint32_t size)
{
  return calloc (1, size);
}

static void
NPW_Libc_MemFree (void *ptr, uint32_t size)
{
  free (ptr);
}

static const NPW_MallocHooks g_libc_hooks = {
  NPW_Libc_MemAlloc,
  NPW_Libc_MemAlloc0,
  NPW_Libc_MemFree
};
#endif

/* ====================================================================== */
/* === Glib support                                                   === */
/* ====================================================================== */

#ifndef USE_MALLOC_GLIB
#define USE_MALLOC_GLIB 0
#endif

#if USE_MALLOC_GLIB
#include <glib.h>

static void *
NPW_Glib_MemAlloc (uint32_t size)
{
  return g_slice_alloc (size);
}

static void *
NPW_Glib_MemAlloc0 (uint32_t size)
{
  return g_slice_alloc0 (size);
}

static void
NPW_Glib_MemFree (void *ptr, uint32_t size)
{
  g_slice_free1 (size, ptr);
}

static const NPW_MallocHooks g_glib_hooks = {
  NPW_Glib_MemAlloc,
  NPW_Glib_MemAlloc0,
  NPW_Glib_MemFree
};
#endif

/* ====================================================================== */
/* === Public interface                                               === */
/* ====================================================================== */

#define N_MALLOC_LIBS (USE_MALLOC_LIBC + USE_MALLOC_GLIB)

#ifndef CONCAT
#define CONCAT_(a,b) a##b
#define CONCAT(a,b)  CONCAT_(a,b)
#endif

#define get_default_malloc_hooks() \
  (&CONCAT(CONCAT(g_,DEFAULT_MALLOC_LIB),_hooks))

#if N_MALLOC_LIBS > 1
static const NPW_MallocHooks *
do_get_malloc_hooks (void)
{
  const char *malloc_lib;
  if ((malloc_lib =  getenv ("NPW_MALLOC_LIB")) != NULL)
    {
#if USE_MALLOC_LIBC
      if (strcmp (malloc_lib, "libc") == 0)
	return &g_libc_hooks;
#endif
#if USE_MALLOC_GLIB
      if (strcmp (malloc_lib, "glib") == 0)
	return &g_glib_hooks;
#endif
    }
  return get_default_malloc_hooks ();
}

static inline const NPW_MallocHooks *
get_malloc_hooks (void)
{
  static const NPW_MallocHooks *malloc_hooks = NULL;
  if (malloc_hooks == NULL)
    malloc_hooks = do_get_malloc_hooks ();
  return malloc_hooks;
}
#else
#define get_malloc_hooks() get_default_malloc_hooks()
#endif

void *
NPW_MemAlloc (uint32_t size)
{
  return npw_mem_alloc (size, NULL, 0);
}

void *
NPW_MemAlloc0 (uint32_t size)
{
  return npw_mem_alloc0 (size, NULL, 0);
}

void *
NPW_MemAllocCopy (uint32_t size, const void *ptr)
{
  return npw_mem_alloc_copy (size, ptr, NULL, 0);
}

void
NPW_MemFree (void *ptr)
{
  npw_mem_free (ptr, NULL, 0);
}

void *
NPW_Debug_MemAlloc (uint32_t size, const char *file, int lineno)
{
  return npw_mem_alloc (size, file, lineno);
}

void *
NPW_Debug_MemAlloc0 (uint32_t size, const char *file, int lineno)
{
  return npw_mem_alloc0 (size, file, lineno);
}

void *
NPW_Debug_MemAllocCopy (uint32_t size, const void *ptr, const char *file, int lineno)
{
  return npw_mem_alloc_copy (size, ptr, file, lineno);
}

void
NPW_Debug_MemFree (void *ptr, const char *file, int lineno)
{
  npw_mem_free (ptr, file, lineno);
}

/* ====================================================================== */
/* === Implementation allowing basic underflow/overflow checks        === */
/* ====================================================================== */

#ifdef ENABLE_MALLOC_CHECK
static bool
is_malloc_check_enabled_1 (void)
{
  const char *malloc_check_str;
  if ((malloc_check_str =  getenv ("NPW_MALLOC_CHECK")) != NULL)
    return ((strcmp (malloc_check_str, "yes") == 0) ||
	    (strcmp (malloc_check_str, "1") == 0));

  /* enable malloc-checks by default for all builds from snapshots */
  return NPW_SNAPSHOT > 0;
}
#endif

#define MALLOC_CHECK_GUARD_MARK 'E'
#define MALLOC_CHECK_GUARD_SIZE malloc_check_guards_size ()

static inline bool
is_malloc_check_enabled (void)
{
#ifdef ENABLE_MALLOC_CHECK
  static int malloc_check = -1;
  if (malloc_check < 0)
    malloc_check = is_malloc_check_enabled_1 ();
  return malloc_check;
#else
  return false;
#endif
}

static inline uint32_t
malloc_check_guards_size (void)
{
  return is_malloc_check_enabled () ? 16 : 0;
}

static void
malloc_check_guards_init (uint8_t *ptr, uint32_t size)
{
  if (!is_malloc_check_enabled ())
    return;

  memset (ptr - MALLOC_CHECK_GUARD_SIZE,
	  MALLOC_CHECK_GUARD_MARK,
	  MALLOC_CHECK_GUARD_SIZE);
  memset (ptr + size,
	  MALLOC_CHECK_GUARD_MARK,
	  MALLOC_CHECK_GUARD_SIZE);
}

static bool
malloc_check_guards_ok (uint8_t *ptr, uint32_t size, int *punderflow, int *poverflow)
{
  if (!is_malloc_check_enabled ())
    return true;

  int i, underflow = 0, overflow = 0;
  for (i = 0; i < MALLOC_CHECK_GUARD_SIZE; i++)
    {
      if (ptr[-(1 + i)] != MALLOC_CHECK_GUARD_MARK)
	++underflow;
      if (ptr[size + i] != MALLOC_CHECK_GUARD_MARK)
	++overflow;
    }
  if (punderflow)
    *punderflow = underflow;
  if (poverflow)
    *poverflow = overflow;
  return !underflow && !overflow;
}

static inline void *
npw_do_mem_alloc (NPW_MemAllocProcPtr mem_alloc_func, uint32_t size, const char *file, int lineno)
{
  uint32_t      real_size;
  NPW_MemBlock *mem;

  real_size = sizeof (*mem) + size + 2 * MALLOC_CHECK_GUARD_SIZE;
  if ((mem = mem_alloc_func (real_size)) == NULL)
    return NULL;

  mem->magic        = NPW_MALLOC_MAGIC;
  mem->real_size    = real_size;
  mem->alloc_size   = size;
  mem->alloc_file   = file;
  mem->alloc_lineno = lineno;

  uint8_t *ptr = (uint8_t *)mem + sizeof (*mem) + MALLOC_CHECK_GUARD_SIZE;
  malloc_check_guards_init (ptr, size);
  return ptr;
}

static void *
npw_mem_alloc (uint32_t size, const char *file, int lineno)
{
  return npw_do_mem_alloc (get_malloc_hooks ()->memalloc, size, file, lineno);
}

static void *
npw_mem_alloc0 (uint32_t size, const char *file, int lineno)
{
  return npw_do_mem_alloc (get_malloc_hooks ()->memalloc0, size, file, lineno);
}

static void *
npw_mem_alloc_copy (uint32_t size, const void *src, const char *file, int lineno)
{
  void *ptr = npw_mem_alloc (size, file, lineno);
  if (ptr)
    memcpy (ptr, src, size);
  return ptr;
}

static void
npw_mem_free (void *ptr, const char *file, int lineno)
{
  if (ptr == NULL)
    return;

  NPW_MemBlock *mem = (NPW_MemBlock *)((char *)ptr - (sizeof (*mem) + MALLOC_CHECK_GUARD_SIZE));
  if (mem->magic == NPW_MALLOC_MAGIC)
    {
      int underflow, overflow;
      if (!malloc_check_guards_ok (ptr, mem->alloc_size, &underflow, &overflow))
	{
	  if (underflow)
	    npw_printf ("ERROR: detected underflow of %d bytes\n"
			"  for block allocated at %s:%d\n"
			"  and released at %s:%d\n",
			underflow,
			mem->alloc_file, mem->alloc_lineno,
			file, lineno);
	  if (overflow)
	    npw_printf ("ERROR: detected overflow of %d bytes\n"
			"  for block allocated at %s:%d\n"
			"  and released at %s:%d\n",
			overflow,
			mem->alloc_file, mem->alloc_lineno,
			file, lineno);
	}
      get_malloc_hooks ()->memfree (mem, mem->real_size);
    }
  else
    {
      npw_printf("ERROR: block %p was not allocated with NPW_MemAlloc(), reverting to libc free()\n", ptr);
      free (ptr);
    }
}
