# TODO: create args for manipulting curl build
# eg: WITH_IDN, IPV6, ...

if(DEBUG)
  set(CURL_DEBUG yes)
else()
  set(CURL_DEBUG no)
endif()

if(WIN32 AND NOT MINGW)
  ExternalProject_Add(
    curl
    URL ${CURL_URL}
    URL_MD5 ${CURL_MD5}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
  )
  ExternalProject_Add_Step(
    curl
    custom_build
    DEPENDEES configure
    DEPENDERS build
    COMMAND nmake /f Makefile.vc MODE=dll DEBUG=${CURL_DEBUG} GEN_PDB=no USE_SSPI=yes USE_IPV6=no USE_IDN=no ENABLE_WINSSL=yes MACHINE=x86
    WORKING_DIRECTORY <SOURCE_DIR>/winbuild
  )
  
  ExternalProject_Get_Property(
    curl
    source_dir
  )
  if(DEBUG)
    set(CURL_INSTALL_DIR ${source_dir}/builds/libcurl-vc-x86-debug-dll-spnego-winssl)
  else()
    set(CURL_INSTALL_DIR ${source_dir}/builds/libcurl-vc-x86-release-dll-spnego-winssl)
  endif()
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
        --disable-manual --enable-optimize=-O2 --disable-ares ${CONFIGURE_DEBUG}
        --prefix=${CURL_INSTALL_DIR}
  )
endif()

set(CURL_BIN_DIRS ${CURL_INSTALL_DIR}/bin)
set(CURL_LIBRARY_DIR ${CURL_INSTALL_DIR}/lib)
set(CURL_INCLUDE_DIRS ${CURL_INSTALL_DIR}/include)

if(WIN32 AND NOT MINGW)
  if(DEBUG)
    list(APPEND CURL_LIBRARIES "${CURL_LIBRARY_DIR}/libcurl_debug.lib")
    install(FILES "${CURL_BIN_DIRS}/libcurl_debug.dll" DESTINATION ${LIB_INSTALL_DIR})
  else()
    list(APPEND CURL_LIBRARIES "${CURL_LIBRARY_DIR}/libcurl.lib")
    install(FILES "${CURL_BIN_DIRS}/libcurl.dll" DESTINATION ${LIB_INSTALL_DIR})
  endif()
else()
  list(APPEND CURL_LIBRARIES "${CURL_LIBRARY_DIR}/libcurl.a")
  list(APPEND CURL_LIBRARIES "${OPENSSL_LIBRARIES}")
  if(MINGW)
    list(APPEND CURL_LIBRARIES "ws2_32")
  else()
    list(APPEND CURL_LIBRARIES "rt")
  endif()
endif()

SET_PROPERTY(TARGET curl                PROPERTY FOLDER "ThirdParty")
