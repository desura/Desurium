set(wxWidgets_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/wxWidgets)

ExternalProject_Add(
    wxWidget-2-9
    SVN_REPOSITORY http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_0
    UPDATE_COMMAND ""
    PATCH_COMMAND patch -p0 -N -i ${CMAKE_SOURCE_DIR}/src/third_party/patches/wxWidgets_patch.patch
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared --enable-unicode
        --enable-monolithic --with-flavour=desura --disable-threads --with-opengl=no
        --disable-joystick --disable-mediactrl --prefix=${wxWidgets_INSTALL_DIR}
)

set(wxWidgets_BIN_DIRS ${wxWidgets_INSTALL_DIR}/bin)
set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib)
set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/gtk2-unicode-release-2.9-desura)

set(wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so.0.0.0")
install(FILES ${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so
              ${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so.0
              ${wxWidgets_LIBRARY_DIRS}/libwx_gtk2u_desura-2.9.so.0.0.0
        DESTINATION ${LIB_INSTALL_DIR})
