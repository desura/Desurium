###############################################################################
# gnerators
###############################################################################

set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ;ZIP")

###############################################################################
# some basic information
###############################################################################

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desura is a gaming client that allows users to one click download and install games and game modification.")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 8)
set(CPACK_PACKAGE_VERSION_PATCH 0)

###############################################################################
# some internal variables
###############################################################################

set(CPACK_IGNORE_FILES "ceflibs/;distro/;install;CMakeCache.txt;install_manifest.txt;/Testing/;/Makefile$;\\\\.tar.gz$;\\\\.so[.0-9]*$;/build/;/_CPack_Packages/;/CMakeFiles/;/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#")
set(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
set(DESTDIR ${CMAKE_BINARY_DIR}/cpackDestDir)
set(CPACK_PACKAGING_INSTALL_PREFIX ${DESTDIR})
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")

###############################################################################
# deb specific values
###############################################################################

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Karol Herbst")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "flex gperf xdg-utils yasm")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
set(CPACK_DEBIAN_PACKAGE_NAME "desura")

###############################################################################
# rpm specific values
###############################################################################



###############################################################################
# include CPack
###############################################################################

include(CPack)
