/*
 *  rpc.h - Remote Procedure Calls
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

#ifndef RPC_H
#define RPC_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error Types
enum {
  RPC_ERROR_NO_ERROR					= 0,
  RPC_ERROR_GENERIC						= -1000,
  RPC_ERROR_ERRNO_SET					= -1001,
  RPC_ERROR_NO_MEMORY					= -1002,
  RPC_ERROR_CONNECTION_NULL				= -1003,
  RPC_ERROR_CONNECTION_CLOSED			= -1004,
  RPC_ERROR_CONNECTION_TYPE_MISMATCH	= -1005,
  RPC_ERROR_MESSAGE_TIMEOUT				= -1006,
  RPC_ERROR_MESSAGE_TRUNCATED			= -1007,
  RPC_ERROR_MESSAGE_TYPE_INVALID		= -1008,
  RPC_ERROR_MESSAGE_HANDLER_INVALID		= -1009,
  RPC_ERROR_MESSAGE_ARGUMENT_MISMATCH	= -1010,
  RPC_ERROR_MESSAGE_ARGUMENT_UNKNOWN	= -1011,
  RPC_ERROR_MESSAGE_ARGUMENT_INVALID	= -1012,
  RPC_ERROR_MESSAGE_SYNC_NOT_ALLOWED	= -1013,
};
extern const char *rpc_strerror(int error) attribute_hidden;

// Connection Handling
typedef struct rpc_connection rpc_connection_t;
extern rpc_connection_t *rpc_connection_ref(rpc_connection_t *connection) attribute_hidden;
extern void rpc_connection_unref(rpc_connection_t *connection) attribute_hidden;

extern rpc_connection_t *rpc_init_server(const char *ident) attribute_hidden;
extern rpc_connection_t *rpc_init_client(const char *ident) attribute_hidden;
extern int rpc_exit(rpc_connection_t *connection) attribute_hidden;
extern int rpc_listen_socket(rpc_connection_t *connection) attribute_hidden;
extern int rpc_listen(rpc_connection_t *connection) attribute_hidden;
extern int rpc_dispatch(rpc_connection_t *connection) attribute_hidden;
extern int rpc_wait_dispatch(rpc_connection_t *connection, int timeout) attribute_hidden;
extern int rpc_dispatch_pending_sync(rpc_connection_t *connection) attribute_hidden;
extern int rpc_socket(rpc_connection_t *connection) attribute_hidden;

extern int rpc_sync(rpc_connection_t *connection) attribute_hidden;
extern int rpc_end_sync(rpc_connection_t *connection) attribute_hidden;

enum {
  RPC_STATUS_BROKEN						= -1,
  RPC_STATUS_CLOSED						= 0,
  RPC_STATUS_ACTIVE						= 1,
};
extern int rpc_status(rpc_connection_t *connection) attribute_hidden;

// Message Passing
enum {
  RPC_TYPE_INVALID						=  0,
  RPC_TYPE_CHAR							= -2000,
  RPC_TYPE_BOOLEAN						= -2001,
  RPC_TYPE_INT32						= -2002,
  RPC_TYPE_UINT32						= -2003,
  RPC_TYPE_UINT64						= -2004,
  RPC_TYPE_DOUBLE						= -2005,
  RPC_TYPE_STRING						= -2006,
  RPC_TYPE_ARRAY						= -2007,
};
typedef struct rpc_message_t rpc_message_t;
extern int rpc_message_send_char(rpc_message_t *message, char c) attribute_hidden;
extern int rpc_message_send_int32(rpc_message_t *message, int32_t value) attribute_hidden;
extern int rpc_message_send_uint32(rpc_message_t *message, uint32_t value) attribute_hidden;
extern int rpc_message_send_uint64(rpc_message_t *message, uint64_t value) attribute_hidden;
extern int rpc_message_send_double(rpc_message_t *message, double value) attribute_hidden;
extern int rpc_message_send_string(rpc_message_t *message, const char *str) attribute_hidden;
extern int rpc_message_send_bytes(rpc_message_t *message, unsigned char *bytes, int count) attribute_hidden;
extern int rpc_message_recv_char(rpc_message_t *message, char *ret) attribute_hidden;
extern int rpc_message_recv_int32(rpc_message_t *message, int32_t *ret) attribute_hidden;
extern int rpc_message_recv_uint32(rpc_message_t *message, uint32_t *ret) attribute_hidden;
extern int rpc_message_recv_uint64(rpc_message_t *message, uint64_t *ret) attribute_hidden;
extern int rpc_message_recv_double(rpc_message_t *message, double *ret) attribute_hidden;
extern int rpc_message_recv_string(rpc_message_t *message, char **ret) attribute_hidden;
extern int rpc_message_recv_bytes(rpc_message_t *message, unsigned char *bytes, int count) attribute_hidden;

typedef int (*rpc_message_callback_t)(rpc_message_t *message, void *p_value);
typedef struct {
  int id;
  int size;
  rpc_message_callback_t send_callback;
  rpc_message_callback_t recv_callback;
} rpc_message_descriptor_t;
extern int rpc_connection_add_message_descriptor(rpc_connection_t *connection, const rpc_message_descriptor_t *desc) attribute_hidden;
extern int rpc_connection_add_message_descriptors(rpc_connection_t *connection, const rpc_message_descriptor_t *descs, int n_descs) attribute_hidden;

// Method Callbacks Handling
typedef int (*rpc_method_callback_t)(rpc_connection_t *connection);
typedef struct {
  int id;
  rpc_method_callback_t callback;
} rpc_method_descriptor_t;
extern int rpc_connection_add_method_descriptor(rpc_connection_t *connection, const rpc_method_descriptor_t *desc) attribute_hidden;
extern int rpc_connection_add_method_descriptors(rpc_connection_t *connection, const rpc_method_descriptor_t *descs, int n_descs) attribute_hidden;

// Remote Procedure Call (method invocation)
extern bool rpc_method_invoke_possible(rpc_connection_t *connection) attribute_hidden;
extern int rpc_method_invoke(rpc_connection_t *connection, int method, ...) attribute_hidden;
extern int rpc_method_wait_for_reply(rpc_connection_t *connection, ...) attribute_hidden;
extern int rpc_method_get_args(rpc_connection_t *connection, ...) attribute_hidden;
extern int rpc_method_send_reply(rpc_connection_t *connection, ...) attribute_hidden;

// Sources
extern GSource *rpc_event_source_new(rpc_connection_t *connection) attribute_hidden;
extern GSource *rpc_sync_source_new(rpc_connection_t *connection) attribute_hidden;

#ifdef __cplusplus
}
#endif

// This callback is called when the connection is closed or broken
typedef void (*rpc_error_callback_t)(rpc_connection_t *connection, void *user_data);

// Set error callback for a connection
void rpc_connection_set_error_callback(rpc_connection_t *connection, rpc_error_callback_t callback, void *callback_data) attribute_hidden;

#endif /* RPC_H */
