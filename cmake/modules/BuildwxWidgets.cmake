
set(WXWIDGET_SVN http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_0)

if(WIN32)
  ExternalProject_Add(
    wxWidget-2-9
    SVN_REPOSITORY ${WXWIDGET_SVN}
    UPDATE_COMMAND ""
    PATCH_COMMAND patch -p0 -N -i ${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch
    CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
  )
  
  ExternalProject_Add_Step(
    wxWidget-2-9
	custom_build
	DEPENDEES configure
	DEPENDERS build
	COMMAND nmake /nologo -f makefile.vc BUILD=release VENDOR=desura SHARED=1 RUNTIME_LIBS=shared MONOLITHIC=1
	WORKING_DIRECTORY <SOURCE_DIR>/build/msw
  )
  
  ExternalProject_Get_Property(
    wxWidget-2-9
    source_dir
  )
  set(wxWidgets_INSTALL_DIR ${source_dir})
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib/vc_dll)
  set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include ${wxWidgets_LIBRARY_DIRS}/mswu)
  set(wxWidgets_LIBRARIES ${wxWidgets_LIBRARY_DIRS}/wxmsw29u.lib)
else()
  set(wxWidgets_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/wxWidgets)

  ExternalProject_Add(
    wxWidget-2-9
    SVN_REPOSITORY ${WXWIDGET_SVN}
    UPDATE_COMMAND ""
    PATCH_COMMAND patch -p0 -N -i ${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared --enable-unicode ${CONFIGURE_DEBUG}
        --enable-monolithic --with-flavour=desura --disable-threads --with-opengl=no
        --disable-joystick --disable-mediactrl --prefix=${wxWidgets_INSTALL_DIR} --enable-permissive
  )
  
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib)
  if(DEBUG_EXTERNAL)
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/gtk2-unicode-debug-2.9-desura)
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/libwx_gtk2ud_desura-2.9.so.0.0.0")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/libwx_gtk2ud_desura-2.9.so.0.0.0
            RENAME libwx_gtk2ud_desura-2.9.so.0
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/gtk2-unicode-release-2.9-desura)
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so.0.0.0")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so.0.0.0
            RENAME libwx_gtk2u_desura-2.9.so.0
            DESTINATION ${LIB_INSTALL_DIR})
  endif()

endif()
