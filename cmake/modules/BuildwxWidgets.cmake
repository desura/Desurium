if(WIN32 AND NOT MINGW)
  ExternalProject_Add(
    wxWidget-2-9
    URL ${WXWIDGET_URL}
    URL_MD5 ${WXWIDGET_MD5}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
  
  message(STATUS "${DEBUG}")
  
  if(DEBUG) 
    ExternalProject_Add_Step(
      wxWidget-2-9
      custom_build
      DEPENDEES configure
      DEPENDERS build
      COMMAND nmake /nologo -f makefile.vc BUILD=debug MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1 RUNTIME_LIBS=dynamic
      WORKING_DIRECTORY <SOURCE_DIR>/build/msw
    )
  else()
    ExternalProject_Add_Step(
      wxWidget-2-9
      custom_build
      DEPENDEES configure
      DEPENDERS build
      COMMAND nmake /nologo -f makefile.vc BUILD=release MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1 RUNTIME_LIBS=dynamic
      WORKING_DIRECTORY <SOURCE_DIR>/build/msw
    )
  endif()
  
  ExternalProject_Get_Property(
    wxWidget-2-9
    source_dir
  )
  set(wxWidgets_INSTALL_DIR ${source_dir})
  set(wxWidgets_BIN_DIR ${wxWidgets_INSTALL_DIR}/bin)
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib/vc_dll)
  
  if (MSVC10)
	set(MSVC_VER vc100)
  elseif (MSVC11)
	set(MSVC_VER vc110)
  elseif (MSVC12)
	set(MSVC_VER vc120)
  else()
	set(MSVC_VER vc)
  endif()
  
  if(DEBUG)
    set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include ${wxWidgets_INSTALL_DIR}/include/msvc)
    set(wxWidgets_LIBRARIES ${wxWidgets_LIBRARY_DIRS}/wxmsw29ud.lib)
    install(FILES ${wxWidgets_LIBRARY_DIRS}/wxmsw293ud_vc_desura.dll
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include ${wxWidgets_INSTALL_DIR}/include/msvc)
    set(wxWidgets_LIBRARIES ${wxWidgets_LIBRARY_DIRS}/wxmsw29u.lib)
    install(FILES ${wxWidgets_LIBRARY_DIRS}/wxmsw293u_vc_desura.dll
            DESTINATION ${LIB_INSTALL_DIR})
  endif()
  
  if(DEBUG)
	CopyOutputFiles(wxWidget-2-9 ${wxWidgets_LIBRARY_DIRS}/wxmsw293ud_vc_desura.dll)
  else()
    CopyOutputFiles(wxWidget-2-9 ${wxWidgets_LIBRARY_DIRS}/wxmsw293u_vc_desura.dll)
  endif() 
  
else()
  if(MINGW)
    set(WX_SETUP_INCLUDE_SUB "msw-unicode-2.9-desura")
    set(WX_SETUP_INCLUDE_SUB_DEBUG "msw-unicode-debug-2.9-desura")
	set(WX_LIB_NAME "libwx_mswu_desura-2.9.dll.a")
	set(WX_LIB_NAME_DEBUG "libwx_mswu_desura-2.9.dll.a")
  else()
    set(WX_SETUP_INCLUDE_SUB "gtk2-unicode-2.9-desura")
    set(WX_SETUP_INCLUDE_SUB_DEBUG ${WX_SETUP_INCLUDE_SUB})
	set(WX_LIB_NAME "libwx_gtk2u_desura-2.9.so.3.0.0")
	set(WX_LIB_NAME_DEBUG ${WX_LIB_NAME})
  endif()

  set(wxWidgets_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/wxWidgets)
  
  if(MINGW)
    set(WX_PATCH_COMMAND "${PATCH_SCRIPT_PATH}" "${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch")
  endif()

  ExternalProject_Add(
    wxWidget-2-9
    URL ${WXWIDGET_URL}
    URL_MD5 ${WXWIDGET_MD5}
    UPDATE_COMMAND ""
    ${WX_PATCH_COMMAND}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./configure
        --enable-shared --enable-unicode ${CONFIGURE_DEBUG}
        --enable-monolithic --with-flavour=desura --enable-threads --with-opengl=no --disable-palette2
        --disable-joystick --disable-mediactrl --prefix=${wxWidgets_INSTALL_DIR} --enable-permissive
  )
  
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib)
  if(DEBUG_EXTERNAL)
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/${WX_SETUP_INCLUDE_SUB_DEBUG})
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME_DEBUG}")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}
            RENAME libwx_gtk2u_desura-2.9.so.3
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/${WX_SETUP_INCLUDE_SUB})
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}
            RENAME libwx_gtk2u_desura-2.9.so.3
            DESTINATION ${LIB_INSTALL_DIR})
  endif()
  set(wxWidgets_BIN_DIR ${wxWidgets_INSTALL_DIR}/bin)
  set(wxWidgets_CONFIG_EXECUTABLE ${wxWidgets_BIN_DIR}/wx-config)
endif()

SET_PROPERTY(TARGET wxWidget-2-9                PROPERTY FOLDER "ThirdParty")
