###############################################################################
# some global defines
###############################################################################

add_definitions(-DBOOST_FILESYSTEM_VERSION=2 -D_LARGE_FILES
    -D_FILE_OFFSET_BITS=64)

###############################################################################
# some compiler flags
###############################################################################

# TODO: if DEBUG is not defined, the binary will be copied
#if(DEBUG)
    add_definitions(-DDEBUG)
#else()
#    add_definitions(-DNDEBUG)
#endif()

add_compiler_flags(-fPIC -pipe -fvisibility=hidden -Wl,-Bsymbolic-functions -lpthread -finline-functions)
add_compiler_flags(DEBUG -rdynamic -fno-omit-frame-pointer -g3)
add_compiler_flags(RELEASE -O2)

add_compiler_flags(CXX -fpermissive)

# Desura uses the awesome new C++ Standard
add_compiler_flags(CXX -std=c++0x)

# wxWidgets config
add_definitions(-D__WXGTK__)
