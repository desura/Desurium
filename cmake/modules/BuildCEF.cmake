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
    COMMAND ${DEPOT_TOOLS_BIN_DIR}/gclient sync --revision src@91424 --jobs 8 --force
    DEPENDEES gclient_config
    WORKING_DIRECTORY <SOURCE_DIR>
)

ExternalProject_Get_Property(
    chromium
    source_dir
)

ExternalProject_Add(
    cef
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "" 
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

ExternalProject_Add_Step(
    cef
    copy_files
    COMMAND cp -r ${CMAKE_THIRD_PARTY_DIR}/cef ./src/
    DEPENDERS download
    WORKING_DIRECTORY ${source_dir}
)

ExternalProject_Add_Step(
    cef
    config_cef
    COMMAND ${CMAKE_SCRIPT_PATH}/configCEF.sh ${DEPOT_TOOLS_BIN_DIR}
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY ${source_dir}/src/cef
)

ExternalProject_Add_Step(
    cef
    build_cef
    COMMAND make cef_desura -j8 BUILDTYPE=Release
    DEPENDERS build
    WORKING_DIRECTORY ${source_dir}/src
)

add_dependencies(chromium depot_tools)
add_dependencies(cef chromium)
