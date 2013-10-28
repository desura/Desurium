if(MINGW)
  set(CONFIGURE_COMMAND "./bootstrap.sh")
  set(BJAM_BINARY "./bjam.exe")
  set(BOOST_EXTRA_BUILD_OPTS "--with-toolset=mingw")
  set(BOOST_PATCH_COMMAND "")
else()
  set(CONFIGURE_COMMAND "bootstrap.bat")
  set(BJAM_BINARY "b2.exe")
  set(BOOST_EXTRA_BUILD_OPTS "")
endif()

set(BOOST_BJAM_LIBS_STATIC --with-chrono --with-filesystem --with-date_time --with-thread --with-system)
set(BOOST_BJAM_LIBS ${BOOST_BJAM_LIBS_STATIC} --with-test)

if (WIN32 AND NOT MINGW)
  if (MSVC10)
    set(TOOLSET_MSVC_VER --toolset=msvc-10.0)
	set(BOOST_EXTRA_BUILD_OPTS vc10)
  elseif (MSVC11)
    set(TOOLSET_MSVC_VER --toolset=msvc-11.0)
	set(BOOST_EXTRA_BUILD_OPTS vc11)
  elseif (MSVC12)
    set(TOOLSET_MSVC_VER --toolset=msvc-12.0)
	set(BOOST_EXTRA_BUILD_OPTS vc12)
	set(BOOST_PATCH_COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/boost_vc12.patch)
  endif()
endif()

if(DEBUG) 
  ExternalProject_Add(
    boost
    URL "${BOOST_URL}"
    URL_MD5 ${BOOST_MD5}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
	PATCH_COMMAND ${BOOST_PATCH_COMMAND}
    CONFIGURE_COMMAND ${CONFIGURE_COMMAND} ${BOOST_EXTRA_BUILD_OPTS}
    BUILD_COMMAND ${BJAM_BINARY} ${BOOST_BJAM_LIBS} --layout=tagged variant=debug link=static
                    threading=multi runtime-link=shared ${TOOLSET_MSVC_VER}
    INSTALL_COMMAND ""
  )
  set(BOOST_LIB_ADD_STRING "mt-gd")
else()
  ExternalProject_Add(
    boost
    URL "${BOOST_URL}"
    URL_MD5 ${BOOST_MD5}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
	PATCH_COMMAND ${BOOST_PATCH_COMMAND}
    CONFIGURE_COMMAND ${CONFIGURE_COMMAND} ${BOOST_EXTRA_BUILD_OPTS}
    BUILD_COMMAND ${BJAM_BINARY} ${BOOST_BJAM_LIBS} --layout=tagged variant=release link=static
                    threading=multi runtime-link=shared ${TOOLSET_MSVC_VER}
    INSTALL_COMMAND ""
  )
  set(BOOST_LIB_ADD_STRING "mt")
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

set(Boost_CHRONO_LIBRARY "${Boost_LIBRARY_DIR}/libboost_chrono-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
set(Boost_DATE_TIME_LIBRARY "${Boost_LIBRARY_DIR}/libboost_date_time-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
set(Boost_FILESYSTEM_LIBRARY "${Boost_LIBRARY_DIR}/libboost_filesystem-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
set(Boost_PRG_EXEC_MONITOR_LIBRARY "${Boost_LIBRARY_DIR}/libboost_prg_exec_monitor-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
set(Boost_SYSTEM_LIBRARY "${Boost_LIBRARY_DIR}/libboost_system-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
set(Boost_TEST_EXEC_MONITOR_LIBRARY "${Boost_LIBRARY_DIR}/libboost_test_exec_monitor-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
set(Boost_THREAD_LIBRARY "${Boost_LIBRARY_DIR}/libboost_thread-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
set(Boost_UNIT_TEST_FRAMEWORK_LIBRARY "${Boost_LIBRARY_DIR}/libboost_unit_test_framework-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")

set_property(TARGET boost PROPERTY FOLDER "ThirdParty")

if (WIN32)
  if(DEBUG)
    ExternalProject_Add(
      boost_s
      UPDATE_COMMAND ""
      DOWNLOAD_COMMAND ""
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ${BJAM_BINARY} ${BOOST_BJAM_LIBS_STATIC} --layout=tagged
                      variant=debug link=static threading=multi runtime-link=static ${TOOLSET_MSVC_VER}
      INSTALL_COMMAND ""
      SOURCE_DIR "${source_dir}"
    )
    set(BOOST_LIB_ADD_STRING "mt-sgd")
  else()
    ExternalProject_Add(
      boost_s
      UPDATE_COMMAND ""
      DOWNLOAD_COMMAND ""
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ${BJAM_BINARY} ${BOOST_BJAM_LIBS_STATIC} --layout=tagged
                      variant=release link=static threading=multi runtime-link=static ${TOOLSET_MSVC_VER}
      INSTALL_COMMAND ""
      SOURCE_DIR "${source_dir}"
    )
    set(BOOST_LIB_ADD_STRING "mt-s")
  endif()
  
  add_dependencies(boost_s boost)

  set(Boost_LIBRARY_DIR_S ${Boost_DIR_S}/stage/lib)

  set_property(TARGET boost_s PROPERTY FOLDER "ThirdParty")

  set(Boost_CHRONO_LIBRARY_S "${Boost_LIBRARY_DIR}/libboost_chrono-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
  set(Boost_DATE_TIME_LIBRARY_S "${Boost_LIBRARY_DIR}/libboost_date_time-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
  set(Boost_FILESYSTEM_LIBRARY_S "${Boost_LIBRARY_DIR}/libboost_filesystem-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
  set(Boost_SYSTEM_LIBRARY_S "${Boost_LIBRARY_DIR}/libboost_system-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
  set(Boost_THREAD_LIBRARY_S "${Boost_LIBRARY_DIR}/libboost_thread-${BOOST_LIB_ADD_STRING}.${BOOST_SUFFIX}")
endif()
