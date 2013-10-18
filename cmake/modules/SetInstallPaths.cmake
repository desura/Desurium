if(UNIX)
  set(DEFAULT_INSTALL_DIR "/usr/local")
elseif(WIN32)
  set(DEFAULT_INSTALL_DIR "install")
endif()

if(MINGW OR WIN32)
  set(SYSTEM_ROOT_DIR "C:/")
else()
  set(SYSTEM_ROOT_DIR "/")
endif()

set(CMAKE_INSTALL_PREFIX ${DEFAULT_INSTALL_DIR}
    CACHE STRING "Desura Install Prefix")

if(WIN32)
  set(BINDIR "${CMAKE_BUILD_TYPE}_Out")
  set(RUNTIME_LIBDIR "${BINDIR}\\bin")
  set(DATADIR "${BINDIR}\\data")
else()
  set(BINDIR ""
      CACHE STRING "Desura Bin Install Dir")
  set(RUNTIME_LIBDIR "lib"
      CACHE STRING "Desura Lib Dir")
  set(DATADIR ""
      CACHE STRING "Desura Data Install Dir")
  set(DESKTOPDIR "/usr/share/applications"
      CACHE STRING "Desktop installation directory")
endif()

# set variables used by cmake
if(IS_ABSOLUTE ${BINDIR})
  set(BIN_INSTALL_DIR ${BINDIR})
  set(BIN_INSOURCE_DIR ${BINDIR})
else()
  set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${BINDIR})
  file(RELATIVE_PATH BIN_INSOURCE_DIR ${SYSTEM_ROOT_DIR} ${BIN_INSTALL_DIR})
endif()

if(IS_ABSOLUTE ${RUNTIME_LIBDIR})
  set(LIB_INSTALL_DIR ${RUNTIME_LIBDIR}/desura)
  set(RUNTIME_LIB_INSOURCE_DIR ${RUNTIME_LIBDIR}/desura)
else()
  set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${RUNTIME_LIBDIR})
  file(RELATIVE_PATH RUNTIME_LIB_INSOURCE_DIR ${BIN_INSTALL_DIR} ${LIB_INSTALL_DIR})
endif()

if(IS_ABSOLUTE ${DATADIR})
  set(DATA_INSTALL_DIR ${DATADIR}/desura)
  set(DATA_INSOURCE_DIR ${DATADIR}/desura)
else()
  set(DATA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${DATADIR})
  if(WIN32)
    file(RELATIVE_PATH DATA_INSOURCE_DIR ${BIN_INSTALL_DIR} ${DATA_INSTALL_DIR})
  else()
    file(RELATIVE_PATH DATA_INSOURCE_DIR ${LIB_INSTALL_DIR} ${DATA_INSTALL_DIR})
  endif()
endif()

if(UNIX AND NOT APPLE)
  if(IS_ABSOLUTE ${DESKTOPDIR})
    set(DESKTOP_INSTALL_DIR ${DESKTOPDIR})
  else()
    message(FATAL_ERROR "DESKTOPDIR has to be absolute, anything else doesn't make sense")
  endif()
endif()
