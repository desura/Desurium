###############################################################################
#
# you can pass some options to cmake via -D<option>[=<value>]
#
#   name            default             description
#
# global options:
#   BUILD_TESTS     ON                  Build various unit tests
#   OFFICIAL_BUILD  OFF                 for official build only
#   BRANDING        depends on          ehich branding should be used?
#                   OFFICIAL_BUILD
#   DEBUG           OFF                 will add debug symbols and other
#                                       debugging stuff
#   PACKAGE_TYPE    DEB                 Which packages should be created with "make package"
#
# unix-only options:
#   DEBUG_EXTERNAL  OFF                 same as DEBUG, but for externap deps
#   WITH_ARES       ON                  build CURL with ares support (c-ares REQUIRED)
#   DIR_TYPE        SINGLE              how should game data be stored?
#                                       SINGLE:   in a single dir in $HOME
#                                       XDG:      like the official xdg specification
#                                       PORTABLE: in the desura installaion dir
#   INSTALL_DESKTOP_FILE                INSTALL a desktop file in /usr/share/applications
#                   OFF
#
# windows-only options:
#
# some dev notes:
#
#   if you want to port desura to another compiler on UNIX bases systems,
#   please create a files for compiler flags in:
#
#     cmake/platform/unix/compiler/${CMAKE_C_COMPILER_ID}.cmake
#
#   Take a look into the other files.
#
###############################################################################

###############################################################################
# some overrides
###############################################################################

set(CMAKE_USER_MAKE_RULES_OVERRIDE
   ${CMAKE_SOURCE_DIR}/cmake/scripts/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
   ${CMAKE_SOURCE_DIR}/cmake/scripts/cxx_flag_overrides.cmake)

# TODO: silent output for externel dependencies
# TODO: get libs automatically for externel builds, maybe make desura a external build itself?
project(Desura)
cmake_minimum_required(VERSION 2.8.5)

###############################################################################
# some globale variables
###############################################################################

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_PLATFORM_PATH "${CMAKE_SOURCE_DIR}/cmake/platform")
set(CMAKE_EXTERNAL_BINARY_DIR "${CMAKE_BINARY_DIR}/external")
set(CMAKE_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/third_party")
set(CMAKE_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/cmake/scripts")
set(CMAKE_PATCH_DIR "${CMAKE_SOURCE_DIR}/cmake/patches")

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
include(ProcessorCount)
include(Package)

###############################################################################
# find all packages which are needed
###############################################################################

if(NOT WIN32)
  set(REQUIRED_ON_UNIX REQUIRED)
endif()

if(NOT WIN32)
  find_package(Boost COMPONENTS date_time filesystem thread system REQUIRED)
  find_package(Notify REQUIRED)
endif()
find_package(BZip2)

if(NOT WITH_ARES)
  find_package(CURL)
endif()

# we patch google breakpad, so we will build it on our own
# find_package(GoogleBreakpad REQUIRED)
if(NOT WIN32)
  find_package(GTK2 REQUIRED)
endif()
if(NOT WIN32)
  find_package(OpenSSL REQUIRED)
endif()
find_package(Sqlite)
find_package(TinyXML)
find_package(v8 ${REQUIRED_ON_UNIX})
# we patch wxWidgets so we will build it on our own
# find_package(wxWidgets 2.9 COMPONENTS richtext REQUIRED)

find_package(PythonInterp REQUIRED)

if(WIN32)
  find_package(PythonLibs REQUIRED)
endif()

###############################################################################
# build some externel projects
###############################################################################

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

include(BuildCEF)

###############################################################################
# build third_party
###############################################################################

if(NOT BZIP2_FOUND)
  add_subdirectory(third_party/bzip2)
endif()

add_subdirectory(third_party/courgette) # for headers only, build via ExternalProject

if(NOT TINYXML_FOUND)
  add_subdirectory(third_party/tinyxml)
endif()

if(NOT SQLITE_FOUND)
  add_subdirectory(third_party/sqlite)
endif()
add_subdirectory(third_party/sqlite3x)

if(NOT Boost_FOUND)
  add_dependencies(sqlite3x boost)
endif()

add_dependencies(cef wxWidget-2-9)

###############################################################################
# build desura
###############################################################################
add_subdirectory(src)
add_subdirectory(build_out)
