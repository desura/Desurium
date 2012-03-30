###############################################################################
# global options
###############################################################################

option(DEBUG "enable debug build" OFF)
option(OFFICIAL_BUILD "do you want to compile an official build?" OFF)

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
