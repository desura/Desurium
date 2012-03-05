# TODO: silent output for externel dependencies
# TODO: get libs automatically for externel builds, maybe make desurium a external build itself?

project(Desura)

# we wanna build some projects
include(ExternalProject)

find_package(Boost COMPONENTS date_time filesystem thread REQUIRED)
find_package(BZip2 REQUIRED)
find_package(CURL REQUIRED)
# we patch google breakpad, so we will build it on our own
# find_package(GoogleBreakpad REQUIRED)
find_package(GTK2 REQUIRED)
find_package(Sqlite REQUIRED)

include(BuildwxWidgets)
if(NOT BREAKPAD_EXCEPTION_HANDLER_FOUND)
  include(BuildGoogleBreakpad)
endif()
#include(BuildChromium)
#include(BuildCEF)
#include(BuildCourgette)

file(COPY branding_default/ DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/branding)

set(GLOBAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Desura uses the awesome new C++ Standard
add_definitions(-std=c++0x -fpermissive -DBUILD_WITH_CMAKE=1 -DBOOST_FILESYSTEM_VERSION=2)

# wxWidgets config
add_definitions(-D__WXGTK__)

# other usefull macros
add_definitions(-DDESURA_CLIENT -DUI_HIDE_MODS=1 -DDEBUG=1)

# build third_party
add_subdirectory(third_party/cef) # for headers only, build via ExternalProject
add_subdirectory(third_party/courgette) # for headers only, build via ExternalProject
add_subdirectory(third_party/sqlite3x)
add_subdirectory(third_party/tinyxml)

# build static libs
add_subdirectory(common)
add_subdirectory(static/managers)
add_subdirectory(static/mdump)
add_subdirectory(static/ipc_pipe)
add_subdirectory(common/service_pipe)
add_subdirectory(static/umcf)
add_subdirectory(static/util)
add_subdirectory(static/util_fs)
add_subdirectory(static/util_thread)
add_subdirectory(static/wx_controls)
set(STATIC_LIBS ${STATIC_LIBS} desuracommon managers mdump ipc_pipe umcf util util_fs service_pipe threads wx_controls)

# build shared libs
add_subdirectory(shared/crashuploader)
add_subdirectory(shared/mcfcore)
add_subdirectory(shared/servicecore)
add_subdirectory(shared/scriptcore)
add_subdirectory(shared/uicore)
if(DEFINED WIN32)
  add_subdirectory(shared/utilcore)
endif()
add_subdirectory(shared/usercore)
add_subdirectory(shared/webcore)

# build the client
add_subdirectory(executable/bootloader_lin)
add_subdirectory(executable/crashdlg_lin)
add_subdirectory(tools/process_data_folder)
add_subdirectory(branding)

# we need some dependencies 
add_dependencies(managers wxWidget-2-9)
add_dependencies(wx_controls wxWidget-2-9)
add_dependencies(Desura wxWidget-2-9)
if(NOT BREAKPAD_EXCEPTION_HANDLER_FOUND)
    add_dependencies(mdump breakpad)
endif()
