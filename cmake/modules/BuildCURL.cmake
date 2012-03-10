set(CURL_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/curl)

if(WITH_ARES)
  set(BUILD_WITH_ARES --enable-ares)
else()
  set(BUILD_WITH_ARES --disable-ares)
endif()

ExternalProject_Add(
    curl
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/third_party/curl
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND sh ${CMAKE_BINARY_DIR}/curl/configure
        --without-librtmp --disable-ldap --disable-debug --disable-curldebug
        --without-zlib --disable-rtsp --disable-manual --enable-static=yes 
        --enable-shared=no --disable-pop3 --disable-imap --disable-dict
        --disable-gopher --disable-verbose --disable-smtp --disable-telnet
        --disable-tftp --disable-file --without-libidn --without-gnutls
        --without-nss --without-cyassl --with-ssl --without-axtls
        --without-libssh2 --enable-hidden-symbols --enable-cookies --without-sspi
        --disable-manual --enable-optimize=-O2 ${BUILD_WITH_ARES}
        --prefix=${CURL_INSTALL_DIR}
)

ExternalProject_Add_Step(
    curl
    copyProject
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/src/third_party/curl ${CMAKE_BINARY_DIR}/curl
)

ExternalProject_Add_Step(
    curl
    preconfigure
    COMMAND sh buildconf
    DEPENDEES copyProject
    DEPENDERS configure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/curl
)

set(CURL_BIN_DIRS ${CURL_INSTALL_DIR}/bin)
set(CURL_LIBRARY_DIR ${CURL_INSTALL_DIR}/lib)
set(CURL_INCLUDE_DIRS ${CURL_INSTALL_DIR}/include)
set(CURL_LIBRARIES ${CURL_LIBRARY_DIR}/libcurl.a)
