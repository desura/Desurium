# - Find v8
# Find the native v8 includes and library
#
#   V8_FOUND       - True if v8 found.
#   V8_INCLUDE_DIR - where to find v8.h, etc.
#   V8_LIBRARIES   - List of libraries when using v8.
#

IF( V8_INCLUDE_DIR )
    # Already in cache, be silent
    SET( v8_FIND_QUIETLY TRUE )
ENDIF( V8_INCLUDE_DIR )

FIND_PATH( V8_INCLUDE_DIR "v8.h" )

FIND_LIBRARY( V8_LIBRARIES
              NAMES "v8ds" )

# handle the QUIETLY and REQUIRED arguments and set TINYXML_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "v8" DEFAULT_MSG V8_INCLUDE_DIR V8_LIBRARIES )

MARK_AS_ADVANCED( V8_INCLUDE_DIR V8_LIBRARIES )
