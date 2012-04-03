set(DEPOT_TOOLS_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/depot_tools)
set(DEPOT_TOOLS_BIN_DIR ${DEPOT_TOOLS_INSTALL_DIR}/src/depot_tools)

ExternalProject_Add(
    depot_tools
    SVN_REPOSITORY http://src.chromium.org/svn/trunk/tools/depot_tools
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    PREFIX ${DEPOT_TOOLS_INSTALL_DIR}
)

set(CHROMIUM_SOURCE_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/chromium)

ExternalProject_Add(
    chromium
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

ExternalProject_Add_Step(
    chromium
    gclient_config
    COMMAND cp ${CMAKE_SCRIPT_PATH}/.gclient .gclient
    DEPENDERS download
    WORKING_DIRECTORY <SOURCE_DIR>
)

ExternalProject_Add_Step(
    chromium
    gclient_download
    COMMAND ${DEPOT_TOOLS_BIN_DIR}/gclient sync --revision src@122508 --jobs 8 --force
    DEPENDEES gclient_config
    WORKING_DIRECTORY <SOURCE_DIR>
)

#ExternalProject_Add(
#    cef
#    SVN_REPOSITORY http://chromiumembedded.googlecode.com/svn/trunk
#    UPDATE_COMMAND ""
#    CONFIGURE_COMMAND ${CMAKE_SCRIPT_PATH}/configCEF.sh ${CEF_BIN_DIR}
#    BUILD_COMMAND ""
#    BUILD_IN_SOURCE 1
#    INSTALL_COMMAND ""
#)

add_dependencies(chromium depot_tools)
#add_dependencies(cef chromium)
