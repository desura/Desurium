if(WIN32 AND NOT MINGW)
  ExternalProject_Add(
    wxWidget
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
      wxWidget
      custom_build
      DEPENDEES configure
      DEPENDERS build
      COMMAND nmake /nologo -f makefile.vc BUILD=debug MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1 RUNTIME_LIBS=dynamic
      WORKING_DIRECTORY <SOURCE_DIR>/build/msw
    )
  else()
    ExternalProject_Add_Step(
      wxWidget
      custom_build
      DEPENDEES configure
      DEPENDERS build
      COMMAND nmake /nologo -f makefile.vc BUILD=release MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1 RUNTIME_LIBS=dynamic
      WORKING_DIRECTORY <SOURCE_DIR>/build/msw
    )
  endif()
  
  ExternalProject_Get_Property(
    wxWidget
    source_dir
  )
  set(wxWidgets_INSTALL_DIR ${source_dir})
  set(wxWidgets_BIN_DIR ${wxWidgets_INSTALL_DIR}/bin)
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib/vc_dll)
  
  if(DEBUG)
    set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include ${wxWidgets_INSTALL_DIR}/include/msvc)
    set(wxWidgets_LIBRARIES ${wxWidgets_LIBRARY_DIRS}/wxmsw30ud.lib)
    install(FILES ${wxWidgets_LIBRARY_DIRS}/wxmsw30ud_vc_desura.dll
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include ${wxWidgets_INSTALL_DIR}/include/msvc)
    set(wxWidgets_LIBRARIES ${wxWidgets_LIBRARY_DIRS}/wxmsw30u.lib)
    install(FILES ${wxWidgets_LIBRARY_DIRS}/wxmsw30u_vc_desura.dll
            DESTINATION ${LIB_INSTALL_DIR})
  endif()
  
else()
  if(MINGW)
    set(WX_SETUP_INCLUDE_SUB "msw-unicode-3.0-desura")
    set(WX_SETUP_INCLUDE_SUB_DEBUG "msw-unicode-debug-3.0-desura")
	set(WX_LIB_NAME "libwx_mswu_desura-3.0.dll.a")
	set(WX_LIB_NAME_DEBUG "libwx_mswu_desura-3.0.dll.a")
  else()
    set(WX_SETUP_INCLUDE_SUB "gtk2-unicode-3.0-desura")
    set(WX_SETUP_INCLUDE_SUB_DEBUG ${WX_SETUP_INCLUDE_SUB})
	set(WX_LIB_NAME "libwx_gtk2u_desura-3.0.so.0.0.0")
	set(WX_LIB_NAME_DEBUG ${WX_LIB_NAME})
  endif()

  set(wxWidgets_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/wxWidgets)
  
  if(MINGW)
    set(WX_PATCH_COMMAND "${PATCH_SCRIPT_PATH}" "${CMAKE_SOURCE_DIR}/cmake/patches/wxWidgets.patch")
  endif()
  
  if(CLANG)
    set(additional_flags "CXXFLAGS=-std=c++0x")
  elseif(GCC)
    set(additional_flags --enable-permissive)
  endif()

  ExternalProject_Add(
    wxWidget
    URL ${WXWIDGET_URL}
    URL_MD5 ${WXWIDGET_MD5}
    UPDATE_COMMAND ""
    ${WX_PATCH_COMMAND}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./configure --disable-all-features
        --enable-animatectrl 
        --enable-button
        --enable-checkbox
        --enable-choice
        --enable-clipboard
        --enable-combobox
        --enable-dataobj
        --enable-datetime
        --enable-dirdlg
        --enable-exceptions
        --enable-file
        --enable-filectrl
        --enable-filedlg
        --enable-filepicker
        --enable-fontmap
        --enable-grid
        --enable-headerctrl
        --enable-hyperlink
        --enable-image
        --with-libpng
        --enable-imaglist
        --enable-intl
        --enable-listctrl
        --enable-log
        --enable-longlong
        --enable-menus
        --enable-msgdlg
        --with-opengl=no
        --enable-radiobtn
        --enable-snglinst
        --enable-stattext
        --enable-streams
        --enable-taskbaricon
        --enable-textctrl
        --enable-timer
        --enable-tooltips
        --enable-treectrl
        --enable-validators
        --enable-shared
        --enable-unicode
        ${CONFIGURE_DEBUG}
        --enable-monolithic
        --with-flavour=desura
        --enable-threads
        --prefix=${wxWidgets_INSTALL_DIR}
        ${additional_flags}
  )
  
  set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib)
  if(DEBUG_EXTERNAL)
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-3.0-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/${WX_SETUP_INCLUDE_SUB_DEBUG})
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME_DEBUG}")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}
            RENAME libwx_gtk2u_desura-3.0.so.0
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(wxWidgets_INCLUDE_DIRS  ${wxWidgets_INSTALL_DIR}/include/wx-3.0-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/${WX_SETUP_INCLUDE_SUB})
    set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}")
    install(FILES ${wxWidgets_LIBRARY_DIRS}/${WX_LIB_NAME}
            RENAME libwx_gtk2u_desura-3.0.so.0
            DESTINATION ${LIB_INSTALL_DIR})
  endif()
  set(wxWidgets_BIN_DIR ${wxWidgets_INSTALL_DIR}/bin)
  set(wxWidgets_CONFIG_EXECUTABLE ${wxWidgets_BIN_DIR}/wx-config)
endif()

SET_PROPERTY(TARGET wxWidget                PROPERTY FOLDER "ThirdParty")
