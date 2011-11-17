/*
 *  debug.c - Debugging utilities
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>

#include "debug.h"


static int g_indent_level = 0;

void npw_indent(int inc)
{
  g_indent_level += inc;
}

static int _can_indent_messages(void)
{
  const char *indent_str = getenv("NPW_INDENT_MESSAGES");
  if (indent_str) {
	// XXX: also check for "yes", "no", "true", "false"
	errno = 0;
	long v = strtol(indent_str, NULL, 10);
	if ((v != LONG_MIN && v != LONG_MAX) || errno != ERANGE)
	  return v;
  }
  return 1;
}

static inline int get_indent_level(void)
{
  static int can_indent_messages = -1;
  if (can_indent_messages < 0)
	can_indent_messages = _can_indent_messages();
  if (can_indent_messages)
	return g_indent_level;
  return 0;
}


static int _get_debug_level(void)
{
  const char *debug_str = getenv("NPW_DEBUG");
  if (debug_str) {
	errno = 0;
	long v = strtol(debug_str, NULL, 10);
	if ((v != LONG_MIN && v != LONG_MAX) || errno != ERANGE)
	  return v;
  }
  return 0;
}

static inline int get_debug_level(void)
{
  static int g_debug_level = -1;
  if (g_debug_level < 0)
	g_debug_level = _get_debug_level();
  return g_debug_level;
}


void npw_dprintf(const char *format, ...)
{
  if (get_debug_level() > 0) {
	va_list args;
	va_start(args, format);
	npw_vprintf(format, args);
	va_end(args);
  }
}

void npw_idprintf(int inc, const char *format, ...)
{
  if (get_debug_level() > 0) {
	if (inc < 0)
	  npw_indent(inc);
	va_list args;
	va_start(args, format);
	npw_vprintf(format, args);
	va_end(args);
	if (inc > 0)
	  npw_indent(inc);
  }
}


static FILE *g_log_file = NULL;

static FILE *npw_log_file(void)
{
  if (g_log_file == NULL) {
	const char *log_file = getenv("NPW_LOG");
	if (log_file) {
	  const char *mode = "w";
#ifdef BUILD_VIEWER
	  /* the wrapper plugin has the responsability to create the file,
		 thus the viewer is only opening it for appending data.  */
	  mode = "a";
#endif
	  g_log_file = fopen(log_file, mode);
	}
	if (log_file == NULL)
	  g_log_file = stderr;
  }
  if (g_log_file != stderr)
	fseek(g_log_file, 0, SEEK_END);
  return g_log_file;
}

static void __attribute__((destructor)) log_file_sentinel(void)
{
  if (g_log_file && g_log_file != stderr) {
	fclose(g_log_file);
	g_log_file = stderr;
  }
}


static void npw_print_indent(FILE *fp)
{
  static const char blanks[] = "                ";
  const int blanks_length = sizeof(blanks) - 1;
  int n_blanks = 2 * get_indent_level();
  for (int i = 0; i < n_blanks / blanks_length; i++)
	fwrite(blanks, blanks_length, 1, fp);
  if ((n_blanks = (n_blanks % blanks_length)) > 0)
	fwrite(blanks, n_blanks, 1, fp);
}

void npw_vprintf(const char *format, va_list args)
{
  FILE *log_file = npw_log_file();
  fprintf(log_file, "*** NSPlugin %s *** ", NPW_COMPONENT_NAME);
  npw_print_indent(log_file);
  vfprintf(log_file, format, args);
  fflush(log_file);
}

void npw_printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  npw_vprintf(format, args);
  va_end(args);
}
