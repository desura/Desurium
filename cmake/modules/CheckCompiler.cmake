###############################################################################
# some global defines
###############################################################################

add_definitions(-DDEBUG -DBOOST_FILESYSTEM_VERSION=2 -D_LARGE_FILES
    -D_FILE_OFFSET_BITS=64 -fPIC -pipe -g3 -fvisibility=hidden -rdynamic
    -fno-omit-frame-pointer -Wl,-Bsymbolic-functions -lpthread)

###############################################################################
# important C/C++ flags
###############################################################################

# Desura uses the awesome new C++ Standard
add_definitions(-std=c++0x)

# Karol Herbst: I get an error while building without that
add_definitions(-fpermissive)

# wxWidgets config
add_definitions(-D__WXGTK__)
