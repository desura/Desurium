ExternalProject_Add(
    chromium
    DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/src/third_party/chromium
    DOWNLOAD_COMMAND gclient sync --revision src@91424
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add_Step(
    chromium
    gclient_config
    COMMAND gclient config http://src.chromium.org/svn/trunk/src
    DEPENDERS download
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/third_party/chromium
)

#ExternalProject_Add_Step(
#    chromium
#    moveCEF
#    COMMAND cp -r ${CMAKE_SOURCE_DIR}/src/third_party/cef ${CMAKE_SOURCE_DIR}/src/third_party/chromium/src/
#    DEPENDEES download
#)
