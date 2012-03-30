###############################################################################
# global options
###############################################################################

set(BRANDING "branding_desurium"
    CACHE STRING "The branding to use for Desura.")
option(DEBUG "enable debug build" OFF)

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
