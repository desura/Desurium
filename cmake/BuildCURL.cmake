ExternalProject_Add(
    curl
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/third_party/curl
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND sh ${CMAKE_SOURCE_DIR}/src/third_party/curl_build/configure
        --without-librtmp --disable-ldap --disable-debug --disable-curldebug
        --without-zlib --disable-rtsp --disable-manual --enable-static=yes 
        --enable-shared=no --disable-pop3 --disable-imap --disable-dict
        --disable-gopher --disable-verbose --disable-smtp --disable-telnet
        --disable-tftp --disable-file --without-libidn --without-gnutls
        --without-nss --without-cyassl --with-ssl --without-axtls
        --without-libssh2 --enable-hidden-symbols --enable-cookies --without-sspi
        --disable-manual --enable-optimize=-O2+ --enable-ares
        --prefix=${CMAKE_SOURCE_DIR}/src/third_party/curl_install
)

ExternalProject_Add_Step(
    curl
    copyProject
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/src/third_party/curl ${CMAKE_SOURCE_DIR}/src/third_party/curl_build
)

ExternalProject_Add_Step(
    curl
    preconfigure
    COMMAND sh buildconf
    DEPENDEES copyProject
    DEPENDERS configure
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/third_party/curl_build
)

set(CURL_INSTALL_DIR ${CMAKE_SOURCE_DIR}/src/third_party/curl_install)
set(CURL_BIN_DIRS ${CURL_INSTALL_DIR}/bin)
set(CURL_LIBRARY_DIR ${CURL_INSTALL_DIR}/lib)
set(CURL_INCLUDE_DIRS ${CURL_INSTALL_DIR}/include)
set(CURL_LIBRARIES ${CURL_LIBRARY_DIR}/libcurl.a)
