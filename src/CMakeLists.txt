cmake_minimum_required(VERSION 2.8)
project(Desura)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Boost COMPONENTS date_time filesystem thread REQUIRED)
find_package(BZip2 REQUIRED)
find_package(CURL REQUIRED)
find_package(GoogleBreakpad REQUIRED)
find_package(GTK2 REQUIRED)
find_package(Sqlite REQUIRED)
find_package(wxWidgets 2.9 REQUIRED)

file(COPY branding_default/ DESTINATION ${CMAKE_SOURCE_DIR}/branding)

set(GLOBAL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)

# Desura uses the awesome new C++ Standard
add_definitions(-std=c++0x -fpermissive -DBUILD_WITH_CMAKE=1 -DBOOST_FILESYSTEM_VERSION=2)

# wxWidgets config
add_definitions(-D__WXGTK__)

# other usefull macros
add_definitions(-DDESURA_CLIENT -DUI_HIDE_MODS=1)

# build third_party
add_subdirectory(third_party/cef) # todo
add_subdirectory(third_party/courgette) # todo
add_subdirectory(third_party/sqlite3x)
add_subdirectory(third_party/tinyxml)

# build the common shared lib
add_subdirectory(common)

# build static libs
add_subdirectory(static/managers)
add_subdirectory(static/mdump)
add_subdirectory(static/ipc_pipe)
add_subdirectory(static/umcf)
add_subdirectory(static/util)
add_subdirectory(static/util_fs)
add_subdirectory(static/util_thread)
add_subdirectory(static/wx_controls)

# build shared libs
add_subdirectory(shared/crashuploader)
add_subdirectory(shared/mcfcore)
add_subdirectory(shared/servicecore)
add_subdirectory(shared/scriptcore)
add_subdirectory(shared/uicore)
add_subdirectory(shared/usercore)
add_subdirectory(shared/webcore)

# build the client
add_subdirectory(executable/bootloader_lin)
add_subdirectory(executable/crashdlg_lin)
add_subdirectory(tools/process_data_folder)
