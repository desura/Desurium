if(BITNESS EQUAL 64)
  set(EXTRA_OPTS "arch=x64")
endif()

ExternalProject_Add(
    v8
    SVN_REPOSITORY http://v8.googlecode.com/svn/tags/3.7.7/
    DOWNLOAD_DIR ${V8_INSTALL_DIR}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND scons snapshot=on mode=release library=shared ${EXTRA_OPTS}
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(
    v8
    source_dir
)

set(V8_INSTALL_DIR ${source_dir})
set(V8_INCLUDE_DIR ${V8_INSTALL_DIR}/include)
set(V8_LIBRARIES "${V8_INSTALL_DIR}/libv8.so;${V8_INSTALL_DIR}/libv8preparser.so")
install(FILES ${V8_LIBRARIES}
        DESTINATION ${LIB_INSTALL_DIR})
