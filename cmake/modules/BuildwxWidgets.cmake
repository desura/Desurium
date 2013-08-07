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
  
  if(DEBUG)
    set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include ${wxWidgets_INSTALL_DIR}/include/msvc)
    set(wxWidgets_LIBRARIES ${wxWidgets_LIBRARY_DIRS}/wxmsw29ud.lib)
  else()
    set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include ${wxWidgets_INSTALL_DIR}/include/msvc)
    set(wxWidgets_LIBRARIES ${wxWidgets_LIBRARY_DIRS}/wxmsw29u.lib)
  endif()
  
else()
  if(MINGW)
    set(WX_SETUP_INCLUDE_SUB "msw-unicode-2.9-desura")
    set(WX_SETUP_INCLUDE_SUB_DEBUG "msw-unicode-debug-2.9-desura")
    set(WX_LIB_NAME "libwx_mswu_desura-2.9.dll.a")
    set(WX_LIB_NAME_DEBUG "libwx_mswu_desura-2.9.dll.a")
  elseif(APPLE)
    set(WX_SETUP_INCLUDE_SUB "osx_cocoa-unicode-2.9-desura")
    set(WX_SETUP_INCLUDE_SUB_DEBUG "osx_cocoa-unicode-debug-2.9-desura")
    set(WX_LIB_NAME "libwx_osx_cocoau_desura-2.9.3.0.0.dylib")
    set(WX_LIB_NAME_DEBUG "libwx_osx_cocoau_desura-2.9.3.0.0.dylib")
    set(WX_LIB_NAME_INSTALL ${WX_LIB_NAME})
    set(WX_LIB_NAME_DEBUG_INSTALL ${WX_LIB_NAME_DEBUG})
  else()
    set(WX_SETUP_INCLUDE_SUB "gtk2-unicode-2.9-desura")
    set(WX_SETUP_INCLUDE_SUB_DEBUG "gtk2-unicode-debug-2.9-desura")
    set(WX_LIB_NAME "libwx_gtk2u_desura-2.9.so.3.0.0")
    set(WX_LIB_NAME_DEBUG "libwx_gtk2ud_desura-2.9.so.3.0.0")
    set(WX_LIB_NAME_INSTALL "libwx_gtk2u_desura-2.9.so.3")
    set(WX_LIB_NAME_DEBUG_INSTALL "libwx_gtk2ud_desura-2.9.so.3")
  endif()
  
  set(wxWidgets_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/wxWidgets)
  
  if(MINGW)
    set(WX_PATCH_COMMAND "${PATCH_SCRIPT_PATH}" "${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch")
  endif()
  
  if(APPLE)
    set(ADDITIONAL_FLAGS --disable-webkit --disable-webview-webkit
      --with-macosx-version-min=10.7 --with-osx_cocoa CPPFLAGS=-stdlib=libc++ CXXFLAGS=-stdlib=libc++ LDFLAGS=-stdlib=libc++ CC=clang CXX=clang++)
  endif()
  
  ExternalProject_Add(
    wxWidget-2-9
    URL ${WXWIDGET_URL}
    URL_MD5 ${WXWIDGET_MD5}
    UPDATE_COMMAND ""
    ${WX_PATCH_COMMAND}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./configure
        --enable-shared --enable-unicode ${CONFIGURE_DEBUG} --disable-webview
        --enable-monolithic --with-flavour=desura --enable-threads --with-opengl=no --disable-palette
        --disable-joystick --disable-mediactrl --prefix=${wxWidgets_INSTALL_DIR} --enable-permissive
        ${ADDITIONAL_FLAGS}
  )
  
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib)
  if(DEBUG_EXTERNAL)
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/${WX_SETUP_INCLUDE_SUB_DEBUG})
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME_DEBUG}")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME_DEBUG}
            RENAME ${WX_LIB_NAME_DEBUG_INSTALL}
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/${WX_SETUP_INCLUDE_SUB})
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}
            RENAME ${WX_LIB_NAME_INSTALL}
            DESTINATION ${LIB_INSTALL_DIR})
  endif()
  set(wxWidgets_BIN_DIR ${wxWidgets_INSTALL_DIR}/bin)
  set(wxWidgets_CONFIG_EXECUTABLE ${wxWidgets_BIN_DIR}/wx-config)
endif()
