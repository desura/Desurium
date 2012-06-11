if(64BIT)
  set(EXTRA_OPTS "arch=x64")
endif()

if(DEBUG) 
ExternalProject_Add(
  boost
  URL ${BOOST_DOWNLOAD_PATH}
  URL_MD5 ${BOOST_DOWNLOAD_MD5}
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
  URL ${BOOST_DOWNLOAD_PATH}
  URL_MD5 ${BOOST_DOWNLOAD_MD5}
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

set(Boost_LIBRARIES "${Boost_LIBRARY_DIR}/libboost_date_time${BAS}.lib;${Boost_LIBRARY_DIR}/libboost_filesystem${BAS}.lib;${Boost_LIBRARY_DIR}/libboost_system${BAS}.lib;${Boost_LIBRARY_DIR}/libboost_thread${BAS}.lib")
