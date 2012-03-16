if(BITNESS EQUAL 64)
  set(EXTRA_OPTS "arch=x64")
endif()

set(V8_SVN http://v8.googlecode.com/svn/tags/3.7.7/)
set(TEST "%")

if(WIN32)
  set(SCONS_COMMAND ${CMAKE_SCRIPT_PATH}/Buildv8.bat ${PYTHON_INCLUDE_DIRS}/../Scripts/scons.bat ${EXTRA_OPTS})
  set(V8_LIB_SUFFIX lib)
else()
  set(SCONS_COMMAND scons snapshot=on mode=release library=shared ${EXTRA_OPTS})
  set(V8_LIB_SUFFIX so)
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
set(V8_LIBRARIES "${V8_INSTALL_DIR}/libv8.${V8_LIB_SUFFIX};${V8_INSTALL_DIR}/libv8preparser.${V8_LIB_SUFFIX}")
install(FILES ${V8_LIBRARIES}
        DESTINATION ${LIB_INSTALL_DIR})
