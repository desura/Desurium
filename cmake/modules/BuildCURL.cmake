# TODO: create args for manipulting curl build
# eg: WITH_IDN, IPV6, ...

if(WITH_ARES)
  find_package(cares REQUIRED)
  set(BUILD_WITH_ARES --enable-ares)
else()
  set(BUILD_WITH_ARES --disable-ares)
endif()

if(WIN32 AND NOT MINGW)
  ExternalProject_Add(
    curl
    GIT_REPOSITORY ${CURL_GIT}
    GIT_TAG ${CURL_VERSION}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND buildconf.bat
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
  ExternalProject_Add_Step(
    curl
    custom_build
    DEPENDEES configure
    DEPENDERS build
    COMMAND nmake /f Makefile.vc MODE=static WITH_SSL=yes DEBUG=no GEN_PDB=no USE_SSPI=no USE_IPV6=no ENABLE_IDN=no
    WORKING_DIRECTORY <SOURCE_DIR>/winbuild
  )
  
  ExternalProject_Get_Property(
    curl
    source_dir
  )
  set(CURL_INSTALL_DIR ${source_dir}/builds/libcurl-release-static/)
else()
  find_package(OpenSSL REQUIRED)
  set(CURL_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/curl)
  ExternalProject_Add(
    curl
    URL ${CURL_URL}
    URL_MD5 ${CURL_MD5}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./configure
        --without-librtmp --disable-ldap --disable-curldebug
        --without-zlib --disable-rtsp --disable-manual --enable-static=yes 
        --enable-shared=no --disable-pop3 --disable-imap --disable-dict
        --disable-gopher --disable-verbose --disable-smtp --disable-telnet
        --disable-tftp --disable-file --without-libidn --without-gnutls
        --without-nss --without-cyassl --with-ssl --without-axtls
        --without-libssh2 --enable-hidden-symbols --enable-cookies --without-sspi
        --disable-manual --enable-optimize=-O2 ${BUILD_WITH_ARES} ${CONFIGURE_DEBUG}
        --prefix=${CURL_INSTALL_DIR}
  )
endif()

set(CURL_BIN_DIRS ${CURL_INSTALL_DIR}/bin)
set(CURL_LIBRARY_DIR ${CURL_INSTALL_DIR}/lib)
set(CURL_INCLUDE_DIRS ${CURL_INSTALL_DIR}/include)

if(WIN32 AND NOT MINGW)
  list(APPEND CURL_LIBRARIES "${CURL_LIBRARY_DIR}/libcurl_a.lib")
else()
  list(APPEND CURL_LIBRARIES "${CURL_LIBRARY_DIR}/libcurl.a")
  list(APPEND CURL_LIBRARIES "${OPENSSL_LIBRARIES}")
  if(MINGW)
    list(APPEND CURL_LIBRARIES "ws2_32")
  else()
    list(APPEND CURL_LIBRARIES "rt")
  endif()
endif()

if(WITH_ARES)
  list(APPEND CURL_LIBRARIES ${CARES_LIBRARIES})
endif()
