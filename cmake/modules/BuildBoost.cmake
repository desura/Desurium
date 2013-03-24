if(MINGW)
  set(CONFIGURE_COMMAND "./bootstrap.sh")
  set(BJAM_BINARY "./bjam.exe")
  set(BOOST_EXTRA_BUILD_OPTS "--with-toolset=mingw")
elseif(APPLE)
  set(CONFIGURE_COMMAND "./bootstrap.sh")
  set(BJAM_BINARY "./bjam")
  set(BOOST_EXTRA_BUILD_OPTS --with-toolset=clang)
  set(EXTRA_BJAM_OPTS cxxflags="-stdlib=libc++" linkflags="-stdlib=libc++")
else()
  set(CONFIGURE_COMMAND "bootstrap.bat")
  set(BJAM_BINARY "b2.exe")
  set(BOOST_EXTRA_BUILD_OPTS "")
endif()

if(DEBUG_EXTERNAL)
  ExternalProject_Add(
    boost
    URL "${BOOST_URL}"
    URL_MD5 ${BOOST_MD5}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE_COMMAND} ${BOOST_EXTRA_BUILD_OPTS}
    BUILD_COMMAND ${BJAM_BINARY} --layout=system --with-date_time --with-filesystem
                    --with-thread --with-system --with-test variant=debug link=static
                    threading=multi runtime-link=shared ${EXTRA_BJAM_OPTS}
    INSTALL_COMMAND ""
  )
else()
  ExternalProject_Add(
    boost
    URL "${BOOST_URL}"
    URL_MD5 ${BOOST_MD5}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE_COMMAND} ${BOOST_EXTRA_BUILD_OPTS}
    BUILD_COMMAND ${BJAM_BINARY} --layout=system --with-date_time --with-filesystem
                    --with-thread --with-system --with-test variant=release link=static
                    threading=multi runtime-link=shared ${EXTRA_BJAM_OPTS}
    INSTALL_COMMAND ""
  )
endif()

ExternalProject_Get_Property(
    boost
    source_dir
)

set(Boost_DIR ${source_dir})
set(Boost_INCLUDE_DIR ${Boost_DIR})
set(Boost_LIBRARY_DIR ${Boost_DIR}/stage/lib)

if(MINGW OR APPLE)
  set(BOOST_SUFFIX a)
else()
  set(BOOST_SUFFIX lib)
endif()

set(Boost_CHRONO_LIBRARY "${Boost_LIBRARY_DIR}/libboost_chrono.${BOOST_SUFFIX}")
set(Boost_DATE_TIME_LIBRARY "${Boost_LIBRARY_DIR}/libboost_date_time.${BOOST_SUFFIX}")
set(Boost_FILESYSTEM_LIBRARY "${Boost_LIBRARY_DIR}/libboost_filesystem.${BOOST_SUFFIX}")
set(Boost_PRG_EXEC_MONITOR_LIBRARY "${Boost_LIBRARY_DIR}/libboost_prg_exec_monitor.${BOOST_SUFFIX}")
set(Boost_SYSTEM_LIBRARY "${Boost_LIBRARY_DIR}/libboost_system.${BOOST_SUFFIX}")
set(Boost_TEST_EXEC_MONITOR_LIBRARY "${Boost_LIBRARY_DIR}/libboost_test_exec_monitor.${BOOST_SUFFIX}")
set(Boost_THREAD_LIBRARY "${Boost_LIBRARY_DIR}/libboost_thread.${BOOST_SUFFIX}")
set(Boost_UNIT_TEST_FRAMEWORK_LIBRARY "${Boost_LIBRARY_DIR}/libboost_unit_test_framework.${BOOST_SUFFIX}")
