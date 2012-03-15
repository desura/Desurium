/*
 *  utils.h - Utility functions
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

#ifndef UTILS_H
#define UTILS_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Hashes
extern bool id_init(void) attribute_hidden;
extern void id_kill(void) attribute_hidden;
extern void id_link(int id, void *ptr) attribute_hidden;
extern int id_create(void *ptr) attribute_hidden;
extern bool id_remove(int id) attribute_hidden;
extern void *id_lookup(int id) attribute_hidden;
extern int id_lookup_value(void *ptr) attribute_hidden;

// String expansions
extern const char *string_of_NPError(int error) attribute_hidden;
extern const char *string_of_NPReason(int reason) attribute_hidden;
extern const char *string_of_NPStreamType(int stype) attribute_hidden;
extern const char *string_of_NPEvent_type(int type) attribute_hidden;
extern const char *string_of_NPPVariable(int variable) attribute_hidden;
extern const char *string_of_NPNVariable(int variable) attribute_hidden;
extern const char *string_of_NPNURLVariable(int variable) attribute_hidden;
extern const char *string_of_NPWindowType(int type) attribute_hidden;

// Misc utility functions
extern void npw_perror(const char *prefix, int error) attribute_hidden;
extern const char *npw_strerror(int error) attribute_hidden;
extern void npw_close_all_open_files(void) attribute_hidden;

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */
