set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desura is a gaming client that allows users to one click download and install games and game modification.")
set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ;ZIP")
set(CPACK_IGNORE_FILES "ceflibs/;distro/;install;CMakeCache.txt;install_manifest.txt;/Testing/;/Makefile$;\\\\.tar.gz$;\\\\.so[.0-9]*$;/build/;/_CPack_Packages/;/CMakeFiles/;/CVS/;/\\\\.svn/;/\\\\.git/;\\\\.swp$;\\\\.#;/#")
set(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})

set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 8)
set(CPACK_PACKAGE_VERSION_PATCH 0)

###############################################################################
# deb specific values
###############################################################################

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Karol Herbst")

include(CPack)
