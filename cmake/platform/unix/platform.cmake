###############################################################################
# some global defines
###############################################################################

# boost-1.53 changed the variable to disable native scoped enum detection
add_definitions(
  -DBOOST_FILESYSTEM_VERSION=3
  -DBOOST_NO_SCOPED_ENUMS=1
  -DBOOST_NO_CXX11_SCOPED_ENUMS=1
  -D_LARGE_FILES
  -D_FILE_OFFSET_BITS=64
)

set(SCRIPT_PREFIX sh)

###############################################################################
# some compiler flags
###############################################################################

if(DEBUG)
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

if(DEBUG_EXTERNAL)
  add_definitions(-D_DEBUG)
  set(CONFIGURE_DEBUG --enable-debug)
  set(MODE_DEBUG mode=debug)
else()
  add_definitions(-DwxDEBUG_LEVEL=0)
  set(CONFIGURE_DEBUG --disable-debug)
  set(MODE_DEBUG mode=release)
endif()

# wxWidgets config
add_definitions(-D__WXGTK__)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(64BIT TRUE)
  message("-- detected 64bit")
  
  option(32BIT_SUPPORT "enable support for 32bit applications (requires 32bit libraries)" OFF)
else()
  set(64BIT FALSE)
  message("-- detected 32bit")
endif()
