ExternalProject_Add(
    breakpad
    SVN_REPOSITORY http://google-breakpad.googlecode.com/svn/trunk -r 699
    UPDATE_COMMAND ""
    PATCH_COMMAND patch -p0 -N -i ${CMAKE_SOURCE_DIR}/third_party/patches/breakpad_patch.patch
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared=no --enable-static=yes
        --prefix=${CMAKE_SOURCE_DIR}/third_party/breakpad
)

set(BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR ${CMAKE_SOURCE_DIR}/third_party/breakpad)
set(BREAKPAD_EXCEPTION_HANDLER_LIBRARY ${BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR}/lib)
set(BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR ${BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR}/include)
set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES "${BREAKPAD_EXCEPTION_HANDLER_LIBRARY}/libbreakpad.a;${BREAKPAD_EXCEPTION_HANDLER_LIBRARY}/libbreakpad_client.a")

ExternalProject_Add_Step(breakpad install_headers
    COMMAND cp -r <SOURCE_DIR>/src ${BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR}
    DEPENDEES install
)
