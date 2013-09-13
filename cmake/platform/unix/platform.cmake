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
  set(CONFIGURE_DEBUG --enable-debug --enable-debug_gdb)
  set(MODE_DEBUG mode=debug)
else()
  add_definitions(-DwxDEBUG_LEVEL=0)
  set(CONFIGURE_DEBUG --disable-debug)
  set(MODE_DEBUG mode=release)
endif()

# wxWidgets config
if(MINGW)
  add_definitions(-D__WXMSW__)
else()
  add_definitions(-D__WXGTK__)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(64BIT TRUE)
  message("-- Detected 64bit")
  try_compile(MULTILIB_TEST_COMPILE
    "${CMAKE_TEST_PROJECTS_BIN}/multilib_test"
    "${CMAKE_TEST_PROJECTS}/multilib_test"
    "multilib_test"
    "multilib_test")
  if(MULTILIB_TEST_COMPILE)
    message("-- Working multilib, enable 32bit support")
    option(32BIT_SUPPORT "enable support for 32bit applications (requires 32bit libraries)" ON)
  else()
    message("-- Not working multilib, disable 32bit support")
    option(32BIT_SUPPORT "enable support for 32bit applications (requires 32bit libraries)" OFF)
  endif()
else()
  set(64BIT FALSE)
  message("-- detected 32bit")
endif()

# some mingw things
if(MINGW)
  set(WIN_TARGET 0x0502) # Windows XP SP2
  set(WIN_SDK_MIN 0x0600) # Windows Vista
  set(WIN_IE_VERSION 0x0603) # IE 6 SP2
  add_definitions(-U__STRICT_ANSI__
                  -DWINVER=${WIN_TARGET}
                  -D_WIN32_WINNT=${WIN_SDK_MIN}
                  -D_WIN32_IE=${WIN_IE_VERSION}
                  -DDONT_INCLUDE_AFXWIN
                  -DMINGW_HAS_SECURE_API
                  -DBOOST_THREAD_USE_LIB)
  # link libgcc staticly
  add_compiler_flags(-static-libgcc)
  macro(use_unicode_here)
    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)
    add_definitions(-DwxUSE_UNICODE)
  endmacro()
endif()
