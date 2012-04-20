###############################################################################
# global options
###############################################################################

option(DEBUG "enable debug build" OFF)
option(OFFICIAL_BUILD "do you want to compile an official build?" OFF)
option(BUILD_CEF "do you want to build cef?" ON)

if(OFFICIAL_BUILD)
  set(BRANDING "branding_desura"
      CACHE STRING "The branding to use for Desura.")
else()
  set(BRANDING "branding_desurium"
      CACHE STRING "The branding to use for Desura.")
endif()

###############################################################################
# unix-only options
###############################################################################

if(UNIX)
  option(WITH_ARES "build cURL with c-ares support" ON)
  option(DEBUG_EXTERNAL "build external libs with debug support" OFF)
endif()

###############################################################################
# windows-only options
###############################################################################
