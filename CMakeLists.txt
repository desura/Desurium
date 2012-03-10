###############################################################################
#
# you can pass some options to cmake via -D<option>[=<value>]
#
#   name            default             description
#
# global options:
#   BRANDING        branding_default    The branding to use for Desurium.
#                                       possible values: branding_default, branding_desura
#
# unix-only options:
#   WITH_ARES       ON                  build CURL with ares support (c-ares REQUIRED)
#
# windows-only options:
#
###############################################################################

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_PLATFORM_PATH "${CMAKE_SOURCE_DIR}/cmake/platform")
set(CMAKE_EXTERNAL_BINARY_DIR "${CMAKE_BINARY_DIR}/external")

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug)
endif()

include(CheckOptions)
include(compilerflags)
include(parsearguments)

include(CheckCompiler)
include(SetInstallPaths)

add_subdirectory(src)
add_subdirectory(build_out)
