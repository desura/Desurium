###############################################################################
# global options
###############################################################################

option(BUILD_TESTS "Build various unit tests." ON)
if(BUILD_TESTS)
  include(CTest)
  enable_testing()
endif()

option(DEBUG "enable debug build" OFF)
option(OFFICIAL_BUILD "do you want to compile an official build?" OFF)

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
  option(INSTALL_DESKTOP_FILE "install the generated desktop file to /usr/share/applications/" OFF)
endif()

###############################################################################
# windows-only options
###############################################################################


###############################################################################
# check set options, print warnings, what ever
###############################################################################

if(NOT WITH_ARES)
  message("WW building Desura without c-ares is not supported.")
  message("WW Make sure, that your system installed cURL has ares support.")
  message("WW Ignore this if you are aware of what you do. See #189 for further information.")
endif()
