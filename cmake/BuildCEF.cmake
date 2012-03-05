ExternalProject_Add(
    cef
    DOWNLOAD_DIR ""
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    INSTALL_COMMAND ""
)

ExternalProject_Add_Step(
    cef
    configure_cef
    COMMAND sh cef_create_projects.sh
    DEPENDEES configure
    DEPENDERS build
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/third_party/chromium/src/cef
)

add_dependencies(cef chromium)
