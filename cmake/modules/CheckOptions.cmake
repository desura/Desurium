###############################################################################
# global options
###############################################################################

option(BUILD_ONLY_CEF "build only cef, this will disable desurium building" OFF)

if(NOT BUILD_ONLY_CEF)
  option(BUILD_TESTS "Build various unit tests." ON)
  if(BUILD_TESTS)
    include(CTest)
    enable_testing()
  endif()

  option(DEBUG "enable debug build" OFF)
  option(OFFICIAL_BUILD "do you want to compile an official build?" OFF)
  option(SET_OWN_EXT_SRC "useful for ebuild or saving traffic" OFF)
  option(BUILD_CEF "Build cef" ON)

  if(OFFICIAL_BUILD)
    set(BRANDING "branding_desura"
      CACHE STRING "The branding to use for Desura.")
  else()
    set(BRANDING "branding_desurium"
      CACHE STRING "The branding to use for Desura.")
  endif()

  option(FORCE_SYS_DEPS "Force the use of system libs")
  if(FORCE_SYS_DEPS)
    set(REQUIRED_IF_OPTION REQUIRED)
  else()
    unset(REQUIRED_IF_OPTION)
  endif()

  ###############################################################################
  # unix-only options
  ###############################################################################

  if(UNIX)
    option(DEBUG_EXTERNAL "build external libs with debug support" OFF)
    option(INSTALL_DESKTOP_FILE "install the generated desktop file to /usr/share/applications/" OFF)
    option(FORCE_BUNDLED_WXGTK "force building of bundled wxGTK" ON)
  endif()

  ###############################################################################
  # windows-only options
  ###############################################################################

  if(WIN32)
    option(DEBUG_CEF "turn on/off debugging in CEF" OFF)
    option(DEBUG_V8 "turn on/off debugging in v8" OFF)
  endif()

  ###############################################################################
  # check set options, print warnings, what ever
  ###############################################################################

  option(WITH_FLASH "enable flash support" ON)
endif()

###############################################################################
# cef-only options
###############################################################################

if(BUILD_CEF OR BUILD_ONLY_CEF)
  option(H264_SUPPORT "build ffmpeg with mpeg-4 support. Be aware of patent and license problems." OFF)
  if(H264_SUPPORT)
    message("H264_SUPPORT is on. Be aware of patent and license problems")
    set(CEF_FFMPEG_BRANDING "Chrome")
  else()
    set(CEF_FFMPEG_BRANDING "Chromium")
  endif()
endif()

