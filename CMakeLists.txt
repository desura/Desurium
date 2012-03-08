cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_PLATFORM_PATH "${CMAKE_SOURCE_DIR}/cmake/platform")

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug)
endif()

include(compilerflags)
include(parsearguments)

include(CheckCompiler)

# TODO: include this
# set this with cmake -DCMAKE_INSTALL_PREFIX=/where/to/install
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
set(BIN_INSTALL_DIR ${INSTALL_DIR}/bin)
set(LIB_INSTALL_DIR ${INSTALL_DIR}/lib)
set(DATA_INSTALL_DIR ${INSTALL_DIR}/data)

add_subdirectory(src)
add_subdirectory(build_out)
