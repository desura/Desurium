# TODO: silent output for externel dependencies
# TODO: get libs automatically for externel builds, maybe make desurium a external build itself?

project(Desurium)

###############################################################################
# we wanna build some external projects
###############################################################################
include(ExternalProject)

###############################################################################
# find all packages which are needed
###############################################################################

if(NOT WIN32)
  set(REQUIRED_ON_UNIX REQUIRED)
endif()

find_package(Boost COMPONENTS date_time filesystem thread system REQUIRED)
find_package(BZip2)

if(WITH_ARES)
  find_package(cares REQUIRED)
else()
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
# find_package(wxWidgets 2.9 REQUIRED)

###############################################################################
# build some externel projects
###############################################################################

if(NOT wxWidgets_FOUND)
  include(BuildwxWidgets)
endif()

if(NOT BREAKPAD_EXCEPTION_HANDLER_FOUND)
  include(BuildGoogleBreakpad)
endif()

if(NOT CURL_FOUND)
  include(BuildCURL)  
endif()

#include(BuildChromium)
#include(BuildCEF)
#include(BuildCourgette)

###############################################################################
# copy the branding_default to branding
###############################################################################
file(COPY branding_default/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/branding)

###############################################################################
# build third_party
###############################################################################

if(NOT BZIP2_FOUND)
  add_subdirectory(third_party/bzip2)
endif()

add_subdirectory(third_party/cef) # for headers only, build via ExternalProject
add_subdirectory(third_party/courgette) # for headers only, build via ExternalProject

if(NOT SQLITE_FOUND)
  add_subdirectory(third_party/sqlite)
endif()
add_subdirectory(third_party/sqlite3x)

if(NOT TINYXML_FOUND)
  add_subdirectory(third_party/tinyxml)
endif()

###############################################################################
# do final steps for setting up the desurium build environment
###############################################################################

set(GLOBAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/common)

include_directories(
    ${GLOBAL_INCLUDE_DIRS}
    ${COMMON_INCLUDE_DIRS}
    ${TINYXML_INCLUDE_DIR}
)

###############################################################################
# build static libs
###############################################################################
add_subdirectory(common)
add_subdirectory(common/service_pipe)
add_subdirectory(static/managers)
add_subdirectory(static/mdump)
add_subdirectory(static/ipc_pipe)
add_subdirectory(static/scriptengine)
add_subdirectory(static/umcf)
add_subdirectory(static/util)
add_subdirectory(static/util_fs)
add_subdirectory(static/util_thread)
add_subdirectory(static/wx_controls)

###############################################################################
# build shared libs
###############################################################################

add_subdirectory(shared/crashuploader)
add_subdirectory(shared/mcfcore)
add_subdirectory(shared/servicecore)
add_subdirectory(shared/scriptcore)
add_subdirectory(shared/uicore)
if(WIN32)
  add_subdirectory(shared/utilcore)
endif()
add_subdirectory(shared/usercore)
add_subdirectory(shared/webcore)

###############################################################################
# build the client
###############################################################################

add_subdirectory(executable/bootloader_lin)
add_subdirectory(executable/crashdlg_lin)
add_subdirectory(tools/process_data_folder)
add_subdirectory(tools/mfc_upload_cli)
add_subdirectory(branding)

###############################################################################
# we need some dependencies 
###############################################################################

#if(NOT wxWidgets_FOUND)
  add_dependencies(managers_window wxWidget-2-9)
  add_dependencies(wx_controls wxWidget-2-9)
  add_dependencies(Desurium wxWidget-2-9)
#endif()

if(NOT BREAKPAD_EXCEPTION_HANDLER_FOUND)
    add_dependencies(mdump breakpad)
endif()

if(NOT CURL_FOUND)
    add_dependencies(util curl)
    add_dependencies(uicore curl)
endif()
