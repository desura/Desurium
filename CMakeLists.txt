###############################################################################
#
# you can pass some options to cmake via -D<option>[=<value>]
#
#   name            default             description
#
# global options:
#   BUILD_CEF       ON                  Build cef
#   BUILD_TESTS     ON                  Build various unit tests
#   BUILD_TOOLS     ON                  Build various Desura tools
#   OFFICIAL_BUILD  OFF                 for official build only
#   BRANDING        depends on          which branding should be used?
#                   OFFICIAL_BUILD
#   DEBUG           OFF                 will add debug symbols and other
#                                       debugging stuff
#   PACKAGE_TYPE    DEB                 Which packages should be created with "make package"
#   FORCE_SYS_DEPS  OFF                 Force the use of system libs
#   WITH_FLASH      ON                  Build with flash support
#
#   BREAKPAD_URL                        URL to breakdpad archive, should be rev. 850
#   CEF_URL                             URL to cef archive, should be rev. 291
#   CHROMIUM_URL                        URL to chromium archive, has to be 15.0.876.0
#   CURL_URL                            URL to curl archive
#   DEPOT_TOOLS_URL                     URL to depot_tools archive
#   V8_URL                              URL to v8 archive
#   WXWIDGET_URL                        URL to wxWidget archive, has to be 2.9.3
#
# cef options:
#   H264_SUPPORT    OFF                 build ffmpeg with mpeg-4 support. Be aware of patent and license problems
#
# installation options:
#   BINDIR          ./bin               the directory where binaries should be installed, a relative path will be
#                                       relative to CMAKE_INSTALL_PREFIX, an absolute will stay absolute
#   RUNTIME_LIBDIR  ./lib/desura        the directory where runtime loaded libs should be stored
#   DATADIR         ./share/desura      same as RUNTIME_LIBDIR only for data files
#   DESKTOPDIR      depends on OS       the directory where the desktop file or link should be installed
#
# unix-only options:
#   FORCE_BUNDLED_WXGTK  		ON: bundle wxGTK 2.9.3
#                   ON			OFF: use system wxGTK (has to be 2.9.x, work stable on 2.9.3)
#   DEBUG_EXTERNAL  OFF                 same as DEBUG, but for externap deps
#   WITH_ARES       ON                  build CURL with ares support (c-ares REQUIRED)
#   DIR_TYPE        SINGLE              how should game data be stored?
#                                       SINGLE:   in a single dir in $HOME
#                                       XDG:      like the official xdg specification
#                                       PORTABLE: in the desura installaion dir
#   INSTALL_DESKTOP_FILE                INSTALL a desktop file in /usr/share/applications
#                   OFF
#
# Linux-only options:
#   DESKTOP_EXE     desura              the value of Exe in the desktop file
#   DESKTOP_ICON    desura              the value of Icon in the desktop file
#
# wxWidgets select correct  version
#   wxWidgets_CONFIG_EXECUTABLE         path to wx-config (version 2.9.x)
#   wxWidgets_wxrc_EXECUTABLE           path to wxrc (version 2.9.x)
#
# windows-only options:
#   BOOST_URL                           URL to boost archive
#   DEBUG_CEF       OFF                 turn on/off debugging in CEF
#   DEBUG_V8        OFF                 turn on/off debugging in v8
#
# some dev notes:
#
#   if you want to port desura to another compiler on UNIX bases systems,
#   please create a file for compiler flags in:
#
#     cmake/platform/unix/compiler/${CMAKE_C_COMPILER_ID}.cmake
#
#   Take a look into the other files.
#
###############################################################################

###############################################################################
# some overrides
###############################################################################

# TODO: silent output for externel dependencies
project(Desura)

if(APPLE) # see #418
  cmake_minimum_required(VERSION 2.8.10)
else()
  cmake_minimum_required(VERSION 2.8.2)
endif()

###############################################################################
# some globale variables
###############################################################################

set(CMAKE_OUTPUT "${CMAKE_SOURCE_DIR}/build_out")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_PLATFORM_PATH "${CMAKE_SOURCE_DIR}/cmake/platform")
set(CMAKE_EXTERNAL_BINARY_DIR "${CMAKE_BINARY_DIR}/external")
set(CMAKE_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")
set(CMAKE_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/cmake/scripts")
set(CMAKE_PATCH_DIR "${CMAKE_SOURCE_DIR}/cmake/patches")
set(CMAKE_GEN_SRC_DIR "${CMAKE_BINARY_DIR}/gen")
set(CMAKE_TEST_PROJECTS "${CMAKE_SOURCE_DIR}/cmake/tests")
set(CMAKE_TEST_PROJECTS_BIN "${CMAKE_BINARY_DIR}/cmake_test")

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

if(DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
else()
  set(CMAKE_BUILD_TYPE Release)
endif()

###############################################################################
# some includes
###############################################################################

include(CheckOptions)
include(ExternalProject)
include(macros)
include(parsearguments)
include(CheckCompiler)
include(SetInstallPaths)
include(Package)
if(UNIX)
  include(FindPkgConfig)
endif()

###############################################################################
# find all packages which are needed
###############################################################################

macro(CopyTargetFiles target)

get_property(target_loc TARGET ${target} PROPERTY LOCATION)

message(WARNING, ${target_loc})

if (${target_loc} MATCHES ".*exe")
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_OUTPUT}")
endif()

if (${target_loc} MATCHES ".*dll")
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_OUTPUT}/bin/.")
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_OUTPUT}/bin/.")
endif()
	
endmacro(CopyTargetFiles)


if(NOT WIN32)
  set(REQUIRED_ON_UNIX REQUIRED)
endif()

if(NOT BUILD_ONLY_CEF)
  if(WIN32)
    set(Boost_ADDITION_LIBS ${Boost_ADDITION_LIBS} chrono)
  endif()
  if(BUILD_TESTS)
    set(Boost_ADDITION_LIBS ${Boost_ADDITION_LIBS} unit_test_framework)
  endif()
  
  set(Boost_USE_MULTITHREADED ON)
  if(WIN32)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_STATIC_RUNTIME OFF)
  endif()
  find_package(Boost COMPONENTS date_time filesystem thread system ${Boost_ADDITION_LIBS})
  
  if(NOT WITH_ARES)
    find_package(CURL ${REQUIRED_IF_OPTION})
  endif()

  if(NOT WIN32)
    find_package(GTK2 REQUIRED)
    find_package(X11 REQUIRED)
    if(NOT X11_Xt_FOUND)
      message(FATAL_ERROR "libXt is required")
    endif()
    pkg_check_modules(GTHREAD2 REQUIRED gthread-2.0)
    find_package(Threads)
    find_package(Notify REQUIRED)
    if(NOT HAVE_LIBNOTIFY_0_7)
      message(FATAL_ERROR "libnotify >= 0.7 is required")
    endif()
    pkg_check_modules(TINYXML ${REQUIRED_IF_OPTION} tinyxml2)
  endif()
  find_package(BZip2 ${REQUIRED_IF_OPTION})
  find_package(Sqlite ${REQUIRED_IF_OPTION})

  find_package(v8 ${REQUIRED_IF_OPTION})

  if(NOT FORCE_BUNDLED_WXGTK)
    find_package(wxWidgets 2.9 COMPONENTS richtext ${REQUIRED_IF_OPTION})
  endif()
endif()

find_package(PythonInterp REQUIRED)

if(WIN32)
  find_package(PythonLibs REQUIRED)
  # only define this if MFC was found
  # VC express and mingw doesn't provide MFC, but other versions of VC do
  find_package(MFC)
  if(MFC_FOUND)
    set(CMAKE_MFC_FLAG 2)
    add_definitions(-DMFC_FOUND -DDONT_INCLUDE_WINH -D_AFXDLL)
    # Visual Studio 9 needs this
    add_definitions(-D_BIND_TO_CURRENT_CRT_VERSION -D_BIND_TO_CURRENT_MFC_VERSION)
  endif()
endif()

find_package(Patch REQUIRED)
if(MSVC)
  configure_file(${CMAKE_SCRIPT_PATH}/patch.bat ${CMAKE_GEN_SRC_DIR}/scripts/patch.bat @ONLY)
  set(PATCH_SCRIPT_PATH ${CMAKE_GEN_SRC_DIR}/scripts/patch.bat)
else()
  set(PATCH_SCRIPT_PATH ${CMAKE_SCRIPT_PATH}/patch.sh)
endif()

###############################################################################
# SVN, HTTP stuff
###############################################################################

if(WIN32)
  set(BOOST_URL_DEFAULT "http://downloads.sourceforge.net/project/boost/boost/1.51.0/boost_1_51_0.zip")
  set(BOOST_URL ${BOOST_URL_DEFAULT}
      CACHE STRING "URL to boost archive")
  if(BOOST_URL STREQUAL BOOST_URL_DEFAULT)
    set(BOOST_MD5 ee8112e48088b05c248d68329cd5d908)
  else()
    unset(BOOST_MD5)
  endif()
endif()

set(BREAKPAD_URL_DEFAULT "http://sourceforge.net/projects/desurium/files/breakpad-850-r1.zip/download")
set(BREAKPAD_URL ${BREAKPAD_URL_DEFAULT}
    CACHE STRING "URL to breakpad")
if(BREAKPAD_URL STREQUAL BREAKPAD_URL_DEFAULT)
  set(BREAKPAD_MD5 b0061a854df1a0d285edc74c5ac37c42)
else()
  unset(BREAKPAD_MD5)
endif()


set(CEF_URL_DEFAULT "http://sourceforge.net/projects/desurium/files/cef-291.tar.gz/download")
set(CEF_URL ${CEF_URL_DEFAULT}
    CACHE STRING "URL to cef, has to be revision 291")
if(CEF_URL STREQUAL CEF_URL_DEFAULT)
  set(CEF_MD5 97e925e33f2d48cd50172247dbe40f30)
else()
  unset(CEF_MD5)
endif()

set(CHROMIUM_URL_DEFAULT "http://commondatastorage.googleapis.com/chromium-browser-official/chromium-15.0.876.0.tar.bz2")
set(CHROMIUM_URL ${CHROMIUM_URL_DEFAULT}
    CACHE STRING "URL to chromium, has to be 15.0.876.0")
if(CHROMIUM_URL STREQUAL CHROMIUM_URL_DEFAULT)
  set(CHROMIUM_MD5 22c8e1db4d1c4b474c600dffbb5fdd0c)
else()
  unset(CHROMIUM_MD5)
endif()

if(WIN32 OR NOT CURL_FOUND)
  set(CURL_URL_DEFAULT http://curl.haxx.se/download/curl-7.32.0.tar.bz2)
  set(CURL_URL ${CURL_URL_DEFAULT}
      CACHE STRING "URL to curl archive")
  if(CURL_URL STREQUAL CURL_URL_DEFAULT)
    set(CURL_MD5 30d04b0a8c43c6770039d1bf033dfe79)
  else()
    unset(CURL_MD5)
  endif()
endif()

set(DEPOT_TOOLS_URL_DEFAULT "http://sourceforge.net/projects/desurium/files/depot_tools-r1.zip/download")
set(DEPOT_TOOLS_URL ${DEPOT_TOOLS_URL_DEFAULT}
    CACHE STRING "URL to depot_tools")
if(DEPOT_TOOLS_URL STREQUAL DEPOT_TOOLS_URL_DEFAULT)
  set(DEPOT_TOOLS_MD5 a27266b8d2388c4dc51645d1159dd0de)
else()
  unset(DEPOT_TOOLS_MD5)
endif()

set(V8_URL_DEFAULT https://commondatastorage.googleapis.com/chromium-browser-official/v8-3.18.5.14.tar.bz2)
set(V8_URL ${V8_URL_DEFAULT}
    CACHE STRING "URL to v8 archive")
if(V8_URL STREQUAL V8_URL_DEFAULT)
  set(V8_MD5 657802ea63c89190fd510ee1a2a719bf)
else()
  unset(V8_MD5)
endif()

set(WXWIDGET_URL_DEFAULT http://garr.dl.sourceforge.net/project/wxwindows/2.9.3/wxWidgets-2.9.3.tar.bz2)
set(WXWIDGET_URL ${WXWIDGET_URL_DEFAULT}
    CACHE STRING "URL to wxWidget 2.9.3 archive")
if(WXWIDGET_URL STREQUAL WXWIDGET_URL_DEFAULT)
  set(WXWIDGET_MD5 6b6003713289ea4d3cd9b49c5db5b721)
else()
  unset(WXWIDGET_MD5)
endif()

###############################################################################
# build some externel projects
###############################################################################

if(NOT BUILD_ONLY_CEF)
  if(NOT wxWidgets_FOUND)
    include(BuildwxWidgets)
  endif()

  if(NOT Boost_FOUND)
    include(BuildBoost)
  endif()

  if(NOT BREAKPAD_EXCEPTION_HANDLER_FOUND)
    include(BuildGoogleBreakpad)
  endif()

  if(NOT CURL_FOUND)
    include(BuildCURL)  
  endif()

  if(NOT V8_FOUND)
    include(Buildv8)
  endif()
endif()

include(BuildCEF)

if(NOT BUILD_ONLY_CEF)
  ###############################################################################
  # build third_party
  ###############################################################################

  if(NOT BZIP2_FOUND)
    add_subdirectory(third_party/bzip2)
  endif()

  add_subdirectory(third_party/courgette) # for headers only, build via ExternalProject

  if(NOT TINYXML_FOUND)
    add_subdirectory(third_party/tinyxml2)
  endif()

  if(NOT SQLITE_FOUND)
    add_subdirectory(third_party/sqlite)
  endif()
  add_subdirectory(third_party/sqlite3x)

  if(NOT Boost_FOUND)
    add_dependencies(sqlite3x boost)
  endif()

  if(WITH_FLASH AND NOT WIN32)
    add_subdirectory(third_party/npwrapper)
  endif()
  ###############################################################################
  # build desura
  ###############################################################################
  add_subdirectory(src)
  add_subdirectory(build_out)
endif()
