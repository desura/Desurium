ExternalProject_Add(
    wxWidget-2-9
    SVN_REPOSITORY http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_0
    UPDATE_COMMAND ""
    PATCH_COMMAND patch -p0 -i ${CMAKE_SOURCE_DIR}/third_party/patches/wxWidgets_patch.patch
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared --enable-unicode
        --enable-monolithic --with-flavour=desura --disable-threads
        --disable-joystick --disable-mediactrl --prefix=${CMAKE_SOURCE_DIR}/third_party/wxWidgets
)

set(wxWidgets_INSTALL_DIR ${CMAKE_SOURCE_DIR}/third_party/wxWidgets)
set(wxWidgets_BIN_DIRS ${wxWidgets_INSTALL_DIR}/bin)
set(wxWidgets_LIBRARY_DIRS ${wxWidgets_INSTALL_DIR}/lib)
set(wxWidgets_INCLUDE_DIRS ${wxWidgets_INSTALL_DIR}/include/wx-2.9-desura ${wxWidgets_LIBRARY_DIRS}/wx/include/gtk2-unicode-release-2.9-desura)

file(GLOB wxWidgets_LIBRARIES "${wxWidgets_LIBRARY_DIRS}/*.so")
