set(CEF_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/cef)
set(CEF_BIN_DIR ${CEF_INSTALL_DIR}/src/depot_tools)

ExternalProject_Add(
    depot_tools
    SVN_REPOSITORY http://src.chromium.org/svn/trunk/tools/depot_tools
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    PREFIX ${CEF_INSTALL_DIR}
)

ExternalProject_Add(
    cef
    SVN_REPOSITORY http://chromiumembedded.googlecode.com/svn/trunk
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

add_dependencies(cef depot_tools)
