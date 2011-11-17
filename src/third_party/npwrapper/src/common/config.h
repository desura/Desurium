#undef BUILD_GENERIC
#define LIB "lib"
#define LIBDIR "/usr/lib"
#define NPW_SNAPSHOT 0
#define NPW_VERSION "1.4.4"
#define NPW_FULL_VERSION "1.4.4-1"
#define NPW_LIBDIR "/usr/lib/nspluginwrapper"
#define NPW_HOST_LIBDIR "/usr/lib/nspluginwrapper/i386/linux"
#define NPW_TARGET_LIBDIR "/usr/lib/nspluginwrapper/i386/linux"
#define NPW_VIEWER_PATHS "/usr/lib/nspluginwrapper/%ARCH%/%OS%"
#define RPC_INIT_TIMEOUT 5
#define USE_MALLOC_GLIB 1
#define USE_MALLOC_LIBC 1
#define DEFAULT_MALLOC_LIB glib
#define attribute_hidden __attribute__((visibility("hidden")))
#define attribute_protected __attribute__((visibility("protected")))
#define HAVE_INITFINI_ARRAY 1
#undef HAVE_SOCKADDR_UN_SUN_LEN
#undef TARGET_LIBC_PROVIDES_SSP
#undef ENABLE_THREAD_CHECK
#undef ENABLE_MALLOC_CHECK
#define HAVE_G_HASH_TABLE_REMOVE_ALL 1
#define HAVE_G_HASH_TABLE_FIND 1
