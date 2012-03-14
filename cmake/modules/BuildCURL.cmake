set(CURL_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/curl)

if(WITH_ARES)
  find_package(cares REQUIRED)
  set(BUILD_WITH_ARES --enable-ares)
else()
  set(BUILD_WITH_ARES --disable-ares)
endif()

set(CURL_GIT git://github.com/bagder/curl.git)

ExternalProject_Add(
    curl
    GIT_REPOSITORY ${CURL_GIT}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND sh <SOURCE_DIR>/configure
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
    preconfigure
    COMMAND sh buildconf
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY <SOURCE_DIR>
)

set(CURL_BIN_DIRS ${CURL_INSTALL_DIR}/bin)
set(CURL_LIBRARY_DIR ${CURL_INSTALL_DIR}/lib)
set(CURL_INCLUDE_DIRS ${CURL_INSTALL_DIR}/include)

if(WITH_ARES)
  set(CURL_LIBRARIES ${CARES_LIBRARIES})
endif()
list(APPEND CURL_LIBRARIES "${CURL_LIBRARY_DIR}/libcurl.a")
