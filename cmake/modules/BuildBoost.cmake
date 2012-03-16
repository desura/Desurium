if(BITNESS EQUAL 64)
  set(EXTRA_OPTS "arch=x64")
endif()

ExternalProject_Add(
  boost
  URL http://downloads.sourceforge.net/project/boost/boost/1.49.0/boost_1_49_0.zip
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND bootstrap.bat
  BUILD_COMMAND b2.exe --layout=system --with-date_time --with-filesystem
                  --with-thread --with-system variant=release link=static
				  threading=multi runtime-link=shared
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(
    boost
    source_dir
)

set(Boost_DIR ${source_dir})
set(Boost_INCLUDE_DIR ${Boost_DIR})
set(Boost_LIBRARY_DIR ${Boost_DIR}/stage/lib)

set(Boost_LIBRARIES "${Boost_LIBRARY_DIR}/libboost_date_time.lib;${Boost_LIBRARY_DIR}/libboost_filesystem.lib;${Boost_LIBRARY_DIR}/libboost_system.lib;${Boost_LIBRARY_DIR}/libboost_thread.lib")
