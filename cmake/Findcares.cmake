# - Find c-ares
# Find the native v8 includes and library
#
#   CARES_FOUND       - True if c-ares found.
#   CARES_INCLUDE_DIR - where to find ares.h, etc.
#   CARES_LIBRARIES   - List of libraries when using c-ares.
#

IF( CARES_INCLUDE_DIR )
    # Already in cache, be silent
    SET( cares_FIND_QUIETLY TRUE )
ENDIF( CARES_INCLUDE_DIR )

FIND_PATH( CARES_INCLUDE_DIR "ares.h" )

FIND_LIBRARY( CARES_LIBRARIES
              NAMES "cares" )

# handle the QUIETLY and REQUIRED arguments and set CARES_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "cares" DEFAULT_MSG CARES_INCLUDE_DIR CARES_LIBRARIES )

MARK_AS_ADVANCED( CARES_INCLUDE_DIR CARES_LIBRARIES )
