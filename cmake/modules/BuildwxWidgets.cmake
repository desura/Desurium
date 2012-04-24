set(WXWIDGET_URL http://sourceforge.net/projects/wxwindows/files/2.9.3/wxWidgets-2.9.3.tar.bz2)
set(WXWIDGET_MD5 6b6003713289ea4d3cd9b49c5db5b721)

if(WIN32)
  ExternalProject_Add(
    wxWidget-2-9
    URL ${WXWIDGET_URL}
    URL_MD5 ${WXWIDGET_MD5}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${CMAKE_SOURCE_DIR}/cmake/scripts/Patch.bat ${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch
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
	COMMAND nmake /nologo -f makefile.vc BUILD=debug MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1
	WORKING_DIRECTORY <SOURCE_DIR>/build/msw
  )
else()
  ExternalProject_Add_Step(
    wxWidget-2-9
	custom_build
	DEPENDEES configure
	DEPENDERS build
	COMMAND nmake /nologo -f makefile.vc BUILD=release MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1
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
  set(wxWidgets_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/wxWidgets)

  ExternalProject_Add(
    wxWidget-2-9
    URL ${WXWIDGET_URL}
    URL_MD5 ${WXWIDGET_MD5}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${CMAKE_SCRIPT_PATH}/patch.sh ${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared --enable-unicode ${CONFIGURE_DEBUG}
        --enable-monolithic --with-flavour=desura --disable-threads --with-opengl=no --disable-palette
        --disable-joystick --disable-mediactrl --prefix=${wxWidgets_INSTALL_DIR} --enable-permissive
  )
  
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib)
  if(DEBUG_EXTERNAL)
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/gtk2-unicode-debug-2.9-desura)
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/libwx_gtk2ud_desura-2.9.so.3.0.0")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/libwx_gtk2ud_desura-2.9.so.3.0.0
            RENAME libwx_gtk2ud_desura-2.9.so.3
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/gtk2-unicode-2.9-desura)
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so.3.0.0")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so.3.0.0
            RENAME libwx_gtk2u_desura-2.9.so.3
            DESTINATION ${LIB_INSTALL_DIR})
  endif()
  set(wxWidgets_BIN_DIR ${wxWidgets_INSTALL_DIR}/bin)
endif()
