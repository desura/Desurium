###############################################################################
# gnerators
###############################################################################

set(PACKAGE_TYPE "DEB"
    CACHE STRING "Type of packages for make package")
set(CPACK_GENERATOR "${PACKAGE_TYPE}")
set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ;ZIP")

###############################################################################
# some basic information
###############################################################################

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desura is a gaming client that allows users to one click download and install games and game modification.")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 8)
set(CPACK_PACKAGE_VERSION_PATCH 0)
# used only for builds, not by cpack!
set(CPACK_PACKAGE_VERSION_ADDITION _rc8-devel)

###############################################################################
# some internal variables
###############################################################################

set(CPACK_IGNORE_FILES "ceflibs/;distro/;install;CMakeCache.txt;install_manifest.txt;/Testing/;/Makefile$;\\\\.tar.gz$;\\\\.so[.0-9]*$;/build/;/_CPack_Packages/;/CMakeFiles/;/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#")
set(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
set(DESTDIR ${CMAKE_BINARY_DIR}/cpackDestDir)
set(CPACK_PACKAGING_INSTALL_PREFIX "${CPACK_INSTALL_PREFIX}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")

###############################################################################
# deb specific values
###############################################################################

set(CPACK_DEBIAN_PACKAGE_NAME "desurium")
set(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
if(64BIT)
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else(64BIT)
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
endif(64BIT)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tomasz Makarewicz <makson96@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-filesystem-dev, libboost-thread-dev, libc-ares2, libevent-dev, libjpeg62, libtinyxml-dev")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "extra")

###############################################################################
# rpm specific values
###############################################################################



###############################################################################
# include CPack
###############################################################################

include(CPack)
