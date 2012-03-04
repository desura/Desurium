cmake_minimum_required(VERSION 2.8)
project(Desura)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Sqlite REQUIRED)

file(COPY branding_default/ DESTINATION ${CMAKE_SOURCE_DIR}/branding)

set(GLOBAL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)

# you shoul use stdint.h, really
# add_definitions(-DDWORD=int32_t -DWORD=int16_t -DBYTE=uint8_t -D__stdcall)

# dirty hacks
# add_Definitions(-DUI_HIDE_MODS=1 -DCopyMemory=memcpy)

# Desura uses the awesome new C++ Standard
add_definitions(-std=c++0x -fpermissive -DBUILD_WITH_CMAKE=1)

# wxWidgets config
add_definitions(-D__WXGTK__)

# build the common shared lib
add_subdirectory(common)

# build third_party
add_subdirectory(third_party/sqlite3x)
add_subdirectory(third_party/tinyxml)

# build static libs (at the time there are all shared)
add_subdirectory(static/managers)
add_subdirectory(static/mdump)
add_subdirectory(static/umcf)
add_subdirectory(static/util_thread)

# add_subdirectory(static/util)

# build uicore
# add_subdirectory(shared/uicore)

# build the client
# add_subdirectory(executable/bootloader_lin)
