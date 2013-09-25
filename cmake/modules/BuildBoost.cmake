if(MINGW)
  set(CONFIGURE_COMMAND "./bootstrap.sh")
  set(BJAM_BINARY "./bjam.exe")
  set(BOOST_EXTRA_BUILD_OPTS "--with-toolset=mingw")
else()
  set(CONFIGURE_COMMAND "bootstrap.bat")
  set(BJAM_BINARY "b2.exe")
  set(BOOST_EXTRA_BUILD_OPTS "")
endif()

if (WIN32 AND NOT MINGW)
  if (MSVC10)
	set(TOOLSET_MSVC_VER --toolset=msvc-10.0)
  elseif (MSVC11)
	set(TOOLSET_MSVC_VER --toolset=msvc-11.0)
  elseif (MSVC12)
	set(TOOLSET_MSVC_VER --toolset=msvc-12.0)
  endif()
 endif()


if(DEBUG) 
ExternalProject_Add(
  boost
  URL "${BOOST_URL}"
  URL_MD5 ${BOOST_MD5}
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ${CONFIGURE_COMMAND} ${BOOST_EXTRA_BUILD_OPTS}
  BUILD_COMMAND ${BJAM_BINARY} --layout=system --with-date_time --with-filesystem --with-chrono
                  --with-thread --with-system --with-test variant=debug link=static
				  threading=multi runtime-link=shared ${TOOLSET_MSVC_VER}
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
  BUILD_COMMAND ${BJAM_BINARY} --layout=system --with-date_time --with-filesystem --with-chrono
                  --with-thread --with-system --with-test variant=release link=static
				  threading=multi runtime-link=shared ${TOOLSET_MSVC_VER}
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

if(MINGW)
  set(BOOST_SUFFIX a)
else()
  set(BOOST_SUFFIX lib)
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

CopyTargetFiles(boost)
SET_PROPERTY(TARGET boost                PROPERTY FOLDER "ThirdParty")

if (WIN32)
	if(DEBUG) 
	ExternalProject_Add(
	  boost_s
	  URL "${BOOST_URL}"
	  URL_MD5 ${BOOST_MD5}
	  UPDATE_COMMAND ""
	  BUILD_IN_SOURCE 1
	  CONFIGURE_COMMAND ${CONFIGURE_COMMAND} ${BOOST_EXTRA_BUILD_OPTS}
	  BUILD_COMMAND ${BJAM_BINARY} --layout=system --with-filesystem --with-chrono
					  --with-thread --with-system variant=debug link=static
					  threading=multi runtime-link=static --toolset=msvc-12.0
	  INSTALL_COMMAND ""
	)
	else()
	ExternalProject_Add(
	  boost_s
	  URL "${BOOST_URL}"
	  URL_MD5 ${BOOST_MD5}
	  UPDATE_COMMAND ""
	  BUILD_IN_SOURCE 1
	  CONFIGURE_COMMAND ${CONFIGURE_COMMAND} ${BOOST_EXTRA_BUILD_OPTS}
	  BUILD_COMMAND ${BJAM_BINARY} --layout=system --with-filesystem --with-chrono
					  --with-thread --with-system variant=release link=static
					  threading=multi runtime-link=static --toolset=msvc-12.0
	  INSTALL_COMMAND ""
	)
	endif()
	
	ExternalProject_Get_Property(boost_s source_dir)	

	set(Boost_DIR_S ${source_dir})
	set(Boost_INCLUDE_DIR_S ${Boost_DIR_S})
	set(Boost_LIBRARY_DIR_S ${Boost_DIR_S}/stage/lib)		
	
	CopyTargetFiles(boost_s)
	SET_PROPERTY(TARGET boost_s PROPERTY FOLDER "ThirdParty")

	set(Boost_CHRONO_LIBRARY_S "${Boost_LIBRARY_DIR_S}/libboost_chrono${BAS}.${BOOST_SUFFIX}")
	set(Boost_FILESYSTEM_LIBRARY_S "${Boost_LIBRARY_DIR_S}/libboost_filesystem${BAS}.${BOOST_SUFFIX}")
	set(Boost_SYSTEM_LIBRARY_S "${Boost_LIBRARY_DIR_S}/libboost_system${BAS}.${BOOST_SUFFIX}")
	set(Boost_THREAD_LIBRARY_S "${Boost_LIBRARY_DIR_S}/libboost_thread${BAS}.${BOOST_SUFFIX}")
endif()

