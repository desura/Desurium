if(64BIT)
  set(EXTRA_OPTS "arch=x64")
endif()

if(DEBUG) 
ExternalProject_Add(
  boost
  URL "${BOOST_URL}"
  URL_MD5 ${BOOST_MD5}
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND bootstrap.bat
  BUILD_COMMAND b2.exe --with-date_time --with-filesystem
                  --with-thread --with-system variant=debug link=static
				  threading=multi runtime-link=static
  INSTALL_COMMAND ""
)
else()
ExternalProject_Add(
  boost
  URL "${BOOST_URL}"
  URL_MD5 ${BOOST_MD5}
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND bootstrap.bat
  BUILD_COMMAND b2.exe --with-date_time --with-filesystem
                  --with-thread --with-system variant=release link=static
				  threading=multi runtime-link=static
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

# todo: get desura build with system layout
if(DEBUG)
	set(BAS "-vc100-mt-s-gd-1_49")
else()
	set(BAS "-vc100-mt-s-1_49")
endif()


set(Boost_CHRONO_LIBRARY "${Boost_LIBRARY_DIR}/libboost_chrono${BAS}.${BOOST_SUFFIX}")
set(Boost_DATE_TIME_LIBRARY "${Boost_LIBRARY_DIR}/libboost_date_time${BAS}.${BOOST_SUFFIX}")
set(Boost_FILESYSTEM_LIBRARY "${Boost_LIBRARY_DIR}/libboost_filesystem${BAS}.${BOOST_SUFFIX}")
set(Boost_PRG_EXEC_MONITOR_LIBRARY "${Boost_LIBRARY_DIR}/libboost_prg_exec_monitor${BAS}.${BOOST_SUFFIX}")
set(Boost_SYSTEM_LIBRARY "${Boost_LIBRARY_DIR}/libboost_system${BAS}.${BOOST_SUFFIX}")
set(Boost_TEST_EXEC_MONITOR_LIBRARY "${Boost_LIBRARY_DIR}/libboost_test_exec_monitor${BAS}.${BOOST_SUFFIX}")
set(Boost_THREAD_LIBRARY "${Boost_LIBRARY_DIR}/libboost_thread${BAS}.${BOOST_SUFFIX}")
set(Boost_UNIT_TEST_FRAMEWORK_LIBRARY "${Boost_LIBRARY_DIR}/libboost_unit_test_framework${BAS}.${BOOST_SUFFIX}")

set(Boost_LIBRARIES "${Boost_DATE_TIME_LIBRARY};${Boost_FILESYSTEM_LIBRARY};${Boost_SYSTEM_LIBRARY};${Boost_THREAD_LIBRARY};${Boost_PRG_EXEC_MONITOR_LIBRARY};${Boost_TEST_EXEC_MONITOR_LIBRARY};${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")

