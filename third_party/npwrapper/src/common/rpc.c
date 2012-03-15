/*
 *  rpc.c - Remote Procedure Calls
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

/*
 *  NOTES:
 *  - this RPC system is very minimal and only suited for 1:1 communication.
 *
 *  TODO:
 *  - better failure conditions
 *  - split for windows rpc support
 */

#include "sysdeps.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>
#include <inttypes.h>
#include <time.h>

#include "rpc.h"
#include "utils.h"

#define DEBUG 0
#include "debug.h"


// Define to use non-blocking I/O
#define NON_BLOCKING_IO 1

// Don't use anonymous sockets by default so that a generic Linux/i386
// build of the viewer can interoperate with non-Linux wrappers. Linux
// distributions can use this code though.
// XXX better clean-up dead sockets properly on failure...
#ifndef BUILD_GENERIC
#if defined(__linux__)
#define USE_ANONYMOUS_SOCKETS 1
#endif
#endif

// Define the maximum amount of time (in seconds) to wait for a message
#ifndef RPC_MESSAGE_TIMEOUT
#define RPC_MESSAGE_TIMEOUT 30
#endif

// Define the maximum amount of time (in seconds) to wait for plugin connection
#ifndef RPC_INIT_TIMEOUT
#define RPC_INIT_TIMEOUT 5
#endif

// Set close-on-exec flag on the newly created socket (Linux >= 2.6.27)
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif


/* ====================================================================== */
/* === Utility functions                                              === */
/* ====================================================================== */

// Use threads (needed for rpc_listen())
#define USE_THREADS

// PThreads glue
#ifndef USE_THREADS
#define pthread_t void *
#define pthread_cancel(th)
#define pthread_join(th, ret)
#define pthread_testcancel()
#define pthread_create(th, attr, start, arg) dummy_thread_create()
static inline int dummy_thread_create(void) { errno = ENOSYS; return -1; }

#undef  pthread_mutex_t
#define pthread_mutex_t volatile int
#undef  pthread_mutex_lock
#define pthread_mutex_lock(m) -1
#undef  pthread_mutex_unlock
#define pthread_mutex_unlock(m) -1
#undef  PTHREAD_MUTEX_INITIALIZER
#define PTHREAD_MUTEX_INITIALIZER 0
#endif

// IEEE754 double representation
typedef union {
  double value;
  struct {
#ifdef WORDS_BIGENDIAN
	unsigned int negative  :  1;
	unsigned int exponent  : 11;
	unsigned int mantissa0 : 20;
	unsigned int mantissa1 : 32;
#else
#ifdef FLOAT_WORDS_BIGENDIAN
	unsigned int mantissa0 : 20;
	unsigned int exponent  : 11;
	unsigned int negative  :  1;
	unsigned int mantissa1 : 32;
#else
	unsigned int mantissa1 : 32;
	unsigned int mantissa0 : 20;
	unsigned int exponent  : 11;
	unsigned int negative  :  1;
#endif
#endif
  } parts;
} ieee_double_shape_t;

// Wait for the specified amount of microseconds (usec shall be < 1 second)
#if defined __linux__
// Linux select() changes its timeout parameter upon return to contain
// the remaining time. Most other unixen leave it unchanged or undefined.
#define SELECT_SETS_REMAINING
#else
#define USE_NANOSLEEP
#endif

static void rpc_delay(uint32_t usec)
{
  int was_error;

#if defined(USE_NANOSLEEP)
  struct timespec elapsed, tv;
#else
  struct timeval tv;
#endif

  // Set the timeout interval - Linux only needs to do this once
#if defined(SELECT_SETS_REMAINING)
  tv.tv_sec = 0;
  tv.tv_usec = usec;
#elif defined(USE_NANOSLEEP)
  elapsed.tv_sec = 0;
  elapsed.tv_nsec = usec * 1000;
#endif

  do {
	errno = 0;
#if defined(USE_NANOSLEEP)
	tv.tv_sec = elapsed.tv_sec;
	tv.tv_nsec = elapsed.tv_nsec;
	was_error = nanosleep(&tv, &elapsed);
#else
	was_error = select(0, NULL, NULL, NULL, &tv);
#endif
  } while (was_error && (errno == EINTR));
}

// Returns the maximum amount of time (in seconds) to wait for a message
static int _rpc_message_timeout(void)
{
  int timeout = 0;
  const char *timeout_str = getenv("NPW_MESSAGE_TIMEOUT");
  if (timeout_str)
	timeout = atoi(timeout_str);
  if (timeout <= 0)
	timeout = RPC_MESSAGE_TIMEOUT;
  D(bug("RPC message timeout set to %d sec\n", timeout));
  return timeout;
}

static inline int rpc_message_timeout(void)
{
  static int timeout = -1;
  if (timeout < 0)
	timeout = _rpc_message_timeout();
  return timeout;
}

// Returns the maximum amount of time (in seconds) to wait for plugin connection
static int _rpc_init_timeout(void)
{
  int timeout = 0;
  const char *timeout_str = getenv("NPW_INIT_TIMEOUT");
  if (timeout_str)
	timeout = atoi(timeout_str);
  if (timeout <= 0)
	timeout = RPC_INIT_TIMEOUT;
  D(bug("RPC init timeout set to %d sec\n", timeout));
  return timeout;
}

static inline int rpc_init_timeout(void)
{
  static int timeout = -1;
  if (timeout < 0)
	timeout = _rpc_init_timeout();
  return timeout;
}

// Returns a string describing the error code passed in the argument ERROR
const char *rpc_strerror(int error)
{
  const char *str;
  switch (error) {
  case RPC_ERROR_NO_ERROR:
	str = "No error";
	break;
  case RPC_ERROR_GENERIC:
	str = "Generic error";
	break;
  case RPC_ERROR_ERRNO_SET:
	str = strerror(errno);
	break;
  case RPC_ERROR_NO_MEMORY:
	str = "Out of memory";
	break;
  case RPC_ERROR_CONNECTION_NULL:
	str = "Connection was NULL";
	break;
  case RPC_ERROR_CONNECTION_CLOSED:
	str = "Connection closed";
	break;
  case RPC_ERROR_CONNECTION_TYPE_MISMATCH:
	str = "Connection type mismatch";
	break;
  case RPC_ERROR_MESSAGE_TIMEOUT:
	str = "Message timeout";
	break;
  case RPC_ERROR_MESSAGE_TRUNCATED:
	str = "Message truncated";
	break;
  case RPC_ERROR_MESSAGE_TYPE_INVALID:
	str = "Message type invalid";
	break;
  case RPC_ERROR_MESSAGE_HANDLER_INVALID:
	str = "Message does not have any handler";
	break;
  case RPC_ERROR_MESSAGE_ARGUMENT_MISMATCH:
	str = "Message argument mismatch";
	break;
  case RPC_ERROR_MESSAGE_ARGUMENT_UNKNOWN:
	str = "Message argument type unknown";
	break;
  case RPC_ERROR_MESSAGE_ARGUMENT_INVALID:
	str = "Message argument invalid";
	break;
  case RPC_ERROR_MESSAGE_SYNC_NOT_ALLOWED:
	str = "SYNC message forbidden";
	break;
  default:
	str = "<unknown>";
	break;
  }
  return str;
}

// Set non blocking I/O on the specified socket
static int rpc_set_non_blocking_io(int socket)
{
#if NON_BLOCKING_IO
  int val = fcntl(socket, F_GETFL, 0);
  if (val < 0)
	return val;
  int ret = fcntl(socket, F_SETFL, val | O_NONBLOCK);
  if (ret < 0)
	return ret;
#endif
  return 0;
}

// Check for pending data on the specified socket
enum {
  RPC_POLL_READ,
  RPC_POLL_WRITE,
};

static inline int rpc_poll(int op, int socket, int timeout)
{
  struct timeval tv;
  tv.tv_sec  = timeout / 1000000;
  tv.tv_usec = timeout % 1000000;

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(socket, &fds);

  int ret = -1;
  switch (op) {
  case RPC_POLL_READ:
	ret = select(socket + 1, &fds, NULL, NULL, &tv);
	break;
  case RPC_POLL_WRITE:
	ret = select(socket + 1, NULL, &fds, NULL, &tv);
	break;
  }
  return ret;
}

// Hash tables (implemented as simple tables at this time)
typedef void (*rpc_map_entry_destroy_func_t)(void *value);

typedef struct {
  void *value;
  int key;
  int use_count;
} rpc_map_entry_t;

typedef struct {
  int n_entries;
  int n_entries_max;
  rpc_map_entry_t *entries;
  rpc_map_entry_destroy_func_t destroy_func;
} rpc_map_t;

static rpc_map_t *rpc_map_new(void)
{
  rpc_map_t *map = (rpc_map_t *)malloc(sizeof(*map));
  if (map == NULL)
	return NULL;
  map->entries = NULL;
  map->n_entries = 0;
  map->n_entries_max = 0;
  map->destroy_func = NULL;
  return map;
}

static rpc_map_t *rpc_map_new_full(rpc_map_entry_destroy_func_t destroy_func)
{
  rpc_map_t *map = rpc_map_new();
  if (map == NULL)
	return NULL;
  map->destroy_func = destroy_func;
  return map;
}

static void rpc_map_destroy(rpc_map_t *map)
{
  if (map == NULL)
	return;
  if (map->entries) {
	if (map->destroy_func) {
	  for (int i = 0; i < map->n_entries; i++) {
		map->destroy_func(map->entries[i].value);
		map->entries[i].value = NULL;
	  }
	}
	free(map->entries);
	map->entries = NULL;
  }
  free(map);
}

static rpc_map_entry_t *_rpc_map_lookup(const rpc_map_t *map, int key)
{
  assert(map != NULL);

  if (map->entries == NULL)
	return NULL;

  for (int i = 0; i < map->n_entries; i++)
	if (map->entries[i].key == key)
	  return &map->entries[i];
  return NULL;
}

static void *rpc_map_lookup(const rpc_map_t *map, int key)
{
  rpc_map_entry_t *entry = _rpc_map_lookup(map, key);
  if (entry == NULL)
	return NULL;
  entry->use_count++;
  return entry->value;
}

static int rpc_map_insert(rpc_map_t *map, int key, void *value)
{
  assert(map != NULL);
  assert(value != NULL);

  const int N_ENTRIES_ALLOC = 7;
  int i = map->n_entries_max;

  // override any existing entry
  rpc_map_entry_t *entry = _rpc_map_lookup(map, key);
  if (entry) {
	entry->value = value;
	entry->use_count = 0;
	return RPC_ERROR_NO_ERROR;
  }

  // look for a free slot
  if (map->entries) {
	for (i = 0; i < map->n_entries_max; i++) {
	  if (map->entries[i].value == NULL)
		break;
	}
  }

  // none found, reallocate
  if (i >= map->n_entries_max) {
	if ((map->entries = (rpc_map_entry_t *)realloc(map->entries, (map->n_entries_max + N_ENTRIES_ALLOC) * sizeof(map->entries[0]))) == NULL)
	  return RPC_ERROR_NO_MEMORY;
	i = map->n_entries;
	memset(&map->entries[i], 0, N_ENTRIES_ALLOC * sizeof(map->entries[0]));
	map->n_entries_max += N_ENTRIES_ALLOC;
  }

  map->entries[i].key = key;
  map->entries[i].value = value;
  map->entries[i].use_count = 0;
  ++map->n_entries;
  return 0;
}


/* ====================================================================== */
/* === RPC Connection Handling                                        === */
/* ====================================================================== */

// Connection type
enum {
  RPC_CONNECTION_SERVER,
  RPC_CONNECTION_CLIENT,
};

// Client / Server connection
struct rpc_connection {
  int type;
  int refcnt;
  int status;
  int socket;
  char *socket_path;
  struct sockaddr_un socket_addr;
  socklen_t socket_addr_len;
  int server_socket;
  int server_thread_active;
  pthread_t server_thread;
  rpc_map_t *types;
  rpc_map_t *methods;
  rpc_error_callback_t error_callback;
  void *error_callback_data;
  int dispatch_depth;
  int invoke_depth;
  int handle_depth;
  bool is_sync;
  int pending_sync_depth;
};

// Increment connection reference count
rpc_connection_t *rpc_connection_ref(rpc_connection_t *connection)
{
  if (connection)
	++connection->refcnt;
  return connection;
}

// Decrement connection reference count and destroy it if it reaches zero
void rpc_connection_unref(rpc_connection_t *connection)
{
  if (connection && --connection->refcnt == 0) {
	D(bug("Close unused connection\n"));
	rpc_exit(connection);
  }
}

// Returns whether we are in sync mode or not (i.e. needs other end sync)
static inline bool _rpc_connection_is_sync_mode(rpc_connection_t *connection)
{
  return connection->type == RPC_CONNECTION_SERVER;
}

// Returns whether we are allowed to synchronize with the other end
static inline bool _rpc_connection_is_sync_allowed(rpc_connection_t *connection)
{
  if (_rpc_connection_is_sync_mode(connection)) {
	npw_printf("ERROR: RPC is not allowed to receive MESSAGE_SYNC\n");
	return false;
  }
  return connection->pending_sync_depth == 0;
}

// Set error callback for a connection
void rpc_connection_set_error_callback(rpc_connection_t *connection,
                                       rpc_error_callback_t callback,
									   void *callback_data)
{
  if (connection == NULL)
	return;
  if (connection->error_callback != NULL)
	return;  
  connection->error_callback = callback;
  connection->error_callback_data = callback_data;
}

// Call error callback if the connection is closed
static inline void _rpc_connection_invoke_error_callback(rpc_connection_t *connection)
{
  connection->error_callback(connection, connection->error_callback_data);
}

static inline void rpc_connection_invoke_error_callback(rpc_connection_t *connection)
{
  if (connection && connection->error_callback)
	_rpc_connection_invoke_error_callback(connection);
}

// Returns connection status
static inline int _rpc_status(rpc_connection_t *connection)
{
  return connection->status;
}

int rpc_status(rpc_connection_t *connection)
{
  if (connection == NULL)
	return RPC_STATUS_BROKEN;
  return _rpc_status(connection);
}

// Set connection status
static void _rpc_set_status(rpc_connection_t *connection, int error)
{
  if (connection->status == RPC_STATUS_ACTIVE) {
	switch (error) {
	case RPC_ERROR_NO_ERROR:
	  connection->status = RPC_STATUS_ACTIVE;
	  break;
	case RPC_ERROR_CONNECTION_CLOSED:
	  connection->status = RPC_STATUS_CLOSED;
	  break;
	default:
	  connection->status = RPC_STATUS_BROKEN;
	  break;
	}
  }
}

// Set error status. The caller must return after a call to this function
// e.g. the usual practise is to "return rpc_error();"
// This is necessary because the error callback can decide to kill the
// RPC connection in that case, so anything beyond rpc_error() should
// be considered garbage.
static int rpc_error(rpc_connection_t *connection, int error)
{
  // XXX: this function must be called only in case of error
  // (otherwise, it's an internal error, hence the assert()s)
  assert(error < 0);
  assert(connection != NULL);
  _rpc_set_status(connection, error);

  switch (_rpc_status(connection)) {
  case RPC_STATUS_CLOSED:
  case RPC_STATUS_BROKEN:
	rpc_connection_invoke_error_callback(connection);
	break;
  default:
	break;
  }
  return error;
}

#if DEBUG
static inline int rpc_error_debug(rpc_connection_t *connection, int error, const char *filename, int line)
{
  npw_printf("ERROR: RPC error %d caught at %s:%d\n", error, filename, line);
  return rpc_error(connection, error);
}
#define rpc_error(connection, error) \
		rpc_error_debug(connection, error, __FILE__, __LINE__)
#endif

// Returns socket fd or -1 if invalid connection
int rpc_socket(rpc_connection_t *connection)
{
  if (connection == NULL)
	return -1;

  return connection->socket;
}

// Prepare socket path for addr.sun_path[]
// Returns the path length minus the terminating '\0'
static int _rpc_socket_path(char **pathp, const char *ident)
{
  if (pathp == NULL)
	return 0;

  char *path;
  int n, len;

#if USE_ANONYMOUS_SOCKETS
  n = 0;
#else
#if 0
  char *tmpdir = getenv("TMPDIR");
  if (tmpdir == NULL)
	tmpdir = "/tmp";
#else
  // XXX don't use TMPDIR because the length of the resulting path
  // could exceed the size of sun_path[] (typically 108 bytes on Linux)
  char *tmpdir = "/tmp";
#endif
  n = strlen(tmpdir);
#endif

  len = strlen(ident);
  if ((path = (char *)malloc(n + 1 + len + 1)) == NULL)
	return 0;

#if USE_ANONYMOUS_SOCKETS
  path[0] = 0;
  strcpy(&path[1], ident);
  n = 1 + len;
#else
  n = sprintf(path, "%s/", tmpdir);
  for (int i = 0; i < len; i++) {
    char ch = ident[i];
    if (ch == '/')
      ch = '_';
    path[n++] = ch;
  }
  path[n] = '\0';
#endif

  if (*pathp)
	free(*pathp);
  *pathp = path;
  return n;
}

// Create a new RPC connection (initialize common structure members)
static rpc_connection_t *rpc_connection_new(int type, const char *ident)
{
  rpc_connection_t *connection;

  if (ident == NULL)
	return NULL;

  if ((connection = (rpc_connection_t *)calloc(1, sizeof(*connection))) == NULL)
	return NULL;

  connection->type = type;
  connection->refcnt = 1;
  connection->status = RPC_STATUS_CLOSED;
  connection->socket = -1;
  connection->server_socket = -1;
  connection->server_thread_active = 0;
  connection->error_callback = NULL;
  connection->error_callback_data = NULL;
  connection->dispatch_depth = 0;
  connection->invoke_depth = 0;
  connection->handle_depth = 0;
  connection->is_sync = false;
  connection->pending_sync_depth = 0;

  if ((connection->types = rpc_map_new_full((free))) == NULL) {
	rpc_exit(connection);
	return NULL;
  }

  if ((connection->methods = rpc_map_new()) == NULL) {
	rpc_exit(connection);
	return NULL;
  }

  int fd = socket(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, 0);
  if (fd < 0) {
	perror("socket");
	rpc_exit(connection);
	return NULL;
  }

  if (type == RPC_CONNECTION_SERVER)
	connection->server_socket = fd;
  else {
	connection->socket = fd;

	if (rpc_set_non_blocking_io(fd) < 0) {
	  perror("socket set non-blocking");
	  rpc_exit(connection);
	  return NULL;
	}
  }

  memset(&connection->socket_addr, 0, sizeof(connection->socket_addr));
  connection->socket_addr.sun_family = AF_UNIX;
  connection->socket_path = NULL;
  connection->socket_addr_len = _rpc_socket_path(&connection->socket_path, ident);
  memcpy(&connection->socket_addr.sun_path[0], connection->socket_path, connection->socket_addr_len);
  connection->socket_addr_len += offsetof(struct sockaddr_un, sun_path); /* though POSIX says size of the actual sockaddr structure */
#ifdef HAVE_SOCKADDR_UN_SUN_LEN
  connection->socket_addr.sun_len = connection->socket_addr_len;
#endif

  return connection;
}

// Destroy an RPC connection
static void rpc_connection_destroy(rpc_connection_t *connection)
{
  if (connection == NULL)
	return;

  if (connection->socket_path) {
	if (connection->socket_path[0])
	  unlink(connection->socket_path);
	free(connection->socket_path);
	connection->socket_path = NULL;
  }

  if (connection->type == RPC_CONNECTION_SERVER) {
	if (connection->server_thread_active) {
	  pthread_cancel(connection->server_thread);
	  pthread_join(connection->server_thread, NULL);
	  connection->server_thread = 0;
	}
	if (connection->socket != -1) {
	  close(connection->socket);
	  connection->socket = -1;
	}
	if (connection->server_socket != -1) {
	  close(connection->server_socket);
	  connection->server_socket = -1;
	}
  }
  else {
	if (connection->socket != -1) {
	  close(connection->socket);
	  connection->socket = -1;
	}
  }

  if (connection->types) {
	rpc_map_destroy(connection->types);
	connection->types = NULL;
  }
  if (connection->methods) {
	rpc_map_destroy(connection->methods);
	connection->methods = NULL;
  }

  free(connection);
}

// Initialize server-side RPC system
rpc_connection_t *rpc_init_server(const char *ident)
{
  D(bug("rpc_init_server ident='%s'\n", ident));

  rpc_connection_t *connection;

  if ((connection = rpc_connection_new(RPC_CONNECTION_SERVER, ident)) == NULL)
	return NULL;

  if (bind(connection->server_socket, (struct sockaddr *)&connection->socket_addr, connection->socket_addr_len) < 0) {
	perror("server bind");
	rpc_exit(connection);
	return NULL;
  }

  if (listen(connection->server_socket, 1) < 0) {
	perror("server listen");
	rpc_exit(connection);
	return NULL;
  }

  connection->status = RPC_STATUS_ACTIVE;
  return connection;
}

// Initialize client-side RPC system
rpc_connection_t *rpc_init_client(const char *ident)
{
  D(bug("rpc_init_client ident='%s'\n", ident));

  rpc_connection_t *connection;

  if ((connection = rpc_connection_new(RPC_CONNECTION_CLIENT, ident)) == NULL)
	return NULL;

  // Wait at most RPC_INIT_TIMEOUT seconds for server to initialize
  const int N_CONNECT_WAIT_DELAY = 10;
  int n_connect_attempts = (rpc_init_timeout() * 1000) / N_CONNECT_WAIT_DELAY;
  if (n_connect_attempts == 0)
	n_connect_attempts = 1;
  while (n_connect_attempts > 0) {
	if (connect(connection->socket, (struct sockaddr *)&connection->socket_addr, connection->socket_addr_len) == 0)
	  break;
	if (n_connect_attempts > 1 && errno != ECONNREFUSED && errno != ENOENT) {
	  perror("client_connect");
	  rpc_exit(connection);
	  return NULL;
	}
	n_connect_attempts--;
	rpc_delay(N_CONNECT_WAIT_DELAY * 1000);
  }
  if (n_connect_attempts == 0) {
	rpc_exit(connection);
	return NULL;
  }

  connection->status = RPC_STATUS_ACTIVE;
  return connection;
}

// Close RPC connection
int rpc_exit(rpc_connection_t *connection)
{
  D(bug("rpc_exit\n"));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;

  rpc_connection_destroy(connection);
  return RPC_ERROR_NO_ERROR;
}

// Wait for a message to arrive on the connection port
static inline int _rpc_wait_dispatch(rpc_connection_t *connection, int timeout)
{
  int ret = rpc_poll(RPC_POLL_READ, connection->socket, timeout);
  return ret >= 0 ? ret : RPC_ERROR_ERRNO_SET;
}

int rpc_wait_dispatch(rpc_connection_t *connection, int timeout)
{
	if (connection == NULL)
		return RPC_ERROR_CONNECTION_NULL;
	if (connection->socket < 0)
		return RPC_ERROR_GENERIC;

	return _rpc_wait_dispatch(connection, timeout);
}

// Process incoming messages in the background
static void *rpc_server_func(void *arg)
{
  rpc_connection_t *connection = (rpc_connection_t *)arg;

  int ret = rpc_listen_socket(connection);
  if (ret < 0)
	return NULL;

  connection->server_thread_active = 1;
  for (;;) {
	// XXX broken MacOS X doesn't implement cancellation points correctly
	pthread_testcancel();

	// wait for data to arrive
	int ret = _rpc_wait_dispatch(connection, 50000);
	if (ret == 0)
	  continue;
	if (ret < 0)
	  break;

	rpc_dispatch(connection);
  }
  connection->server_thread_active = 0;
  return NULL;
}

// Return listen socket of RPC connection
int rpc_listen_socket(rpc_connection_t *connection)
{
  D(bug("rpc_listen_socket\n"));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;
  if (connection->type != RPC_CONNECTION_SERVER)
	return RPC_ERROR_CONNECTION_TYPE_MISMATCH;

  struct sockaddr_un addr;
  socklen_t addr_len = sizeof(addr);
  if ((connection->socket = accept(connection->server_socket, (struct sockaddr *)&addr, &addr_len)) < 0)
	return RPC_ERROR_ERRNO_SET;

  if (rpc_set_non_blocking_io(connection->socket) < 0)
	return RPC_ERROR_ERRNO_SET;

  return connection->socket;
}

// Listen for incoming messages on RPC connection
#ifdef USE_THREADS
int rpc_listen(rpc_connection_t *connection)
{
  D(bug("rpc_listen\n"));

  if (pthread_create(&connection->server_thread, NULL, rpc_server_func, connection) != 0)
	return RPC_ERROR_ERRNO_SET;

  return RPC_ERROR_NO_ERROR;
}
#endif


/* ====================================================================== */
/* === Message Passing                                                === */
/* ====================================================================== */

// Message markers
enum {
  RPC_MESSAGE_START		= -3000,
  RPC_MESSAGE_ARGS		= -3001,
  RPC_MESSAGE_END		= -3002,
  RPC_MESSAGE_ACK		= -3003,
  RPC_MESSAGE_REPLY		= -3004,
  RPC_MESSAGE_FAILURE	= -3005,
  RPC_MESSAGE_SYNC		= -3006,
  RPC_MESSAGE_SYNC_END	= -3007,
  RPC_MESSAGE_SYNC_ACK	= -3008,
};

// Message type
struct rpc_message_t {
  const rpc_map_t *types;
  int socket;
  int offset;
  unsigned char buffer[BUFSIZ];
};

// Add a user-defined marshaler
int rpc_connection_add_message_descriptor(rpc_connection_t *connection, const rpc_message_descriptor_t *idesc)
{
  D(bug("rpc_connection_add_message_descriptor for type %d\n", idesc->id));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;

  rpc_message_descriptor_t *desc;
  if ((desc = (rpc_message_descriptor_t *)rpc_map_lookup(connection->types, idesc->id)) != NULL) {
	if (memcmp(desc, idesc, sizeof(*desc)) == 0)
	  return RPC_ERROR_NO_ERROR;
	fprintf(stderr, "duplicate message type %d\n", desc->id);
	return RPC_ERROR_GENERIC;
  }

  if ((desc = (rpc_message_descriptor_t *)malloc(sizeof(*desc))) == NULL)
	return RPC_ERROR_NO_MEMORY;
  memcpy(desc, idesc, sizeof(*desc));

  return rpc_map_insert(connection->types, desc->id, desc);
}

// Add user-defined marshalers
int rpc_connection_add_message_descriptors(rpc_connection_t *connection, const rpc_message_descriptor_t *descs, int n_descs)
{
  D(bug("rpc_connection_add_message_descriptors\n"));

  for (int i = 0; i < n_descs; i++) {
	int error = rpc_connection_add_message_descriptor(connection, &descs[i]);
	if (error < 0)
	  return error;
  }

  return RPC_ERROR_NO_ERROR;
}

// Find user-defined marshaler
static inline rpc_message_descriptor_t *rpc_message_descriptor_lookup(rpc_message_t *message, int id)
{
  D(bug("rpc_message_descriptor_lookup\n"));

  if (message && message->types)
	return (rpc_message_descriptor_t *)rpc_map_lookup(message->types, id);

  return NULL;
}

// Initialize message
static inline void rpc_message_init(rpc_message_t *message, rpc_connection_t *connection)
{
  message->types  = connection->types;
  message->socket = connection->socket;
  message->offset = 0;
}

// Send BYTES
static inline int _rpc_message_send_bytes(rpc_message_t *message, unsigned char *bytes, int count)
{
  do {
	int n = send(message->socket, bytes, count, 0);
	if (n >= 0) { // XXX is this correct behaviour for n == 0?
	  count -= n;
	  bytes += n;
	}
	else {
	  if (errno == ECONNRESET)
		return RPC_ERROR_CONNECTION_CLOSED;
	  else if (errno == EAGAIN || errno == EWOULDBLOCK) {
		// wait for a reasonable amount of time until we could send()
		// again with probably more success, or return timeout error
		int ret = rpc_poll(RPC_POLL_WRITE, message->socket, rpc_message_timeout() * 1000000);
		if (ret > 0)
		  continue;
		if (ret == 0)
		  return RPC_ERROR_MESSAGE_TIMEOUT;
	  }
	  if (errno == EINTR)
		continue;
	  return RPC_ERROR_ERRNO_SET;
	}
  } while (count > 0);
  return RPC_ERROR_NO_ERROR;
}

// Send message on wire
static inline int rpc_message_flush(rpc_message_t *message)
{
  int error = _rpc_message_send_bytes(message, message->buffer, message->offset);
  message->offset = 0;
  return error;
}

// Send BYTES
int rpc_message_send_bytes(rpc_message_t *message, unsigned char *bytes, int count)
{
  if (message->offset + count >= sizeof(message->buffer)) {
	int error = rpc_message_flush(message);
	if (error != RPC_ERROR_NO_ERROR)
	  return error;
	return _rpc_message_send_bytes(message, bytes, count);
  }
  memcpy(&message->buffer[message->offset], bytes, count);
  message->offset += count;
  return RPC_ERROR_NO_ERROR;
}

// Send CHAR
int rpc_message_send_char(rpc_message_t *message, char c)
{
  D(bug("  send CHAR '%c'\n", c));

  unsigned char e_value = c;
  return rpc_message_send_bytes(message, (unsigned char *)&e_value, sizeof(e_value));
}

// Send INT32
int rpc_message_send_int32(rpc_message_t *message, int32_t value)
{
  D(bug("  send INT32 %d\n", value));

  int32_t e_value = htonl(value);
  return rpc_message_send_bytes(message, (unsigned char *)&e_value, sizeof(e_value));
}

// Send UINT32
int rpc_message_send_uint32(rpc_message_t *message, uint32_t value)
{
  D(bug("  send UINT32 %u\n", value));

  uint32_t e_value = htonl(value);
  return rpc_message_send_bytes(message, (unsigned char *)&e_value, sizeof(e_value));
}

// Send UINT64
int rpc_message_send_uint64(rpc_message_t *message, uint64_t value)
{
  D(bug("  send UINT64 %" PRIx64 "\n", value));

  int error = rpc_message_send_uint32(message, (uint32_t)(value >> 32));
  if (error < 0)
	return error;
  return rpc_message_send_uint32(message, (uint32_t)value);
}

// Send DOUBLE
int rpc_message_send_double(rpc_message_t *message, double value)
{
  D(bug("  send DOUBLE %g\n", value));

  int error;
  ieee_double_shape_t dbl;
  dbl.value = value;
  if ((error = rpc_message_send_uint32(message, (uint32_t)dbl.parts.negative)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, (uint32_t)dbl.parts.exponent)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, (uint32_t)dbl.parts.mantissa0)) < 0)
	return error;
  if ((error = rpc_message_send_uint32(message, (uint32_t)dbl.parts.mantissa1)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

// Send STRING
int rpc_message_send_string(rpc_message_t *message, const char *str)
{
  D(bug("  send STRING \"%s\"\n", str));

  int error, length = str ? strlen(str) : -1;
  uint32_t e_value = htonl(length);
  error = rpc_message_send_bytes(message, (unsigned char *)&e_value, sizeof(e_value));
  if (error != RPC_ERROR_NO_ERROR)
	return error;
  if (length > 0)
	return rpc_message_send_bytes(message, (unsigned char *)str, length);
  return RPC_ERROR_NO_ERROR;
}

// Send message arguments
static int rpc_message_send_args(rpc_message_t *message, va_list args)
{
  int type, error, nargs = 0;
  rpc_message_descriptor_t *desc;
  while ((type = va_arg(args, int)) != RPC_TYPE_INVALID) {
	if (++nargs == 1) {
	  error = rpc_message_send_int32(message, RPC_MESSAGE_ARGS);
	  if (error != RPC_ERROR_NO_ERROR)
		return error;
	}
	error = rpc_message_send_int32(message, type);
	if (error != RPC_ERROR_NO_ERROR)
	  return error;
	switch (type) {
	case RPC_TYPE_CHAR:
	  error = rpc_message_send_char(message, (char )va_arg(args, int));
	  break;
	case RPC_TYPE_BOOLEAN:
	case RPC_TYPE_INT32:
	  error = rpc_message_send_int32(message, va_arg(args, int));
	  break;
	case RPC_TYPE_UINT32:
	  error = rpc_message_send_uint32(message, va_arg(args, unsigned int));
	  break;
	case RPC_TYPE_UINT64:
	  error = rpc_message_send_uint64(message, va_arg(args, uint64_t));
	  break;
	case RPC_TYPE_DOUBLE:
	  error = rpc_message_send_double(message, va_arg(args, double));
	  break;
	case RPC_TYPE_STRING:
	  error = rpc_message_send_string(message, va_arg(args, char *));
	  break;
	case RPC_TYPE_ARRAY: {
	  int i;
	  int array_type = va_arg(args, int32_t);
	  unsigned int array_size = va_arg(args, uint32_t);
	  void *array_arg = va_arg(args, void *);
	  bool is_valid_array = array_size > 0 && array_arg != NULL;
	  if ((error = rpc_message_send_int32(message, array_type)) < 0)
		return error;
	  if ((error = rpc_message_send_uint32(message, array_size)) < 0)
		return error;
	  if ((error = rpc_message_send_char(message, is_valid_array)) < 0)
		return error;
	  if (!is_valid_array)
		break;
	  switch (array_type) {
	  case RPC_TYPE_CHAR: {
		unsigned char *array = (unsigned char *)array_arg;
		error = rpc_message_send_bytes(message, array, array_size);
		break;
	  }
	  case RPC_TYPE_BOOLEAN:
	  case RPC_TYPE_INT32: {
		int32_t *array = (int32_t *)array_arg;
		for (i = 0; i < array_size; i++) {
		  if ((error = rpc_message_send_int32(message, array[i])) < 0)
			break;
		}
		break;
	  }
	  case RPC_TYPE_UINT32: {
		uint32_t *array = (uint32_t *)array_arg;
		for (i = 0; i < array_size; i++) {
		  if ((error = rpc_message_send_uint32(message, array[i])) < 0)
			break;
		}
		break;
	  }
	  case RPC_TYPE_UINT64: {
		uint64_t *array = (uint64_t *)array_arg;
		for (i = 0; i < array_size; i++) {
		  if ((error = rpc_message_send_uint64(message, array[i])) < 0)
			break;
		}
		break;
	  }
	  case RPC_TYPE_DOUBLE: {
		double *array = (double *)array_arg;
		for (i = 0; i < array_size; i++) {
		  if ((error = rpc_message_send_double(message, array[i])) < 0)
			break;
		}
		break;
	  }
	  case RPC_TYPE_STRING: {
		char **array = (char **)array_arg;
		for (i = 0; i < array_size; i++) {
		  if ((error = rpc_message_send_string(message, array[i])) < 0)
			break;
		}
		break;
	  }
	  default:
		if ((desc = rpc_message_descriptor_lookup(message, array_type)) != NULL) {
		  // arguments are passed by value (XXX: needs a way to differenciate reference/value)
		  uint8_t *array = (uint8_t *)array_arg;
		  for (i = 0; i < array_size; i++) {
			if ((error = desc->send_callback(message, &array[i * desc->size])) < 0)
			  break;
		  }
		}
		else {
		  fprintf(stderr, "unknown array arg type %d to send\n", type);
		  error = RPC_ERROR_MESSAGE_ARGUMENT_UNKNOWN;
		}
		break;
	  }
	  break;
	}
	default:
	  if ((desc = rpc_message_descriptor_lookup(message, type)) != NULL)
		error = desc->send_callback(message, va_arg(args, uint8_t *));
	  else {
		fprintf(stderr, "unknown arg type %d to send\n", type);
		error = RPC_ERROR_MESSAGE_ARGUMENT_UNKNOWN;
	  }
	  break;
	}
	if (error != RPC_ERROR_NO_ERROR)
	  return error;
  }
  if (nargs) {
	error = rpc_message_send_int32(message, RPC_MESSAGE_END);
	if (error != RPC_ERROR_NO_ERROR)
	  return error;
  }
  return RPC_ERROR_NO_ERROR;
}

// Receive raw BYTES
static inline int _rpc_message_recv_bytes(rpc_message_t *message, unsigned char *bytes, int count)
{
  do {
	int n = recv(message->socket, bytes, count, 0);
	if (n > 0) {
	  count -= n;
	  bytes += n;
	}
	else if (n == 0)
	  return RPC_ERROR_CONNECTION_CLOSED;
	else {
	  if (errno == EAGAIN || errno == EWOULDBLOCK) {
		// wait for a reasonable amount of time until we could recv()
		// again with probably more success, or return timeout error
		int ret = rpc_poll(RPC_POLL_READ, message->socket, rpc_message_timeout() * 1000000);
		if (ret > 0)
		  continue;
		if (ret == 0)
		  return RPC_ERROR_MESSAGE_TIMEOUT;
	  }
	  if (errno == EINTR)
		continue;
	  return RPC_ERROR_ERRNO_SET;
	}
  } while (count > 0);
  return RPC_ERROR_NO_ERROR;
}

int rpc_message_recv_bytes(rpc_message_t *message, unsigned char *bytes, int count)
{
  return _rpc_message_recv_bytes(message, bytes, count);
}

// Receive CHAR
int rpc_message_recv_char(rpc_message_t *message, char *ret)
{
  char r_value;
  int error;
  if ((error = _rpc_message_recv_bytes(message, (unsigned char *)&r_value, sizeof(r_value))) < 0)
	return error;
  *ret = r_value;
  D(bug("  recv CHAR '%c'\n", *ret));
  return RPC_ERROR_NO_ERROR;
}

// Receive INT32
int rpc_message_recv_int32(rpc_message_t *message, int32_t *ret)
{
  int32_t r_value;
  int error;
  if ((error = _rpc_message_recv_bytes(message, (unsigned char *)&r_value, sizeof(r_value))) < 0)
	return error;
  *ret = ntohl(r_value);
  D(bug("  recv INT32 %d\n", *ret));
  return RPC_ERROR_NO_ERROR;
}

// Receive UINT32
int rpc_message_recv_uint32(rpc_message_t *message, uint32_t *ret)
{
  uint32_t r_value;
  int error;
  if ((error = _rpc_message_recv_bytes(message, (unsigned char *)&r_value, sizeof(r_value))) < 0)
	return error;
  *ret = ntohl(r_value);
  D(bug("  recv UINT32 %u\n", *ret));
  return RPC_ERROR_NO_ERROR;
}

// Receive UINT64
int rpc_message_recv_uint64(rpc_message_t *message, uint64_t *ret)
{
  uint32_t hi, lo;
  int error;
  if ((error = rpc_message_recv_uint32(message, &hi)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &lo)) < 0)
	return error;
  *((uint64_t *)ret) = (((uint64_t)hi) << 32) | lo;
  D(bug("  recv UINT64 0x%016" PRIx64 "\n", *ret));
  return RPC_ERROR_NO_ERROR;
}

// Receive DOUBLE
int rpc_message_recv_double(rpc_message_t *message, double *ret)
{
  uint32_t negative, exponent, mantissa0, mantissa1;

  int error;
  if ((error = rpc_message_recv_uint32(message, &negative)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &exponent)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &mantissa0)) < 0)
	return error;
  if ((error = rpc_message_recv_uint32(message, &mantissa1)) < 0)
	return error;

  ieee_double_shape_t dbl;
  dbl.parts.negative = negative;
  dbl.parts.exponent = exponent;
  dbl.parts.mantissa0 = mantissa0;
  dbl.parts.mantissa1 = mantissa1;
  *ret = dbl.value;

  D(bug("  recv DOUBLE %g\n", *ret));
  return RPC_ERROR_NO_ERROR;
}

// Receive STRING
int rpc_message_recv_string(rpc_message_t *message, char **ret)
{
  char *str;
  int length;
  uint32_t r_value;
  int error;
  if ((error = _rpc_message_recv_bytes(message, (unsigned char *)&r_value, sizeof(r_value))) < 0)
	return error;
  length = ntohl(r_value);
  if (length == -1)
	str = NULL;
  else {
	if ((str = (char *)malloc(length + 1)) == NULL)
	  return RPC_ERROR_NO_MEMORY;
	if (length > 0) {
	  if ((error = _rpc_message_recv_bytes(message, (unsigned char *)str, length)) < 0)
		return error;
	}
	str[length] = '\0';
  }
  *ret = str;
  D(bug("  recv STRING \"%s\"\n", *ret));
  return RPC_ERROR_NO_ERROR;
}

// Receive message arguments
static int rpc_message_recv_args(rpc_message_t *message, va_list args)
{
  int nargs = 0;
  int expected_type, error;
  rpc_message_descriptor_t *desc;

  while ((expected_type = va_arg(args, int)) != RPC_TYPE_INVALID) {
	if (++nargs == 1) {
	  int32_t value;
	  if ((error = rpc_message_recv_int32(message, &value)) < 0)
		return error;
	  if (value != RPC_MESSAGE_ARGS)
		return RPC_ERROR_MESSAGE_TRUNCATED;
	}
	void *p_value = va_arg(args, void *);
	int32_t type;
	if ((error = rpc_message_recv_int32(message, &type)) < 0)
	  return error;
	if (type != expected_type)
	  return RPC_ERROR_MESSAGE_ARGUMENT_MISMATCH;
	switch (type) {
	case RPC_TYPE_CHAR:
	  error = rpc_message_recv_char(message, (char *)p_value);
	  break;
	case RPC_TYPE_BOOLEAN:
	case RPC_TYPE_INT32:
	  error = rpc_message_recv_int32(message, (int32_t *)p_value);
	  break;
	case RPC_TYPE_UINT32:
	  error = rpc_message_recv_uint32(message, (uint32_t *)p_value);
	  break;
	case RPC_TYPE_UINT64:
	  error = rpc_message_recv_uint64(message, (uint64_t *)p_value);
	  break;
	case RPC_TYPE_DOUBLE:
	  error = rpc_message_recv_double(message, (double *)p_value);
	  break;
	case RPC_TYPE_STRING:
	  error = rpc_message_recv_string(message, (char **)p_value);
	  break;
	case RPC_TYPE_ARRAY: {
	  int i;
	  int32_t array_type;
	  uint32_t array_size;
	  char is_valid_array;
	  if ((error = rpc_message_recv_int32(message, &array_type)) < 0)
		return error;
	  if ((error = rpc_message_recv_uint32(message, &array_size)) < 0)
		return error;
	  if ((error = rpc_message_recv_char(message, &is_valid_array)) < 0)
		return error;
	  p_value = va_arg(args, void *);
	  *((int32_t *)p_value) = array_size;
	  p_value = va_arg(args, void *);
	  if (!is_valid_array) {
		*((void **)p_value) = NULL;
		break;
	  }
	  assert(array_size > 0); // otherwise, it's an internal error, see send()
	  switch (array_type) {
	  case RPC_TYPE_CHAR: {
		unsigned char *array;
		if ((array = (unsigned char *)malloc(array_size * sizeof(*array))) == NULL)
		  return RPC_ERROR_NO_MEMORY;
		error = _rpc_message_recv_bytes(message, array, array_size);
		if (error != RPC_ERROR_NO_ERROR)
		  return error;
		*((void **)p_value) = (void *)array;
		break;
	  }
	  case RPC_TYPE_BOOLEAN:
	  case RPC_TYPE_INT32: {
		int *array;
		if ((array = (int *)malloc(array_size * sizeof(*array))) == NULL)
		  return RPC_ERROR_NO_MEMORY;
		for (i = 0; i < array_size; i++) {
		  int32_t value;
		  if ((error = rpc_message_recv_int32(message, &value)) < 0)
			return error;
		  array[i] = value;
		}
		*((void **)p_value) = (void *)array;
		break;
	  }
	  case RPC_TYPE_UINT32: {
		uint32_t *array;
		if ((array = (uint32_t *)malloc(array_size * sizeof(*array))) == NULL)
		  return RPC_ERROR_NO_MEMORY;
		for (i = 0; i < array_size; i++) {
		  uint32_t value;
		  if ((error = rpc_message_recv_uint32(message, &value)) < 0)
			return error;
		  array[i] = value;
		}
		*((void **)p_value) = (void *)array;
		break;
	  }
	  case RPC_TYPE_UINT64: {
		uint64_t *array;
		if ((array = (uint64_t *)malloc(array_size * sizeof(*array))) == NULL)
		  return RPC_ERROR_NO_MEMORY;
		for (i = 0; i < array_size; i++) {
		  uint64_t value;
		  if ((error = rpc_message_recv_uint64(message, &value)) < 0)
			return error;
		  array[i] = value;
		}
		*((void **)p_value) = (void *)array;
		break;
	  }
	  case RPC_TYPE_DOUBLE: {
		double *array;
		if ((array = (double *)malloc(array_size * sizeof(*array))) == NULL)
		  return RPC_ERROR_NO_MEMORY;
		for (i = 0; i < array_size; i++) {
		  double value;
		  if ((error = rpc_message_recv_double(message, &value)) < 0)
			return error;
		  array[i] = value;
		}
		*((void **)p_value) = (void *)array;
		break;
	  }
	  case RPC_TYPE_STRING: {
		char **array;
		if ((array = (char **)malloc(array_size * sizeof(*array))) == NULL)
		  return RPC_ERROR_NO_MEMORY;
		for (i = 0; i < array_size; i++) {
		  char *str;
		  if ((error = rpc_message_recv_string(message, &str)) < 0)
			return error;
		  array[i] = str;
		}
		*((void **)p_value) = (void *)array;
		break;
	  }
	  default:
		if ((desc = rpc_message_descriptor_lookup(message, array_type)) != NULL) {
		  // arguments are passed by value (XXX: needs a way to differenciate reference/value)
		  uint8_t *array;
		  if ((array = (uint8_t *)malloc(array_size * desc->size)) == NULL)
			return RPC_ERROR_NO_MEMORY;
		  for (i = 0; i < array_size; i++) {
			if ((error = desc->recv_callback(message, &array[i * desc->size])) < 0)
			  return error;
		  }
		  *((void **)p_value) = array;
		}
		else {
		  fprintf(stderr, "unknown array arg type %d to receive\n", type);
		  error = RPC_ERROR_MESSAGE_ARGUMENT_UNKNOWN;
		}
		break;
	  }
	  break;
	}
	default:
	  if ((desc = rpc_message_descriptor_lookup(message, type)) != NULL)
		error = desc->recv_callback(message, p_value);
	  else {
		fprintf(stderr, "unknown arg type %d to send\n", type);
		error = RPC_ERROR_MESSAGE_ARGUMENT_UNKNOWN;
	  }
	  break;
	}
	if (error != RPC_ERROR_NO_ERROR)
	  return error;
  }
  if (nargs) {
	int32_t value;
	if ((error = rpc_message_recv_int32(message, &value)) < 0)
	  return error;
	if (value != RPC_MESSAGE_END)
	  return RPC_ERROR_MESSAGE_TRUNCATED;
  }
  return RPC_ERROR_NO_ERROR;
}

static inline rpc_method_callback_t rpc_lookup_callback(rpc_connection_t *connection, int method)
{
  return (rpc_method_callback_t)rpc_map_lookup(connection->methods, method);
}

// Dispatch message received in the server loop
static int _rpc_dispatch_1(rpc_connection_t *connection, rpc_message_t *message)
{
  // recv: <invoke> (body: <method-id> MESSAGE_END
  D(bug("receiving message\n"));
  int32_t method;
  int error = rpc_message_recv_int32(message, &method);
  if (error != RPC_ERROR_NO_ERROR)
	return error;
  int32_t msg_tag;
  error = rpc_message_recv_int32(message, &msg_tag);
  if (error != RPC_ERROR_NO_ERROR)
	return error;
  if (msg_tag != RPC_MESSAGE_END)
	return RPC_ERROR_MESSAGE_TYPE_INVALID;
  D(bug("  -- message received [%d]\n", method));

  // call: <method>
  rpc_method_callback_t callback = rpc_lookup_callback(connection, method);
  if (callback)
	error = callback(connection);
  else
	error = RPC_ERROR_MESSAGE_HANDLER_INVALID;
  if (error != RPC_ERROR_NO_ERROR) {
	int error_code = error;

	// send: MESSAGE_FAILURE <error-code> ("high-level" error codes only)
	switch (error) {
	case RPC_ERROR_GENERIC:
	case RPC_ERROR_ERRNO_SET:
	case RPC_ERROR_NO_MEMORY:
	  error = rpc_message_send_int32(message, RPC_MESSAGE_FAILURE);
	  if (error != RPC_ERROR_NO_ERROR)
		return error;
	  error = rpc_message_send_int32(message, error_code);
	  if (error != RPC_ERROR_NO_ERROR)
		return error;
	  error = rpc_message_flush(message);
	  if (error != RPC_ERROR_NO_ERROR)
		return error;
	  break;
	}
	return error_code;
  }

  return method;
}

static int _rpc_dispatch(rpc_connection_t *connection, rpc_message_t *message)
{
  ++connection->dispatch_depth;
  int ret = _rpc_dispatch_1(connection, message);
  --connection->dispatch_depth;
  return ret;
}

// Dispatch pending remote calls until we get MSG_TAG
static int _rpc_dispatch_until(rpc_connection_t *connection, rpc_message_t *message, int32_t expected_msg_tag)
{
  assert(expected_msg_tag != 0);
  for (;;) {
	int32_t msg_tag;
	int error = rpc_message_recv_int32(message, &msg_tag);
	if (error != RPC_ERROR_NO_ERROR)
	  return error;
	if (msg_tag == expected_msg_tag)
	  break;
	switch (msg_tag) {
	case RPC_MESSAGE_SYNC:
	  if (!_rpc_connection_is_sync_allowed(connection))
		return RPC_ERROR_MESSAGE_SYNC_NOT_ALLOWED;
	  connection->pending_sync_depth = connection->invoke_depth;
	  break;
	case RPC_MESSAGE_START:
	  if ((error = _rpc_dispatch(connection, message)) < 0)
		return error;
	  break;
	default:
	  return RPC_ERROR_MESSAGE_TYPE_INVALID;
	}
  }
  return RPC_ERROR_NO_ERROR;
}

// Dispatch pending remote calls in SYNC mode (i.e. until MESSAGE_SYNC_END)
static int _rpc_dispatch_sync(rpc_connection_t *connection)
{
  rpc_message_t message;
  rpc_message_init(&message, connection);

  // send: MESSAGE_SYNC_ACK
  int error = rpc_message_send_int32(&message, RPC_MESSAGE_SYNC_ACK);
  if (error != RPC_ERROR_NO_ERROR)
	return error;
  error = rpc_message_flush(&message);
  if (error != RPC_ERROR_NO_ERROR)
	return error;

  // call: rpc_dispatch() (pending remote calls)
  // recv: MESSAGE_SYNC_END
  GTimer *timer = g_timer_new();
  error = _rpc_dispatch_until(connection, &message, RPC_MESSAGE_SYNC_END);
  D(bug("blocked for %lf seconds for SYNC_END\n", g_timer_elapsed(timer, NULL)));
  g_timer_destroy(timer);
  return error;
}

// Dispatch message received in the server loop (public entry point)
int rpc_dispatch(rpc_connection_t *connection)
{
  int32_t msg_tag;
  rpc_message_t message;
  rpc_message_init(&message, connection);

  // recv: <invoke> (header: MESSAGE_START)
  int error = rpc_message_recv_int32(&message, &msg_tag);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  if (msg_tag == RPC_MESSAGE_SYNC) // optional
	return _rpc_dispatch_sync(connection);
  if (msg_tag != RPC_MESSAGE_START)
	return rpc_error(connection, RPC_ERROR_MESSAGE_TYPE_INVALID);

  int method = _rpc_dispatch(connection, &message);
  if (method < 0)
	return rpc_error(connection, method);
  return method;
}

// Returns true if we have a pending SYNC or SYNC_ACK to reply to.
static bool rpc_has_pending_sync(rpc_connection_t *connection)
{
  // Don't run this on nested message loops. Chromium seems to pump
  // the message loop in the plugin process when waiting for
  // (Chromium-internal) IPC.
  if (connection->invoke_depth > 0 || connection->handle_depth > 0) {
	D(bug("rpc_has_pending_sync called on a nested event loop!\n"));
	return false;
  }
  return connection->pending_sync_depth;
}

// Close any pending sync requests. This should be called from the
// event loop. In the wrapper, if we received a SYNC, we SYNC_ACK it
// now and wait for the SYNC_END. This is so interleaving is
// done at event loop iteration boudaries.
int rpc_dispatch_pending_sync(rpc_connection_t *connection)
{
  D(bug("rpc_dispatch_pending_sync\n"));
  // Don't run this on nested message loops. Chromium seems to pump
  // the message loop in the plugin process when waiting for
  // (Chromium-internal) IPC.
  if (connection->invoke_depth > 0 || connection->handle_depth > 0) {
	D(bug("rpc_dispatch_pending_sync called on a nested event loop!\n"));
	return RPC_ERROR_NO_ERROR;
  }

  // send: MESSAGE_SYNC_ACK (pending message)
  if (connection->pending_sync_depth) {
	D(bug("sending delayed MESSAGE_SYNC_ACK\n"));
	assert(connection->pending_sync_depth == 1);
	assert(_rpc_wait_dispatch(connection, 0) == 0);

	connection->pending_sync_depth = 0;
	return _rpc_dispatch_sync(connection);
  }

  return RPC_ERROR_NO_ERROR;
}

int rpc_sync(rpc_connection_t *connection)
{
  rpc_message_t message;

  /* Strategy to synchronize the connections.

	 The purpose of connection synchronization is to ensure that the
	 other end is ready to receive our rpc_method_invoke() calls,
	 i.e. make sure it is not already processing RPC, or has not
	 started an rpc_method_invoke() call itself.

	 More importantly, it is to ensure that, at any point, at most one
	 of the wrapper and viewer are running at a time. NPAPI is a
	 synchronous API and thus assumes the two threads run on the same
	 event loop. For instance, if the browser requests two paints in a
	 row, we must ensure the plugin does dispatch an event loop source
	 in between.

	 To that end, the viewer must ask permission of the wrapper before
	 doing anything on the plugin thread:

	 - Viewer sends MSG_SYNC, and block until we get MSG_SYNC_ACK
	 - Process any pending wrapper calls while we are blocked
	 - After MSG_SYNC_ACK, run an event loop iteration and send MSG_SYNC_END

	 When receiving a MSG_SYNC, the wrapper responds:

	 - If MSG_SYNC was received in rpc_dispatch (at the top of an
       event loop), MSG_SYNC_ACK immediately.
	 - Otherwise, make a note and respond when we next return to the
       event loop.
  */

  D(bug("rpc_sync\n"));
  assert(!connection->is_sync);

  // send: MESSAGE_SYNC
  rpc_message_init(&message, connection);
  int error = rpc_message_send_int32(&message, RPC_MESSAGE_SYNC);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_flush(&message);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);

  GTimer *timer = g_timer_new();
  // call: rpc_dispatch() (pending remote calls)
  error = _rpc_dispatch_until(connection, &message, RPC_MESSAGE_SYNC_ACK);
  D(bug("blocked for %lf seconds for SYNC_ACK\n", g_timer_elapsed(timer, NULL)));
  g_timer_destroy(timer);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);

  connection->is_sync = true;
  return RPC_ERROR_NO_ERROR;
}

int rpc_end_sync(rpc_connection_t *connection)
{
  D(bug("rpc_end_sync\n"));

  if (!connection->is_sync) {
	// This sometimes triggers when we kill the wrapper at a bad time.
	npw_printf("ERROR: rpc_end_sync called when not in sync!\n");
	return rpc_error(connection, RPC_ERROR_GENERIC);
  }

  // send: MESSAGE_SYNC_END (done pending message)
  rpc_message_t message;
  rpc_message_init(&message, connection);
  // send: MESSAGE_SYNC_END
  int error = rpc_message_send_int32(&message, RPC_MESSAGE_SYNC_END);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_flush(&message);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);

  connection->is_sync = false;

  return RPC_ERROR_NO_ERROR;
}


/* ====================================================================== */
/* === Method Callbacks Handling                                      === */
/* ====================================================================== */

// Add a user-defined method callback (server side)
int rpc_connection_add_method_descriptor(rpc_connection_t *connection, const rpc_method_descriptor_t *idesc)
{
  D(bug("rpc_connection_add_method_descriptor for method %d\n", idesc->id));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;

  rpc_method_callback_t callback;
  if ((callback = (rpc_method_callback_t)rpc_map_lookup(connection->methods, idesc->id)) != NULL) {
	if (callback == idesc->callback)
	  return RPC_ERROR_NO_ERROR;
	fprintf(stderr, "duplicate method %d\n", idesc->id);
	return RPC_ERROR_GENERIC;
  }

  return rpc_map_insert(connection->methods, idesc->id, (void *)idesc->callback);
}

// Add user-defined method callbacks (server side)
int rpc_connection_add_method_descriptors(rpc_connection_t *connection, const rpc_method_descriptor_t *descs, int n_descs)
{
  D(bug("rpc_connection_add_method_descriptors\n"));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;

  while (--n_descs >= 0) {
	int error = rpc_connection_add_method_descriptor(connection, &descs[n_descs]);
	if (error != RPC_ERROR_NO_ERROR)
	  return error;
  }

  return RPC_ERROR_NO_ERROR;
}


/* ====================================================================== */
/* === Remote Procedure Call (method invocation)                      === */
/* ====================================================================== */

// Returns whether it is possible to rpc_method_invoke() now or not
bool rpc_method_invoke_possible(rpc_connection_t *connection)
{
  if (rpc_status(connection) != RPC_STATUS_ACTIVE)
	return false;
  /* XXX: at this time, we can can call rpc_method_invoke() only if we
	 are not processing incoming calls already or if we are
	 "synchronized" with the other side. i.e. we are between an
	 rpc_method_get_args() and rpc_method_send_reply() in an RPC handler
	 function.  */
  return connection->dispatch_depth == connection->handle_depth;
}

// Invoke remote procedure (client side)
static int _rpc_method_invoke_valist(rpc_connection_t *connection, int method, va_list args)
{
  rpc_message_t message;
  rpc_message_init(&message, connection);

  // send: <invoke> = MESSAGE_START <method-id> MESSAGE_END
  int error = rpc_message_send_int32(&message, RPC_MESSAGE_START);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_send_int32(&message, method);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_send_int32(&message, RPC_MESSAGE_END);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_flush(&message);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);

  // send optional arguments
  va_list args_copy;
  va_copy(args_copy, args);
  int arg_type = va_arg(args, int);
  if (arg_type != RPC_TYPE_INVALID) {

	// send: <method-args> = MESSAGE_ARGS [ <arg-type> <arg-value> ]+ MESSAGE_END
	error = rpc_message_send_args(&message, args_copy);
	va_end(args_copy);
	if (error != RPC_ERROR_NO_ERROR)
	  return rpc_error(connection, error);
	error = rpc_message_flush(&message);
	if (error != RPC_ERROR_NO_ERROR)
	  return rpc_error(connection, error);
  }

  return RPC_ERROR_NO_ERROR;
}

int rpc_method_invoke(rpc_connection_t *connection, int method, ...)
{
  D(bug("rpc_method_invoke method=%d\n", method));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;
  if (_rpc_status(connection) == RPC_STATUS_CLOSED)
	return RPC_ERROR_CONNECTION_CLOSED;

  ++connection->invoke_depth;

  va_list args;
  va_start(args, method);
  int ret = _rpc_method_invoke_valist(connection, method, args);
  va_end(args);

  return ret;
}

// Retrieve procedure arguments (server side)
static int _rpc_method_get_args_valist(rpc_connection_t *connection, va_list args)
{
  rpc_message_t message;
  rpc_message_init(&message, connection);

  // we can't have pending calls here because the method invocation
  // message and its arguments are atomic (i.e. they are sent right
  // away, no wait for ACK)

  // recv: <method-args>
  int error = rpc_message_recv_args(&message, args);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);

  return RPC_ERROR_NO_ERROR;
}

int rpc_method_get_args(rpc_connection_t *connection, ...)
{
  D(bug("rpc_method_get_args\n"));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;
  if (_rpc_status(connection) == RPC_STATUS_CLOSED)
	return RPC_ERROR_CONNECTION_CLOSED;

  ++connection->handle_depth;

  va_list args;
  va_start(args, connection);
  int ret = _rpc_method_get_args_valist(connection, args);
  va_end(args);

  return ret;
}

// Wait for a reply from the remote procedure (client side)
static int _rpc_method_wait_for_reply_valist(rpc_connection_t *connection, va_list args)
{
  int error;
  int32_t msg_tag;
  rpc_message_t message;
  rpc_message_init(&message, connection);

  // call: rpc_dispatch() (pending remote calls)
  // recv: MESSAGE_REPLY
  error = _rpc_dispatch_until(connection, &message, RPC_MESSAGE_REPLY);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);

  // receive optional arguments
  va_list args_copy;
  va_copy(args_copy, args);
  int type = va_arg(args, int);
  if (type != RPC_TYPE_INVALID) {

	// recv: [ <method-args> ]
	error = rpc_message_recv_args(&message, args_copy);
	va_end(args_copy);
	if (error != RPC_ERROR_NO_ERROR)
	  return rpc_error(connection, error);
  }

  // recv: MESSAGE_END
  error = rpc_message_recv_int32(&message, &msg_tag);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  if (msg_tag != RPC_MESSAGE_END)
	return rpc_error(connection, RPC_ERROR_MESSAGE_TYPE_INVALID);

  return RPC_ERROR_NO_ERROR;
}

int rpc_method_wait_for_reply(rpc_connection_t *connection, ...)
{
  D(bug("rpc_method_wait_for_reply\n"));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;
  if (_rpc_status(connection) == RPC_STATUS_CLOSED)
	return RPC_ERROR_CONNECTION_CLOSED;

  va_list args;
  va_start(args, connection);
  int ret = _rpc_method_wait_for_reply_valist(connection, args);
  va_end(args);

  --connection->invoke_depth;

  return ret;
}

// Send a reply to the client (server side)
static int _rpc_method_send_reply_valist(rpc_connection_t *connection, va_list args)
{
  rpc_message_t message;
  rpc_message_init(&message, connection);

  // send: <reply> = MESSAGE_REPLY [ <method-args> ] MESSAGE_END
  int error = rpc_message_send_int32(&message, RPC_MESSAGE_REPLY);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_send_args(&message, args);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_send_int32(&message, RPC_MESSAGE_END);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);
  error = rpc_message_flush(&message);
  if (error != RPC_ERROR_NO_ERROR)
	return rpc_error(connection, error);

  return RPC_ERROR_NO_ERROR;
}

int rpc_method_send_reply(rpc_connection_t *connection, ...)
{
  D(bug("rpc_method_send_reply\n"));

  if (connection == NULL)
	return RPC_ERROR_CONNECTION_NULL;
  if (_rpc_status(connection) == RPC_STATUS_CLOSED)
	return RPC_ERROR_CONNECTION_CLOSED;

  va_list args;
  va_start(args, connection);
  int ret = _rpc_method_send_reply_valist(connection, args);
  va_end(args);

  --connection->handle_depth;

  return ret;
}


/* ====================================================================== */
/* === Remote Procedure Call (method invocation)                      === */
/* ====================================================================== */

typedef struct _RpcSource {
  GSource parent;
  rpc_connection_t *connection;
  GPollFD poll_fd;
} RpcSource;

static gboolean rpc_event_prepare(GSource *source, gint *timeout)
{
  *timeout = -1;
  return FALSE;
}

static gboolean rpc_event_check(GSource *source)
{
  RpcSource *rsource = (RpcSource *) source;
  return rpc_wait_dispatch(rsource->connection, 0) > 0;
}

static gboolean rpc_event_dispatch(GSource *source, GSourceFunc callback, gpointer data)
{
  RpcSource *rsource = (RpcSource *) source;
  return rpc_dispatch(rsource->connection) != RPC_ERROR_CONNECTION_CLOSED;
}

static void rpc_event_finalize(GSource *source)
{
  RpcSource *rsource = (RpcSource *) source;
  rpc_connection_unref(rsource->connection);
}

static GSourceFuncs rpc_event_funcs = {
  rpc_event_prepare,
  rpc_event_check,
  rpc_event_dispatch,
  rpc_event_finalize,
  (GSourceFunc)NULL,
  (GSourceDummyMarshal)NULL
};

GSource *rpc_event_source_new(rpc_connection_t *connection)
{
  GSource *source;
  RpcSource *rsource;

  source = g_source_new(&rpc_event_funcs, sizeof(RpcSource));
  rsource = (RpcSource*)source;

  rsource->connection = rpc_connection_ref(connection);
  rsource->poll_fd.fd = rpc_socket(connection);
  rsource->poll_fd.events = G_IO_IN;
  rsource->poll_fd.revents = 0;
  g_source_add_poll(source, &rsource->poll_fd);

  return source;
}

typedef struct _RpcSyncSource {
  GSource parent;
  rpc_connection_t *connection;
} RpcSyncSource;

static gboolean rpc_sync_prepare(GSource *source, gint *timeout)
{
  RpcSyncSource *rsource = (RpcSyncSource *) source;
  if (rpc_has_pending_sync(rsource->connection)) {
	*timeout = 0;
	return TRUE;
  }
  *timeout = -1;
  return FALSE;
}

static gboolean rpc_sync_check(GSource *source)
{
  RpcSyncSource *rsource = (RpcSyncSource *) source;
  return rpc_has_pending_sync(rsource->connection);
}

static gboolean rpc_sync_dispatch(GSource *source, GSourceFunc callback, gpointer data)
{
  RpcSyncSource *rsource = (RpcSyncSource *) source;
  return rpc_dispatch_pending_sync(rsource->connection) != RPC_ERROR_CONNECTION_CLOSED;
}

static void rpc_sync_finalize(GSource *source)
{
  RpcSyncSource *rsource = (RpcSyncSource *) source;
  rpc_connection_unref(rsource->connection);
}

static GSourceFuncs rpc_sync_funcs = {
  rpc_sync_prepare,
  rpc_sync_check,
  rpc_sync_dispatch,
  rpc_sync_finalize,
  (GSourceFunc)NULL,
  (GSourceDummyMarshal)NULL
};

GSource *rpc_sync_source_new(rpc_connection_t *connection)
{
  GSource *source;
  RpcSyncSource *rsource;

  source = g_source_new(&rpc_sync_funcs, sizeof(RpcSyncSource));
  rsource = (RpcSyncSource*)source;
  rsource->connection = rpc_connection_ref(connection);

  return source;
}


/* ====================================================================== */
/* === Test Program                                                   === */
/* ====================================================================== */

#ifdef TEST_RPC
// User-defined method IDs
enum {
  TEST_RPC_METHOD_ADD = 1,
  TEST_RPC_METHOD_CHILD,
  TEST_RPC_METHOD_PID,
  TEST_RPC_METHOD_ECHO,
  TEST_RPC_METHOD_PRINT,
  TEST_RPC_METHOD_STRINGS,
  TEST_RPC_METHOD_POINTS,
  TEST_RPC_METHOD_EXIT,
};

// User-defined marshalers
enum {
  RPC_TYPE_POINT = 100,
};

struct Point {
  int x, y;
};

static int do_send_point(rpc_message_t *message, void *p_value)
{
  D(bug("do_send_point\n"));

  struct Point *pt = (struct Point *)p_value;
  int error;

  if ((error = rpc_message_send_int32(message, pt->x)) < 0)
	return error;
  if ((error = rpc_message_send_int32(message, pt->y)) < 0)
	return error;
  return RPC_ERROR_NO_ERROR;
}

static int do_recv_point(rpc_message_t *message, void *p_value)
{
  D(bug("do_recv_point\n"));

  struct Point *pt = (struct Point *)p_value;
  int error;
  int32_t value;

  if ((error = rpc_message_recv_int32(message, &value)) < 0)
	return error;
  pt->x = value;
  if ((error = rpc_message_recv_int32(message, &value)) < 0)
	return error;
  pt->y = value;
  return RPC_ERROR_NO_ERROR;
}

static const rpc_message_descriptor_t point_desc = {
  RPC_TYPE_POINT,
  sizeof(struct Point),
  do_send_point,
  do_recv_point
};

// Global connections
static rpc_connection_t *g_npn_connection;
static char g_npn_connection_path[PATH_MAX];
static rpc_connection_t *g_npp_connection;
static char g_npp_connection_path[PATH_MAX];
static int g_client_pid;
static int g_server_pid;

// handle ADD(INT32, INT32, INT32) -> INT32
static int handle_ADD(rpc_connection_t *connection)
{
  D(bug("handle_ADD\n"));

  int error;
  int32_t a, b, c;
  if ((error = rpc_method_get_args(connection, RPC_TYPE_INT32, &a, RPC_TYPE_INT32, &b, RPC_TYPE_INT32, &c, RPC_TYPE_INVALID)) < 0)
	return error;

  printf("  > %d, %d, %d\n", a, b, c);
  return rpc_method_send_reply(connection, RPC_TYPE_INT32, a + b + c, RPC_TYPE_INVALID);
}

// handle PID(VOID) -> INT32
static int handle_server_PID(rpc_connection_t *connection)
{
  D(bug("handle_server_PID\n"));
  return rpc_method_send_reply(connection, RPC_TYPE_INT32, g_server_pid, RPC_TYPE_INVALID);
}

static int handle_client_PID(rpc_connection_t *connection)
{
  D(bug("handle_client_PID\n"));
  return rpc_method_send_reply(connection, RPC_TYPE_INT32, g_client_pid, RPC_TYPE_INVALID);
}

// handle CHILD(VOID) -> INT32
static int handle_CHILD(rpc_connection_t *connection)
{
  D(bug("handle_CHILD\n"));

  int error;
  int32_t pid;
  if ((error = rpc_method_invoke(g_npp_connection, TEST_RPC_METHOD_PID, RPC_TYPE_INVALID)) < 0)
	return error;
  if ((error = rpc_method_wait_for_reply(g_npp_connection, RPC_TYPE_INT32, &pid, RPC_TYPE_INVALID)) < 0)
	return error;
  printf("  > %d\n", pid);
  return rpc_method_send_reply(connection, RPC_TYPE_INT32, pid + 1, RPC_TYPE_INVALID);
}

// handle ECHO(STRING) -> VOID
static int handle_ECHO(rpc_connection_t *connection)
{
  D(bug("handle_ECHO\n"));

  int error;
  char *str;
  if ((error = rpc_method_get_args(connection, RPC_TYPE_STRING, &str, RPC_TYPE_INVALID)) < 0)
	return error;

  printf("  > %s\n", str);
  free(str);
  return RPC_ERROR_NO_ERROR;
}

// handle PRINT(STRING, UINT64, DOUBLE) -> VOID
static int handle_PRINT(rpc_connection_t *connection)
{
  D(bug("handle_PRINT\n"));

  char *str;
  uint64_t val;
  double dbl;

  int error = rpc_method_get_args(connection,
								  RPC_TYPE_STRING, &str,
								  RPC_TYPE_UINT64, &val,
								  RPC_TYPE_DOUBLE, &dbl,
								  RPC_TYPE_INVALID);
  if (error < 0)
	return error;

  printf("  > '%s', 0x%016" PRIx64 ", %f\n", str, val, dbl);
  free(str);
  return RPC_ERROR_NO_ERROR;
}

// handle STRINGS(ARRAY of STRING) -> VOID
static int handle_STRINGS(rpc_connection_t *connection)
{
  D(bug("handle_STRINGS\n"));

  int i, error;
  char **strtab;
  int32_t strtab_length;
  if ((error = rpc_method_get_args(connection, RPC_TYPE_ARRAY, RPC_TYPE_STRING, &strtab_length, &strtab, RPC_TYPE_INVALID)) < 0)
	return error;

  for (i = 0; i < strtab_length; i++) {
	char *str = strtab[i];
	printf("  > %s\n", str);
	free(str);
  }
  free(strtab);
  return RPC_ERROR_NO_ERROR;
}

// handle POINTS(ARRAY of STRING) -> POINT
static int handle_POINTS(rpc_connection_t *connection)
{
  D(bug("handle_POINTS\n"));

  int i, error;
  struct Point *pttab;
  int32_t pttab_length;
  if ((error = rpc_method_get_args(connection, RPC_TYPE_ARRAY, RPC_TYPE_POINT, &pttab_length, &pttab, RPC_TYPE_INVALID)) < 0)
	return error;

  struct Point ptret = { 0, 0 };
  for (i = 0; i < pttab_length; i++) {
	struct Point *pt = &pttab[i];
	printf("  > { %d, %d }\n", pt->x, pt->y);
	ptret.x += pt->x;
	ptret.y += pt->y;
  }
  free(pttab);

  return rpc_method_send_reply(connection, RPC_TYPE_POINT, &ptret, RPC_TYPE_INVALID);
}

// handle EXIT(VOID) -> VOID
static int handle_EXIT(rpc_connection_t *connection)
{
  D(bug("handle_EXIT\n"));

  return RPC_ERROR_NO_ERROR;
}

// Run server (NPN aka browser-side)
static int run_server(void)
{
  rpc_connection_t *connection;

  g_server_pid = getpid();
  printf("Server PID: %d\n", g_server_pid);

  if ((connection = rpc_init_server(g_npn_connection_path)) == NULL) {
	fprintf(stderr, "ERROR: failed to initialize RPC server connection to NPN\n");
	return 0;
  }
  g_npn_connection = connection;

  if (rpc_connection_add_message_descriptors(g_npn_connection, &point_desc, 1) < 0) {
	fprintf(stderr, "ERROR: failed to add server-side Point marshaler\n");
	return 0;
  }

  if ((g_npp_connection = rpc_init_client(g_npp_connection_path)) == NULL) {
	fprintf(stderr, "ERROR: failed to initialize RPC server connection to NPP\n");
	return 0;
  }

  if (rpc_connection_add_message_descriptors(g_npp_connection, &point_desc, 1) < 0) {
	fprintf(stderr, "ERROR: failed to add client-side Point marshaler\n");
	return 0;
  }

  static const rpc_method_descriptor_t vtable1[] = {
	{ TEST_RPC_METHOD_ADD,		handle_ADD },
	{ TEST_RPC_METHOD_ECHO,		handle_ECHO },
	{ TEST_RPC_METHOD_PRINT,	handle_PRINT },
	{ TEST_RPC_METHOD_PID,		handle_server_PID },
	{ TEST_RPC_METHOD_EXIT,		handle_EXIT },
  };
  if (rpc_connection_add_method_descriptors(connection, vtable1, sizeof(vtable1) / sizeof(vtable1[0])) < 0) {
	fprintf(stderr, "ERROR: failed to setup method callbacks\n");
	return 0;
  }
  if (rpc_connection_remove_method_descriptor(connection, TEST_RPC_METHOD_PID) < 0) {
	fprintf(stderr, "ERROR: failed to remove superfluous callback %d\n", TEST_RPC_METHOD_PID);
	return 0;
  }
  static const rpc_method_descriptor_t vtable2[] = {
	{ TEST_RPC_METHOD_CHILD,	handle_CHILD },
	{ TEST_RPC_METHOD_STRINGS,	handle_STRINGS },
	{ TEST_RPC_METHOD_POINTS,	handle_POINTS },
  };
  if (rpc_connection_add_method_descriptors(connection, vtable2, sizeof(vtable2) / sizeof(vtable2[0])) < 0) {
	fprintf(stderr, "ERROR: failed to setup method callbacks\n");
	return 0;
  }
  if (rpc_listen(connection) < 0) {
	fprintf(stderr, "ERROR: failed to initialize RPC server thread\n");
	return 0;
  }

  printf("Waiting for client to terminate\n");
  int status;
  while (waitpid(g_client_pid, &status, 0) != g_client_pid)
	;
  if (WIFEXITED(status))
	printf("  client exitted with status=%d\n", WEXITSTATUS(status));
  rpc_exit(g_npp_connection);
  printf("  client connection closed\n");

  rpc_exit(g_npn_connection);
  printf("Server exitted\n");
  return 1;
}

// Run client (NPP aka plugin-side)
static int run_client(void)
{
  rpc_connection_t *connection;
  int error;

  g_client_pid = getpid();
  printf("Client PID: %d\n", g_client_pid);

  if ((connection = rpc_init_client(g_npn_connection_path)) == NULL) {
	fprintf(stderr, "ERROR: failed to initialize RPC client connection to NPN\n");
	return 0;
  }
  g_npn_connection = connection;

  if (rpc_connection_add_message_descriptors(g_npn_connection, &point_desc, 1) < 0) {
	fprintf(stderr, "ERROR: failed to add server-side Point marshaler\n");
	return 0;
  }

  if ((g_npp_connection = rpc_init_server(g_npp_connection_path)) == NULL) {
	fprintf(stderr, "ERROR: failed to initialize RPC server connection to NPP\n");
	return 0;
  }

  if (rpc_connection_add_message_descriptors(g_npp_connection, &point_desc, 1) < 0) {
	fprintf(stderr, "ERROR: failed to add client-side Point marshaler\n");
	return 0;
  }

  static const rpc_method_descriptor_t vtable[] = {
	{ TEST_RPC_METHOD_PID,		handle_client_PID },
	{ TEST_RPC_METHOD_EXIT,		handle_EXIT },
  };
  if (rpc_connection_add_method_descriptors(g_npp_connection, vtable, sizeof(vtable) / sizeof(vtable[0])) < 0) {
	fprintf(stderr, "ERROR: failed to setup method callbacks\n");
	return 0;
  }
  if (rpc_listen(g_npp_connection) < 0) {
	fprintf(stderr, "ERROR: failed to initialize RPC server thread\n");
	return 0;
  }

  printf("Call CHILD\n");
  int32_t pid;
  if ((error = rpc_method_invoke(connection, TEST_RPC_METHOD_CHILD, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to send CHILD message [%d]\n", error);
	return 0;
  }
  if ((error = rpc_method_wait_for_reply(connection, RPC_TYPE_INT32, &pid, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to receive CHILD reply [%d]\n", error);
	return 0;
  }
  if (pid != g_client_pid + 1) {
	fprintf(stderr, "ERROR: failed to receive correct pid of this child\n");
	return 0;
  }
  printf("  result: %d\n", pid - 1);

  printf("Call ADD\n");
  int32_t value;
  if ((error = rpc_method_invoke(connection, TEST_RPC_METHOD_ADD, RPC_TYPE_INT32, 1, RPC_TYPE_INT32, 2, RPC_TYPE_INT32, 3, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to send ADD message [%d]\n", error);
	return 0;
  }
  if ((error = rpc_method_wait_for_reply(connection, RPC_TYPE_INT32, &value, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to receive ADD reply [%d]\n", error);
	return 0;
  }
  printf("  result: %d\n", value);
  printf("  done\n");

  printf("Call ECHO\n");
  const char *str = "Coucou";
  if ((error = rpc_method_invoke(connection, TEST_RPC_METHOD_ECHO, RPC_TYPE_STRING, str, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to send ECHO message [%d]\n", error);
	return 0;
  }
  if ((error = rpc_method_wait_for_reply(connection, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to receive ECHO ack [%d]\n", error);
	return 0;
  }
  printf("  done\n");

  printf("Call PRINT\n");
  error = rpc_method_invoke(connection,
							TEST_RPC_METHOD_PRINT,
							RPC_TYPE_STRING, "A string",
							RPC_TYPE_UINT64, 0x0123456789abcdefull,
							RPC_TYPE_DOUBLE, 3.14159265358979323846,
							RPC_TYPE_INVALID);
  if (error < 0) {
	fprintf(stderr, "ERROR: failed to send PRINT message [%d]\n", error);
	return 0;
  }
  if ((error = rpc_method_wait_for_reply(connection, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to receive PRINT ack [%d]\n", error);
	return 0;
  }
  printf("  done\n");

  printf("Call STRINGS\n");
  const char *strtab[] = { "un", "deux", "trois", "quatre" };
  if ((error = rpc_method_invoke(connection, TEST_RPC_METHOD_STRINGS, RPC_TYPE_ARRAY, RPC_TYPE_STRING, 4, strtab, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to send STRINGS message [%d]\n", error);
	return 0;
  }
  if ((error = rpc_method_wait_for_reply(connection, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to receive STRINGS ack [%d]\n", error);
	return 0;
  }
  printf("  done\n");

  printf("Call POINTS\n");
  const struct Point pttab[] = {
	{ -1,  0 },
	{  2, -1 },
	{  1,  4 },
	{ -2, -2 }
  };
  if ((error = rpc_method_invoke(connection, TEST_RPC_METHOD_POINTS, RPC_TYPE_ARRAY, RPC_TYPE_POINT, sizeof(pttab) / sizeof(pttab[0]), pttab, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to send POINTS message [%d]\n", error);
	return 0;
  }
  struct Point pt;
  if ((error = rpc_method_wait_for_reply(connection, RPC_TYPE_POINT, &pt, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to receive POINTS reply [%d]\n", error);
	return 0;
  }
  printf("  result: { %d, %d }\n", pt.x, pt.y);
  printf("  done\n");

  printf("Call EXIT\n");
  if ((error = rpc_method_invoke(connection, TEST_RPC_METHOD_EXIT, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to send EXIT message [%d]\n", error);
	return 0;
  }
  if ((error = rpc_method_wait_for_reply(connection, RPC_TYPE_INVALID)) < 0) {
	fprintf(stderr, "ERROR: failed to receive EXIT ack [%d]\n", error);
	return 0;
  }
  printf("  done\n");

  printf("Sleep 2 seconds\n");
  sleep(2);

  rpc_exit(connection);
  printf("Client exitted\n");
  return 1;
}

int main(void)
{
  sprintf(g_npn_connection_path, "/org/wrapper/NSPlugin/NPN/%d", getpid());
  sprintf(g_npp_connection_path, "/org/wrapper/NSPlugin/NPP/%d", getpid());

  g_client_pid = fork();
  if (g_client_pid == 0) {
	if (!run_client())
	  return 1;
  }
  else {
	if (!run_server())
	  return 1;
  }
  return 0;
}
#endif
