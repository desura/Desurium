/*
 *  npw-malloc.h - Memory allocation
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

#ifndef NPW_MALLOC_H
#define NPW_MALLOC_H

void *
NPW_MemAlloc (uint32_t size) attribute_hidden;

void *
NPW_MemAlloc0 (uint32_t size) attribute_hidden;

void *
NPW_MemAllocCopy (uint32_t size, const void *ptr) attribute_hidden;

void
NPW_MemFree (void *ptr) attribute_hidden;

void *
NPW_Debug_MemAlloc (uint32_t size, const char *file, int lineno) attribute_hidden;

void *
NPW_Debug_MemAlloc0 (uint32_t size, const char *file, int lineno) attribute_hidden;

void *
NPW_Debug_MemAllocCopy (uint32_t size, const void *ptr, const char *file, int lineno) attribute_hidden;

void
NPW_Debug_MemFree (void *ptr, const char *file, int lineno) attribute_hidden;

#define NPW_MemNew(type, n) \
  ((type *) NPW_MemAlloc ((n) * sizeof (type)))

#define NPW_MemNew0(type, n) \
  ((type *) NPW_MemAlloc0 ((n) * sizeof (type)))

#define NPW_MemClone(type, ptr) \
  ((type *) NPW_MemAllocCopy (sizeof (type), ptr))

#ifdef ENABLE_MALLOC_CHECK
# define NPW_MemAlloc(SIZE)		NPW_Debug_MemAlloc(SIZE, __FILE__, __LINE__)
# define NPW_MemAlloc0(SIZE)		NPW_Debug_MemAlloc0(SIZE, __FILE__, __LINE__)
# define NPW_MemAllocCopy(SIZE, PTR)	NPW_Debug_MemAllocCopy(SIZE, PTR, __FILE__, __LINE__)
# define NPW_MemFree(PTR)		NPW_Debug_MemFree(PTR, __FILE__, __LINE__)
#endif

#endif /* NPW_MALLOC_H */
