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
#   CEF_BIN_URL							URL to cef bin files for windows
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
  if(OFFICIAL_BUILD)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
  else()
    set(CMAKE_BUILD_TYPE Release)
  endif()
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
    find_package(CURL 7.19.1 ${REQUIRED_IF_OPTION})
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

  if(WITH_GTEST)
    find_package(GTest ${REQUIRED_IF_OPTION})
    add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY -DWITH_GTEST)
  endif()
endif()

find_package(PythonInterp REQUIRED)

if(WIN32)
  find_package(PythonLibs REQUIRED)
endif()

find_package(Patch REQUIRED)
if(MSVC)
  configure_file(${CMAKE_SCRIPT_PATH}/patch.bat ${CMAKE_GEN_SRC_DIR}/scripts/patch.bat @ONLY)
  set(PATCH_SCRIPT_PATH ${CMAKE_GEN_SRC_DIR}/scripts/patch.bat)
else()
  set(PATCH_SCRIPT_PATH ${CMAKE_SCRIPT_PATH}/patch.sh)
endif()

if(WIN32)
  find_package(NMake REQUIRED)
endif()

###############################################################################
# SVN, HTTP stuff
###############################################################################

set(DOWNLOAD_URL "https://s3-us-west-2.amazonaws.com/lecs.desura.lindenlab.com/" CACHE STRING "Download mirror for third party source code and binaries")


macro(SetupDownloadUrl name url md5)
  set(${name}_URL_DEFAULT "${DOWNLOAD_URL}${url}")
  set(${name}_URL ${${name}_URL_DEFAULT} CACHE STRING "URL to get ${name} archive")
  if(${name}_URL STREQUAL ${name}_URL_DEFAULT)
    set(${name}_MD5 ${md5} CACHE STRING "Md5 for ${name} archive")
  else()
    unset(${name}_MD5)
  endif()
endmacro()

if(WIN32)
  SetupDownloadUrl(BOOST "boost_1_53_0.zip" c618e030fd4882e4dbacf54baf824544)
  SetupDownloadUrl(CEF_BIN "cef_bin.zip" c9c0379579d9d26557ca8f4b42f937f2)
  SetupDownloadUrl(DESURA_EXTRA_BIN "desura_extra_bin.zip" fdb2a4dd704a90e5174c64d3eaf8392b) 
endif()

SetupDownloadUrl(BREAKPAD "breakpad-850-r1.zip" b0061a854df1a0d285edc74c5ac37c42)
SetupDownloadUrl(CEF "cef-291.tar.gz" 97e925e33f2d48cd50172247dbe40f30)
SetupDownloadUrl(CHROMIUM "chromium-15.0.876.0.tar.bz2" 22c8e1db4d1c4b474c600dffbb5fdd0c)

if(WIN32 OR NOT CURL_FOUND)
  SetupDownloadUrl(CURL "curl-7.32.0.tar.bz2" 30d04b0a8c43c6770039d1bf033dfe79)
endif()

SetupDownloadUrl(DEPOT_TOOLS "depot_tools-r1.zip" a27266b8d2388c4dc51645d1159dd0de)
SetupDownloadUrl(GTEST "gtest-1.7.0.zip" 2d6ec8ccdf5c46b05ba54a9fd1d130d7)
SetupDownloadUrl(WXWIDGET "wxWidgets-2.9.3.tar.bz2" 6b6003713289ea4d3cd9b49c5db5b721)
SetupDownloadUrl(V8 "v8-3.18.5.14.tar.bz2" 657802ea63c89190fd510ee1a2a719bf)

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

  if(NOT GTEST_FOUND AND WITH_GTEST)
    include(BuildGTest)
  endif()

  if(NOT V8_FOUND)
    include(Buildv8)
  endif()
  
  if(WIN32)
    include(BuildDesuraExtraBin)
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

  if(WIN32)
    add_subdirectory(third_party/desura_extra)
  endif()

  ###############################################################################
  # build desura
  ###############################################################################
  add_subdirectory(src)
  add_subdirectory(build_out)
endif()
