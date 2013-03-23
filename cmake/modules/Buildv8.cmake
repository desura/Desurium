if(64BIT)
  set(EXTRA_OPTS "arch=x64")
endif()

if(MINGW)
  set(SCONS_COMMAND scons.py snapshot=on ${MODE_DEBUG} library=shared ${EXTRA_OPTS} toolchain=gcc)
  set(V8_LIB_SUFFIX dll)
elseif(WIN32)
  if(DEBUG)
    set(SCONS_COMMAND ${CMAKE_SCRIPT_PATH}/Buildv8.bat ${PYTHON_INCLUDE_DIRS}/../Scripts/scons.bat debug ${EXTRA_OPTS})
  else()
    set(SCONS_COMMAND ${CMAKE_SCRIPT_PATH}/Buildv8.bat ${PYTHON_INCLUDE_DIRS}/../Scripts/scons.bat release ${EXTRA_OPTS})
  endif()
  set(V8_LIB_SUFFIX lib)
else()
  set(SCONS_COMMAND scons snapshot=on ${MODE_DEBUG} library=shared ${EXTRA_OPTS})
  set(V8_LIB_SUFFIX so)
  set(V8_LIB_PREFIX lib)
endif()

ExternalProject_Add(
  v8
  SVN_REPOSITORY ${V8_SVN}
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ${SCONS_COMMAND}
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(
  v8
  source_dir
)

set(V8_INSTALL_DIR ${source_dir})
set(V8_INCLUDE_DIR ${V8_INSTALL_DIR}/include)

if(DEBUG)
set(V8_LIBRARIES "${V8_INSTALL_DIR}/${V8_LIB_PREFIX}v8_g.${V8_LIB_SUFFIX};${V8_INSTALL_DIR}/${V8_LIB_PREFIX}v8preparser_g.${V8_LIB_SUFFIX}")
else()
set(V8_LIBRARIES "${V8_INSTALL_DIR}/${V8_LIB_PREFIX}v8.${V8_LIB_SUFFIX};${V8_INSTALL_DIR}/${V8_LIB_PREFIX}v8preparser.${V8_LIB_SUFFIX}")
endif()
install(FILES ${V8_LIBRARIES}
        DESTINATION ${LIB_INSTALL_DIR})
