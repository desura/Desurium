if(UNIX)
  set(DEFAULT_INSTALL_DIR "/usr/local")
elseif(WIN32)
  set(DEFAULT_INSTALL_DIR "")
endif()

set(CMAKE_INSTALL_PREFIX ${DEFAULT_INSTALL_DIR}
    CACHE STRING "Desura Install Prefix")
set(BINDIR "./bin"
    CACHE STRING "Desura Bin Install Dir")
set(LIBDIR "./bin/lib"
    CACHE STRING "Desura Lib Dir")
set(DATADIR "./share/desura"
    CACHE STRING "Desura Data Install Dir")

# set variables used by cmake
if(IS_ABSOLUTE ${BINDIR})
  set(BIN_INSTALL_DIR ${BINDIR})
else()
  set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${BINDIR})
endif()

if(IS_ABSOLUTE ${LIBDIR})
  set(LIB_INSTALL_DIR ${LIBDIR})
else()
  set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${LIBDIR})
endif()

if(IS_ABSOLUTE ${DATADIR})
  set(DATA_INSTALL_DIR ${DATADIR})
  set(DATA_INSOURCE_DIR ${DATADIR})
else()
  set(DATA_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${DATADIR})
  file(RELATIVE_PATH DATA_INSOURCE_DIR ${BIN_INSTALL_DIR} ${DATA_INSTALL_DIR})
endif()

