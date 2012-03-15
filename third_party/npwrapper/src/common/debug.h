/*
 *  debug.h - Debugging utilities
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

#ifndef DEBUG_H
#define DEBUG_H

#undef assert
#define assert(x) ;

#include <glib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern G_GNUC_PRINTF(1, 2) void npw_dprintf(const char *format, ...) attribute_hidden;

extern void npw_indent(int inc) attribute_hidden;
extern G_GNUC_PRINTF(2, 3) void npw_idprintf(int inc, const char *format, ...) attribute_hidden;

extern G_GNUC_PRINTF(1, 2) void npw_printf(const char *format, ...) attribute_hidden;
extern void npw_vprintf(const char *format, va_list args) attribute_hidden;

#if DEBUG
/* Very verbose mode that uses the ##__VA_ARGS__ GCC extension */
# if 0 && (defined(__GNUC__) && (__GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ == 96)))
#  define bug(format, ...) \
     npw_dprintf("[%-20s:%4d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
#  define bugiI(format, ...) \
     npw_idprintf(+1, "[%-20s:%4d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
#  define bugiD(format, ...) \
     npw_idprintf(-1, "[%-20s:%4d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
# else
#  define bug npw_dprintf
#  define bugiI(...) npw_idprintf(+1, __VA_ARGS__)
#  define bugiD(...) npw_idprintf(-1, __VA_ARGS__)
# endif
# define D(x) x
#else
# define D(x) ;
#endif

/* XXX: add an ENABLE_CHECKS config option? */
#if DEBUG
# define npw_return_if_fail_warning(expr_str) do {				\
  npw_printf("WARNING:(%s:%d):%s: assertion failed: (%s)\n",	\
			 __FILE__, __LINE__, __func__, expr_str);			\
} while (0)
# define npw_return_if_fail(expr)			do {	\
  if (!(expr)) {									\
	npw_return_if_fail_warning(#expr);				\
	return;											\
  }													\
} while (0)
# define npw_return_val_if_fail(expr, val)	do {	\
  if (!(expr)) {									\
	npw_return_if_fail_warning(#expr);				\
	return (val);									\
  }													\
} while (0)
#else
# define npw_return_if_fail(expr)			do {} while (0)
# define npw_return_val_if_fail(expr, val)	do {} while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */
